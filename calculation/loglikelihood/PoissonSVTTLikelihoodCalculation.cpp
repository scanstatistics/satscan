//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "PoissonSVTTLikelihoodCalculation.h"
#include "SVTTCluster.h"

bool IncreasingOrDecreasingTrend(const CTimeTrend& InsideTrend, const CTimeTrend& GlobalTrend) {
  if (InsideTrend.GetStatus() != CTimeTrend::TREND_CONVERGED)
    return false;
  return InsideTrend.GetBeta() != GlobalTrend.GetBeta();
}

bool IncreasingTrend(const CTimeTrend& InsideTrend, const CTimeTrend& GlobalTrend) {
  if (InsideTrend.GetStatus() != CTimeTrend::TREND_CONVERGED)
    return false;

  if (InsideTrend.IsNegative())
    return false;
  return InsideTrend.GetBeta() > GlobalTrend.GetBeta();
}

bool DecreasingTrend(const CTimeTrend& InsideTrend, const CTimeTrend& GlobalTrend) {
  if (InsideTrend.GetStatus() != CTimeTrend::TREND_CONVERGED)
    return false;

  if (!InsideTrend.IsNegative())
    return false;
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

/** needs documentation */
double PoissonSVTTLikelihoodCalculator::CalcSVTTLogLikelihood(count_t*   pCases, measure_t* pMeasure,
                                                          count_t pTotalCases, double nAlpha,
                                                          double nBeta, int nStatus) const {
  double nLL;

  /* Check for extremes where all the cases in the first or last interval
     or 0 cases in given tract.  For now there are multiple returns. */
  if (nStatus == CTimeTrend::TREND_UNDEF) // No cases in tract
  {
    nLL = 0;
  }
  else if (nStatus == CTimeTrend::TREND_INF_BEGIN || nStatus == CTimeTrend::TREND_INF_END) // All cases in first/last t.i.
  {
    nLL = (pTotalCases*log(pTotalCases) - pTotalCases )/**-
          ((CSVTTData&)gData).m_pSumLogs[pTotalCases])**/;
    // -x+x*log(x)-log(x!) from M Kulldorff
  }
  else
  {
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
  double nLogLikelihood   = 0.0;
  double nGlobalAlphaIn   = 0.0;
  double nGlobalAlphaOut = 0.0;

  //log likelihood can only be calculated when the number of cases inside and outside the cluster are less than 2.
  if (ClusterData.gtTotalCasesInsideCluster <= 1 || ClusterData.gtTotalCasesOutsideCluster <= 1)
    return 0.0;

  //calculate time trend inside cluster
  ClusterData.gTimeTrendInside.CalculateAndSet(ClusterData.gpCasesInsideCluster, ClusterData.gpMeasureInsideCluster,
                                               gDataHub.m_nTimeIntervals, gDataHub.GetParameters().GetTimeTrendConvergence());

  //just return if time trend is not the trend we are looking for
  if (!gpTrendOfInterest(ClusterData.gTimeTrendInside, GlobalTimeTrend))
    return 0.0;

  //calculate time trend outside cluster
  ClusterData.gTimeTrendOutside.CalculateAndSet(ClusterData.gpCasesOutsideCluster, ClusterData.gpMeasureOutsideCluster,
                                                gDataHub.m_nTimeIntervals, gDataHub.GetParameters().GetTimeTrendConvergence());

  nGlobalAlphaIn = ClusterData.gTimeTrendInside.Alpha(ClusterData.gtTotalCasesInsideCluster,
                                                  ClusterData.gpMeasureInsideCluster,
                                                  gDataHub.m_nTimeIntervals,
                                                  GlobalTimeTrend.GetBeta());

  nGlobalAlphaOut = ClusterData.gTimeTrendOutside.Alpha(ClusterData.gtTotalCasesOutsideCluster,
                                                    ClusterData.gpMeasureOutsideCluster,
                                                    gDataHub.m_nTimeIntervals,
                                                    GlobalTimeTrend.GetBeta());

  nLogLikelihood = (CalcSVTTLogLikelihood(ClusterData.gpCasesInsideCluster,
                                          ClusterData.gpMeasureInsideCluster,
                                          ClusterData.gtTotalCasesInsideCluster,
                                          ClusterData.gTimeTrendInside.GetAlpha(),
                                          ClusterData.gTimeTrendInside.GetBeta(),
                                          ClusterData.gTimeTrendInside.GetStatus())
                    +
                    CalcSVTTLogLikelihood(ClusterData.gpCasesOutsideCluster,
                                          ClusterData.gpMeasureOutsideCluster,
                                          ClusterData.gtTotalCasesOutsideCluster,
                                          ClusterData.gTimeTrendOutside.GetAlpha(),
                                          ClusterData.gTimeTrendOutside.GetBeta(),
                                          ClusterData.gTimeTrendOutside.GetStatus()))
                    -
                   (CalcSVTTLogLikelihood(ClusterData.gpCasesInsideCluster,
                                          ClusterData.gpMeasureInsideCluster,
                                          ClusterData.gtTotalCasesInsideCluster,
                                          nGlobalAlphaIn,
                                          GlobalTimeTrend.GetBeta(),
                                          ClusterData.gTimeTrendInside.GetStatus())
                    +
                    CalcSVTTLogLikelihood(ClusterData.gpCasesOutsideCluster,
                                          ClusterData.gpMeasureOutsideCluster,
                                          ClusterData.gtTotalCasesOutsideCluster,
                                          nGlobalAlphaOut,
                                          GlobalTimeTrend.GetBeta(),
                                          ClusterData.gTimeTrendOutside.GetStatus()));

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
 
