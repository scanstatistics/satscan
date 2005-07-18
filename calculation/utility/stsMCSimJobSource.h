//---------------------------------------------------------------------------

#ifndef stsMCSimJobSourceH
#define stsMCSimJobSourceH

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
#include "PrintQueue.h"

//---------------------------------------------------------------------------


class stsMCSimJobSource
{
public://types/enums
  typedef unsigned int param_type;
  typedef double successful_result_type;
  typedef std::pair<bool, std::pair<successful_result_type, ZdException> > result_type;
  typedef unsigned long job_id_type;
  typedef std::pair<job_id_type, param_type> job_info_type;

private://types/enums
  typedef void (stsMCSimJobSource::*result_registration_fn_type)(job_id_type const &, param_type const &, result_type const &);
  typedef std::map<job_id_type,std::pair<param_type,result_type> > overflow_jobs_container_type;

private://data members
  unsigned int guiJobCount;
  unsigned int guiNextJobParam;
  unsigned int guiUnregisteredJobLowerBound;//gbsUnregisteredJobs[0] refers to the job with this index.  gbsUnregisteredJobs[n] refers to the job at index 'guiUnregisteredJobLowerBound'+n .
  boost::dynamic_bitset<> gbsUnregisteredJobs;

  std::deque<std::pair<job_id_type,std::pair<param_type,result_type> > > gvExceptions;

  static const unsigned guaAutoAbortCheckPoints[];
  static const float guaAutoAbortCheckCutoffValues[];
  static const unsigned guMaxAutoAbortCheckCount = 4;//the two above arrays must have at least this many elements.

  result_registration_fn_type gfnRegisterResult;
  boost::dynamic_bitset<> gAutoAbortResultsRegistered;//contains bits for only the first guaAutoAbortCheckPoints[guAutoAbortCheckIndex] jobs.
  overflow_jobs_container_type gmapOverflowResults;//contains results for only the jobs after the first guaAutoAbortCheckPoints[guAutoAbortCheckIndex] jobs, sorted by jobid.
  unsigned guAutoAbortCheckCount;//how many short-circuit tests will we have?
  unsigned guAutoAbortCheckIdx;//which short-circuit test comes next?
  unsigned guPreviousAutoAbortCheckPoint;//what was the previous check point?

  clock_t const gConstructionTime;
  MostLikelyClustersContainer & grMLCs;
  PrintQueue & grPrintDirection;
  const char * gszReplicationFormatString;
  std::auto_ptr<LoglikelihoodRatioWriter> gRatioWriter;
  AnalysisRunner & grRunner;

private://functions
  void WriteResultToStructures(successful_result_type const & rResult);
  void RegisterResult_NoAutoAbort(job_id_type const & rJobId, param_type const & rParam, result_type const & rResult);
  void RegisterResult_AutoAbort(job_id_type const & rJobId, param_type const & rParam, result_type const & rResult);
  void RegisterResult_AutoAbortConditionExists(job_id_type const & rJobId, param_type const & rParam, result_type const & rResult);
  void RegisterResult_ExceptionConditionExists(job_id_type const & rJobId, param_type const & rParam, result_type const & rResult);
  void RegisterResult_CancelConditionExists(job_id_type const & rJobId, param_type const & rParam, result_type const & rResult);

  bool CancelRequested() const;
  bool AllResultsCollectedForAutoAbortCheck() const;
  void ReleaseAutoAbortCheckResources();
  static void DynamicBitsetPopFrontN(boost::dynamic_bitset<> & operand, unsigned long N);

public:
//  stsMCSimJobSource(CParameters const & rParameters) : guiJobCount(0), guiNextJobParam(1), guiUnfinishedJobLowerBound(1);
  stsMCSimJobSource(
    CParameters const & rParameters
   ,clock_t tCurrentTime
   ,MostLikelyClustersContainer & rMLCs
   ,PrintQueue & rPrintDirection
   ,const char * szReplicationFormatString
   ,AnalysisRunner & rRunner
  );
  bool is_exhausted() const;
  void acquire(job_id_type & dst_job_id, param_type & dst_param);
//  void unacquire(job_id_type const & job_id) {}
  void register_result(job_id_type const & job_id, param_type const & param, result_type const & result);
//  void register_failure(job_id_type job_id) {}

  unsigned int GetSuccessfullyCompletedJobCount() const;
  unsigned int GetUnregisteredJobCount() const;
  std::deque<unsigned int> GetUnregisteredJobs() const;
  bool AutoAbortConditionExists() const { return gfnRegisterResult == &stsMCSimJobSource::RegisterResult_AutoAbortConditionExists; }
  bool CancelConditionExists() const { return gfnRegisterResult == &stsMCSimJobSource::RegisterResult_CancelConditionExists; }
  bool ExceptionConditionExists() const { return gfnRegisterResult == &stsMCSimJobSource::RegisterResult_ExceptionConditionExists; }
  unsigned int GetExceptionCount() const;
  std::deque<std::pair<job_id_type,std::pair<param_type,result_type> > > GetExceptions() const;
  void Assert_NoExceptionsCaught() const;
};



#endif
