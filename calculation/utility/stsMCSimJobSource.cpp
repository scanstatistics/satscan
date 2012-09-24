//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "stsMCSimJobSource.h"

//constructor
stsMCSimJobSource::stsMCSimJobSource(
  CParameters const & rParameters
 ,boost::posix_time::ptime CurrentTime
 ,PrintQueue & rPrintDirection
 ,AnalysisRunner & rRunner
 ,unsigned int num_replica
)
 : guiNextJobParam(1)
 , guiUnregisteredJobLowerBound(1)
 , gfnRegisterResult(&stsMCSimJobSource::RegisterResult_AutoAbort)//initialize to the most feature-laden
 , gConstructionTime(CurrentTime)
 , grPrintDirection(rPrintDirection)
 , grRunner(rRunner)
 , guiJobCount(num_replica)
 , guiNextProcessingJobId(1)
 , guiJobsReported(0)
{
  if (rParameters.GetTerminateSimulationsEarly()) {
    gfnRegisterResult = &stsMCSimJobSource::RegisterResult_AutoAbort;
  }
  else {
    gfnRegisterResult = &stsMCSimJobSource::RegisterResult_NoAutoAbort;
  }

  if (rParameters.GetOutputSimLoglikeliRatiosFiles())
    gRatioWriter.reset(new LoglikelihoodRatioWriter(rParameters, grRunner.giAnalysisCount > 1));
  if (rParameters.GetLogLikelihoodRatioIsTestStatistic())
    gszReplicationFormatString = "SaTScan test statistic for #%u of %u replications: %7.2lf\n";
  else
    gszReplicationFormatString = "SaTScan log likelihood ratio for #%u of %u replications: %7.2lf\n";
}


void stsMCSimJobSource::acquire(job_id_type & dst_job_id, param_type & dst_param)
{
  if (is_exhausted())
    throw std::runtime_error("can't acquire a job from an exhausted source.");

  gbsUnregisteredJobs.push_back(true);

  //all exception-unsafe ops have been executed, so do updates:
  dst_param = dst_job_id = guiNextJobParam;
  ++guiNextJobParam;
}

void stsMCSimJobSource::Assert_NoExceptionsCaught() const
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

    CarrierException<exception_sequence_type> lclException(gvExceptions, "", "stsMCSimJobSource");
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

bool stsMCSimJobSource::CancelRequested() const
{
  return grPrintDirection.GetIsCanceled();
}

//How many jobs have registered a successful result?
//This is all jobs that:
//1. completed without an exception and
//2. were not discarded in the event of an auto-abort condition.
unsigned int stsMCSimJobSource::GetSuccessfullyCompletedJobCount() const
{
  unsigned int uiResult = guiUnregisteredJobLowerBound-1;
  if (AutoAbortConditionExists())
     uiResult = grRunner.gSimVars.get_sim_count();
  else
    uiResult += (gbsUnregisteredJobs.size()-gbsUnregisteredJobs.count()) - gvExceptions.size();
  return uiResult;
}

//How many jobs are there that have been acquired but whose results have not
//been registered?
unsigned int stsMCSimJobSource::GetUnregisteredJobCount() const
{
  return gbsUnregisteredJobs.count();
}

std::deque<unsigned int> stsMCSimJobSource::GetUnregisteredJobs() const
{
  std::deque<unsigned int> seqResult;
  for (unsigned int ui=guiUnregisteredJobLowerBound, uiCurr=0, uiEnd=gbsUnregisteredJobs.size(); uiCurr < uiEnd; ++ui,++uiCurr)
    if (gbsUnregisteredJobs.test(uiCurr))
      seqResult.push_back(ui);
  return seqResult;
}

//From how many jobs were exceptions caught?
unsigned int stsMCSimJobSource::GetExceptionCount() const
{
  return gvExceptions.size();
}

stsMCSimJobSource::exception_sequence_type stsMCSimJobSource::GetExceptions() const
{
  return gvExceptions;
}

bool stsMCSimJobSource::is_exhausted() const
{
  return
    CancelConditionExists()
   || ExceptionConditionExists()
   || AutoAbortConditionExists()
   || (guiNextJobParam > guiJobCount);
}

//Remove the first N bits from operand.
void stsMCSimJobSource::DynamicBitsetPopFrontN(boost::dynamic_bitset<> & operand, unsigned long N)
{
  operand >>= N;//shift all bits down
  operand.resize(N > operand.size() ? 0 : operand.size()-N);//pop the back bits off
}

void stsMCSimJobSource::register_result(job_id_type const & job_id, param_type const & param, result_type const & result)
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
    e.addTrace("register_result()", "stsMCSimJobSource");
    throw;
  }
}

//register a result when analysis has been canceled.  This will be called for
//all subsequent job registrations (that were already running when cancel
//got triggered), which are ignored.
void stsMCSimJobSource::RegisterResult_CancelConditionExists(job_id_type const & rJobId, param_type const & rParam, result_type const & rResult)
{
//  try
//  {
//  }
//  catch (prg_exception & e)
//  {
//    e.addTrace("RegisterResult_CancelConditionExists()", "stsMCSimJobSource");
//    throw;
//  }
}

