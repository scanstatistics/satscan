//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "NormalModel.h"

/** constructor */
CNormalModel::CNormalModel(CParameters & Parameters, CSaTScanData & Data, BasePrint & PrintDirection)
             :CModel(Parameters, Data, PrintDirection) {}

/** destructor */
CNormalModel::~CNormalModel() {}

double CNormalModel::CalcLogLikelihood(count_t n, measure_t u) {
  ZdGenerateException("CalcLogLikelihood() not implementated.","CNormalModel");
  return 0;
}

/** Throws exception. Defined in parent class as pure virtual. */
double CNormalModel::CalcLogLikelihoodRatio(count_t tCases, measure_t tMeasure, count_t tTotalCases, measure_t tTotalMeasure) {
  //ZdGenerateException("CalcLogLikelihoodRatio() not implementated.","CNormalModel");
  return 4.9;
}

/** Calculates loglikelihood  ratio for normal data */
double CNormalModel::CalcLogLikelihoodRatioEx(count_t tCases, measure_t tMeasure, measure_t tMeasure2, count_t tTotalCases, measure_t tTotalMeasure) {
//  double        dLogLikelihoodRatio=0;
//
//  // Lan - fill in code here -
//
// return dLogLikelihoodRatio;



  double    dLogLikelihood;

  // calculate the loglikelihood
  if (tCases != tTotalCases && tCases != 0 && tTotalMeasure-tMeasure)
    dLogLikelihood = tCases *log(tCases/tMeasure) + (tTotalCases-tCases)*log((tTotalCases-tCases)/(tTotalMeasure-tMeasure));
  else if (tCases == 0 && tTotalMeasure-tMeasure)
    dLogLikelihood = (tTotalCases-tCases) * log((tTotalCases-tCases)/(tTotalMeasure-tMeasure));
  else
    dLogLikelihood = tCases*log(tCases/tMeasure);

  // return the logliklihood ratio (loglikelihood - loglikelihood for total)
  return (dLogLikelihood - (tTotalCases * log(tTotalCases/tTotalMeasure)));
}

/** Throws exception. Defined in parent class as pure virtual. */
double CNormalModel::CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster) {
  ZdGenerateException("CalcMonotoneLogLikelihood() not implementated.","CNormalModel");
  return 0;
}

/** NEEDS DOCUMENTATION */
double CNormalModel::CalcSVTTLogLikelihoodRatio(size_t tStream, CSVTTCluster* Cluster, CTimeTrend GlobalTimeTrend) {
  ZdGenerateException("CalcSVTTLogLikelihoodRatio() not implementated.","CNormalModel");
  return 0;
}

double CNormalModel::GetLogLikelihoodForTotal() const {
  // Lan - fill in code here -

  return 0;
}

double CNormalModel::GetPopulation(int m_iEllipseOffset, tract_t nCenter, tract_t nTracts, int nStartInterval, int nStopInterval) {
  ZdGenerateException("GetPopulation() not implementated.","CNormalModel");
  return 0;
}

