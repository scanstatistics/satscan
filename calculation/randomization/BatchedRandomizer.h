//******************************************************************************
#ifndef __BatchedRandomizer_H
#define __BatchedRandomizer_H
//******************************************************************************
#include "DenominatorDataRandomizer.h"
#include "boost/tuple/tuple.hpp"
#include <boost/optional.hpp>

class CSaTScanData; // forward class declarations
class CCluster;

/** Batched model randomizer for null hypothesis. */
class BatchedRandomizer : public AbstractDenominatorDataRandomizer {
    public:
        typedef boost::tuple<unsigned short, unsigned int, int, tract_t> BatchEntry_t; // positive, batch size, interval, location
        typedef std::vector<BatchEntry_t> BatchEntryContainer_t;
        typedef std::pair<unsigned int, unsigned int> BatchIntervalTotals_t;
        typedef std::vector<BatchIntervalTotals_t> BatchIntervalTotalsContainer_t;

    protected:
        const CSaTScanData& _data_hub;
        BatchEntryContainer_t _batch_entries;

        boost::dynamic_bitset<> _positive_batches_indexes;
        measure_t _total_negative_batches;
        measure_t _total_positive_batches;
        std::vector<measure_t> _sum_negative_batches_by_time;
        std::vector<measure_t> _sum_positive_batches_by_time;
        std::vector<boost::dynamic_bitset<>> _positive_batches_indexes_by_time;

        void randomize(const RealDataSet& RealSet, DataSet& SimSet);
        void randomizePurelyTemporal(const RealDataSet& RealSet, DataSet& SimSet);
        void randomizeStratified(const RealDataSet& RealSet, DataSet& SimSet);
        boost::dynamic_bitset<>& randomizePositiveBatches(
            const BatchEntryContainer_t& batches, boost::dynamic_bitset<>& positiveBatches, 
            unsigned int totalPositive, unsigned int sumBatchSizes, boost::optional<int> interval=boost::none
        );

    public:
        BatchedRandomizer(const CSaTScanData& data_hub, long lInitialSeed = RandomNumberGenerator::glDefaultSeed);
        virtual ~BatchedRandomizer() {}

        virtual BatchedRandomizer * Clone() const { return new BatchedRandomizer(*this); }

        void AddBatch(tract_t tid, count_t batchCount, Julian nDate, count_t batchSize, count_t positive);
        void AssignFromAttributes(RealDataSet& RealSet);

        const BatchEntryContainer_t& getBatches() const { return _batch_entries; }
        BatchEntryContainer_t& getPositiveBatches(BatchEntryContainer_t& batches, const boost::dynamic_bitset<>& batchIndexesOfInterest) const;
        BatchEntryContainer_t& getPositiveBatchesForInterval(BatchEntryContainer_t& batches, const boost::dynamic_bitset<>& batchIndexesOfInterest, int interval) const;
        BatchEntryContainer_t& getBatchesInSet(BatchEntryContainer_t& batches, const boost::dynamic_bitset<>& batchIndexesOfInterest) const;
        BatchEntryContainer_t& getBatchesInSetForInterval(BatchEntryContainer_t& batches, const boost::dynamic_bitset<>& batchIndexesOfInterest, int interval) const;
        BatchEntryContainer_t& getClusterBatches(const CCluster& cluster, BatchEntryContainer_t& batches) const;
        const boost::dynamic_bitset<>& getPositiveIndexesOfRandomization() const { return _positive_batches_indexes; }

        measure_t getSumNegativeBatches() const { return _total_negative_batches; }
        measure_t getSumPositiveBatches() const { return _total_positive_batches; }
        const std::vector<measure_t> & getSumNegativeBatchesByTime() const { return _sum_negative_batches_by_time; }
        const std::vector<measure_t> & getSumPositiveBatchesByTime() const { return _sum_positive_batches_by_time; }
        const std::vector<boost::dynamic_bitset<>>& getPositiveBatchesByTime() const { return _positive_batches_indexes_by_time; }

        virtual void RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation);
        virtual void RemoveCase(int iTimeInterval, tract_t tTractIndex);
};
//******************************************************************************
#endif
