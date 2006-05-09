//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "PoissonLikelihoodCalculation.h"
#include "PurelySpatialMonotoneCluster.h"
#include "SVTTCluster.h"

/** constructor */
PoissonLikelihoodCalculator::PoissonLikelihoodCalculator(const CSaTScanData& DataHub)
                            :AbstractLikelihoodCalculator(DataHub), gParameters(DataHub.GetParameters()) {
  //store data set loglikelihoods under null
  for (size_t t=0; t < DataHub.GetDataSetHandler().GetNumDataSets(); ++t) {
    count_t   N = DataHub.GetDataSetHandler().GetDataSet(t).GetTotalCases();
    measure_t U = DataHub.GetDataSetHandler().GetDataSet(t).GetTotalMeasure();
    gvDataSetLogLikelihoodUnderNull.push_back((N*log(N/U)));
  }
}

/** destructor */
PoissonLikelihoodCalculator::~PoissonLikelihoodCalculator() {}

/** calculates the Poisson log likelihood given the number of observed and expected cases
    - the total cases and expected cases used are that of first data set */
double PoissonLikelihoodCalculator::CalcLogLikelihood(count_t n, measure_t u) const {
  count_t   N = gvDataSetTotals[0].first;
  measure_t U = gvDataSetTotals[0].second;

   if (n != N && n != 0)
     return n*log(n/u) + (N-n)*log((N-n)/(U-u));
   else if (n == 0)
     return (N-n) * log((N-n)/(U-u));
   else
     return n*log(n/u);
}

/** calculates the Poisson log likelihood ratio given the number of observed, expected cases and data set index. */
double PoissonLikelihoodCalculator::CalcLogLikelihoodRatio(count_t n, measure_t u, size_t tSetIndex) const {
  count_t   N = gvDataSetTotals[tSetIndex].first;
  measure_t U = gvDataSetTotals[tSetIndex].second;
  double    dLogLikelihood;

  // calculate the loglikelihood
   if (n != N && n != 0)
     dLogLikelihood = n*log(n/u) + (N-n)*log((N-n)/(U-u));
   else if (n == 0)
     dLogLikelihood = (N-n) * log((N-n)/(U-u));
   else
     dLogLikelihood = n*log(n/u);

  // return the logliklihood ratio (loglikelihood - loglikelihood for total)
  return dLogLikelihood - (gvDataSetLogLikelihoodUnderNull[tSetIndex]);
}

/** needs documentation */
double PoissonLikelihoodCalculator::CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster) const {
  double nLogLikelihood=0;

  for (int i=0; i < PSMCluster.m_nSteps; i++) {
     if (PSMCluster.gvCasesList.at(i) != 0)
       nLogLikelihood += PSMCluster.gvCasesList.at(i) * log(PSMCluster.gvCasesList.at(i)/PSMCluster.gvMeasureList.at(i));
  }
  return nLogLikelihood;
}

