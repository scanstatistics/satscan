//******************************************************************************
#ifndef __LoglikelihoodRatioUnifier_H
#define __LoglikelihoodRatioUnifier_H
//******************************************************************************
#include "SaTScan.h"
#include "Parameters.h"
#include "IncidentRate.h"
#include "SSException.h"

class AbstractLikelihoodCalculator; /** forward class declaration */

/* Accumulates attributes of data streams for clusters in evaluation. */
class DataStreamAccumulator {
    friend class MultivariateUnifierRiskThreshold;
    friend class MultivariateUnifierHighRate;
    friend class MultivariateUnifierLowRate;
    friend class AdjustmentUnifier;
    friend class AdjustmentUnifierRiskThreshold;
    friend class AdjustmentUnifierBatchModelTimeStratified;

    protected:
        /* variables used with relative risk restriction */
        count_t _sum_observed;
        count_t _sum_case_totals;
        measure_t _sum_expected;
        measure_t _sum_total_expected;

    public:
        DataStreamAccumulator() { reset(); }

        count_t   getObserved() const { return _sum_observed; }
        count_t   getCaseTotal() const { return _sum_case_totals; }
        measure_t getExpected() const { return _sum_expected; }
        measure_t getTotalExpected() const { return _sum_total_expected; }
        void      reset() { _sum_observed = 0; _sum_expected = 0.0; _sum_case_totals = 0; _sum_total_expected = 0; }
};


/** Abstract base class for unifying calculated data set loglikelihood ratios. */
class AbstractLoglikelihoodRatioUnifier {
    protected:
        ProbabilityModelType _probability_model;
        boost::dynamic_bitset<> _unified_sets;

    public:
        typedef std::pair<double, const DataStreamAccumulator*> AccumulationPair_t;

        AbstractLoglikelihoodRatioUnifier(ProbabilityModelType probability_model) : _probability_model(probability_model) {}
        virtual ~AbstractLoglikelihoodRatioUnifier() {}

        virtual AbstractLoglikelihoodRatioUnifier * Clone() const = 0;

        virtual void AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, size_t tSetIndex) = 0;
        virtual void AdjoinRatioNonparametric(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, count_t totalCases, measure_t totalMeasure, size_t tSetIndex) = 0;
        virtual void AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, measure_t tMeasureAux, size_t tSetIndex) = 0;
        virtual void AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) = 0;
        virtual void AdjoinRatio(AbstractLikelihoodCalculator& Calculator, const std::vector<count_t>& vOrdinalCases, size_t tSetIndex) = 0;
        virtual void AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, measure_t tMeasureAux, measure_t tMeasureAux2, const boost::dynamic_bitset<>& positiveBatches, const boost::dynamic_bitset<>& Batches, size_t tSetIndex) = 0;
        virtual void AdjoinRatioSimulation(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, measure_t tMeasureAux, measure_t tMeasureAux2, const boost::dynamic_bitset<>& positiveBatches, const boost::dynamic_bitset<>& Batches, size_t tSetIndex) = 0;
        virtual void AdjoinRatio(double llr, AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure) {/* nop */}
        virtual bool isScanRate() const;
        virtual double GetLoglikelihoodRatio() const = 0;
        virtual void Reset() = 0;
        virtual const boost::dynamic_bitset<>& getUnifiedSets() const {
            return _unified_sets;
        }

        virtual count_t getObservedCount() const {
            throw prg_error("Not implemented", "getObservedCount()"); return 0;
        }
        virtual double GetRawLoglikelihoodRatio() const {
            throw prg_error("Not implemented", "GetRawLoglikelihoodRatio()"); return 0.0;
        }
        virtual AccumulationPair_t getHighRateAccumulationPair() const {
            throw prg_error("Not implemented", "getHighRateAccumulationPair()"); return std::make_pair(0.0, (const DataStreamAccumulator*)0);
        }
        virtual AccumulationPair_t getLowRateAccumulationPair() const {
            throw prg_error("Not implemented", "getLowRateAccumulationPair()"); return std::make_pair(0.0, (const DataStreamAccumulator*)0);
        }

        // Methods used for risk threshold evaluation.
        virtual count_t getObserved() const { throw prg_error("Not implemented", "getObserved()"); return 0;  }
        virtual count_t getCaseTotal() const { throw prg_error("Not implemented", "getCaseTotal()"); return 0; }
        virtual measure_t getExpected() const { throw prg_error("Not implemented", "getExpected()"); return 0.0; }
        virtual measure_t getTotalExpected() const { throw prg_error("Not implemented", "getExpected()"); return 0.0; }

};