//register a result when AutoAbort (early termination) isn't active
void stsMCSimJobSource::RegisterResult_AutoAbort(job_id_type const & rJobID, param_type const & rParam, result_type const & rResult)
{
  try
  {
    //check exception condition first.  Want to report an exception even if
    //cancel is requested.
    if (!rResult.bUnExceptional)
    {
      //populate stored exceptions:
      gvExceptions.push_back(std::make_pair(rJobID, std::make_pair(rParam,rResult)));
      gfnRegisterResult = &stsMCSimJobSource::RegisterResult_ExceptionConditionExists;
      return;
    }
    else if (CancelRequested())
    {
      gfnRegisterResult = &stsMCSimJobSource::RegisterResult_CancelConditionExists;
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
         if (grRunner.gSimVars.get_llr_counters().front().second >= grRunner.gParameters.GetExecuteEarlyTermThreshold()) {
            //auto-abort is triggered
            gfnRegisterResult = &stsMCSimJobSource::RegisterResult_AutoAbortConditionExists;
            ReleaseAutoAbortCheckResources();
            return;
         }
    }
  }
  catch (prg_exception & e)
  {
    e.addTrace("RegisterResult_AutoAbort()", "stsMCSimJobSource");
    throw;
  }
}

//register a result when AutoAbort has been triggered.  This will be called for
//all subsequent job registrations (the ones that were already running when auto-abort
//got triggered).  Their results are ignored.
void stsMCSimJobSource::RegisterResult_AutoAbortConditionExists(job_id_type const & rJobId, param_type const & rParam, result_type const & rResult)
{
//  try
//  {
//  }
//  catch (prg_exception & e)
//  {
//    e.AddCallpath("RegisterResult_AutoAbortConditionExists()", "stsMCSimJobSource");
//    throw;
//  }
}

//register a result when a previously registered result indicated an exception.
void stsMCSimJobSource::RegisterResult_ExceptionConditionExists(job_id_type const & rJobID, param_type const & rParam, result_type const & rResult)
{
  try
  {
    if (!rResult.bUnExceptional)
      gvExceptions.push_back(std::make_pair(rJobID, std::make_pair(rParam,rResult)));
  }
  catch (prg_exception & e)
  {
    e.addTrace("RegisterResult_ExceptionConditionExists()", "stsMCSimJobSource");
    throw;
  }
}

//register a result when no extended conditions (AutoAbort[early termination],
//thrown exceptions, cancelation) are active.
void stsMCSimJobSource::RegisterResult_NoAutoAbort(job_id_type const & rJobID, param_type const & rParam, result_type const & rResult)
{
  try
  {
    //check exception condition first.  Want to report an exception even if
    //cancel is requested.
    if (!rResult.bUnExceptional)
    {
      //populate stored exceptions:
      gvExceptions.push_back(std::make_pair(rJobID, std::make_pair(rParam,rResult)));
      gfnRegisterResult = &stsMCSimJobSource::RegisterResult_ExceptionConditionExists;
      return;
    }
    else if (CancelRequested())
    {
      gfnRegisterResult = &stsMCSimJobSource::RegisterResult_CancelConditionExists;
      return;
    }

    //update ratios, significance, etc.
    WriteResultToStructures(rResult.dSuccessfulResult);
    ++guiJobsReported;

    //if appropriate, estimate time required to complete all jobs and report it.
    unsigned int uiJobsProcessedCount = (gbsUnregisteredJobs.size()-gbsUnregisteredJobs.count()) + guiUnregisteredJobLowerBound; //this one hasn't been reset in gbsUnregisteredJobs yet.
    grPrintDirection.Printf(gszReplicationFormatString, BasePrint::P_STDOUT, guiJobsReported, guiJobCount, rResult.dSuccessfulResult);
    if (uiJobsProcessedCount==10) {
      ::ReportTimeEstimate(gConstructionTime, guiJobCount, rParam, &grPrintDirection);
      SaTScan::Timestamp tsReleaseTime; tsReleaseTime.Now(); tsReleaseTime.AddSeconds(3);//queue lines until 3 seconds from now
      grPrintDirection.SetThresholdPolicy(TimedReleaseThresholdPolicy(tsReleaseTime));
    }
  }
  catch (prg_exception & e)
  {
    e.addTrace("RegisterResult_NoAutoAbort()", "stsMCSimJobSource");
    throw;
  }
}

//When we're through checking for auto-abort, we want to release any resources
//used.  (This is mostly for cancel and exception conditions that occur while
//auto-abort checking is active.)
void stsMCSimJobSource::ReleaseAutoAbortCheckResources()
{
  gmapOverflowResults.clear();
}

void stsMCSimJobSource::WriteResultToStructures(successful_result_type const & rResult)
{
  try
  {
    //update most likely clusters given latest simulated loglikelihood ratio
    grRunner._clusterRanker.update(rResult);
    //update significance indicator
    grRunner.UpdateSignificantRatiosList(rResult);
    if (gRatioWriter.get()) gRatioWriter->Write(rResult);
    grRunner.gSimVars.add_llr(rResult);
    grRunner.gSimVars.increment_sim_count();
  }
  catch (prg_exception & e)
  {
    e.addTrace("WriteResultToStructures()", "stsMCSimJobSource");
    throw;
  }
}

