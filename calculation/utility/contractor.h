//---------------------------------------------------------------------------

#ifndef contractorH
#define contractorH
//---------------------------------------------------------------------------
#include <vector>
#include <deque>
#include <list>
#include <algorithm>
#include "boost/thread/thread.hpp"
#include "boost/thread/recursive_mutex.hpp"
//---------------------------------------------------------------------------

template <typename ParamsType, typename ResultsType, typename Reporter, typename ContinuationPolicy>
class contractor
{
public:
  typedef ParamsType params_type;
  typedef ResultsType results_type;

private:
  typedef boost::recursive_mutex access_mutex_t;
  typedef std::vector<std::pair<void const *, long> > current_subcontracts_type;
  std::deque<long> m_job_id_queue;//m_job_id_queue[i] is an index into m_jobs.
  std::deque< std::pair<ParamsType, ResultsType> > & m_jobs;//each job's id is (1 + its index in this structure).
  current_subcontracts_type m_current_subcontracts;//first: subcontractors who have acquired a job and haven't yet registered results for it; second: job id
  std::vector<bool> m_registered_results;//at(i)==false, jobid i+1 results have not been registered; true: results have been registered
  Reporter m_reporter;
  ContinuationPolicy m_continuation_policy;
  mutable access_mutex_t m_access_mutex;
  unsigned int m_results_registered_count;

  void setup()
  { for (long l=1; (unsigned)l <= m_jobs.size(); ++l)
    { m_job_id_queue.push_back(l);
      m_registered_results.push_back(false);
    }
  }

public:
  contractor(std::deque< std::pair<ParamsType, ResultsType> > & jobs, Reporter const & reporter, ContinuationPolicy const & continuation_policy)
   : m_jobs(jobs)
   , m_results_registered_count(0)
   , m_reporter(reporter)
   , m_continuation_policy(continuation_policy)
  { setup();
  }
  contractor(std::deque< std::pair<ParamsType, ResultsType> > & jobs, Reporter const & reporter)
   : m_jobs(jobs)
   , m_results_registered_count(0)
   , m_reporter(reporter)
   , m_continuation_policy()
  { setup();
  }
  contractor(std::deque< std::pair<ParamsType, ResultsType> > & jobs)
   : m_jobs(jobs)
   , m_results_registered_count(0)
   , m_reporter()
   , m_continuation_policy()
  { setup();
  }

  bool is_finished() const
  { access_mutex_t::scoped_lock lcl_lock(m_access_mutex);
    return m_job_id_queue.empty() || !m_continuation_policy.ShouldContinue();
  }
  unsigned int results_registered_count() const { return m_results_registered_count; }

  template <typename SubcontractorType>
  bool job_acquired(SubcontractorType const & subcontractor, ParamsType & job_params)
  { access_mutex_t::scoped_lock lcl_lock(m_access_mutex);

    {//check to make sure subcontractor doesn't have an uncompleted job:
      current_subcontracts_type::const_iterator itr = m_current_subcontracts.begin();
      current_subcontracts_type::const_iterator itrend = m_current_subcontracts.end();
      for (; (itr != itrend) && (itr->first != &subcontractor); ++itr) {}
      if (itr != m_current_subcontracts.end())
        throw std::runtime_error("Subcontractor has not registered results of previously acquired job.");
    }

    if (is_finished())
      return false;
    long id(m_job_id_queue.front());
    m_current_subcontracts.push_back(std::make_pair(&subcontractor, id));
    job_params = m_jobs[id-1].first;
    m_reporter.Report_SubcontractLet(job_params,results_registered_count(),m_jobs.size());
    m_job_id_queue.pop_front();
    return true;
  }

  template <typename SubcontractorType>
  void register_results(SubcontractorType const & subcontractor, ResultsType const & job_results)
  { access_mutex_t::scoped_lock lcl_lock(m_access_mutex);

    current_subcontracts_type::iterator itr = m_current_subcontracts.begin();
    {//check to make sure subcontractor has an uncompleted job:
      current_subcontracts_type::const_iterator itrend = m_current_subcontracts.end();
      for (; (itr != itrend) && (itr->first != &subcontractor); ++itr) {}
      if (itr == m_current_subcontracts.end())
        throw std::runtime_error("Subcontractor has not acquired a job.");
    }

    long id(itr->second);
    std::pair<params_type, results_type> & job(m_jobs[id-1]);
    job.second = job_results;
    m_registered_results[id-1] = true;
    ++m_results_registered_count;
    m_current_subcontracts.erase(itr);

    m_reporter.Report_ResultsRegistered(job.first,job_results,results_registered_count(),m_jobs.size());
    m_continuation_policy.Report_ResultsRegistered(job.first,job_results,results_registered_count(),m_jobs.size());
  }
};

template <typename ContractorType, typename Function>
class subcontractor
{
  ContractorType & m_contractor;
  Function m_function;

public:
  subcontractor(ContractorType & contractor, Function function)
   : m_contractor(contractor)
   , m_function(function)
  {}

  void operator() ()
  {
    while (!m_contractor.is_finished())
    {
      typename ContractorType::params_type params;
      if (m_contractor.job_acquired(*this, params))
      {
        typename ContractorType::results_type results(m_function(params));
        m_contractor.register_results(*this, results);
      }
    }
  }

};

template <typename ParamsType, typename ResultsType>
class null_contractor_reporter
{
  typedef ParamsType params_type;
  typedef ResultsType results_type;

public:
  void Report_SubcontractLet(params_type const & params,long results_registered_count,long jobs_total_count) {}
  void Report_ResultsRegistered(params_type const & params,results_type const & result,long results_registered_count,long jobs_total_count) {}
};

template <typename ParamsType, typename ResultsType>
class null_contractor_continuation_policy
{
  typedef ParamsType params_type;
  typedef ResultsType results_type;

public:
  bool ShouldContinue() const { return true; }
  void Report_ResultsRegistered(params_type const & params,results_type const & result,long results_registered_count,long jobs_total_count) {}
};

#endif
