//---------------------------------------------------------------------------

#ifndef stsMCSimContinuationPolicyH
#define stsMCSimContinuationPolicyH
//---------------------------------------------------------------------------


//template <typename ParamsType, typename ResultsType>
class stsMCSimContinuationPolicy
{
  typedef long params_type;
  typedef double results_type;

public:


  bool ShouldContinue() const { return true; }
  void Report_ResultsRegistered(params_type const & params,results_type const & result,long results_registered_count,long jobs_total_count) {}
};



#endif

