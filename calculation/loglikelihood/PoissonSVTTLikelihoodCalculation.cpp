//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "PoissonSVTTLikelihoodCalculation.h"
#include "SVTTCluster.h"

bool IncreasingOrDecreasingTrend(const CTimeTrend& InsideTrend, const CTimeTrend& GlobalTrend) {
  if (InsideTrend.GetStatus() == CTimeTrend::UNDEFINED) return false;
  return InsideTrend.GetBeta() != GlobalTrend.GetBeta();
}

bool IncreasingTrend(const CTimeTrend& InsideTrend, const CTimeTrend& GlobalTrend) {
  switch (InsideTrend.GetStatus()) {
    case CTimeTrend::UNDEFINED         : return false;
    case CTimeTrend::NEGATIVE_INFINITY : return false;
    case CTimeTrend::POSITIVE_INFINITY : return true;
    default : if (InsideTrend.GetBeta() < 0) return false;
  };
  return InsideTrend.GetBeta() > GlobalTrend.GetBeta();
}

bool DecreasingTrend(const CTimeTrend& InsideTrend, const CTimeTrend& GlobalTrend) {
  switch (InsideTrend.GetStatus()) {
    case CTimeTrend::UNDEFINED         : return false;
    case CTimeTrend::NEGATIVE_INFINITY : return true;
    case CTimeTrend::POSITIVE_INFINITY : return false;
    default : if (InsideTrend.GetBeta() >= 0) return false;
  };
  return InsideTrend.GetBeta() < GlobalTrend.GetBeta();
}

/** constructor */
PoissonSVTTLikelihoodCalculator::PoissonSVTTLikelihoodCalculator(const CSaTScanData& DataHub)
                            :AbstractLikelihoodCalculator(DataHub), gParameters(DataHub.GetParameters()) {
  //store data set loglikelihoods under null
  for (size_t t=0; t < DataHub.GetDataSetHandler().GetNumDataSets(); ++t) {
    count_t   N = DataHub.GetDataSetHandler().GetDataSet(t).getTotalCases();
    measure_t U = DataHub.GetDataSetHandler().GetDataSet(t).getTotalMeasure();
    gvDataSetLogLikelihoodUnderNull.push_back((N*log(N/U)));
  }
  switch (gParameters.GetExecuteScanRateType()) {
    case LOW  : gpTrendOfInterest = DecreasingTrend; break;
    case HIGH : gpTrendOfInterest = IncreasingTrend; break;
    default   : gpTrendOfInterest = IncreasingOrDecreasingTrend;
  }
}

/** destructor */
PoissonSVTTLikelihoodCalculator::~PoissonSVTTLikelihoodCalculator() {}

/** calculates the Poisson log likelihood given the number of observed and expected cases
    - the total cases and expected cases used are that of first data set */
double PoissonSVTTLikelihoodCalculator::CalcLogLikelihoodSpatialOnly(count_t n, measure_t u) const {
  count_t   N = gvDataSetTotals[0].first;
  measure_t U = gvDataSetTotals[0].second;

   if (n != N && n != 0)
     return n*log(n/u) + (N-n)*log((N-n)/(U-u));
   else if (n == 0)
     return (N-n) * log((N-n)/(U-u));
   else
     return n*log(n/u);
}

