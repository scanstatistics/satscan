//---------------------------------------------------------------------------

#include "SaTScan.h"
#pragma hdrstop

#include "stsMCSimContinuationPolicy.h"

//ClassDesc Begin stsMCSimContinuationPolicy
//A class that complies with the contractor.ContinuationPolicy pattern, assuming
//that params_type is unsigned int and results_type is double.
//Declares class stsMCSimReporter to be a friend.  The private data member,
//'gbShortCircuitConditionExists', is manipulated by an stsMCSimReporter object.
//ClassDesc End stsMCSimContinuationPolicy


//---------------------------------------------------------------------------

#pragma package(smart_init)

stsMCSimContinuationPolicy::stsMCSimContinuationPolicy(BasePrint const & rPrintDirection)
 : grPrintDirection(rPrintDirection)
 , gbShortCircuitConditionExists(false)
{}

bool stsMCSimContinuationPolicy::ShouldContinue() const
{
  return !(gbShortCircuitConditionExists || UserCancelConditionExists());
}

void stsMCSimContinuationPolicy::Report_ResultsRegistered(unsigned job_idx,boost::dynamic_bitset<> const & result_registration_conditions,std::deque< std::pair<params_type, results_type> > const & jobs)
{
}

