//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "stsCentricAlgoJobSource.h"

//Class stsCentricAlgoJobSource
//Job source for centric algorithm running asynchronously

//constructor
stsCentricAlgoJobSource::stsCentricAlgoJobSource(
    unsigned long ulJobCount
   ,boost::posix_time::ptime CurrentTime
   ,AsynchronouslyAccessible<PrintQueue> & rPrintDirection
)
 : guiJobCount(ulJobCount)
 , guiNextJobParam(1)
 , guiUnregisteredJobLowerBound(1)
 , gbIsExplicitlyExhausted(false)
 , gfnRegisterResult(&stsCentricAlgoJobSource::RegisterResult_Simple)
 , gConstructionTime(CurrentTime)
 , grPrintDirection(rPrintDirection)
 , _frequent_estimations(false)
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
  static const char * szExceptionMessageTitle = "\nException message: ";
  static const char * szExceptionCallPathTitle = "\nException call path:\n";

  if (GetExceptionCount() > 0) {
    //scan collection of exceptions for memory_exception type, this type trumps all others -- take first found
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

    CarrierException<exception_sequence_type> lclException(gvExceptions, "", "stsCentricAlgoJobSource");
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

bool stsCentricAlgoJobSource::CancelRequested() const
{
  return grPrintDirection.Unlocked().Value().GetIsCanceled();
}

//How many jobs have registered a successful (no exceptions) result?
unsigned int stsCentricAlgoJobSource::GetSuccessfullyCompletedJobCount() const
{
  return (guiUnregisteredJobLowerBound-1) + gbsUnregisteredJobs.count() - gvExceptions.size();
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
    gbIsExplicitlyExhausted
   || CancelConditionExists()
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
  catch (prg_exception & e)
  {
    e.addTrace("register_result()", "stsCentricAlgoJobSource");
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
//  catch (prg_exception & e)
//  {
//    e.addTrace("RegisterResult_CancelConditionExists()", "stsCentricAlgoJobSource");
//    throw;
//  }
}

//register a result when a previously registered result indicated an exception.
void stsCentricAlgoJobSource::RegisterResult_ExceptionConditionExists(job_id_type const & rJobID, param_type const & rParam, result_type const & rResult)
{
  try
  {
    if (!rResult.bExceptional)
      gvExceptions.push_back(std::make_pair(rJobID, std::make_pair(rParam,rResult)));
  }
  catch (prg_exception & e)
  {
    e.addTrace("RegisterResult_ExceptionConditionExists()", "stsCentricAlgoJobSource");
    throw;
  }
}

//register a result when no extended conditions (thrown exceptions, cancelation)
//are active.
void stsCentricAlgoJobSource::RegisterResult_Simple(job_id_type const & rJobID, param_type const & rParam, result_type const & rResult) {
    try {
        //check exception condition first.  Want to report an exception even if
        //cancel is requested.
        if (rResult.bExceptional) {
            //populate stored exceptions:
            gvExceptions.push_back(std::make_pair(rJobID, std::make_pair(rParam,rResult)));
            gfnRegisterResult = &stsCentricAlgoJobSource::RegisterResult_ExceptionConditionExists;
            return;
        } else if (CancelRequested()) {
            gfnRegisterResult = &stsCentricAlgoJobSource::RegisterResult_CancelConditionExists;
            return;
        }

        //if appropriate, estimate time required to complete all jobs and report it.
        unsigned int uiJobsProcessedCount = (gbsUnregisteredJobs.size()-gbsUnregisteredJobs.count()) + guiUnregisteredJobLowerBound;//this one hasn't been reset in gbsUnregisteredJobs yet.
        AsynchronouslyAccessible<PrintQueue>::LockWrapper lclLockedPrintQueue(grPrintDirection.Locked());
        PrintQueue & rPrintQueue = lclLockedPrintQueue.Value();
        rPrintQueue.Printf("Evaluating centroid %i of %i\n", BasePrint::P_STDOUT, uiJobsProcessedCount, guiJobCount);
        if (uiJobsProcessedCount == 10 || (_frequent_estimations && (uiJobsProcessedCount % SIMULATION_EST_MODULAS == 0))) {
            _frequent_estimations = ::ReportTimeEstimate(gConstructionTime, guiJobCount, rParam, rPrintQueue, true, uiJobsProcessedCount != 10) > FREQUENT_ESTIMATES_SECONDS;
            SaTScan::Timestamp tsReleaseTime; tsReleaseTime.Now(); tsReleaseTime.AddSeconds(3);//queue lines until 3 seconds from now
            rPrintQueue.SetThresholdPolicy(TimedReleaseThresholdPolicy(tsReleaseTime));
        }
    } catch (prg_exception & e) {
        e.addTrace("RegisterResult_Simple()", "stsCentricAlgoJobSource");
        throw;
    }
}


