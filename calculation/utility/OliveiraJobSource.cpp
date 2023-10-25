//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "OliveiraJobSource.h"

//constructor
OliveiraJobSource::OliveiraJobSource(AnalysisExecution & rExecution, boost::posix_time::ptime CurrentTime, PrintQueue & rPrintDirection)
 : guiNextJobParam(1)
 , guiUnregisteredJobLowerBound(1)
 , gfnRegisterResult(&OliveiraJobSource::RegisterResult_AutoAbort)//initialize to the most feature-laden
 , gConstructionTime(CurrentTime)
 , grPrintDirection(rPrintDirection)
 , grExecution(rExecution)
 , guiJobCount(rExecution._parameters.getNumRequestedOliveiraSets())
 , guiNextProcessingJobId(1)
 , guiJobsReported(0)
 , StartTime(::GetCurrentTime_HighResolution())
 , _frequent_estimations(false)
{
    gfnRegisterResult = &OliveiraJobSource::RegisterResult_NoAutoAbort;

    // define location relevance tracker and bitsets to track presence in a particular data set
	grExecution._relevance_tracker.reset(new LocationRelevance(grExecution.getDataHub()));

    // determine the number of significant clusters present in cluster collections
    MostLikelyClustersContainer::ClusterList_t significantClusters;
    // _reportClusters contains either most likely cluster or hierarchical clusters, so calculate the number of significant clusters at oliveira cutoff
	grExecution._reportClusters.getSignificantClusters(grExecution.getDataHub(), grExecution._sim_vars, grExecution._parameters.getOliveiraPvalueCutoff(), significantClusters);
    _numSignificantMLC = std::min(static_cast<size_t>(1), significantClusters.size()); // significant clusters for neither hierarchical nor gini
    _numSignificantHierarchical = significantClusters.size(); // significant clusters for hierarchical

    /* We're disabling the gini portion for the time being: https://www.squishlist.com/ims/satscan/66323/
    _optimalSignificantCluster.resize(grRunner.gParameters.getExecuteSpatialWindowStops().size(), 0); // significant clusters for optimal gini
    */

    // initialize variables based on which parameter settings are requested
    if (!(grExecution._parameters.getReportHierarchicalClusters() || grExecution._parameters.getReportGiniOptimizedClusters()) || grExecution._parameters.getReportHierarchicalClusters())
        _presence_hierarchical.resize(grExecution.getDataHub().GetNumIdentifiers() + grExecution.getDataHub().GetNumMetaIdentifiers()); // most likely cluster only or hierarachical clusters

    /* We're disabling the gini portion for the time being: https://www.squishlist.com/ims/satscan/66323/
    if (grExecution._parameters.getReportGiniOptimizedClusters()) { // gini clusters
        _presence_gini_optimal.resize(grExecution.getDataHub().GetNumIdentifiers() + grExecution.getDataHub().GetNumMetaIdentifiers());
        _presence_gini_maxima.resize(grExecution.getDataHub().GetNumIdentifiers() + grExecution.getDataHub().GetNumMetaIdentifiers());
        // gini clusters are derived from gTopClustersContainers -- calculate the optimal gini collection
        const MostLikelyClustersContainer * optimal = grExecution.getOptimalGiniContainerByPValue(grExecution._top_clusters_containers, grExecution._parameters.getGiniIndexPValueCutoff());
        if (optimal) {
            MostLikelyClustersContainer::ClusterList_t clusterList;
            // calculate the number of significant clusters in optimal gini collection at oliveira cutoff
            optimal->getSignificantClusters(grExecution.getDataHub(), grExecution._sim_vars, grExecution._parameters.getOliveiraPvalueCutoff(), clusterList);
            // when calculating the gini coefficient, use the same limit for all maxima
            std::fill(_optimalSignificantCluster.begin(), _optimalSignificantCluster.end(), clusterList.size());
        }
        for (MLC_Collections_t::const_iterator itrMLC=grExecution._top_clusters_containers.begin(); itrMLC != grExecution._top_clusters_containers.end(); ++itrMLC) {
            MostLikelyClustersContainer::ClusterList_t clusterList;
            // calculate the number of significant clusters for this maxima collection at oliveira cutoff
            itrMLC->getSignificantClusters(grExecution.getDataHub(), grExecution._sim_vars, grExecution._parameters.getOliveiraPvalueCutoff(), clusterList);
            // when calculating the gini coefficient at this maxima, use same # of significant clusters at real data
            _maximaSignificantCluster.push_back(clusterList.size());
        }
    }
    */
}