/* Multivariate unification class for high rating scanning - log likelihoods are simply added together. */
class MultivariateUnifierHighRate : public AbstractLoglikelihoodRatioUnifier {
    protected:
        double _llr;
        DataStreamAccumulator _data_stream_accumulator;

    public:
        MultivariateUnifierHighRate(size_t numSets, ProbabilityModelType probability_model) : AbstractLoglikelihoodRatioUnifier(probability_model), _llr(0.0) {
            _unified_sets.resize(numSets);
        }
        virtual ~MultivariateUnifierHighRate() {}

        virtual AbstractLoglikelihoodRatioUnifier * Clone() const { return new MultivariateUnifierHighRate(*this); };
        const DataStreamAccumulator * getDataStreamAccumulator() const { return &_data_stream_accumulator; }

        virtual void AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, size_t tSetIndex);
        virtual void AdjoinRatioNonparametric(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, count_t totalCases, measure_t totalMeasure, size_t tSetIndex);
        virtual void AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, measure_t tMeasureAux, size_t tSetIndex);
        virtual void AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex);
        virtual void AdjoinRatio(AbstractLikelihoodCalculator& Calculator, const std::vector<count_t>& vOrdinalCases, size_t tSetIndex);
        virtual void AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, measure_t tMeasureAux, measure_t tMeasureAux2, const boost::dynamic_bitset<>& positiveBatches, const boost::dynamic_bitset<>& Batches, size_t tSetIndex);
        virtual void AdjoinRatioSimulation(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, measure_t tMeasureAux, measure_t tMeasureAux2, const boost::dynamic_bitset<>& positiveBatches, const boost::dynamic_bitset<>& Batches, size_t tSetIndex);

        virtual double GetLoglikelihoodRatio() const { return _llr; }
        virtual void Reset() { _llr = 0.0; _data_stream_accumulator.reset(); _unified_sets.reset(); }

        // Method used for minimum number of cases evaulation.
        virtual count_t getObservedCount() const { return _data_stream_accumulator._sum_observed; };
        // Methods used for risk threshold evaluation.
        virtual count_t getObserved() const { return _data_stream_accumulator.getObserved(); }
        virtual count_t getCaseTotal() const { return _data_stream_accumulator.getCaseTotal(); }
        virtual measure_t getExpected() const { return _data_stream_accumulator.getExpected(); }
        virtual measure_t getTotalExpected() const { return _data_stream_accumulator.getTotalExpected(); }
};

/* Multivariate unification class for low rating scanning - log likelihoods are simply added together. */
class MultivariateUnifierLowRate : public AbstractLoglikelihoodRatioUnifier {
    protected:
        double _llr;
        DataStreamAccumulator _data_stream_accumulator;

    public:
        MultivariateUnifierLowRate(size_t numSets, ProbabilityModelType probability_model) : AbstractLoglikelihoodRatioUnifier(probability_model), _llr(0.0) {
            _unified_sets.resize(numSets);
        }
        virtual ~MultivariateUnifierLowRate() {}

        virtual AbstractLoglikelihoodRatioUnifier * Clone() const { return new MultivariateUnifierLowRate(*this); };
        const DataStreamAccumulator * getDataStreamAccumulator() const { return &_data_stream_accumulator; }

        virtual void AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, size_t tSetIndex);
        virtual void AdjoinRatioNonparametric(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, count_t totalCases, measure_t totalMeasure, size_t tSetIndex);
        virtual void AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, measure_t tMeasureAux, size_t tSetIndex);
        virtual void AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex);
        virtual void AdjoinRatio(AbstractLikelihoodCalculator& Calculator, const std::vector<count_t>& vOrdinalCases, size_t tSetIndex);
        virtual void AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, measure_t tMeasureAux, measure_t tMeasureAux2, const boost::dynamic_bitset<>& positiveBatches, const boost::dynamic_bitset<>& Batches, size_t tSetIndex);
        virtual void AdjoinRatioSimulation(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, measure_t tMeasureAux, measure_t tMeasureAux2, const boost::dynamic_bitset<>& positiveBatches, const boost::dynamic_bitset<>& Batches, size_t tSetIndex);

        virtual double GetLoglikelihoodRatio() const { return _llr; }
        virtual void Reset() { _llr = 0.0; _data_stream_accumulator.reset(); _unified_sets.reset(); }

        // Method used for minimum number of cases evaulation.
        virtual count_t getObservedCount() const { return _data_stream_accumulator._sum_observed; };
        // Methods used for risk threshold evaluation.
        virtual count_t getObserved() const { return _data_stream_accumulator.getObserved(); }
        virtual count_t getCaseTotal() const { return _data_stream_accumulator.getCaseTotal(); }
        virtual measure_t getExpected() const { return _data_stream_accumulator.getExpected(); }
        virtual measure_t getTotalExpected() const { return _data_stream_accumulator.getTotalExpected(); }
};

