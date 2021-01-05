//******************************************************************************
#ifndef __LoglikelihoodRatioUnifier_H
#define __LoglikelihoodRatioUnifier_H
//******************************************************************************
#include "SaTScan.h"
#include "Parameters.h"
#include "IncidentRate.h"
#include "SSException.h"

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
                                        count_t tCases,
                                        measure_t tMeasure,
                                        count_t casesInPeriod, 
                                        measure_t measureInPeriod,
                                        size_t tSetIndex) = 0;
        virtual void        AdjoinRatio(AbstractLikelihoodCalculator& Calculator, const std::vector<count_t>& vOrdinalCases, size_t tSetIndex) = 0;
        virtual double      GetLoglikelihoodRatio() const = 0;
        virtual void        Reset() = 0;

        // Methods used for risk threshold evaluation.
        virtual count_t     avgObserved() const { throw prg_error("Not implemented", "getAverageObserved()"); return 0;  }
        virtual count_t     avgCaseTotal() const { throw prg_error("Not implemented", "getAverageCaseTotal()"); return 0; }
        virtual measure_t   avgExpected() const { throw prg_error("Not implemented", "getAverageExpected()"); return 0.0; }
        virtual measure_t   avgExpectedTotal() const { throw prg_error("Not implemented", "getAverageExpectedTotal()"); return 0.0; }
};

/** Unifies calculated data set loglikelihood ratios, in such a way, that if
     scanning rate is:
     - high or low  ; No special behavior occurs, call to GetLoglikelihoodRatio()
                      returns all calculated loglikelihood ratios added together.
     - high and low ; Like loglikelihood ratios are added together, call to
                      GetLoglikelihoodRatio() returns the greater of summed
                      loglikelihoods for high rates vs low rates. */
class MultivariateUnifier : public AbstractLoglikelihoodRatioUnifier {
    protected:
        double                  gdHighRateRatios;          /** loglikelihood ratios for high rates */
        double                  gdLowRateRatios;           /** loglikelihood ratios for low rates */
        bool                    gbScanHighRates;           /** indicates whether to scan for high rates */
        bool                    gbScanLowRates;            /** indicates whether to scan for low rates */
        ProbabilityModelType    geProbabilityModelType;

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
                                        count_t tCases,
                                        measure_t tMeasure,
                                        count_t casesInPeriod,
                                        measure_t measureInPeriod,
                                        size_t tSetIndex);
        virtual void        AdjoinRatio(AbstractLikelihoodCalculator& Calculator, const std::vector<count_t>& vOrdinalCases, size_t tSetIndex);
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
        void                GetHighLowRatio(AbstractLikelihoodCalculator& Calculator,
                                            count_t tCases,
                                            measure_t tMeasure,
                                            count_t casesInPeriod,
                                            measure_t measureInPeriod,
                                            size_t tSetIndex,
                                            std::pair<double, double>& prHighLowRatios);
        void                GetHighLowRatioOrdinal(AbstractLikelihoodCalculator& Calculator,
                                                   const std::vector<count_t>& vOrdinalCases,
                                                   size_t tSetIndex,
                                                   std::pair<double, double>& prHighLowRatios);
        virtual double      GetLoglikelihoodRatio() const;
        virtual void        Reset() { gdHighRateRatios = gdLowRateRatios = 0; }
};

class UnifierRiskThreshold {
    friend class MultivariateUnifierRiskThreshold;
    friend class AdjustmentUnifierRiskThreshold;

    protected:
        /* variables used with relative risk restriction */
        count_t _sum_observed;
        count_t _sum_case_totals;
        measure_t _sum_expected;
        measure_t _sum_expected_totals;
        unsigned int _num_in_sum;

    public:
        UnifierRiskThreshold() { reset(); }

        count_t   averageObserved() const { return _num_in_sum ? _sum_observed / static_cast<count_t>(_num_in_sum) : 0; }
        count_t   averageCaseTotal() const { return _num_in_sum ? _sum_case_totals / static_cast<count_t>(_num_in_sum) : 0; }
        measure_t averageExpected() const { return _num_in_sum ? _sum_expected / static_cast<measure_t>(_num_in_sum) : 0.0; }
        measure_t averageExpectedTotal() const { return _num_in_sum ? _sum_expected_totals / static_cast<measure_t>(_num_in_sum) : 0.0; }
        void      reset() { _sum_observed = 0; _sum_expected = 0.0; _sum_case_totals = 0; _sum_expected_totals = 0.0; _num_in_sum = 0; }
};

class MultivariateUnifierRiskThreshold : public MultivariateUnifier {
    protected:
        UnifierRiskThreshold _risk_threshold;

    public:
        MultivariateUnifierRiskThreshold(AreaRateType eScanningArea, ProbabilityModelType eProbabilityModelType) 
            : MultivariateUnifier(eScanningArea, eProbabilityModelType) {}
        virtual ~MultivariateUnifierRiskThreshold() {}

        virtual void        AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, size_t tSetIndex);
        virtual void        Reset() { MultivariateUnifier::Reset(); _risk_threshold.reset(); }

        // Methods used for risk threshold evaluation.
        virtual count_t     avgObserved() const { return _risk_threshold.averageObserved(); }
        virtual count_t     avgCaseTotal() const { return _risk_threshold.averageCaseTotal(); }
        virtual measure_t   avgExpected() const { return _risk_threshold.averageExpected(); }
        virtual measure_t   avgExpectedTotal() const { return _risk_threshold.averageExpectedTotal(); }
};

/** Unifies calculated data set loglikelihood ratios, in such a way, that if
     scanning rate is:
     - high         ; Call to GetLoglikelihoodRatio() returns summed loglikelihood ratios.
     - low          ; Call to GetLoglikelihoodRatio() returns summed loglikelihood ratios
                      times negative one.
     - high and low ; Call to GetLoglikelihoodRatio() returns the absolute value of summed
                      loglikelihood ratios. */
class AdjustmentUnifier : public AbstractLoglikelihoodRatioUnifier {
    protected:
        double       gdRatio;              /** summed log likelihood ratios */
        AreaRateType geScanningArea;       /** type of area scanned for */

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
        virtual void        AdjoinRatio(AbstractLikelihoodCalculator& Calculator,
                                        count_t tCases,
                                        measure_t tMeasure,
                                        count_t casesInPeriod,
                                        measure_t measureInPeriod,
                                        size_t tSetIndex);
        virtual double      GetLoglikelihoodRatio() const;
        double              GetRawLoglikelihoodRatio() const {return gdRatio;}
        virtual void        Reset() { gdRatio = 0; }
};

class AdjustmentUnifierRiskThreshold : public AdjustmentUnifier {
    protected:
        UnifierRiskThreshold _risk_threshold;

    public:
        AdjustmentUnifierRiskThreshold(AreaRateType eScanningArea) : AdjustmentUnifier(eScanningArea) {}
        virtual ~AdjustmentUnifierRiskThreshold() {}

        virtual void        AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, size_t tSetIndex);
        virtual void        Reset() { AdjustmentUnifier::Reset(); _risk_threshold.reset(); }

        // Methods used for risk threshold evaluation.
        virtual count_t     avgObserved() const { return _risk_threshold.averageObserved(); }
        virtual count_t     avgCaseTotal() const { return _risk_threshold.averageCaseTotal(); }
        virtual measure_t   avgExpected() const { return _risk_threshold.averageExpected(); }
        virtual measure_t   avgExpectedTotal() const { return _risk_threshold.averageExpectedTotal(); }
};
//******************************************************************************
#endif