void OliveiraJobSource::acquire(job_id_type & dst_job_id, param_type & dst_param)
{
  if (is_exhausted())
    throw std::runtime_error("can't acquire a job from an exhausted source.");

  gbsUnregisteredJobs.push_back(true);

  //all exception-unsafe ops have been executed, so do updates:
  dst_param = dst_job_id = guiNextJobParam;
  ++guiNextJobParam;
}

void OliveiraJobSource::Assert_NoExceptionsCaught() const
{
  static const char * szExceptionIntroFormatString = "An exception was thrown from simulation #%d.";
  static const char * szExceptionMessageTitle = "\nException message: ";
  static const char * szExceptionCallPathTitle = "\nException call path:\n";

  if (GetExceptionCount() > 0) {
    //scan collection of exceptions for ZdMemory exception type, this type trumps all -- take first
    std::deque<exception_type>::const_iterator itr = gvExceptions.begin();
    for (; itr != gvExceptions.end(); ++itr) {
       if (itr->second.second.eException_type == job_result::memory) {
         std::string sTemp;
         printString(sTemp, szExceptionIntroFormatString, itr->first);
         sTemp += szExceptionMessageTitle;
         sTemp += itr->second.second.Exception.what();
         sTemp += szExceptionCallPathTitle;
         sTemp += itr->second.second.Exception.trace();
         memory_exception MemoryException(sTemp.c_str());
         throw MemoryException;
       }
    }

    CarrierException<exception_sequence_type> lclException(gvExceptions, "", "OliveiraJobSource");
    exception_type const & rFirstException(lclException->front());
    std::string sTemp;
    printString(sTemp, szExceptionIntroFormatString, rFirstException.first);
    lclException.addWhat(sTemp.c_str());
    lclException.addWhat(szExceptionMessageTitle);
    lclException.addWhat(rFirstException.second.second.Exception.what());
    lclException.addWhat(szExceptionCallPathTitle);
    lclException.addWhat(rFirstException.second.second.Exception.trace());

    throw lclException;
  }
}

bool OliveiraJobSource::CancelRequested() const
{
  return grPrintDirection.GetIsCanceled();
}

//How many jobs have registered a successful result?
//This is all jobs that:
//1. completed without an exception and
//2. were not discarded in the event of an auto-abort condition.
unsigned int OliveiraJobSource::GetSuccessfullyCompletedJobCount() const
{
  unsigned int uiResult = guiUnregisteredJobLowerBound-1;
  uiResult += (gbsUnregisteredJobs.size()-gbsUnregisteredJobs.count()) - gvExceptions.size();
  return uiResult;
}

//How many jobs are there that have been acquired but whose results have not
//been registered?
unsigned int OliveiraJobSource::GetUnregisteredJobCount() const
{
  return gbsUnregisteredJobs.count();
}

std::deque<unsigned int> OliveiraJobSource::GetUnregisteredJobs() const
{
  std::deque<unsigned int> seqResult;
  for (unsigned int ui=guiUnregisteredJobLowerBound, uiCurr=0, uiEnd=gbsUnregisteredJobs.size(); uiCurr < uiEnd; ++ui,++uiCurr)
    if (gbsUnregisteredJobs.test(uiCurr))
      seqResult.push_back(ui);
  return seqResult;
}

//From how many jobs were exceptions caught?
unsigned int OliveiraJobSource::GetExceptionCount() const
{
  return gvExceptions.size();
}

OliveiraJobSource::exception_sequence_type OliveiraJobSource::GetExceptions() const
{
  return gvExceptions;
}

bool OliveiraJobSource::is_exhausted() const
{
  return
    CancelConditionExists()
   || ExceptionConditionExists()
   || AutoAbortConditionExists()
   || (guiNextJobParam > guiJobCount);
}

//Remove the first N bits from operand.
void OliveiraJobSource::DynamicBitsetPopFrontN(boost::dynamic_bitset<> & operand, unsigned long N)
{
  operand >>= N;//shift all bits down
  operand.resize(N > operand.size() ? 0 : operand.size()-N);//pop the back bits off
}

