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
};

/** Batched model log-likelihood calculator. */
class BatchedLikelihoodCalculator : public AbstractLikelihoodCalculator {
  private:
      mutable std::vector<double> _probabilities; // probability of being positive for each data set
      mutable std::vector<double> _log_likelihoods; // log-likelihood of being positive for each data set
      // log-likelihood of being positive for each data set, by time interval (probability, log-likelihood)
      mutable std::unique_ptr<TwoDimensionArrayHandler<std::pair<double, double>>> _log_likelihoods_by_time;
      boost::shared_ptr<RandomizerContainer_t> _randomizer_container; // associated randomizers when running simulations
      bool _always_full_statistic; // indicator that we're always calculating full log-likelihood

      double LL(double q, double Sn, const BatchedRandomizer::BatchEntryContainer_t& positiveBatches) const;
      double probabilityPositive(measure_t C, measure_t N, measure_t Sc, measure_t Sn, const BatchedRandomizer::BatchEntryContainer_t& positiveBatches) const;

  public:
    BatchedLikelihoodCalculator(const CSaTScanData& Data);
    virtual ~BatchedLikelihoodCalculator() {}

    void                      associateRandomizers(boost::shared_ptr<RandomizerContainer_t> rc);
    void                      calculateLoglikelihoodsForAll() const;
    bool                      isScanArea(const ProbabilitiesAOI& probabilities, count_t nCases) const;
    bool                      isScanArea(measure_t positiveCases, measure_t totalCases, const boost::dynamic_bitset<>& BatchIndexes, size_t tSetIndex = 0) const;

    ProbabilitiesAOI        & CalculateProbabilities(ProbabilitiesAOI& probabilities, count_t n, measure_t u, measure_t Sc, measure_t Sn, const boost::dynamic_bitset<>& positiveBatchIndexes, size_t tSetIndex = 0) const;
    ProbabilitiesAOI        & CalculateProbabilitiesByTimeInterval(ProbabilitiesAOI& probabilities, count_t n, measure_t u, measure_t Sc, measure_t Sn, const boost::dynamic_bitset<>& positiveBatchIndexes, int interval, size_t tSetIndex = 0) const;

    double                    getLoglikelihoodRatio(ProbabilitiesAOI& probabilities, size_t tSetIndex = 0) const;
    double                    getLoglikelihoodRatioForInterval(ProbabilitiesAOI& probabilities, int interval, size_t tSetIndex = 0) const;

    ProbabilitiesAOI        & CalculateProbabilitiesForSimulation(ProbabilitiesAOI& probabilities, count_t n, measure_t u, measure_t Sc, measure_t Sn, const boost::dynamic_bitset<>& positiveBatchIndexes, size_t tSetIndex = 0) const;
    ProbabilitiesAOI        & CalculateProbabilitiesForSimulationByTimeInterval(ProbabilitiesAOI& probabilities, count_t n, measure_t u, measure_t Sc, measure_t Sn, const boost::dynamic_bitset<>& positiveBatchIndexes, int interval, size_t tSetIndex = 0) const;

    double                    getMaximizingValue(ProbabilitiesAOI& probabilities, size_t tSetIndex = 0) const;
    virtual double            CalculateFullStatistic(double dMaximizingValue, size_t tSetIndex = 0) const;

    double                    getClusterExpected(const CCluster& cluster, size_t tSetIndex = 0) const;
    double                    getClusterExpected(measure_t totalBatches, const boost::dynamic_bitset<>& BatchIndexes, int windowIndex, size_t tSetIndex = 0) const;
    double                    getClusterRelativeRisk(const CCluster& cluster, size_t tSetIndex = 0) const;
    std::pair<double, double> getProbabilityPositive(const CCluster& cluster, size_t tSetIndex = 0) const;
};
//*****************************************************************************
#endif
