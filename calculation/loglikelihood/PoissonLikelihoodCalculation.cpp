//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "PoissonLikelihoodCalculation.h"
#include "PurelySpatialMonotoneCluster.h"
#include "SVTTCluster.h"

/** constructor */
PoissonLikelihoodCalculator::PoissonLikelihoodCalculator(CSaTScanData & Data)
                            :AbstractLikelihoodCalculator(Data.GetTotalCases(), Data.GetTotalMeasure()),
                             gData(Data), gParameters(Data.GetParameters()) {}

/** destructor */
PoissonLikelihoodCalculator::~PoissonLikelihoodCalculator() {}

/** calculates the Poisson log likelihood given the number of observed and expected cases */
double PoissonLikelihoodCalculator::CalcLogLikelihood(count_t n, measure_t u) {
   count_t   N = gtTotalCasesInDataSet;
   measure_t U = gtTotalMeasureInDataSet;

   if (n != N && n != 0)
     return n*log(n/u) + (N-n)*log((N-n)/(U-u));
   else if (n == 0)
     return (N-n) * log((N-n)/(U-u));
   else
     return n*log(n/u);
}

/** calculates the Poisson log likelihood ratio given the number of observed and expected cases */
double PoissonLikelihoodCalculator::CalcLogLikelihoodRatio(count_t n, measure_t u, count_t N, measure_t U) {
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
double PoissonLikelihoodCalculator::CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster) {
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
                                                          double nBeta, int nStatus) {
  double nLL;

  /* Check for extremes where all the cases in the first or last interval
     or 0 cases in given tract.  For now there are multiple returns. */
  if (nStatus == TREND_UNDEF) // No cases in tract
  {
    nLL = 0;
  }
  else if (nStatus == TREND_INF) // All cases in first/last t.i.
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

    for (i=0; i<(gData.m_nTimeIntervals); i++)
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
double PoissonLikelihoodCalculator::CalcSVTTLogLikelihoodRatio(size_t tStream, CSVTTCluster* Cluster, CTimeTrend& GlobalTimeTrend) {
  double nLogLikelihood   = 0.0;
  double nGlobalAlphaIn   = 0.0;
  double nGlobalAlphaOut = 0.0;

  SVTTClusterStreamData & StreamData = Cluster->GetStream(tStream);

  StreamData.gTimeTrendInside.CalculateAndSet(StreamData.gpCasesInsideCluster,         // Inside Cluster
                                              StreamData.gpMeasureInsideCluster,
                                              gData.m_nTimeIntervals,
                                              gParameters.GetTimeTrendConvergence());

  nGlobalAlphaIn = StreamData.gTimeTrendInside.Alpha(StreamData.gtTotalCasesInsideCluster,
                                                     StreamData.gpMeasureInsideCluster,
                                                     gData.m_nTimeIntervals,
                                                     GlobalTimeTrend.m_nBeta);

  StreamData.gTimeTrendOutside.CalculateAndSet(StreamData.gpCasesOutsideCluster,         // Outside Cluster
                                               StreamData.gpMeasureOutsideCluster,
                                               gData.m_nTimeIntervals,
                                               gParameters.GetTimeTrendConvergence());

  nGlobalAlphaOut = StreamData.gTimeTrendOutside.Alpha(StreamData.gtTotalCasesOutsideCluster,
                                                       StreamData.gpMeasureOutsideCluster,
                                                       gData.m_nTimeIntervals,
                                                       GlobalTimeTrend.m_nBeta);
  #if DEBUGMODEL
  fprintf(m_pDebugModelFile, "Inside                Outside\n");
  fprintf(m_pDebugModelFile, "Cases    Msr          Cases    Msr\n");
  fprintf(m_pDebugModelFile, "==========================================\n");
  for (int i=0; i<m_pData->m_nTimeIntervals; i++)
    fprintf (m_pDebugModelFile, "%i       %.2f        %i       %.2f\n",
                                Cluster->m_pCumCases[i], Cluster->m_pCumMeasure[i],
                                Cluster->m_pRemCases[i], Cluster->m_pRemMeasure[i]);
  fprintf(m_pDebugModelFile, "------------------------------------------\n");
  fprintf (m_pDebugModelFile, "%i                   %i\n\n",
                              Cluster->GetCaseCount(0), Cluster->m_nRemCases);

//  fprintf(m_pDebugModelFile, "\nGlobal Time Trend: Alpha = %f, Beta = %f\n\n",
//          GlobalTimeTrend.m_nAlpha, GlobalTimeTrend.m_nBeta);
  #endif

  nLogLikelihood = (CalcSVTTLogLikelihood(StreamData.gpCasesInsideCluster,
                                          StreamData.gpMeasureInsideCluster,
                                          StreamData.gtTotalCasesInsideCluster,
                                          StreamData.gTimeTrendInside.m_nAlpha,
                                          StreamData.gTimeTrendInside.m_nBeta,
                                          StreamData.gTimeTrendInside.m_nStatus)
                    +
                    CalcSVTTLogLikelihood(StreamData.gpCasesOutsideCluster,
                                          StreamData.gpMeasureOutsideCluster,
                                          StreamData.gtTotalCasesOutsideCluster,
                                          StreamData.gTimeTrendOutside.m_nAlpha,
                                          StreamData.gTimeTrendOutside.m_nBeta,
                                          StreamData.gTimeTrendOutside.m_nStatus))
                    -
                   (CalcSVTTLogLikelihood(StreamData.gpCasesInsideCluster,
                                          StreamData.gpMeasureInsideCluster,
                                          StreamData.gtTotalCasesInsideCluster,
                                          nGlobalAlphaIn,
                                          GlobalTimeTrend.m_nBeta,
                                          StreamData.gTimeTrendInside.m_nStatus)
                    +
                    CalcSVTTLogLikelihood(StreamData.gpCasesOutsideCluster,
                                          StreamData.gpMeasureOutsideCluster,
                                          StreamData.gtTotalCasesOutsideCluster,
                                          nGlobalAlphaOut,
                                          GlobalTimeTrend.m_nBeta,
                                          StreamData.gTimeTrendOutside.m_nStatus));


  #if DEBUGMODEL
  fprintf(m_pDebugModelFile, "\nTotal LogLikelihood = %f\n\n\n", nLogLikelihood);
  #endif

  return nLogLikelihood;
}

/** returns log likelihood for total */
double PoissonLikelihoodCalculator::GetLogLikelihoodForTotal() const {
  count_t   N = gtTotalCasesInDataSet;
  measure_t U = gtTotalMeasureInDataSet;

  return N*log(N/U);
}
 
