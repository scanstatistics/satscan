//*****************************************************************************
#ifndef __PoissonLikelihoodCalculation_H
#define __PoissonLikelihoodCalculation_H
//*****************************************************************************
#include "LikelihoodCalculation.h"
#include "Parameters.h"

/** Poisson log likelihood calculator. */
class PoissonLikelihoodCalculator : public AbstractLikelihoodCalculator {
  protected:
    const CParameters & gParameters;   /** const reference to CParameters object */

    double              CalcSVTTLogLikelihood(count_t* pCases, measure_t* pMeasure,
                                              count_t pTotalCases, double nAlpha,
                                              double nBeta, int nStatus) const;
  public:
    PoissonLikelihoodCalculator(const CSaTScanData& Data);
    virtual ~PoissonLikelihoodCalculator();

    virtual double      CalcLogLikelihood(count_t n, measure_t u) const;
    virtual double      CalcLogLikelihoodRatio(count_t tCases, measure_t tMeasure, count_t tTotalCases, measure_t tTotalMeasure) const;
    virtual double      CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster) const;
    virtual double      CalcSVTTLogLikelihood(size_t tSetIndex, CSVTTCluster* Cluster, const CTimeTrend& GlobalTimeTrend) const;
    virtual double      GetLogLikelihoodForTotal() const;
};
//*****************************************************************************
#endif
