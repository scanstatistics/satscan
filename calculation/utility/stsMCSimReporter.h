//---------------------------------------------------------------------------

#ifndef stsMCSimReporterH
#define stsMCSimReporterH
//---------------------------------------------------------------------------
#include "AnalysisRun.h"
#include "stsMCSimContinuationPolicy.h"
#pragma warn -8012
#pragma warn -8008
#pragma warn -8066
#pragma warn -8055
#include "boost/dynamic_bitset.hpp"
#pragma warn +8012
#pragma warn +8008
#pragma warn +8066
#pragma warn +8055
//---------------------------------------------------------------------------


class stsMCSimReporter
{
public:
  typedef unsigned int params_type;
  typedef double results_type;

private:
  static const unsigned guaShortCircuitCheckPoints[];
  static const float guaShortCircuitCheckCutoffValues[];
  static const unsigned guMaxShortCircuitCheckCount = 4;//the two above arrays must have at least this many elements.

//  CParameters const & grParameters;
  stsMCSimContinuationPolicy & grContinuationPolicy;
  MostLikelyClustersContainer & grMLCs;
  CSignificantRatios05 & grSignificantRatios;
  BasePrint & grPrintDirection;
  const char * gszReplicationFormatString;

  boost::dynamic_bitset<> gResultRegistrationConditions;//contains bits for only the first guaShortCircuitCheckPoints[guShortCircuitCheckIndex] bits.
  bool gbShortCircuitConditionExists;
  unsigned guShortCircuitCheckCount;//how many short-circuit tests will we have?
  unsigned guShortCircuitCheckIdx;//which short-circuit test comes next?
  unsigned guResultsNotShortCircuitedCount;//how many results have been communicated to ratios, etc?

public:
  stsMCSimReporter
   ( CParameters const & rParameters
    ,stsMCSimContinuationPolicy & rCtPlcy
    ,CSignificantRatios05 & theSignificantRatios
    ,MostLikelyClustersContainer & theMLCs
    ,BasePrint & thePrintDirection
    ,const char * szReplicationFormatString
   );

  void Report_SubcontractLet(unsigned job_idx,boost::dynamic_bitset<> const & result_registration_conditions,std::deque< std::pair<params_type, results_type> > const & jobs)
  {}
  void Report_ResultsRegistered(unsigned job_idx,boost::dynamic_bitset<> const & result_registration_conditions,std::deque< std::pair<params_type, results_type> > const & jobs);

  bool ShortCircuitConditionExists() const { return gbShortCircuitConditionExists; }
  unsigned ResultsNotShortCircuitedCount() const { return guResultsNotShortCircuitedCount; }//Meaningful only if ShortCircuitConditionExists()
};



#endif
