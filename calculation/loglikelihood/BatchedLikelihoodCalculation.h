//*****************************************************************************
#ifndef __BatchedLikelihoodCalculation_H
#define __BatchedLikelihoodCalculation_H
//*****************************************************************************
#include "LikelihoodCalculation.h"
#include "BatchedRandomizer.h"

class CSaTScanData;
class CCluster;

typedef std::pair<double, double> InsideOutsideP_t;

/** Calculated probabilities inside area of interest and outside of that area. */
struct ProbabilitiesAOI {
    double _pinside; // probability positive inside
    double _sn_inside; // sum of negative batches inside
    BatchedRandomizer::BatchEntryContainer_t _positive_batches; // positive batches inside
    double _poutside; // probability positive inside
    double _sn_outside; // sum of negative batches outside
    BatchedRandomizer::BatchEntryContainer_t _positive_batches_outside; // positive batches outside

    ProbabilitiesAOI(): _pinside(0), _sn_inside(0), _poutside(0), _sn_outside(0){}
    bool operator==(const ProbabilitiesAOI& other) {
        auto equalsOther = [](const BatchedRandomizer::BatchEntryContainer_t& a, const BatchedRandomizer::BatchEntryContainer_t& b) {
            if (a.size() != b.size()) return false;
            for (size_t t = 0; t < a.size(); ++t)
                if (a[t].get<0>() != b[t].get<0>() || a[t].get<1>() != b[t].get<1>() ||
                    a[t].get<2>() != b[t].get<2>() || a[t].get<3>() != b[t].get<3>()) return false;
            return true;
        };
        return _pinside == other._pinside && _sn_inside == other._sn_inside &&
            _poutside == other._poutside && _sn_outside == other._sn_outside &&
            equalsOther(_positive_batches, other._positive_batches) &&
            equalsOther(_positive_batches_outside, other._positive_batches_outside);
    }
};

/* Structure used to store ProbabilitiesAOI objects during cluster scanning and enable
   caching of probabilities (verse repeated recalculation). */
struct ProbabilitiesRange {
    ProbabilitiesAOI _paoi; // probabilities inside area of interest and outside of that area
    size_t _range_idx; // associated range
    double _llr; // calculated log-likelihood
    ProbabilitiesRange(size_t ridx = 0) : _range_idx(ridx), _llr(std::numeric_limits<double>::infinity()) {}
    bool llrIsSet() const { return _llr != std::numeric_limits<double>::infinity(); }
};
typedef boost::shared_ptr<ProbabilitiesRange> ProbabilitiesRange_t;

class BatchedSpaceTimeData;

/** Batched model log-likelihood calculator. */
class BatchedLikelihoodCalculator : public AbstractLikelihoodCalculator {
  public:
      typedef std::vector<ProbabilitiesRange_t> ProbabilitiesContainer_t;
      typedef std::map<WindowRange_t, ProbabilitiesRange_t> ProbabilitiesCache_t;
      typedef std::vector<ProbabilitiesCache_t> DataSetsProbabilitiesCache_t;

  private:
      mutable std::vector<double> _probabilities; // probability of being positive for each data set
      mutable std::vector<double> _log_likelihoods; // log-likelihood of being positive for each data set
      // log-likelihood of being positive for each data set, by time interval (probability, log-likelihood)
      mutable std::unique_ptr<TwoDimensionArrayHandler<std::pair<double, double>>> _log_likelihoods_by_time;
      // log-likelihood of being positive for each data set, by time interval range (probability, log-likelihood)
      mutable std::unique_ptr<TwoDimensionArrayHandler<std::pair<double, double>>> _log_likelihoods_by_range;
      mutable DataSetsProbabilitiesCache_t _probabilities_cache; // probabilities cache used in interval ranges
      boost::shared_ptr<RandomizerContainer_t> _randomizer_container; // associated randomizers when running simulations
      bool _always_full_statistic; // indicator that we're always calculating full log-likelihood

