//---------------------------------------------------------------------------

#pragma hdrstop

#include "contractor.h"

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//ReportFn concept:
//void Report_SubcontractLet
// (
//   params_type const & params
//  ,long results_registered_count
//  ,long total_jobs_count
// )
//void Report_ResultsRegistered
// (
//   params_type const & params
//  ,result_type const & result
//  ,long results_registered_count
//  ,long total_jobs_count
// )
//---------------------------------------------------------------------------

#pragma package(smart_init)

void test_fn()
{
  std::deque<std::pair<long, long> > jobs;
  jobs.push_back(std::make_pair(7, 0));
//  contractor<long, long> cntor(jobs);
  bool b;
  int i;
  long l;

//  b = cntor.job_acquired(i, l);
//  cntor.register_results(i, l);
}