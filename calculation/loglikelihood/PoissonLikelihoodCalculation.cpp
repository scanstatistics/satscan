//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "PoissonLikelihoodCalculation.h"
#include "PurelySpatialMonotoneCluster.h"
#include "SVTTCluster.h"

/** constructor */
PoissonLikelihoodCalculator::PoissonLikelihoodCalculator(const CSaTScanData& DataHub)
                            :AbstractLikelihoodCalculator(DataHub), gParameters(DataHub.GetParameters()) {}

/** destructor */
PoissonLikelihoodCalculator::~PoissonLikelihoodCalculator() {}

/** calculates the Poisson log likelihood given the number of observed and expected cases
    - the total cases and expected cases used are that of first data set */
double PoissonLikelihoodCalculator::CalcLogLikelihood(count_t n, measure_t u) const {
   count_t   N = gtTotalCasesInFirstDataSet;
   measure_t U = gtTotalMeasureInFirstDataSet;

   if (n != N && n != 0)
     return n*log(n/u) + (N-n)*log((N-n)/(U-u));
   else if (n == 0)
     return (N-n) * log((N-n)/(U-u));
   else
     return n*log(n/u);
}

/** calculates the Poisson log likelihood ratio given the number of observed and expected cases */
double PoissonLikelihoodCalculator::CalcLogLikelihoodRatio(count_t n, measure_t u, count_t N, measure_t U) const {
  double    dLogLikelihood;

  // calculate the loglikelihood
   if (n != N && n != 0)
     dLogLikelihood = n*log(n/u) + (N-n)*log((N-n)/(U-u));
   else if (n == 0)
     dLogLikelihood = (N-n) * log((N-n)/(U-u));
   else
     dLogLikelihood = n*log(n/u);

  // return the logliklihood ratio (loglikelihood - loglikelihood for total)
  return dLogLikelihood - (N*log(N/U));
}

/** needs documentation */
double PoissonLikelihoodCalculator::CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster) const {
  double nLogLikelihood=0;

  for (int i=0; i < PSMCluster.m_nSteps; i++) {
     if (PSMCluster.m_pCasesList[i] != 0)
       nLogLikelihood += PSMCluster.m_pCasesList[i] * log(PSMCluster.m_pCasesList[i]/PSMCluster.m_pMeasureList[i]);
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

/** returns log likelihood for total
    - the total cases and expected cases used are that of first data set */
double PoissonLikelihoodCalculator::GetLogLikelihoodForTotal() const {
  count_t   N = gtTotalCasesInFirstDataSet;
  measure_t U = gtTotalMeasureInFirstDataSet;

  return N*log(N/U);
}

/** Returns log likelihood ratio given passed log likelihood.  */
double PoissonLikelihoodCalculator::GetLogLikelihoodRatio(double dLogLikelihood) const {
  count_t   N = gtTotalCasesInFirstDataSet;
  measure_t U = gtTotalMeasureInFirstDataSet;

  return dLogLikelihood - (N*log(N/U));
}

