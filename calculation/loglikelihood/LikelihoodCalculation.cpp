//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "LikelihoodCalculation.h"

/** constructor */
AbstractLikelihoodCalculator::AbstractLikelihoodCalculator(count_t tTotalCases, measure_t tTotalMeasure)
                             :gtTotalCasesInDataSet(tTotalCases), gtTotalMeasureInDataSet(tTotalMeasure) {
  Init();
}

/** destructor */
AbstractLikelihoodCalculator::~AbstractLikelihoodCalculator() {}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalcLogLikelihoodRatioEx(count_t tCases,
                                                              measure_t tMeasure,
                                                              measure_t tMeasure2,
                                                              count_t tTotalCases,
                                                              measure_t tTotalMeasure) {
  ZdGenerateException("CalcLogLikelihoodRatioEx() not implementated.","AbstractLikelihoodCalculator");
  return 0;
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster) {
  ZdGenerateException("CalcMonotoneLogLikelihood() not implementated.","AbstractLikelihoodCalculator");
  return 0;
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalcSVTTLogLikelihoodRatio(size_t tStream, CSVTTCluster* Cluster, CTimeTrend& GlobalTimeTrend) {
  ZdGenerateException("CalcSVTTLogLikelihoodRatio() not implementated.","AbstractLikelihoodCalculator");
  return 0;
}

