//---------------------------------------------------------------------------

#ifndef stsMCSimContinuationPolicyH
#define stsMCSimContinuationPolicyH
//---------------------------------------------------------------------------
#include "BasePrint.h"
#include "boost/dynamic_bitset_fwd.hpp"

class stsMCSimReporter;//forward declared for friendship declaration

//template <typename ParamsType, typename ResultsType>
class stsMCSimContinuationPolicy
{
public:
  typedef unsigned int params_type;
  typedef double results_type;

  friend class stsMCSimReporter;

private:

  BasePrint const & grPrintDirection;
  bool gbShortCircuitConditionExists;

public:
  stsMCSimContinuationPolicy(BasePrint const & rPrintDirection);

  bool ShouldContinue() const;
  void Report_ResultsRegistered(unsigned job_idx,boost::dynamic_bitset<> const & result_registration_conditions,std::deque< std::pair<params_type, results_type> > const & jobs);

  bool ShortCircuitConditionExists() const { return gbShortCircuitConditionExists; }
  bool UserCancelConditionExists() const { return grPrintDirection.GetIsCanceled(); }
};



#endif