void OliveiraJobSource::register_result(job_id_type const & job_id, param_type const & param, result_type const & result)
{
  try
  {
    //the job_id must be one of the unfinished jobs:
    assert(job_id >= guiUnregisteredJobLowerBound);
    assert(job_id < guiNextJobParam);

    (this->*gfnRegisterResult)(job_id, param, result);

    //after everything else is done, update gbsUnfinishedJobs:
    if (job_id != guiUnregisteredJobLowerBound)
      gbsUnregisteredJobs.reset(job_id - guiUnregisteredJobLowerBound);
    else
    {//remove leading bit plus the block of zero bits that follows:
      unsigned long ulN=1;
      for (; (ulN < gbsUnregisteredJobs.size()) && !gbsUnregisteredJobs.test(ulN); ++ulN);//count the number of zero bits that follow (until the first 1 bit)
      DynamicBitsetPopFrontN(gbsUnregisteredJobs, ulN);
      guiUnregisteredJobLowerBound += ulN;
    }
  }
  catch (prg_exception & e)
  {
    e.addTrace("register_result()", "OliveiraJobSource");
    throw;
  }
}

//register a result when analysis has been canceled.  This will be called for
//all subsequent job registrations (that were already running when cancel
//got triggered), which are ignored.
void OliveiraJobSource::RegisterResult_CancelConditionExists(job_id_type const & rJobId, param_type const & rParam, result_type const & rResult)
{
//  try
//  {
//  }
//  catch (prg_exception & e)
//  {
//    e.addTrace("RegisterResult_CancelConditionExists()", "OliveiraJobSource");
//    throw;
//  }
}

//register a result when AutoAbort (early termination) isn't active
void OliveiraJobSource::RegisterResult_AutoAbort(job_id_type const & rJobID, param_type const & rParam, result_type const & rResult)
{
  try
  {
    //check exception condition first.  Want to report an exception even if
    //cancel is requested.
    if (!rResult.bUnExceptional)
    {
      //populate stored exceptions:
      gvExceptions.push_back(std::make_pair(rJobID, std::make_pair(rParam,rResult)));
      gfnRegisterResult = &OliveiraJobSource::RegisterResult_ExceptionConditionExists;
      return;
    }
    else if (CancelRequested())
    {
      gfnRegisterResult = &OliveiraJobSource::RegisterResult_CancelConditionExists;
      ReleaseAutoAbortCheckResources();
      return;
    }

    // Add this job to the cache of results.
    gmapOverflowResults.insert(std::make_pair(rJobID,std::make_pair(rParam,rResult)));
    // process cached completed jobs sequencely
    while (!gmapOverflowResults.empty() && guiNextProcessingJobId == gmapOverflowResults.begin()->first) {
         //gAutoAbortResultsRegistered.set(gmapOverflowResults.begin()->first - guPreviousAutoAbortCheckPoint - 1);
         RegisterResult_NoAutoAbort(gmapOverflowResults.begin()->first, gmapOverflowResults.begin()->second.first, gmapOverflowResults.begin()->second.second);
         gmapOverflowResults.erase(gmapOverflowResults.begin());
         ++guiNextProcessingJobId;
    }
  }
  catch (prg_exception & e)
  {
    e.addTrace("RegisterResult_AutoAbort()", "OliveiraJobSource");
    throw;
  }
}

//register a result when AutoAbort has been triggered.  This will be called for
//all subsequent job registrations (the ones that were already running when auto-abort
//got triggered).  Their results are ignored.
void OliveiraJobSource::RegisterResult_AutoAbortConditionExists(job_id_type const & rJobId, param_type const & rParam, result_type const & rResult)
{
//  try
//  {
//  }
//  catch (prg_exception & e)
//  {
//    e.AddCallpath("RegisterResult_AutoAbortConditionExists()", "OliveiraJobSource");
//    throw;
//  }
}

//register a result when a previously registered result indicated an exception.
void OliveiraJobSource::RegisterResult_ExceptionConditionExists(job_id_type const & rJobID, param_type const & rParam, result_type const & rResult)
{
  try
  {
    if (!rResult.bUnExceptional)
      gvExceptions.push_back(std::make_pair(rJobID, std::make_pair(rParam,rResult)));
  }
  catch (prg_exception & e)
  {
    e.addTrace("RegisterResult_ExceptionConditionExists()", "OliveiraJobSource");
    throw;
  }
}

