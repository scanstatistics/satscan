//---------------------------------------------------------------------------

#include "SaTScan.h"
#pragma hdrstop

#include "stsMCSimReporter.h"

//ClassDesc Begin stsMCSimReporter
//A class that complies with the contractor.Reporter pattern, assuming
//that params_type is unsigned int and results_type is double.
//The complexity of the function, 'Report_ResultsRegistered', could use a little
//description.  It implements a "short-circuit" logic by which it determines,
//based on the simulations calculated to this point, whether or not any more
//simulations need to be calculated.  The check is made at several discrete
//points known at compile time (see AnalysisRunner::CheckForEarlyTermination for
//the check that assumes serial calculation of simulations).  The logic allows
//parallel threads to calculate simulations whose results may not be needed.
//For instance, the first check point is 99; that means we'll check the first
//99 results to determine if we need to spend time checking any more.  If 10
//processes are running simulations in parallel and they have calculated results
//for simulations 1-85 and 87-99, we can allow 9 processes to continue
//calculating while we wait for the 86'th one to complete.  If, when we acquire
//get all of the first 99 and check them, we decided that no more simulations
//need to be calculated, we discard the results that are not needed, otherwise
//we keep the extra results (as we have determined that they are needed).
//(Assuming that we're using multiple processors, no time is wasted by this
//approach.)
//Finally, if we determine at the final checkpoint that we need to continue,
//then all remaining results will be retained.
//After the threads have all ended, we can figure out what results were needed
//by querying ShortCircuitConditionExists().  If so, we can find out how many
//simulations were considered by querying ResultsNotShortCircuitedCount().  This
//count should never be used unless a short-circuit condition exists!
//ClassDesc End stsMCSimReporter

//---------------------------------------------------------------------------

#pragma package(smart_init)

const unsigned stsMCSimReporter::guaShortCircuitCheckPoints[] = { 99, 199, 499, 999 };
const float stsMCSimReporter::guaShortCircuitCheckCutoffValues[] = { 0.5, 0.4, 0.2, 0.1 };

stsMCSimReporter::stsMCSimReporter
   ( CParameters const & rParameters
    ,stsMCSimContinuationPolicy & rCtPlcy
    ,CSignificantRatios05 & theSignificantRatios
    ,MostLikelyClustersContainer & theMLCs
    ,BasePrint & thePrintDirection
    ,const char * szReplicationFormatString
   )
   : grContinuationPolicy(rCtPlcy)
   , grSignificantRatios(theSignificantRatios)
   , grMLCs(theMLCs)
   , grPrintDirection(thePrintDirection)
   , gszReplicationFormatString(szReplicationFormatString)
   , gbShortCircuitConditionExists(false)
   , guShortCircuitCheckIdx(0)
   , guResultsNotShortCircuitedCount(0)
{
  guShortCircuitCheckCount = 0;
  if (rParameters.GetTerminateSimulationsEarly()) {
    while ((guShortCircuitCheckCount < guMaxShortCircuitCheckCount) && (guaShortCircuitCheckPoints[guShortCircuitCheckCount] < rParameters.GetNumReplicationsRequested())) {
      ++guShortCircuitCheckCount;
    }
  }
  gResultRegistrationConditions.resize(guShortCircuitCheckIdx < guShortCircuitCheckCount ? guaShortCircuitCheckPoints[guShortCircuitCheckIdx] : rParameters.GetNumReplicationsRequested());
}

void stsMCSimReporter::Report_ResultsRegistered(unsigned job_idx,boost::dynamic_bitset<> const & result_registration_conditions,std::deque< std::pair<params_type, results_type> > const & jobs)
{
  try {
    if (guShortCircuitCheckIdx < guShortCircuitCheckCount) {//if we're checking for short-circuit conditions...
      if (!gbShortCircuitConditionExists) {
        if (job_idx < guaShortCircuitCheckPoints[guShortCircuitCheckIdx]) {
          gResultRegistrationConditions.set(job_idx);
          if (gResultRegistrationConditions.count() == gResultRegistrationConditions.size()) {
            //update ratios, significance, etc., for job_ids updated since last short-circuit check:
            for (; guResultsNotShortCircuitedCount < guaShortCircuitCheckPoints[guShortCircuitCheckIdx] ; ++guResultsNotShortCircuitedCount) {
              results_type current_result(jobs[guResultsNotShortCircuitedCount].second);
              //update most likely clusters given latest simulated loglikelihood ratio
              grMLCs.UpdateTopClustersRank(current_result);
              //update significance indicator
              grSignificantRatios.AddRatio(current_result);
            }
            gbShortCircuitConditionExists = grContinuationPolicy.gbShortCircuitConditionExists = grMLCs.GetTopRankedCluster().GetPValue(gResultRegistrationConditions.size()) > guaShortCircuitCheckCutoffValues[guShortCircuitCheckIdx];
            if (!gbShortCircuitConditionExists) {
              ++guShortCircuitCheckIdx;
              gResultRegistrationConditions = result_registration_conditions;
              if (guShortCircuitCheckIdx < guShortCircuitCheckCount) {
                //assert(guaShortCircuitCheckPoints[guShortCircuitCheckIdx] < jobs.size());
                gResultRegistrationConditions.resize(guaShortCircuitCheckPoints[guShortCircuitCheckIdx]);
              }
              else {
                //update ratios, significance, etc., for job_ids updated since last short-circuit check:
                for (unsigned u = guResultsNotShortCircuitedCount; u < result_registration_conditions.size(); ++u) {
                  if (result_registration_conditions.test(u)) {//result registered for this job
                    results_type current_result(jobs[u].second);
                    //update most likely clusters given latest simulated loglikelihood ratio
                    grMLCs.UpdateTopClustersRank(current_result);
                    //update significance indicator
                    grSignificantRatios.AddRatio(current_result);
                  }
                }
              }
            }
          }
        }
      }
    }
    else {
      //update ratios, significance, etc.:
      results_type current_result(jobs[job_idx].second);
      //update most likely clusters given latest simulated loglikelihood ratio
      grMLCs.UpdateTopClustersRank(current_result);
      //update significance indicator
      grSignificantRatios.AddRatio(current_result);
    }
    //if appropriate, estimate time required to complete all jobs and report it.
    grPrintDirection.SatScanPrintf(gszReplicationFormatString, result_registration_conditions.count(), result_registration_conditions.size(), jobs[job_idx].second);
  }
  catch (ZdException & e)
  {
    e.AddCallpath("Report_ResultsRegistered", "stsMCSimReporter");
    throw;
  }
}