/* Multivariate unification class for simultaneous high and low rating scanning. 
   High log likelihoods ratios and low log likelihood ratios are maintained separately. 
   The greater log likelihood summation determines whether cluster is high or low. */
class MultivariateUnifierHighLowRate : public AbstractLoglikelihoodRatioUnifier {
    MultivariateUnifierLowRate _low_rate;
    MultivariateUnifierHighRate _high_rate;

    public:
        MultivariateUnifierHighLowRate(size_t numSets, ProbabilityModelType probability_model) :
            AbstractLoglikelihoodRatioUnifier(probability_model), _low_rate(numSets, probability_model), _high_rate(numSets, probability_model) {}
        virtual ~MultivariateUnifierHighLowRate() {}

        virtual AbstractLoglikelihoodRatioUnifier * Clone() const { return new MultivariateUnifierHighLowRate(*this); };

        virtual void AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, size_t tSetIndex);
        virtual void AdjoinRatioNonparametric(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, count_t totalCases, measure_t totalMeasure, size_t tSetIndex);
        virtual void AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, measure_t tMeasureAux, size_t tSetIndex);
        virtual void AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex);
        virtual void AdjoinRatio(AbstractLikelihoodCalculator& Calculator, const std::vector<count_t>& vOrdinalCases, size_t tSetIndex);
        virtual void AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, measure_t tMeasureAux, measure_t tMeasureAux2, const boost::dynamic_bitset<>& positiveBatches, const boost::dynamic_bitset<>& Batches, size_t tSetIndex);
        virtual void AdjoinRatioSimulation(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, measure_t tMeasureAux, measure_t tMeasureAux2, const boost::dynamic_bitset<>& positiveBatches, const boost::dynamic_bitset<>& Batches, size_t tSetIndex);

        virtual const boost::dynamic_bitset<>& getUnifiedSets() const {
            if (_high_rate.GetLoglikelihoodRatio() > _low_rate.GetLoglikelihoodRatio())
                return _high_rate.getUnifiedSets();
            return _low_rate.getUnifiedSets();
        }

        virtual double GetLoglikelihoodRatio() const { return std::max(_low_rate.GetLoglikelihoodRatio(), _high_rate.GetLoglikelihoodRatio()); }
        virtual void Reset() { _low_rate.Reset(); _high_rate.Reset(); }

        virtual AccumulationPair_t getHighRateAccumulationPair() const {
            return std::make_pair(_high_rate.GetLoglikelihoodRatio(), _high_rate.getDataStreamAccumulator());
        }
        virtual AccumulationPair_t getLowRateAccumulationPair() const {
            return std::make_pair(_low_rate.GetLoglikelihoodRatio(), _low_rate.getDataStreamAccumulator());
        }
};

/* Adjustment unification class. Calculates data set loglikelihood ratios, in such a way, that if scanning rate is:
     - high; GetLoglikelihoodRatio() returns summed loglikelihood ratios.
     - low; GetLoglikelihoodRatio() returns summed loglikelihood ratios times negative one.
     - high and low; GetLoglikelihoodRatio() returns the absolute value of summed loglikelihood ratios. */
class AdjustmentUnifier : public AbstractLoglikelihoodRatioUnifier {
    protected:
        double _llr;
        AreaRateType _scan_area;
        DataStreamAccumulator _data_stream_accumulator;

    public:
        AdjustmentUnifier(AreaRateType scan_area, ProbabilityModelType probability_model): AbstractLoglikelihoodRatioUnifier(probability_model), _scan_area(scan_area), _llr(0.0){}
        virtual ~AdjustmentUnifier() {}

        virtual AbstractLoglikelihoodRatioUnifier * Clone() const {return new AdjustmentUnifier(*this);};