/** needs documentation */
double PoissonLikelihoodCalculator::CalcSVTTLogLikelihood(count_t*   pCases, measure_t* pMeasure,
                                                          count_t pTotalCases, double nAlpha,
                                                          double nBeta, int nStatus) const {
  double nLL;

  /* Check for extremes where all the cases in the first or last interval
     or 0 cases in given tract.  For now there are multiple returns. */
  if (nStatus == CTimeTrend::TREND_UNDEF) // No cases in tract
  {
    nLL = 0;
  }
  else if (nStatus == CTimeTrend::TREND_INF) // All cases in first/last t.i.
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
double PoissonLikelihoodCalculator::CalcSVTTLogLikelihood(size_t tSetIndex, CSVTTCluster* Cluster, const CTimeTrend& GlobalTimeTrend) const {
  double nLogLikelihood   = 0.0;
  double nGlobalAlphaIn   = 0.0;
  double nGlobalAlphaOut = 0.0;

  SVTTClusterSetData& DataSet = Cluster->GetDataSet(tSetIndex);

  //calculate time trend inside of clusters tSetIndex'th dataset
  //TODO: The status of the time trend needs to be checked after CalculateAndSet() returns.
  //      The correct behavior for anything other than CTimeTrend::TREND_CONVERGED
  //      has not been decided yet.
  DataSet.gTimeTrendInside.CalculateAndSet(DataSet.gpCasesInsideCluster,         // Inside Cluster
                                           DataSet.gpMeasureInsideCluster,
                                           gDataHub.m_nTimeIntervals,
                                           gParameters.GetTimeTrendConvergence());

  nGlobalAlphaIn = DataSet.gTimeTrendInside.Alpha(DataSet.gtTotalCasesInsideCluster,
                                                  DataSet.gpMeasureInsideCluster,
                                                  gDataHub.m_nTimeIntervals,
                                                  GlobalTimeTrend.GetBeta());

  //calculate time trend outside of clusters tSetIndex'th dataset
  //TODO: The status of the time trend needs to be checked after CalculateAndSet() returns.
  //      The correct behavior for anything other than CTimeTrend::TREND_CONVERGED
  //      has not been decided yet.
  DataSet.gTimeTrendOutside.CalculateAndSet(DataSet.gpCasesOutsideCluster,         // Outside Cluster
                                            DataSet.gpMeasureOutsideCluster,
                                            gDataHub.m_nTimeIntervals,
                                            gParameters.GetTimeTrendConvergence());

  nGlobalAlphaOut = DataSet.gTimeTrendOutside.Alpha(DataSet.gtTotalCasesOutsideCluster,
                                                    DataSet.gpMeasureOutsideCluster,
                                                    gDataHub.m_nTimeIntervals,
                                                    GlobalTimeTrend.GetBeta());
  #if DEBUGMODEL
  fprintf(m_pDebugModelFile, "Inside                Outside\n");
  fprintf(m_pDebugModelFile, "Cases    Msr          Cases    Msr\n");
  fprintf(m_pDebugModelFile, "==========================================\n");
  for (int i=0; i<m_pData->m_nTimeIntervals; i++)
    fprintf (m_pDebugModelFile, "%i       %.2f        %i       %.2f\n",
                                DataSet.gpCasesInsideCluster[i], DataSet.gpMeasureInsideCluster[i],
                                DataSet.gpCasesOutsideCluster[i], DataSet.gpMeasureOutsideCluster[i]);
  fprintf(m_pDebugModelFile, "------------------------------------------\n");
  fprintf (m_pDebugModelFile, "%i                   %i\n\n",
                              DataSet.gtTotalCasesInsideCluster, DataSet.gtTotalCasesOutsideCluster);

//  fprintf(m_pDebugModelFile, "\nGlobal Time Trend: Alpha = %f, Beta = %f\n\n",
//          GlobalTimeTrend.GetAlpha(), GlobalTimeTrend.GetBeta());
  #endif

  nLogLikelihood = (CalcSVTTLogLikelihood(DataSet.gpCasesInsideCluster,
                                          DataSet.gpMeasureInsideCluster,
                                          DataSet.gtTotalCasesInsideCluster,
                                          DataSet.gTimeTrendInside.GetAlpha(),
                                          DataSet.gTimeTrendInside.GetBeta(),
                                          DataSet.gTimeTrendInside.GetStatus())
                    +
                    CalcSVTTLogLikelihood(DataSet.gpCasesOutsideCluster,
                                          DataSet.gpMeasureOutsideCluster,
                                          DataSet.gtTotalCasesOutsideCluster,
                                          DataSet.gTimeTrendOutside.GetAlpha(),
                                          DataSet.gTimeTrendOutside.GetBeta(),
                                          DataSet.gTimeTrendOutside.GetStatus()))
                    -
                   (CalcSVTTLogLikelihood(DataSet.gpCasesInsideCluster,
                                          DataSet.gpMeasureInsideCluster,
                                          DataSet.gtTotalCasesInsideCluster,
                                          nGlobalAlphaIn,
                                          GlobalTimeTrend.GetBeta(),
                                          DataSet.gTimeTrendInside.GetStatus())
                    +
                    CalcSVTTLogLikelihood(DataSet.gpCasesOutsideCluster,
                                          DataSet.gpMeasureOutsideCluster,
                                          DataSet.gtTotalCasesOutsideCluster,
                                          nGlobalAlphaOut,
                                          GlobalTimeTrend.GetBeta(),
                                          DataSet.gTimeTrendOutside.GetStatus()));


  #if DEBUGMODEL
  fprintf(m_pDebugModelFile, "\nTotal LogLikelihood = %f\n\n\n", nLogLikelihood);
  #endif

  return nLogLikelihood;
}

/** Calculates the full loglikelihood ratio/test statistic given passed maximizing value and
    data set index. For the Poisson calculator, the maximizing value would be the loglikelihood in
    a particular clustering. If maximizing value equals negative double max value, zero is returned
    as this indicates that no significant maximizing value was calculated. */
double PoissonLikelihoodCalculator::CalculateFullStatistic(double dMaximizingValue, size_t tSetIndex) const {
  if (dMaximizingValue == -std::numeric_limits<double>::max()) return 0.0;

  return dMaximizingValue - (gvDataSetLogLikelihoodUnderNull[tSetIndex]);
}

/** Calculates the maximizing value given observed cases, expected cases and data set index.
    For the Poisson calculator, the maximizing value is the loglikelihood. */
double PoissonLikelihoodCalculator::CalculateMaximizingValue(count_t n, measure_t u, size_t tDataSetIndex) const {
  count_t   N = gvDataSetTotals[tDataSetIndex].first;
  measure_t U = gvDataSetTotals[tDataSetIndex].second;

  if (n != N && n != 0)
    return n*log(n/u) + (N-n)*log((N-n)/(U-u));
  else if (n == 0)
    return (N-n) * log((N-n)/(U-u));
  else
    return n*log(n/u);
}

/** returns log likelihood for total data set at index */
double PoissonLikelihoodCalculator::GetLogLikelihoodForTotal(size_t tSetIndex) const {
  return gvDataSetLogLikelihoodUnderNull[tSetIndex];
}

