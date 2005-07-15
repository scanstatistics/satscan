//---------------------------------------------------------------------------

#include "SaTScan.h"
#pragma hdrstop

#include "stsCentricAlgoJobSource.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

//Class stsCentricAlgoJobSource
//Job source for centric algorithm running asynchronously

//constructor
stsCentricAlgoJobSource::stsCentricAlgoJobSource(
    unsigned long ulJobCount
   ,clock_t tCurrentTime
   ,AsynchronouslyAccessible<PrintQueue> & rPrintDirection
)
 : guiJobCount(ulJobCount)
 , guiNextJobParam(1)
 , guiUnregisteredJobLowerBound(1)
 , gfnRegisterResult(&stsCentricAlgoJobSource::RegisterResult_Simple)
 , gConstructionTime(tCurrentTime)
 , grPrintDirection(rPrintDirection)
{
}

void stsCentricAlgoJobSource::acquire(job_id_type & dst_job_id, param_type & dst_param)
{
  if (is_exhausted())
    throw std::runtime_error("can't acquire a job from an exhausted source.");

  gbsUnregisteredJobs.push_back(true);

  //all exception-unsafe ops have been executed, so do updates:
  dst_param = dst_job_id = guiNextJobParam;
  ++guiNextJobParam;
}

void stsCentricAlgoJobSource::Assert_NoExceptionsCaught() const
{
  typedef std::pair<job_id_type,std::pair<param_type,result_type> > exception_type;
  typedef std::deque<exception_type> exception_sequence_type;
  static const char * szExceptionIntroFormatString = "An exception was thrown from task #%d.";
  static const char * szExceptionTypeTitle = "\nException type: ";
  static const char * szExceptionMessageTitle = "\nException message: ";
  static const char * szExceptionCallPathTitle = "\nException call path:\n";

  if (GetExceptionCount() > 0) {
    ZdCarrierException<exception_sequence_type> lclException(gvExceptions, "", "stsCentricAlgoJobSource");
    exception_type const & rFirstException(lclException->front());
    ZdString sTemp;
    sTemp.printf(szExceptionIntroFormatString, rFirstException.first);
    lclException.AddMessage(sTemp.GetCString(), false);
//    lclException.AddMessage(szExceptionTypeTitle, false);
    lclException.AddMessage(szExceptionMessageTitle, false);
    lclException.AddMessage(rFirstException.second.second.second.GetErrorMessage(), false);
    lclException.AddMessage(szExceptionCallPathTitle, false);
    lclException.AddMessage(rFirstException.second.second.second.GetCallpath(), false);

    throw lclException;
  }
}

bool stsCentricAlgoJobSource::CancelRequested() const
{
  return grPrintDirection.Unlocked().Value().GetIsCanceled();
}

//How many jobs are there that have been acquired but whose results have not
//been registered?
unsigned int stsCentricAlgoJobSource::GetUnregisteredJobCount() const
{
  return gbsUnregisteredJobs.count();
}

std::deque<unsigned int> stsCentricAlgoJobSource::GetUnregisteredJobs() const
{
  std::deque<unsigned int> seqResult;
  for (unsigned int ui=guiUnregisteredJobLowerBound, uiCurr=0, uiEnd=gbsUnregisteredJobs.size(); uiCurr < uiEnd; ++ui,++uiCurr)
    if (gbsUnregisteredJobs.test(uiCurr))
      seqResult.push_back(ui);
  return seqResult;
}

//From how many jobs were exceptions caught?
unsigned int stsCentricAlgoJobSource::GetExceptionCount() const
{
  return gvExceptions.size();
}

std::deque<std::pair<stsCentricAlgoJobSource::job_id_type,std::pair<stsCentricAlgoJobSource::param_type,stsCentricAlgoJobSource::result_type> > >
stsCentricAlgoJobSource::GetExceptions() const
{
  return gvExceptions;
}

bool stsCentricAlgoJobSource::is_exhausted() const
{
  return
    CancelConditionExists()
   || ExceptionConditionExists()
   || (guiNextJobParam > guiJobCount);
}

//Remove the first N bits from operand.
void stsCentricAlgoJobSource::DynamicBitsetPopFrontN(boost::dynamic_bitset<> & operand, unsigned long N)
{
  operand >>= N;//shift all bits down
  operand.resize(N > operand.size() ? 0 : operand.size()-N);//pop the back bits off
}

void stsCentricAlgoJobSource::register_result(job_id_type const & job_id, param_type const & param, result_type const & result)
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
    e.AddCallpath("register_result()", "stsCentricAlgoJobSource");
    throw;
  }
}

//register a result when analysis has been canceled.  This will be called for
//all subsequent job registrations (that were already running when cancel
//got triggered), which are ignored.
void stsCentricAlgoJobSource::RegisterResult_CancelConditionExists(job_id_type const & rJobId, param_type const & rParam, result_type const & rResult)
{
//  try
//  {
//  }
//  catch (ZdException & e)
//  {
//    e.AddCallpath("RegisterResult_CancelConditionExists()", "stsCentricAlgoJobSource");
//    throw;
//  }
}

//register a result when a previously registered result indicated an exception.
void stsCentricAlgoJobSource::RegisterResult_ExceptionConditionExists(job_id_type const & rJobID, param_type const & rParam, result_type const & rResult)
{
  try
  {
    if (!rResult.first)
      gvExceptions.push_back(std::make_pair(rJobID, std::make_pair(rParam,rResult)));
  }
  catch (ZdException & e)
  {
    e.AddCallpath("RegisterResult_ExceptionConditionExists()", "stsCentricAlgoJobSource");
    throw;
  }
}

//register a result when no extended conditions (thrown exceptions, cancelation)
//are active.
void stsCentricAlgoJobSource::RegisterResult_Simple(job_id_type const & rJobID, param_type const & rParam, result_type const & rResult)
{
  try
  {
    //check exception condition first.  Want to report an exception even if
    //cancel is requested.
    if (rResult.first)
    {
      //populate stored exceptions:
      gvExceptions.push_back(std::make_pair(rJobID, std::make_pair(rParam,rResult)));
      gfnRegisterResult = &stsCentricAlgoJobSource::RegisterResult_ExceptionConditionExists;
      return;
    }
    else if (CancelRequested())
    {
      gfnRegisterResult = &stsCentricAlgoJobSource::RegisterResult_CancelConditionExists;
      return;
    }

    //if appropriate, estimate time required to complete all jobs and report it.
    unsigned int uiJobsProcessedCount = (gbsUnregisteredJobs.size()-gbsUnregisteredJobs.count()) + guiUnregisteredJobLowerBound;//this one hasn't been reset in gbsUnregisteredJobs yet.
    AsynchronouslyAccessible<PrintQueue>::LockWrapper lclLockedPrintQueue(grPrintDirection.Locked());
    PrintQueue & rPrintQueue = lclLockedPrintQueue.Value();
    rPrintQueue.SatScanPrintf("Evaluating centroid %i of %i\n", uiJobsProcessedCount, guiJobCount);
    if (uiJobsProcessedCount==10) {
      ::ReportTimeEstimate(gConstructionTime, guiJobCount, rParam, &rPrintQueue);
      ZdTimestamp tsReleaseTime; tsReleaseTime.Now(); tsReleaseTime.AddSeconds(3);//queue lines until 3 seconds from now
      rPrintQueue.SetThresholdPolicy(TimedReleaseThresholdPolicy(tsReleaseTime));
    }
  }
  catch (ZdException & e)
  {
    e.AddCallpath("RegisterResult_Simple()", "stsCentricAlgoJobSource");
    throw;
  }
}


