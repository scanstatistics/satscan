//*****************************************************************************
#ifndef __PoissonLikelihoodCalculation_H
#define __PoissonLikelihoodCalculation_H
//*****************************************************************************
#include "LikelihoodCalculation.h"
#include "Parameters.h"

class PoissonLikelihoodCalculator : public AbstractLikelihoodCalculator {
  protected:
    const CParameters         & gParameters;
    const CSaTScanData        & gData;

    double                      CalcSVTTLogLikelihood(count_t* pCases, measure_t* pMeasure,
                                                      count_t pTotalCases, double nAlpha,
                                                      double nBeta, int nStatus);  
  public:
    PoissonLikelihoodCalculator(const CSaTScanData& Data);
    virtual ~PoissonLikelihoodCalculator();

    virtual double      CalcLogLikelihood(count_t n, measure_t u);
    virtual double      CalcLogLikelihoodRatio(count_t tCases, measure_t tMeasure, count_t tTotalCases, measure_t tTotalMeasure);
    virtual double      CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster);
    virtual double      CalcSVTTLogLikelihood(size_t tStream, CSVTTCluster* Cluster, const CTimeTrend& GlobalTimeTrend);
    virtual double      GetLogLikelihoodForTotal() const;
};
//*****************************************************************************
#endif