      double LL(double q, double Sn, const BatchedRandomizer::BatchEntryContainer_t& positiveBatches) const;
      double probabilityPositive(measure_t C, measure_t N, measure_t Sc, measure_t Sn, const BatchedRandomizer::BatchEntryContainer_t& positiveBatches) const;
      double getProbabilityForInterval(int interval, size_t tSetIndex) const;

  public:
    BatchedLikelihoodCalculator(const CSaTScanData& Data);
    virtual ~BatchedLikelihoodCalculator() {}

    void                      clearCache() { std::for_each(_probabilities_cache.begin(), _probabilities_cache.end(), [](auto& m) { m.clear(); }); }

    void                      associateRandomizers(boost::shared_ptr<RandomizerContainer_t> rc);
    void                      calculateLoglikelihoodsForAll() const;

    ProbabilitiesAOI        & CalculateProbabilities(ProbabilitiesAOI& probabilities, count_t n, measure_t u, measure_t Sc, measure_t Sn, const boost::dynamic_bitset<>& positiveBatchIndexes, size_t tSetIndex = 0) const;
    ProbabilitiesRange_t    & CalculateProbabilitiesByTimeInterval(ProbabilitiesRange_t& probabilities, count_t n, measure_t u, measure_t Sc, measure_t Sn, const boost::dynamic_bitset<>& positiveBatchIndexes, int interval, size_t tSetIndex = 0) const;
    void                      CalculateProbabilitiesForWindow(BatchedSpaceTimeData& Data, int iWindowStart, int iWindowEnd, ProbabilitiesContainer_t& probabilities, size_t tSetIndex = 0);
    void                      CalculateProbabilitiesForWindowForSimulation(BatchedSpaceTimeData& Data, int iWindowStart, int iWindowEnd, ProbabilitiesContainer_t& probabilities, size_t tSetIndex = 0);
    ProbabilitiesAOI        & CalculateProbabilitiesForSimulation(ProbabilitiesAOI& probabilities, count_t n, measure_t u, measure_t Sc, measure_t Sn, const boost::dynamic_bitset<>& positiveBatchIndexes, size_t tSetIndex = 0) const;
    ProbabilitiesRange_t    & CalculateProbabilitiesForSimulationByTimeInterval(ProbabilitiesRange_t& probabilities, count_t n, measure_t u, measure_t Sc, measure_t Sn, const boost::dynamic_bitset<>& positiveBatchIndexes, int interval, size_t tSetIndex = 0) const;

    double                    getLoglikelihoodRatio(ProbabilitiesAOI& probabilities, size_t tSetIndex = 0) const;
    double                    getLoglikelihoodRatioForInterval(ProbabilitiesRange& probabilities, int interval, size_t tSetIndex = 0) const;
    double                    getLoglikelihoodRatioForRange(ProbabilitiesRange& probabilities, size_t tSetIndex = 0) const;

    double                    getMaximizingValue(ProbabilitiesAOI& probabilities, size_t tSetIndex = 0) const;
    virtual double            CalculateFullStatistic(double dMaximizingValue, size_t tSetIndex = 0) const;

    double                    getClusterExpected(const CCluster& cluster, size_t tSetIndex = 0) const;
    double                    getExpectedInWindow(int windowStart, int windowEnd, size_t tSetIndex = 0) const;
    double                    getExpectedInWindow(int windowStart, int windowEnd, const std::vector<tract_t>& locationIdexes, size_t tSetIndex = 0) const;
    double                    getClusterExpectedAtWindow(const boost::dynamic_bitset<>& BatchIndexes, int windowIndex, size_t tSetIndex = 0) const;
    double                    getExpectedForBatches(const boost::dynamic_bitset<>& BatchIndexes, size_t tSetIndex = 0) const;
    double                    getClusterRelativeRisk(const CCluster& cluster, size_t tSetIndex = 0) const;
    std::pair<double, double> getProbabilityPositive(const CCluster& cluster, size_t tSetIndex = 0) const;
};
//*****************************************************************************
#endif
