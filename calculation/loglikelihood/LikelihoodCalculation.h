//*****************************************************************************
#ifndef __LikelihoodCalculation_H
#define __LikelihoodCalculation_H
//*****************************************************************************
#include "SaTScan.h"
#include "Parameters.h"
#include "IncidentRate.h"

class AbstractLikelihoodCalculator; /** forward class declaration */

/** Abstract base class for unifying calculated data stream loglikelihood ratios. */
class AbstractLoglikelihoodRatioUnifier {
  public:
    AbstractLoglikelihoodRatioUnifier() {}
    ~AbstractLoglikelihoodRatioUnifier() {}

    virtual void        AdjoinRatio(AbstractLikelihoodCalculator& Calculator,
                                    unsigned int iStream,
                                    count_t tCases,
                                    measure_t tMeasure,
                                    count_t tTotalCases,
                                    measure_t tTotalMeasure) = 0;
    virtual double      GetLoglikelihoodRatio() const = 0;
    virtual void        Reset() = 0;
};

/** Unifies calculated data stream loglikelihood ratios, in such a way, that if
     scanning rate is:
     - high or low  ; No special behavior occurs, call to GetLoglikelihoodRatio()
                      returns all calculated loglikelihood ratios added together.
     - high and low ; Like loglikelihood ratios are added together, call to
                      GetLoglikelihoodRatio() returns the greater of summed
                      loglikelihoods for high rates vs low rates. */
class MultivariateUnifier : public AbstractLoglikelihoodRatioUnifier {
   double       gdHighRateRatios;          /** loglikelihood ratios for high rates */
   double       gdLowRateRatios;           /** loglikelihood ratios for low rates */
   bool         gbScanHighRates;           /** indicates whether to scan for high rates */
   bool         gbScanLowRates;            /** indicates whether to scan for low rates */

  public:
    MultivariateUnifier(AreaRateType eScanningArea);
    ~MultivariateUnifier() {}

    virtual void        AdjoinRatio(AbstractLikelihoodCalculator& Calculator,
                                    unsigned int iStream,    
                                    count_t tCases,
                                    measure_t tMeasure,
                                    count_t tTotalCases,
                                    measure_t tTotalMeasure);
    virtual double      GetLoglikelihoodRatio() const;
    virtual void        Reset();
};

/** Unifies calculated data stream loglikelihood ratios, in such a way, that if
     scanning rate is:
     - high         ; Call to GetLoglikelihoodRatio() returns summed loglikelihood ratios.
     - low          ; Call to GetLoglikelihoodRatio() returns summed loglikelihood ratios
                      times negative one.
     - high and low ; Call to GetLoglikelihoodRatio() returns the absolute value of summed
                      loglikelihood ratios. */
class AdjustmentUnifier : public AbstractLoglikelihoodRatioUnifier {
   double               gdRatio;              /** summed log likelihood ratios */
   RATE_FUNCPTRTYPE     gfRateOfInterest;     /** function pointer to scanning area funciton */
   AreaRateType         geScanningArea;       /** type of area scanned for */

  public:
    AdjustmentUnifier(AreaRateType eScanningArea);
    ~AdjustmentUnifier() {}

    virtual void        AdjoinRatio(AbstractLikelihoodCalculator& Calculator,
                                    unsigned int iStream,
                                    count_t tCases,
                                    measure_t tMeasure,
                                    count_t tTotalCases,
                                    measure_t tTotalMeasure);
    virtual double      GetLoglikelihoodRatio() const;
    virtual void        Reset();
};


class CPSMonotoneCluster; /** forward class declaration */
class CSaTScanData;       /** forward class declaration */
class CSVTTCluster;       /** forward class declaration */
class CTimeTrend;         /** forward class declaration */

/** Abstract interface for which to calculate log likelihoods and log likelihood ratios. */
class AbstractLikelihoodCalculator {
  protected:
    const CSaTScanData                & gData;                    /** const reference to data hub */
    count_t                             gtTotalCasesInDataSet;    /** total number of cases, in all data streams */
    measure_t                           gtTotalMeasureInDataSet;  /** total number of expected cases, in all data streams */
    AbstractLoglikelihoodRatioUnifier * gpUnifier;                /** log likelihood ratio unifier for multiple data streams */

    void                                Init() {gpUnifier=0;}
    void                                Setup();

  public:
    AbstractLikelihoodCalculator(const CSaTScanData& Data);
    virtual ~AbstractLikelihoodCalculator();

    virtual double                      CalcLogLikelihood(count_t n, measure_t u) const = 0;
    virtual double                      CalcLogLikelihoodRatio(count_t tCases, measure_t tMeasure, count_t tTotalCases, measure_t tTotalMeasure) const = 0;
    virtual double                      CalcLogLikelihoodRatioEx(count_t tCases, measure_t tMeasure, measure_t tMeasure2, count_t tTotalCases, measure_t tTotalMeasure) const;
    virtual double                      CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster) const;
    virtual double                      CalcSVTTLogLikelihood(size_t tStream, CSVTTCluster* Cluster, const CTimeTrend& GlobalTimeTrend) const;
    const CSaTScanData                & GetDataHub() const {return gData;}
    virtual double                      GetLogLikelihoodForTotal() const = 0;
    AbstractLoglikelihoodRatioUnifier & GetUnifier() const;
};
//*****************************************************************************
#endif