        virtual void        AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, size_t tSetIndex);
        virtual void        AdjoinRatioNonparametric(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, count_t totalCases, measure_t totalMeasure, size_t tSetIndex);
        virtual void        AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, measure_t tMeasureAux, size_t tSetIndex);
        virtual void        AdjoinRatio(AbstractLikelihoodCalculator& Calculator, const std::vector<count_t>& vOrdinalCases, size_t tSetIndex);
        virtual void        AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex);
        virtual void        AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, measure_t tMeasureAux, measure_t tMeasureAux2, const boost::dynamic_bitset<>& positiveBatches, const boost::dynamic_bitset<>& Batches, size_t tSetIndex);
        virtual void        AdjoinRatioSimulation(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, measure_t tMeasureAux, measure_t tMeasureAux2, const boost::dynamic_bitset<>& positiveBatches, const boost::dynamic_bitset<>& Batches, size_t tSetIndex);
        virtual double      GetLoglikelihoodRatio() const;
        virtual double      GetRawLoglikelihoodRatio() const {return _llr;}
        virtual void        Reset() { _llr = 0.0; _data_stream_accumulator.reset(); }

        // Method used for minimum number of cases evaulation.
        virtual count_t getObservedCount() const { return _data_stream_accumulator._sum_observed; };
        // Methods used for risk threshold evaluation.
        virtual count_t getObserved() const { return _data_stream_accumulator.getObserved(); }
        virtual count_t getCaseTotal() const { return _data_stream_accumulator.getCaseTotal(); }
        virtual measure_t getExpected() const { return _data_stream_accumulator.getExpected(); }
        virtual measure_t getTotalExpected() const { return _data_stream_accumulator.getTotalExpected(); }
};

/* Adjustment unifier for the batch model with a space-time analysis performing the time stratified temporal adjustment. */
class AdjustmentUnifierBatchModelTimeStratified : public AbstractLoglikelihoodRatioUnifier {
protected:
    double _llr;
    AreaRateType _scan_area;
    DataStreamAccumulator _data_stream_accumulator;

public:
    AdjustmentUnifierBatchModelTimeStratified(AreaRateType scan_area) : AbstractLoglikelihoodRatioUnifier(BATCHED), _scan_area(scan_area), _llr(0.0) {}
    virtual ~AdjustmentUnifierBatchModelTimeStratified() {}

    virtual AbstractLoglikelihoodRatioUnifier* Clone() const { return new AdjustmentUnifierBatchModelTimeStratified(*this); };

    virtual void        AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, size_t tSetIndex);
    virtual void        AdjoinRatioNonparametric(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, count_t totalCases, measure_t totalMeasure, size_t tSetIndex);
    virtual void        AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, measure_t tMeasureAux, size_t tSetIndex);
    virtual void        AdjoinRatio(AbstractLikelihoodCalculator& Calculator, const std::vector<count_t>& vOrdinalCases, size_t tSetIndex);
    virtual void        AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex);

    virtual void        AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, measure_t tMeasureAux, measure_t tMeasureAux2, const boost::dynamic_bitset<>& positiveBatches, const boost::dynamic_bitset<>& Batches, size_t tSetIndex);
    virtual void        AdjoinRatioSimulation(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, measure_t tMeasureAux, measure_t tMeasureAux2, const boost::dynamic_bitset<>& positiveBatches, const boost::dynamic_bitset<>& Batches, size_t tSetIndex);

    virtual void        AdjoinRatio(double llr, AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure);
    virtual bool        isScanRate() const;

    virtual double      GetLoglikelihoodRatio() const;
    virtual double      GetRawLoglikelihoodRatio() const { return _llr; }
    virtual void        Reset() { _llr = 0.0; _data_stream_accumulator.reset(); }

    // Method used for minimum number of cases evaulation.
    virtual count_t getObservedCount() const { return _data_stream_accumulator._sum_observed; };
    // Methods used for risk threshold evaluation.
    virtual count_t getObserved() const { return _data_stream_accumulator.getObserved(); }
    virtual count_t getCaseTotal() const { return _data_stream_accumulator.getCaseTotal(); }
    virtual measure_t getExpected() const { return _data_stream_accumulator.getExpected(); }
    virtual measure_t getTotalExpected() const { return _data_stream_accumulator.getTotalExpected(); }
};

/* Adjustment unification class specialized for risk threshold evaluation. */
class AdjustmentUnifierRiskThreshold : public AdjustmentUnifier {
    public:
        AdjustmentUnifierRiskThreshold(AreaRateType eScanningArea, ProbabilityModelType probability_model) : AdjustmentUnifier(eScanningArea, probability_model) {}
        virtual ~AdjustmentUnifierRiskThreshold() {}

        virtual void        AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, size_t tSetIndex);
        virtual void        AdjoinRatioNonparametric(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, count_t totalCases, measure_t totalMeasure, size_t tSetIndex);
};
//******************************************************************************
#endif