/** needs documentation */
double PoissonSVTTLikelihoodCalculator::CalcSVTTLogLikelihood(count_t*   pCases, measure_t* pMeasure,
                                                          count_t pTotalCases, double nAlpha,
                                                          double nBeta, int nStatus) const {
  double nLL;

  /* Check for extremes where all the cases in the first or last interval
     or 0 cases in given tract.  For now there are multiple returns. */
  if (nStatus == CTimeTrend::UNDEFINED) {// No cases in tract
    nLL = 0;
  }
  else if (nStatus == CTimeTrend::NEGATIVE_INFINITY) {// All cases in first t.i.
    nLL = CalcLogLikelihoodSpatialOnly(pCases[0], pMeasure[0]);
  }
  else if (nStatus == CTimeTrend::POSITIVE_INFINITY) {// All cases in last t.i.
    nLL = CalcLogLikelihoodSpatialOnly(pCases[gDataHub.m_nTimeIntervals-1], pMeasure[gDataHub.m_nTimeIntervals-1]);
  }
  else {
    double nSum1 = 0;
    double nSum2 = 0;
    double nSum3 = 0;
    int i;

    for (i=0; i < (gDataHub.m_nTimeIntervals); i++)
    {
      nSum1 += pCases[i] * (log(pMeasure[i]) + nAlpha + (nBeta)*i);
      nSum2 += pMeasure[i] * exp(nAlpha + (nBeta)*i);
      /**nSum3 += ((CSVTTData&)gData).m_pSumLogs[pCases[i]]; // Modified 990916 GG**/
    }
    nLL = nSum1-nSum2-nSum3;
  }

  #if DEBUGMODEL
  fprintf(m_pDebugModelFile, "Alpha = %f  nBeta = %f  nStatus = %i  LogLikelihood = %f\n", nAlpha, nBeta, nStatus, nLL);
  #endif

  return(nLL);
}

/** needs documentation */
double PoissonSVTTLikelihoodCalculator::CalcSVTTLogLikelihood(size_t tSetIndex, SVTTClusterData& ClusterData, const CTimeTrend& GlobalTimeTrend) const {
  double nLogLikelihood(0.0), nGlobalAlphaIn(0.0), nGlobalAlphaOut(0.0), nLogLikelihoodInside(0.0), nLogLikelihoodOutside(0.0);

  //log likelihood can only be calculated when the number of cases inside and outside the cluster are less than 2.
  if (GlobalTimeTrend.GetStatus() == CTimeTrend::UNDEFINED || ClusterData.gtTotalCasesInsideCluster <= 1 || ClusterData.gtTotalCasesOutsideCluster <= 1)
    return 0.0;

  //calculate time trend inside cluster
  ClusterData.gTimeTrendInside.CalculateAndSet(ClusterData.gpCasesInsideCluster, ClusterData.gpMeasureInsideCluster,
                                               gDataHub.m_nTimeIntervals, gDataHub.GetParameters().GetTimeTrendConvergence());
                                               
  if (ClusterData.gTimeTrendInside.GetStatus() == CTimeTrend::NOT_CONVERGED)
    throw prg_error("The time trend inside of cluster did not converge.\n", "CalcSVTTLogLikelihood()");

  //just return if time trend is not the trend we are looking for
  if (!gpTrendOfInterest(ClusterData.gTimeTrendInside, GlobalTimeTrend))
    return 0.0;

  //calculate time trend outside cluster
  ClusterData.gTimeTrendOutside.CalculateAndSet(ClusterData.gpCasesOutsideCluster, ClusterData.gpMeasureOutsideCluster,
                                                gDataHub.m_nTimeIntervals, gDataHub.GetParameters().GetTimeTrendConvergence());
  if (ClusterData.gTimeTrendOutside.GetStatus() == CTimeTrend::UNDEFINED)
    return 0.0;
  if (ClusterData.gTimeTrendOutside.GetStatus() == CTimeTrend::NOT_CONVERGED)
    throw prg_error("The time trend outside of cluster did not converge.\n", "CalcSVTTLogLikelihood()");

  if (ClusterData.gTimeTrendInside.GetStatus() == CTimeTrend::NEGATIVE_INFINITY || ClusterData.gTimeTrendInside.GetStatus() == CTimeTrend::POSITIVE_INFINITY) {
    nLogLikelihoodInside = CalcLogLikelihoodSpatialOnly(ClusterData.gtTotalCasesInsideCluster, ClusterData.gtTotalMeasureInsideCluster);
  }
  else {
    nLogLikelihoodInside = CalcSVTTLogLikelihood(ClusterData.gpCasesInsideCluster, ClusterData.gpMeasureInsideCluster,
                                                 ClusterData.gtTotalCasesInsideCluster, ClusterData.gTimeTrendInside.GetAlpha(),
                                                 ClusterData.gTimeTrendInside.GetBeta(), ClusterData.gTimeTrendInside.GetStatus());

    nGlobalAlphaIn = ClusterData.gTimeTrendInside.Alpha(ClusterData.gtTotalCasesInsideCluster, ClusterData.gpMeasureInsideCluster,
                                                        gDataHub.m_nTimeIntervals, GlobalTimeTrend.GetBeta());
    nLogLikelihoodInside -= CalcSVTTLogLikelihood(ClusterData.gpCasesInsideCluster, ClusterData.gpMeasureInsideCluster,
                                                  ClusterData.gtTotalCasesInsideCluster, nGlobalAlphaIn,
                                                  GlobalTimeTrend.GetBeta(), ClusterData.gTimeTrendInside.GetStatus());
  }


  if (ClusterData.gTimeTrendOutside.GetStatus() == CTimeTrend::NEGATIVE_INFINITY || ClusterData.gTimeTrendOutside.GetStatus() == CTimeTrend::POSITIVE_INFINITY) {
    nLogLikelihoodInside = CalcLogLikelihoodSpatialOnly(ClusterData.gtTotalCasesOutsideCluster, ClusterData.gtTotalMeasureOutsideCluster);
  }
  else {
    nLogLikelihoodOutside = CalcSVTTLogLikelihood(ClusterData.gpCasesOutsideCluster, ClusterData.gpMeasureOutsideCluster,
                                                  ClusterData.gtTotalCasesOutsideCluster, ClusterData.gTimeTrendOutside.GetAlpha(),
                                                  ClusterData.gTimeTrendOutside.GetBeta(), ClusterData.gTimeTrendOutside.GetStatus());

    nGlobalAlphaOut = ClusterData.gTimeTrendOutside.Alpha(ClusterData.gtTotalCasesOutsideCluster, ClusterData.gpMeasureOutsideCluster,
                                                          gDataHub.m_nTimeIntervals, GlobalTimeTrend.GetBeta());
    nLogLikelihoodOutside -= CalcSVTTLogLikelihood(ClusterData.gpCasesOutsideCluster, ClusterData.gpMeasureOutsideCluster,
                                                   ClusterData.gtTotalCasesOutsideCluster, nGlobalAlphaOut,
                                                   GlobalTimeTrend.GetBeta(), ClusterData.gTimeTrendOutside.GetStatus());
  }
  nLogLikelihood = nLogLikelihoodInside + nLogLikelihoodOutside;
  return nLogLikelihood;
}

