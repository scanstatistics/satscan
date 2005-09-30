//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "stsMCSimJobSource.h"

const unsigned stsMCSimJobSource::guaAutoAbortCheckPoints[] = { 99, 199, 499, 999 };
const float stsMCSimJobSource::guaAutoAbortCheckCutoffValues[] = { 0.5, 0.4, 0.2, 0.1 };
//BOOST_STATIC_ASSERT(((sizeof(stsMCSimJobSource::guaAutoAbortCheckPoints)/sizeof(unsigned))==stsMCSimJobSource::guMaxAutoAbortCheckCount));
//BOOST_STATIC_ASSERT(((sizeof(stsMCSimJobSource::guaAutoAbortCheckCutoffValues)/sizeof(float))==stsMCSimJobSource::guMaxAutoAbortCheckCount));

//constructor
stsMCSimJobSource::stsMCSimJobSource(
  CParameters const & rParameters
 ,boost::posix_time::ptime CurrentTime
 ,MostLikelyClustersContainer & rMLCs
 ,PrintQueue & rPrintDirection
 ,const char * szReplicationFormatString
 ,AnalysisRunner & rRunner
)
 : guiNextJobParam(1)
 , guiUnregisteredJobLowerBound(1)
 , gfnRegisterResult(&stsMCSimJobSource::RegisterResult_AutoAbort)//initialize to the most feature-laden
 , guAutoAbortCheckIdx(0)
 , guPreviousAutoAbortCheckPoint(0)
 , gConstructionTime(CurrentTime)
 , grMLCs(rMLCs)
 , grPrintDirection(rPrintDirection)
 , gszReplicationFormatString(szReplicationFormatString)
 , grRunner(rRunner)
{
  guiJobCount = rParameters.GetNumReplicationsRequested();
  guAutoAbortCheckCount = 0;
  if (rParameters.GetTerminateSimulationsEarly()) {
    gfnRegisterResult = &stsMCSimJobSource::RegisterResult_AutoAbort;
    gAutoAbortResultsRegistered.resize(guaAutoAbortCheckPoints[guAutoAbortCheckIdx]);//contains bits for only the first guaAutoAbortCheckPoints[guAutoAbortCheckIndex] jobs.
    while ((guAutoAbortCheckCount < guMaxAutoAbortCheckCount) && (guaAutoAbortCheckPoints[guAutoAbortCheckCount] < guiJobCount)) {
      ++guAutoAbortCheckCount;
    }
  }
  else {
    gfnRegisterResult = &stsMCSimJobSource::RegisterResult_NoAutoAbort;
  }

  if (rParameters.GetOutputSimLoglikeliRatiosFiles())
    gRatioWriter.reset(new LoglikelihoodRatioWriter(rParameters));
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

//Have all the results needed for the current auto-abort check been collected?
bool stsMCSimJobSource::AllResultsCollectedForAutoAbortCheck() const
{
  return (gAutoAbortResultsRegistered.count() == gAutoAbortResultsRegistered.size());
}

void stsMCSimJobSource::Assert_NoExceptionsCaught() const
{
  typedef std::pair<job_id_type,std::pair<param_type,result_type> > exception_type;
  typedef std::deque<exception_type> exception_sequence_type;
  static const char * szExceptionIntroFormatString = "An exception was thrown from simulation #%d.";
  static const char * szExceptionTypeTitle = "\nException type: ";
  static const char * szExceptionMessageTitle = "\nException message: ";
  static const char * szExceptionCallPathTitle = "\nException call path:\n";

  if (GetExceptionCount() > 0) {
    //scan collection of exceptions for ZdMemory exception type, this type trumps all -- take first
    std::deque<exception_type>::const_iterator itr = gvExceptions.begin();
    for (; itr != gvExceptions.end(); ++itr) {
       if (itr->second.second.eException_type == job_result::zdmemory) {
         ZdString sTemp;
         sTemp.printf(szExceptionIntroFormatString, itr->first);

         ZdMemoryException MemoryException(sTemp.GetCString());
         MemoryException.AddMessage(sTemp.GetCString(), false);
         MemoryException.AddMessage(szExceptionMessageTitle, false);
         MemoryException.AddMessage(itr->second.second.Exception.GetErrorMessage(), false);
         MemoryException.AddMessage(szExceptionCallPathTitle, false);
         MemoryException.AddMessage(itr->second.second.Exception.GetCallpath(), false);
         throw MemoryException;
       }
    }

    ZdCarrierException<exception_sequence_type> lclException(gvExceptions, "", "stsMCSimJobSource");
    exception_type const & rFirstException(lclException->front());
    ZdString sTemp;
    sTemp.printf(szExceptionIntroFormatString, rFirstException.first);
    lclException.AddMessage(sTemp.GetCString(), false);
    lclException.AddMessage(szExceptionMessageTitle, false);
    lclException.AddMessage(rFirstException.second.second.Exception.GetErrorMessage(), false);
    lclException.AddMessage(szExceptionCallPathTitle, false);
    lclException.AddMessage(rFirstException.second.second.Exception.GetCallpath(), false);

    throw lclException;
  }
}

bool stsMCSimJobSource::CancelRequested() const
{
  return grPrintDirection.GetIsCanceled();
}

//How many jobs have registered a successful (no exceptions) result?
unsigned int stsMCSimJobSource::GetSuccessfullyCompletedJobCount() const
{
  unsigned int uiResult = guiUnregisteredJobLowerBound-1;
  if (!AutoAbortConditionExists())
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

std::deque<std::pair<stsMCSimJobSource::job_id_type,std::pair<stsMCSimJobSource::param_type,stsMCSimJobSource::result_type> > >
stsMCSimJobSource::GetExceptions() const
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
  catch (ZdException & e)
  {
    e.AddCallpath("register_result()", "stsMCSimJobSource");
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
//  catch (ZdException & e)
//  {
//    e.AddCallpath("RegisterResult_CancelConditionExists()", "stsMCSimJobSource");
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

    if (rJobID <= guaAutoAbortCheckPoints[guAutoAbortCheckIdx])
    {
      gAutoAbortResultsRegistered.set(rJobID-guPreviousAutoAbortCheckPoint-1);
      RegisterResult_NoAutoAbort(rJobID, rParam, rResult);
      //process overflow results until there aren't enough results to make the next check point:
      while ((guAutoAbortCheckIdx < guAutoAbortCheckCount) && (!AutoAbortConditionExists()) && AllResultsCollectedForAutoAbortCheck())
      {
        if (grMLCs.GetTopRankedCluster().GetPValue(guaAutoAbortCheckPoints[guAutoAbortCheckIdx]) > guaAutoAbortCheckCutoffValues[guAutoAbortCheckIdx])
        {//auto-abort is triggered:
          gfnRegisterResult = &stsMCSimJobSource::RegisterResult_AutoAbortConditionExists;
          ReleaseAutoAbortCheckResources();
        }
        else
        {
          guPreviousAutoAbortCheckPoint = guaAutoAbortCheckPoints[guAutoAbortCheckIdx];
          ++guAutoAbortCheckIdx;
          if (guAutoAbortCheckIdx == guAutoAbortCheckCount)
          {//this is the last check, and it passed, so we're not in auto-abort mode anymore:
            //register all overflow jobs...
            for (overflow_jobs_container_type::const_iterator itrCur=gmapOverflowResults.begin(), itrEnd=gmapOverflowResults.end(); itrCur!=itrEnd; ++itrCur)
              RegisterResult_NoAutoAbort(itrCur->first, itrCur->second.first, itrCur->second.second);
            gmapOverflowResults.clear();

            gfnRegisterResult = &stsMCSimJobSource::RegisterResult_NoAutoAbort;
            ReleaseAutoAbortCheckResources();
          }
          else
          {//this is not the last check.  Process overflow results to prepare for the next check:
            //resize the results-registered bitset for the next check:
            gAutoAbortResultsRegistered.resize(guaAutoAbortCheckPoints[guAutoAbortCheckIdx]-guaAutoAbortCheckPoints[guAutoAbortCheckIdx-1]);
            gAutoAbortResultsRegistered.reset();
            //process just the results that apply to this check; the rest remain "overflow" results:
            while ((!gmapOverflowResults.empty()) && (gmapOverflowResults.begin()->first <= guaAutoAbortCheckPoints[guAutoAbortCheckIdx]))
            {
              gAutoAbortResultsRegistered.set(gmapOverflowResults.begin()->first - guPreviousAutoAbortCheckPoint - 1);
              RegisterResult_NoAutoAbort(gmapOverflowResults.begin()->first, gmapOverflowResults.begin()->second.first, gmapOverflowResults.begin()->second.second);
              gmapOverflowResults.erase(gmapOverflowResults.begin());
            }
          }
        }
      }
    }
    else
    {//this is an "overflow" result.  save it to be posted after the next auto-abort
     //check has been run:
      gmapOverflowResults.insert(std::make_pair(rJobID,std::make_pair(rParam,rResult)));
    }
  }
  catch (ZdException & e)
  {
    e.AddCallpath("RegisterResult_AutoAbort()", "stsMCSimJobSource");
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
//  catch (ZdException & e)
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
  catch (ZdException & e)
  {
    e.AddCallpath("RegisterResult_ExceptionConditionExists()", "stsMCSimJobSource");
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

    //update ratios, significance, etc.:
    WriteResultToStructures(rResult.dSuccessfulResult);

    //if appropriate, estimate time required to complete all jobs and report it.
    unsigned int uiJobsProcessedCount =
      (guAutoAbortCheckIdx < guAutoAbortCheckCount)//auto-abort checking
     ? guPreviousAutoAbortCheckPoint + gAutoAbortResultsRegistered.count()
     : (gbsUnregisteredJobs.size()-gbsUnregisteredJobs.count()) + guiUnregisteredJobLowerBound;//this one hasn't been reset in gbsUnregisteredJobs yet.
    grPrintDirection.SatScanPrintf(gszReplicationFormatString, uiJobsProcessedCount, guiJobCount, rResult.dSuccessfulResult);
    if (uiJobsProcessedCount==10) {
      ::ReportTimeEstimate(gConstructionTime, guiJobCount, rParam, &grPrintDirection);
      ZdTimestamp tsReleaseTime; tsReleaseTime.Now(); tsReleaseTime.AddSeconds(3);//queue lines until 3 seconds from now
      grPrintDirection.SetThresholdPolicy(TimedReleaseThresholdPolicy(tsReleaseTime));
    }
  }
  catch (ZdException & e)
  {
    e.AddCallpath("RegisterResult_NoAutoAbort()", "stsMCSimJobSource");
    throw;
  }
}

//When we're through checking for auto-abort, we want to release any resources
//used.  (This is mostly for cancel and exception conditions that occur while
//auto-abort checking is active.)
void stsMCSimJobSource::ReleaseAutoAbortCheckResources()
{
  gAutoAbortResultsRegistered.clear();
  gmapOverflowResults.clear();
}

void stsMCSimJobSource::WriteResultToStructures(successful_result_type const & rResult)
{
  try
  {
    //update most likely clusters given latest simulated loglikelihood ratio
    grMLCs.UpdateTopClustersRank(rResult);
    //update significance indicator
    grRunner.UpdateSignificantRatiosList(rResult);
    if (gRatioWriter.get()) gRatioWriter->Write(rResult);
    //update power calculations
    grRunner.UpdatePowerCounts(rResult);
  }
  catch (ZdException & e)
  {
    e.AddCallpath("WriteResultToStructures()", "stsMCSimJobSource");
    throw;
  }
}

