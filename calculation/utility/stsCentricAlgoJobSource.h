//******************************************************************************
#ifndef stsCentricAlgoJobSourceH
#define stsCentricAlgoJobSourceH
//******************************************************************************
#include <utility>
#include <deque>
#pragma warn -8012
#pragma warn -8008
#pragma warn -8066
#pragma warn -8055
#include "boost/dynamic_bitset.hpp"
#pragma warn +8012
#pragma warn +8008
#pragma warn +8066
#pragma warn +8055
#include "AnalysisRun.h"
#include "LoglikelihoodRatioWriter.h"
#include "AsynchronouslyAccessible.h"
#include "PrintQueue.h"

class stsCentricAlgoJobSource
{
public://types/enums
  struct job_result {
    enum                        exception_type {unknown=0, std, zd, zdmemory};

    bool                        bExceptional;
    exception_type              eException_type;
    ZdException                 Exception;
  };

  typedef unsigned int param_type;
//  typedef std::pair<bool, ZdException> result_type;
  typedef job_result result_type;
  typedef unsigned long job_id_type;
  typedef std::pair<job_id_type, param_type> job_info_type;

private://types/enums
  typedef void (stsCentricAlgoJobSource::*result_registration_fn_type)(job_id_type const &, param_type const &, result_type const &);
  typedef std::map<job_id_type,std::pair<param_type,result_type> > overflow_jobs_container_type;

private://data members
  unsigned int guiJobCount;
  unsigned int guiNextJobParam;
  unsigned int guiUnregisteredJobLowerBound;//gbsUnregisteredJobs[0] refers to the job with this index.  gbsUnregisteredJobs[n] refers to the job at index 'guiUnregisteredJobLowerBound'+n .
  boost::dynamic_bitset<> gbsUnregisteredJobs;

  std::deque<std::pair<job_id_type,std::pair<param_type,result_type> > > gvExceptions;

  result_registration_fn_type gfnRegisterResult;

  bool gbIsExplicitlyExhausted;
  boost::posix_time::ptime const gConstructionTime;
  AsynchronouslyAccessible<PrintQueue> & grPrintDirection;

private://functions
  void RegisterResult_Simple(job_id_type const & rJobId, param_type const & rParam, result_type const & rResult);
  void RegisterResult_ExceptionConditionExists(job_id_type const & rJobId, param_type const & rParam, result_type const & rResult);
  void RegisterResult_CancelConditionExists(job_id_type const & rJobId, param_type const & rParam, result_type const & rResult);

  bool CancelRequested() const;
  static void DynamicBitsetPopFrontN(boost::dynamic_bitset<> & operand, unsigned long N);

public:
  stsCentricAlgoJobSource(
    unsigned long ulJobCount
   ,boost::posix_time::ptime CurrentTime
   ,AsynchronouslyAccessible<PrintQueue> & rPrintDirection
  );
  bool is_exhausted() const;
  void acquire(job_id_type & dst_job_id, param_type & dst_param);
//  void unacquire(job_id_type const & job_id) {}
  void register_result(job_id_type const & job_id, param_type const & param, result_type const & result);
//  void register_failure(job_id_type job_id) {}

  void Exhaust() { gbIsExplicitlyExhausted = true; }
  unsigned int GetSuccessfullyCompletedJobCount() const;
  unsigned int GetUnregisteredJobCount() const;
  std::deque<unsigned int> GetUnregisteredJobs() const;
  bool CancelConditionExists() const { return gfnRegisterResult == &stsCentricAlgoJobSource::RegisterResult_CancelConditionExists; }
  bool ExceptionConditionExists() const { return gfnRegisterResult == &stsCentricAlgoJobSource::RegisterResult_ExceptionConditionExists; }
  unsigned int GetExceptionCount() const;
  std::deque<std::pair<job_id_type,std::pair<param_type,result_type> > > GetExceptions() const;
  void Assert_NoExceptionsCaught() const;
};
//******************************************************************************
#endif