///** Calculates the full loglikelihood ratio/test statistic given passed maximizing value and
//    data set index. For the Poisson calculator, the maximizing value would be the loglikelihood in
//    a particular clustering. If maximizing value equals negative double max value, zero is returned
//    as this indicates that no significant maximizing value was calculated. */
//double PoissonSVTTLikelihoodCalculator::CalculateFullStatistic(double dMaximizingValue, size_t tSetIndex) const {
//  if (dMaximizingValue == -std::numeric_limits<double>::max()) return 0.0;
//
// return dMaximizingValue - (gvDataSetLogLikelihoodUnderNull[tSetIndex]);
//}

///** Calculates the maximizing value given observed cases, expected cases and data set index.
//    For the Poisson calculator, the maximizing value is the loglikelihood. */
//double PoissonSVTTLikelihoodCalculator::CalculateMaximizingValue(count_t n, measure_t u, size_t tDataSetIndex) const {
//  count_t   N = gvDataSetTotals[tDataSetIndex].first;
// measure_t U = gvDataSetTotals[tDataSetIndex].second;
//
//  if (n != N && n != 0)
//    return n*log(n/u) + (N-n)*log((N-n)/(U-u));
//  else if (n == 0)
//    return (N-n) * log((N-n)/(U-u));
//  else
//    return n*log(n/u);
//}

/** returns log likelihood for total data set at index */
double PoissonSVTTLikelihoodCalculator::GetLogLikelihoodForTotal(size_t tSetIndex) const {
  return gvDataSetLogLikelihoodUnderNull[tSetIndex];
}
 