//register a result when no extended conditions (AutoAbort[early termination],
//thrown exceptions, cancelation) are active.
void OliveiraJobSource::RegisterResult_NoAutoAbort(job_id_type const & rJobID, param_type const & rParam, result_type const & rResult) {
    try {
        //check exception condition first.  Want to report an exception even if
        //cancel is requested.
        if (!rResult.bUnExceptional) {
            //populate stored exceptions:
            gvExceptions.push_back(std::make_pair(rJobID, std::make_pair(rParam,rResult)));
            gfnRegisterResult = &OliveiraJobSource::RegisterResult_ExceptionConditionExists;
            return;
        } else if (CancelRequested()) {
            gfnRegisterResult = &OliveiraJobSource::RegisterResult_CancelConditionExists;
            return;
        }

        //update ratios, significance, etc.
        WriteResultToStructures(rResult.dSuccessfulResult);
        ++guiJobsReported;

        //if appropriate, estimate time required to complete all jobs and report it.
        unsigned int uiJobsProcessedCount = (gbsUnregisteredJobs.size()-gbsUnregisteredJobs.count()) + guiUnregisteredJobLowerBound; //this one hasn't been reset in gbsUnregisteredJobs yet.
        if (uiJobsProcessedCount == 10 || (_frequent_estimations && (uiJobsProcessedCount % SIMULATION_EST_MODULAS == 0))) {
            _frequent_estimations = ::ReportTimeEstimate(gConstructionTime, guiJobCount, rParam, grPrintDirection, false, uiJobsProcessedCount != 10) > FREQUENT_ESTIMATES_SECONDS;
            SaTScan::Timestamp tsReleaseTime; tsReleaseTime.Now(); tsReleaseTime.AddSeconds(3);//queue lines until 3 seconds from now
            grPrintDirection.SetThresholdPolicy(TimedReleaseThresholdPolicy(tsReleaseTime));
        }
    } catch (prg_exception & e) {
        e.addTrace("RegisterResult_NoAutoAbort()", "OliveiraJobSource");
        throw;
    }
}

//When we're through checking for auto-abort, we want to release any resources
//used.  (This is mostly for cancel and exception conditions that occur while
//auto-abort checking is active.)
void OliveiraJobSource::ReleaseAutoAbortCheckResources()
{
  gmapOverflowResults.clear();
}

void OliveiraJobSource::WriteResultToStructures(successful_result_type const & rResult) {
    try {
        LocationRelevance& relevance(*grExecution._relevance_tracker);
        if (!(grExecution._parameters.getReportHierarchicalClusters() || grExecution._parameters.getReportGiniOptimizedClusters())) // most likely cluster
            relevance.update(grExecution.getDataHub(), *rResult.first, _numSignificantMLC, _presence_hierarchical, relevance._most_likely_only);
        if (grExecution._parameters.getReportHierarchicalClusters()) // hierarachical clusters
            relevance.update(grExecution.getDataHub(), *rResult.first, _numSignificantHierarchical, _presence_hierarchical, relevance._hierarchical);

        /* We're disabling the gini portion for the time being: https://www.squishlist.com/ims/satscan/66323/
        if (grRunner.gParameters.getReportGiniOptimizedClusters()) { // gini clusters
            // instead of getting optimal gini collection by p-value (since these clusters don't have a p-value), limit by # significant in optimal collection for real data
            AnalysisExecution::OptimalGiniByLimit_t optimalOliveira = grExecution.getOptimalGiniContainerByLimit(*rResult.second, _optimalSignificantCluster);
            if (optimalOliveira.first) relevance.update(grExecution.getDataHub(), *(optimalOliveira.first), optimalOliveira.second, _presence_gini_optimal, relevance._gini_optimal);
            // instead of getting optimal gini collection by p-value (since these clusters don't have a p-value), limit by # significant at corresponding maxima of real data
            optimalOliveira = grExecution.getOptimalGiniContainerByLimit(*rResult.second, _maximaSignificantCluster);
            if (optimalOliveira.first) relevance.update(grExecution.getDataHub(), *(optimalOliveira.first), optimalOliveira.second, _presence_gini_maxima, relevance._gini_maxima);
        }
        if (grExecution._parameters.getReportHierarchicalClusters() && grExecution._parameters.getReportGiniOptimizedClusters()) { // both hierarachical and gini clusters
            boost::dynamic_bitset<> location_presence(grExecution.getDataHub().GetNumIdentifiers() + grExecution.getDataHub().GetNumMetaIdentifiers());
            location_presence = _presence_hierarchical | _presence_gini_optimal;
            relevance.updateRelevance(location_presence, relevance._hierarchical_gini_optimal);
            location_presence.reset();
            location_presence = _presence_hierarchical | _presence_gini_maxima;
            relevance.updateRelevance(location_presence, relevance._hierarchical_gini_maxima);
        }
        */
    } catch (prg_exception & e) {
        e.addTrace("WriteResultToStructures()", "OliveiraJobSource");
        throw;
    }
}
