//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
#include "contractor.h"
//extern "C" void tss_cleanup_implemented(void) {} 

namespace boost
{
    void tss_cleanup_implemented() {}
}
//---------------------------------------------------------------------------
//ClassDesc Begin contractor
//A contractor object manages the repeated execution of a process with different
//arguments for each repetition.
//It is templated on:
// typename ParamsType: the type of the parameters with which each repetition
//   will be executed,
// typename ResultsType: the type of the result generated by each repetition,
// typename Reporter: the type of the object that will be invoked each time
//   results are registered for an execution,
// typename ContinuationPolicy: the type of the object that will be queried to
//   determine whether to continue or be finished.
//The model works like this:
//A process, 'prc1', queries "job_acquired()", passing a reference to itself
//(the contractor uses this for bookkeeping) and a reference to a ParamsType
//object.  If this query returns "true", the ParamsType argument holds a value
//that should be processed.  Otherwise, the value in the ParamsType argument is
//meaningless.
//'prc1' then executes until it has calculated a ResultsType value, at which
//point it calls "register_results()", passing a reference to itself and the
//ResultsType object.
//'prc1' then queries "is_finished()" to determine whether or not to repeat the
//steps.
//There are constraints on the types Reporter and ContinuationPolicy.  They must
//provide a minimum interface:
//Reporter:
//  For an object of type Reporter, 'rp1', the following calls must be valid:
//1: rp1.Report_SubcontractLet(/*unsigned*/ job_idx
//         ,/*boost::dynamic_bitset<> const &*/ result_registration_conditions
//         ,/*std::deque< std::pair<params_type, results_type> > const &*/ jobs)
//2: rp1.Report_ResultsRegistered(/*unsigned*/ job_idx
//         ,/*boost::dynamic_bitset<> const &*/ result_registration_conditions
//         ,/*std::deque< std::pair<params_type, results_type> > const &*/ jobs)
//ContinuationPolicy:
//  For an object of type ContinuationPolicy, 'cp1', the following calls must be
//valid:
//1: cp1.Report_ResultsRegistered(/*unsigned*/ job_idx
//         ,/*boost::dynamic_bitset<> const &*/ result_registration_conditions
//         ,/*std::deque< std::pair<params_type, results_type> > const &*/ jobs)
//The classes, null_contractor_reporter and null_contractor_continuation_policy,
//are examples of conforming types.
//The Reporter::Report_SubcontractLet and function is called by
//contractor::job_acquired after all contractor bookkeeping is updated.
//The Reporter::Report_ResultsRegistered and
//ContinuationPolicy::Report_ResultsRegistered functions are called (in that
//order) by contractor::register_results after all contractor bookkeeping is
//updated, implying that result_registration_conditions.test(job_idx)==true and
//jobs.at(job_idx).second contains the result for the job.
//ClassDesc End contractor
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
//ClassDesc Begin subcontractor
//A subcontractor is a functor (it contains an operator() -- pronounced,
//"operator function call") whose behavior is to repeatedly execute a function
//with different arguments for each repetition, querying a 'contractor' object
//for the new arguments and registering the results.  It repeats for as long as
//contractor::is_finished()==false.
//It is templated on:
// typename ContractorType: the type of the contractor that will "dispense"
//   arguments and "accept" results
// typename Function: the type of the function to be repeatedly executed
//There are constraints on the types ContractorType and Function.  They must
//provide a minimum interface:
//ContractorType:
//  For an object of type ContractorType, 'ct1', the following expressions must
//be valid:
//1: typename ContractorType::params_type
//2: typename ContractorType::results_type
//3: bool b = ct1.job_acquired(*this
//             ,/*typename ContractorType::params_type*/ params)
//4: ct1.register_results(*this
//    ,/*typename ContractorType::results_type*/ results);
//Function:
//  For an object of type Function, 'fn1', the following expressions must be
//valid:
//1: Function fn2(fn1);//must have a copy constructor
//2: typename ContractorType::results_type results(m_function(params));//the
//   //operator() must take a ContractorType::params_type argument and return
//   //a ContractorType::results_type.
//ClassDesc End subcontractor
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

class test_job_source
{
  short gw;
public:
  typedef short param_type;
  typedef float result_type;
  typedef unsigned long job_id_type;
  typedef std::pair<job_id_type, param_type> job_info_type;

public:
  test_job_source() : gw(0) {}
  bool is_exhausted() const { return gw >= 10; }
  void acquire(job_id_type & dst_job_id, param_type & dst_param) { if (is_exhausted()) throw std::runtime_error("can't acquire a job from an exhausted source."); ++gw; dst_job_id = gw; }
//  void unacquire(job_id_type const & job_id) {}
  void register_result(job_id_type const & job_id, param_type const & param, result_type const & result) { }
//  void register_failure(job_id_type job_id) {}
};

void test_fn()
{
  test_job_source jobs;
  contractor<test_job_source> cntor(jobs);
  short w;
  int i;
  long l=0;

  cntor.job_acquired(i, w);
  cntor.register_result(i, w, static_cast<float>(l));
}


