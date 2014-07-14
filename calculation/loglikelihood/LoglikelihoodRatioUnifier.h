//******************************************************************************
#ifndef __LoglikelihoodRatioUnifier_H
#define __LoglikelihoodRatioUnifier_H
//******************************************************************************
#include "SaTScan.h"
#include "Parameters.h"
#include "IncidentRate.h"

class AbstractLikelihoodCalculator; /** forward class declaration */

/** Abstract base class for unifying calculated data set loglikelihood ratios. */
class AbstractLoglikelihoodRatioUnifier {
  public:
    AbstractLoglikelihoodRatioUnifier() {}
    virtual ~AbstractLoglikelihoodRatioUnifier() {}

    virtual AbstractLoglikelihoodRatioUnifier * Clone() const = 0;

    virtual void        AdjoinRatio(AbstractLikelihoodCalculator& Calculator,
                                    count_t tCases,
                                    measure_t tMeasure,
                                    size_t tSetIndex) = 0;
    virtual void        AdjoinRatio(AbstractLikelihoodCalculator& Calculator,
                                    count_t tCases,
                                    measure_t tMeasure,
                                    measure_t tMeasureAux,
                                    size_t tSetIndex) = 0;
    virtual void        AdjoinRatio(AbstractLikelihoodCalculator& Calculator,
                                    const std::vector<count_t>& vOrdinalCases, size_t tSetIndex) = 0;
    virtual double      GetLoglikelihoodRatio() const = 0;
    virtual void        Reset() = 0;
};

/** Unifies calculated data set loglikelihood ratios, in such a way, that if
     scanning rate is:
     - high or low  ; No special behavior occurs, call to GetLoglikelihoodRatio()
                      returns all calculated loglikelihood ratios added together.
     - high and low ; Like loglikelihood ratios are added together, call to
                      GetLoglikelihoodRatio() returns the greater of summed
                      loglikelihoods for high rates vs low rates. */
class MultivariateUnifier : public AbstractLoglikelihoodRatioUnifier {
  private:
   double       gdHighRateRatios;          /** loglikelihood ratios for high rates */
   double       gdLowRateRatios;           /** loglikelihood ratios for low rates */
   bool         gbScanHighRates;           /** indicates whether to scan for high rates */
   bool         gbScanLowRates;            /** indicates whether to scan for low rates */
   ProbabilityModelType geProbabilityModelType;

  public:
    MultivariateUnifier(AreaRateType eScanningArea, ProbabilityModelType eProbabilityModelType);
    virtual ~MultivariateUnifier() {}

    virtual AbstractLoglikelihoodRatioUnifier * Clone() const {return new MultivariateUnifier(*this);};

    virtual void        AdjoinRatio(AbstractLikelihoodCalculator& Calculator,
                                    count_t tCases,
                                    measure_t tMeasure,
                                    size_t tSetIndex);
    virtual void        AdjoinRatio(AbstractLikelihoodCalculator& Calculator,
                                    count_t tCases,
                                    measure_t tMeasure,
                                    measure_t tMeasureAux,
                                    size_t tSetIndex);
    virtual void        AdjoinRatio(AbstractLikelihoodCalculator& Calculator,
                                    const std::vector<count_t>& vOrdinalCases, size_t tSetIndex);
    void                GetHighLowRatio(AbstractLikelihoodCalculator& Calculator,
                                        count_t tCases,
                                        measure_t tMeasure,
                                        size_t tSetIndex,
                                        std::pair<double, double>& prHighLowRatios);
    void                GetHighLowRatio(AbstractLikelihoodCalculator& Calculator,
                                        count_t tCases,
                                        measure_t tMeasure,
                                        measure_t tMeasureAux,
                                        size_t tSetIndex,
                                        std::pair<double, double>& prHighLowRatios);
    void                GetHighLowRatioOrdinal(AbstractLikelihoodCalculator& Calculator,
                                               const std::vector<count_t>& vOrdinalCases,
                                               size_t tSetIndex,
                                               std::pair<double, double>& prHighLowRatios);
    virtual double      GetLoglikelihoodRatio() const;
    virtual void        Reset();
};

/** Unifies calculated data set loglikelihood ratios, in such a way, that if
     scanning rate is:
     - high         ; Call to GetLoglikelihoodRatio() returns summed loglikelihood ratios.
     - low          ; Call to GetLoglikelihoodRatio() returns summed loglikelihood ratios
                      times negative one.
     - high and low ; Call to GetLoglikelihoodRatio() returns the absolute value of summed
                      loglikelihood ratios. */
class AdjustmentUnifier : public AbstractLoglikelihoodRatioUnifier {
   double               gdRatio;              /** summed log likelihood ratios */
   AreaRateType         geScanningArea;       /** type of area scanned for */

  public:
    AdjustmentUnifier(AreaRateType eScanningArea);
    virtual ~AdjustmentUnifier() {}

    virtual AbstractLoglikelihoodRatioUnifier * Clone() const {return new AdjustmentUnifier(*this);};

    virtual void        AdjoinRatio(AbstractLikelihoodCalculator& Calculator,
                                    count_t tCases,
                                    measure_t tMeasure,
                                    size_t tSetIndex);
    virtual void        AdjoinRatio(AbstractLikelihoodCalculator& Calculator,
                                    count_t tCases,
                                    measure_t tMeasure,
                                    measure_t tMeasureAux,
                                    size_t tSetIndex);
    virtual void        AdjoinRatio(AbstractLikelihoodCalculator& Calculator,
                                    const std::vector<count_t>& vOrdinalCases, size_t tSetIndex);
    virtual double      GetLoglikelihoodRatio() const;
    double              GetRawLoglikelihoodRatio() const {return gdRatio;}
    virtual void        Reset();
};
//******************************************************************************
#endif
