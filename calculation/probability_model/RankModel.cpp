//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "RankModel.h"
#include "SaTScanData.h"

/** constructor */
CRankModel::CRankModel(CParameters & Parameters, CSaTScanData & Data, BasePrint & PrintDirection)
           :CModel(Parameters, Data, PrintDirection) {}

/** destructor */
CRankModel::~CRankModel() {}

/** not implemented - throws exception */
double CRankModel::CalcLogLikelihood(count_t n, measure_t u) {
  //ZdGenerateException("CalcLogLikelihood() not implementated.","CRankModel");
  //return 0;

   gPrintDirection.PrintLine("CalcLogLikelihood() of Rank model called using Poisson for testing ...\n");

   double    nLogLikelihood;
   count_t   N = gData.GetTotalCases();
   measure_t U = gData.GetTotalMeasure();

   if (n != N && n != 0)
     nLogLikelihood = n*log(n/u) + (N-n)*log((N-n)/(U-u));
   else if (n == 0)
     nLogLikelihood = (N-n) * log((N-n)/(U-u));
   else
     nLogLikelihood = n*log(n/u);

   return (nLogLikelihood);
}

/** calculates loglikelihood ratio */
double CRankModel::CalcLogLikelihoodRatio(count_t tCases, measure_t tMeasure, count_t tTotalCases, measure_t tTotalMeasure) {
  //ZdGenerateException("Hi from CalcLogLikelihoodRatio(). I need to be implementated.","CRankModel");
  //return 0;

  //#! taken from Poisson for testing !#
  double    dLogLikelihood;

  gPrintDirection.PrintLine("CalcLogLikelihoodRatio() of Rank model called using Poisson for testing ...\n");

  // calculate the loglikelihood
  if (tCases != tTotalCases && tCases != 0)
    dLogLikelihood = tCases *log(tCases/tMeasure) + (tTotalCases-tCases)*log((tTotalCases-tCases)/(tTotalMeasure-tMeasure));
  else if (tCases == 0)
    dLogLikelihood = (tTotalCases-tCases) * log((tTotalCases-tCases)/(tTotalMeasure-tMeasure));
  else
    dLogLikelihood = tCases*log(tCases/tMeasure);

  // return the logliklihood ratio (loglikelihood - loglikelihood for total)
  return (dLogLikelihood - (tTotalCases * log(tTotalCases/tTotalMeasure)));
}

/** not implemented - throws exception */
double CRankModel::CalcLogLikelihoodRatioEx(count_t tCases, measure_t tMeasure, measure_t tMeasure2, count_t tTotalCases, measure_t tTotalMeasure) {
  ZdGenerateException("CalcLogLikelihoodRatioEx() not implementated.","CRankModel");
  return 0;
}

/** not implemented - throws exception */
double CRankModel::CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster) {
  ZdGenerateException("CalcMonotoneLogLikelihood() not implementated.","CRankModel");
  return 0;
}

/** not implemented - throws exception */
double CRankModel::CalcSVTTLogLikelihoodRatio(size_t tStream, CSVTTCluster* Cluster, CTimeTrend GlobalTimeTrend) {
  ZdGenerateException("CalcSVTTLogLikelihoodRatio() not implementated.","CRankModel");
  return 0;
}

/** returns loglikelihood for total data set */
double CRankModel::GetLogLikelihoodForTotal() const {
  //ZdGenerateException("Hi from GetLogLikelihoodForTotal(). I need to be implementated.","CRankModel");
  //return 0;

  gPrintDirection.PrintLine("GetLogLikelihoodForTotal() of Rank model called using Poisson for testing ...\n");
  //#! taken from Poisson for testing !#
  count_t   N = gData.GetTotalCases();
  measure_t U = gData.GetTotalMeasure();

  return N*log(N/U);
}

/** not implemented - throws exception */
double CRankModel::GetPopulation(int m_iEllipseOffset, tract_t nCenter, tract_t nTracts, int nStartInterval, int nStopInterval) {
  //ZdGenerateException("GetPopulation() not implementated.","CRankModel");
  return 0;
}
 
