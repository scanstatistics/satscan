//******************************************************************************
#ifndef __LikelihoodCalculation_H
#define __LikelihoodCalculation_H
//******************************************************************************
#include "SaTScan.h"

class CPSMonotoneCluster; /** forward class declaration */
class CSaTScanData;       /** forward class declaration */
class CSVTTCluster;       /** forward class declaration */
class CTimeTrend;         /** forward class declaration */
class AbstractLoglikelihoodRatioUnifier; /** forward class declaration */

/** Abstract interface for which to calculate log likelihoods and log likelihood ratios. */
class AbstractLikelihoodCalculator {
  protected:
    const CSaTScanData                & gDataHub;                      /** const reference to data hub */
    count_t                             gtTotalCasesInFirstDataSet;    /** total number of cases, in first data set */
    measure_t                           gtTotalMeasureInFirstDataSet;  /** total number of expected cases, in first data set */
    AbstractLoglikelihoodRatioUnifier * gpUnifier;                     /** log likelihood ratio unifier for multiple data sets */

    void                                Setup();

  public:
    AbstractLikelihoodCalculator(const CSaTScanData& DataHub);
    virtual ~AbstractLikelihoodCalculator();

    virtual double                      CalcLogLikelihood(count_t n, measure_t u) const = 0;
    virtual double                      CalcLogLikelihoodRatio(count_t tCases, measure_t tMeasure, count_t tTotalCases, measure_t tTotalMeasure) const = 0;
    virtual double                      CalcLogLikelihoodRatioOrdinal(const std::vector<count_t>& vOrdinalCases, size_t tSetIndex=0) const;
    virtual double                      CalcLogLikelihoodRatioNormal(count_t tCases, measure_t tMeasure, measure_t tMeasure2, count_t tTotalCases, measure_t tTotalMeasure, measure_t tTotalMeasureSq) const;
    virtual double                      CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster) const;
    virtual double                      CalcSVTTLogLikelihood(size_t tSetIndex, CSVTTCluster* Cluster, const CTimeTrend& GlobalTimeTrend) const;
    const CSaTScanData                & GetDataHub() const {return gDataHub;}
    virtual double                      GetLogLikelihoodForTotal() const = 0;
    virtual double                      GetLogLikelihoodRatio(double dLogLikelihood) const = 0;
    AbstractLoglikelihoodRatioUnifier & GetUnifier() const;
};
//******************************************************************************
#endif

