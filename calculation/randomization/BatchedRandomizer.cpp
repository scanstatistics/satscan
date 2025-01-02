//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "BatchedRandomizer.h"
#include "SSException.h"
#include "SaTScanData.h"
#include "cluster.h"

//********** BatchedRandomizer ***********

/** constructor */
BatchedRandomizer::BatchedRandomizer(const CSaTScanData& data_hub, long lInitialSeed)
    :AbstractDenominatorDataRandomizer(lInitialSeed), _data_hub(data_hub), _total_negative_batches(0), _total_positive_batches(0) {
    if (_data_hub.GetParameters().GetIsSpaceTimeAnalysis() && _data_hub.GetParameters().GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION) {
        //_batch_entries_by_time.reset(new BatchEntryByTime_t(_data_hub.GetNumTimeIntervals()));
    }
}

/** Adds batch record to collection of batches. */
void BatchedRandomizer::AddBatch(tract_t tid, count_t batchCount, Julian date, count_t batchSize, count_t positive) {
    int intervalIdx = _data_hub.GetTimeIntervalOfDate(date);
    for (count_t t=0; t < batchCount; ++t) {
        _batch_entries.push_back(
            BatchEntry_t(static_cast<unsigned short>(positive), static_cast<unsigned int>(batchSize), intervalIdx, tid)
        );
    }
}

/* Allocates and assigns data strutures of RealDataSet object from randomizer data. */
void BatchedRandomizer::AssignFromAttributes(RealDataSet& RealSet) {
    count_t totalPositive=0, ** ppCases = RealSet.allocateCaseData().GetArray(); // number of positive batches by interval and location
    measure_t ** ppMeasure = RealSet.allocateMeasureData().GetArray(); // number of batches by interval and location
    measure_t sumPosBatchSizes=0, ** ppMeasureAux = RealSet.allocateMeasureData_Aux().GetArray(); // totaled negative batch sizes by interval and location
    measure_t sumNegBatchSizes = 0, ** ppMeasureAux2 = RealSet.allocateMeasureData_Aux2().GetArray(); // totaled positive batch sizes by interval and location
    boost::dynamic_bitset<> ** ppBatches = RealSet.allocateBatchData(_batch_entries.size()).GetArray();
    boost::dynamic_bitset<> ** ppPositiveBatches = RealSet.allocatePositiveBatchData(_batch_entries.size()).GetArray();

    for (unsigned int t = 0; t < _batch_entries.size(); ++t) {
        const auto& be = _batch_entries[t];
        ppCases[be.get<2>()][be.get<3>()] += be.get<0>() * 1;
        ppMeasure[be.get<2>()][be.get<3>()] += 1;
        if (be.get<0>()) {
            totalPositive += 1;
            sumPosBatchSizes += be.get<1>();
            ppMeasureAux2[be.get<2>()][be.get<3>()] += be.get<1>();
            ppPositiveBatches[be.get<2>()][be.get<3>()].set(t); // mark postive batch at time and location
        } else {
            sumNegBatchSizes += be.get<1>();
            ppMeasureAux[be.get<2>()][be.get<3>()] += be.get<1>();
        }
        ppBatches[be.get<2>()][be.get<3>()].set(t); // mark batch at time and location
    }
    // set totals
    RealSet.setTotalCases(totalPositive);
    RealSet.setTotalMeasure(static_cast<measure_t>(_batch_entries.size()));
    RealSet.setTotalPopulation(static_cast<measure_t>(_batch_entries.size()));
    RealSet.setTotalMeasureAux(sumNegBatchSizes);
    RealSet.setTotalMeasureAux2(sumPosBatchSizes);
    // set as cumulative with respect to time intervals
    int iNumTracts = RealSet.getLocationDimension(), iNumTimeIntervals = RealSet.getIntervalDimension();
    for (int tTract = 0; tTract < iNumTracts; ++tTract) {
        for (int i = iNumTimeIntervals - 2; i >= 0; --i) {
            ppMeasure[i][tTract] = ppMeasure[i + 1][tTract] + ppMeasure[i][tTract];
            ppCases[i][tTract] = ppCases[i + 1][tTract] + ppCases[i][tTract];
            ppMeasureAux[i][tTract] = ppMeasureAux[i + 1][tTract] + ppMeasureAux[i][tTract];
            ppMeasureAux2[i][tTract] = ppMeasureAux2[i + 1][tTract] + ppMeasureAux2[i][tTract];
            ppBatches[i][tTract] |= ppBatches[i + 1][tTract];
            ppPositiveBatches[i][tTract] |= ppPositiveBatches[i + 1][tTract];
        }
    }
}

/** Returns batch entries in cluster with respect to space and time. */
BatchedRandomizer::BatchEntryContainer_t& BatchedRandomizer::getClusterBatches(const CCluster& cluster, BatchEntryContainer_t& batches) const {
    std::vector<tract_t> indexes;
    cluster.getIdentifierIndexes(_data_hub, indexes, true);
    batches.clear();
    for (const auto& be : _batch_entries) {
        if (cluster.m_nFirstInterval <= be.get<2>() && be.get<2>() < cluster.m_nLastInterval &&  std::find(indexes.begin(), indexes.end(), be.get<3>()) != indexes.end()) {
            batches.push_back(be);
        }
    }
    return batches;
}

/* Returns collection of positive batches in the set defined by batchIndexesOfInterest, ensuring that index actually references a positive batch. */
BatchedRandomizer::BatchEntryContainer_t& BatchedRandomizer::getPositiveBatches(BatchEntryContainer_t& batches, const boost::dynamic_bitset<>& batchIndexesOfInterest) const {
    batches.clear();
    for (boost::dynamic_bitset<>::size_type b = batchIndexesOfInterest.find_first(); b != boost::dynamic_bitset<>::npos; b = batchIndexesOfInterest.find_next(b)) {
        const auto& be = _batch_entries[b];
        if (be.get<0>())
            batches.push_back(be);
    }
    return batches;
}

/* Returns collection of positive batches in the set defined by batchIndexesOfInterest and time interval, ensuring that index actually references a positive batch. */
BatchedRandomizer::BatchEntryContainer_t& BatchedRandomizer::getPositiveBatchesForInterval(BatchEntryContainer_t& batches, const boost::dynamic_bitset<>& batchIndexesOfInterest, int interval) const {
    batches.clear();
    for (boost::dynamic_bitset<>::size_type b = batchIndexesOfInterest.find_first(); b != boost::dynamic_bitset<>::npos; b = batchIndexesOfInterest.find_next(b)) {
        const auto& be = _batch_entries[b];
        if (be.get<0>() && be.get<2>() == interval)
            batches.push_back(be);
    }
    return batches;
}

/* Returns collection of positive batches in the set defined by batchIndexesOfInterest. */
BatchedRandomizer::BatchEntryContainer_t& BatchedRandomizer::getBatchesInSet(BatchEntryContainer_t& batches, const boost::dynamic_bitset<>& batchIndexesOfInterest) const {
    batches.clear();
    for (boost::dynamic_bitset<>::size_type b = batchIndexesOfInterest.find_first(); b != boost::dynamic_bitset<>::npos; b = batchIndexesOfInterest.find_next(b)) {
        batches.push_back(_batch_entries[b]);
    }
    return batches;
}

/* Returns collection of positive batches in the set defined by batchIndexesOfInterest and restricted to time interval. */
BatchedRandomizer::BatchEntryContainer_t& BatchedRandomizer::getBatchesInSetForInterval(BatchEntryContainer_t& batches, const boost::dynamic_bitset<>& batchIndexesOfInterest, int interval) const {
    batches.clear();
    for (boost::dynamic_bitset<>::size_type b = batchIndexesOfInterest.find_first(); b != boost::dynamic_bitset<>::npos; b = batchIndexesOfInterest.find_next(b)) {
        const auto& be = _batch_entries[b];
        if (be.get<2>() == interval)
            batches.push_back(be);
    }
    return batches;
}

/** Randomizes simulation data set. */
void BatchedRandomizer::randomize(const RealDataSet& RealSet, DataSet& SimSet) {
    // randomize which batches are positive and record in bitset.
    randomizePositiveBatches(
        _batch_entries, _positive_batches_indexes, static_cast<unsigned int>(RealSet.getTotalCases()),
        static_cast<unsigned int>(RealSet.getTotalMeasureAux() + RealSet.getTotalMeasureAux2())
    );
    // Clear data from last simulation.
    SimSet.getCaseData().Set(0);
    SimSet.getMeasureData_Aux().Set(0);
    SimSet.getMeasureData_Aux2().Set(0);
    SimSet.getPositiveBatchData().Set(boost::dynamic_bitset<>(_batch_entries.size()));
    // Assign randomized data to SimSet object.
    unsigned int tNumTracts = RealSet.getLocationDimension(), tNumTimeIntervals = RealSet.getIntervalDimension();
    count_t ** ppSimCases = SimSet.getCaseData().GetArray();
    measure_t ** ppMeasureAux = SimSet.allocateMeasureData_Aux().GetArray(); // totaled negative batch sizes by interval and location
    measure_t ** ppMeasureAux2 = SimSet.allocateMeasureData_Aux2().GetArray(); // totaled positive batch sizes by interval and location
    boost::dynamic_bitset<>** ppPositiveBatches = SimSet.allocatePositiveBatchData(_batch_entries.size()).GetArray();
    _total_negative_batches = 0;
    _total_positive_batches = 0;
    for (unsigned int t = 0; t < _batch_entries.size(); ++t) {
        const auto& be = _batch_entries[t];
        if (_positive_batches_indexes.test(t)) {
            ++ppSimCases[be.get<2>()][be.get<3>()];
            ppMeasureAux2[be.get<2>()][be.get<3>()] += be.get<1>();
            ppPositiveBatches[be.get<2>()][be.get<3>()].set(t);
            _total_positive_batches += be.get<1>();
        } else {
            ppMeasureAux[be.get<2>()][be.get<3>()] += be.get<1>();
            _total_negative_batches += be.get<1>();
        }
    }
    // set as cumulative with respect to time intervals
    for (int tTract = 0; tTract < RealSet.getLocationDimension(); ++tTract) {
        for (int i = RealSet.getIntervalDimension() - 2; i >= 0; --i) {
            ppSimCases[i][tTract] = ppSimCases[i + 1][tTract] + ppSimCases[i][tTract];
            ppMeasureAux[i][tTract] = ppMeasureAux[i + 1][tTract] + ppMeasureAux[i][tTract];
            ppMeasureAux2[i][tTract] = ppMeasureAux2[i + 1][tTract] + ppMeasureAux2[i][tTract];
            ppPositiveBatches[i][tTract] |= ppPositiveBatches[i + 1][tTract];
        }
    }
}

/** Randomizes purely temporal simulation data set. */
void BatchedRandomizer::randomizePurelyTemporal(const RealDataSet& RealSet, DataSet& SimSet) {
    // randomize which batches are positive and record in bitset.
    randomizePositiveBatches(
        _batch_entries, _positive_batches_indexes, static_cast<unsigned int>(RealSet.getTotalCases()),
        static_cast<unsigned int>(RealSet.getTotalMeasureAux() + RealSet.getTotalMeasureAux2())
    );
    // Assign randomized data to SimSet object.
    count_t * pSimCases = SimSet.allocateCaseData_PT();
    measure_t * pMeasureAux = SimSet.allocateMeasureData_PT_Aux(); // totaled negative batch sizes by interval and location
    measure_t * pMeasureAux2 = SimSet.allocateMeasureData_PT_Aux2(); // totaled positive batch sizes by interval and location
    boost::dynamic_bitset<> * pPositiveBatches = SimSet.allocatePositiveBatchData_PT(_batch_entries.size());
    _total_negative_batches = 0;
    _total_positive_batches = 0;
    for (unsigned int t = 0; t < _batch_entries.size(); ++t) {
        const auto& be = _batch_entries[t];
        if (_positive_batches_indexes.test(t)) {
            ++pSimCases[be.get<2>()];
            pMeasureAux2[be.get<2>()] += be.get<1>();
            pPositiveBatches[be.get<2>()].set(t);
            _total_positive_batches += be.get<1>();
        } else {
            pMeasureAux[be.get<2>()] += be.get<1>();
            _total_negative_batches += be.get<1>();
        }
    }
    // set as cumulative with respect to time intervals
    for (int i = RealSet.getIntervalDimension() - 2; i >= 0; --i) {
        pSimCases[i] = pSimCases[i + 1] + pSimCases[i];
        pMeasureAux[i] = pMeasureAux[i + 1] + pMeasureAux[i];
        pMeasureAux2[i] = pMeasureAux2[i + 1] + pMeasureAux2[i];
        pPositiveBatches[i] |= pPositiveBatches[i + 1];
    }
}

/** Randomizes simulation data set when applying time stratified temporal adjustment. */
void BatchedRandomizer::randomizeStratified(const RealDataSet& RealSet, DataSet& SimSet) {
    // Randomized data by time intervals
    _positive_batches_indexes.reset();
    _positive_batches_indexes.resize(_batch_entries.size());
    boost::dynamic_bitset<> pos_batches_indexes;
    count_t * pt_cases = RealSet.getCaseData_PT_NC();
    measure_t * measure_aux = RealSet.getMeasureData_PT_Aux(), * measure_aux2 = RealSet.getMeasureData_PT_Aux2();
    for (int i = 0; i < RealSet.getIntervalDimension(); ++i) {
        if (pt_cases[i]) {
            randomizePositiveBatches(
                _batch_entries, pos_batches_indexes, pt_cases[i], 
                (measure_aux[i] - measure_aux[i + 1]) + (measure_aux2[i] - measure_aux2[i + 1]), i
            );
            _positive_batches_indexes |= pos_batches_indexes;
        }
    }
    // Clear data from last simulation.
    SimSet.getCaseData().Set(0);
    SimSet.getMeasureData_Aux().Set(0);
    SimSet.getMeasureData_Aux2().Set(0);
    SimSet.getPositiveBatchData().Set(boost::dynamic_bitset<>(_batch_entries.size()));
    // Assign randomized data to SimSet object.
    count_t** ppSimCases = SimSet.getCaseData().GetArray();
    measure_t ** ppMeasureAux = SimSet.allocateMeasureData_Aux().GetArray(); // totaled negative batch sizes by interval and location
    measure_t ** ppMeasureAux2 = SimSet.allocateMeasureData_Aux2().GetArray(); // totaled positive batch sizes by interval and location
    boost::dynamic_bitset<>** ppPositiveBatches = SimSet.allocatePositiveBatchData(_batch_entries.size()).GetArray();
    _total_negative_batches = 0;
    _total_positive_batches = 0;
    _sum_negative_batches_by_time.clear();
    _sum_negative_batches_by_time.resize(RealSet.getIntervalDimension() + 1);
    _sum_positive_batches_by_time.clear();
    _sum_positive_batches_by_time.resize(RealSet.getIntervalDimension() + 1);
    _positive_batches_indexes_by_time.clear();
    _positive_batches_indexes_by_time.resize(RealSet.getIntervalDimension() + 1, boost::dynamic_bitset<>(RealSet.getTotalMeasure()));
    for (unsigned int t = 0; t < _batch_entries.size(); ++t) {
        const auto& be = _batch_entries[t];
        if (_positive_batches_indexes.test(t)) {
            ++ppSimCases[be.get<2>()][be.get<3>()];
            ppMeasureAux2[be.get<2>()][be.get<3>()] += be.get<1>();
            ppPositiveBatches[be.get<2>()][be.get<3>()].set(t);
            _total_positive_batches += be.get<1>();
            _sum_positive_batches_by_time[be.get<2>()] += be.get<1>();
            _positive_batches_indexes_by_time[be.get<2>()].set(t);
        } else {
            ppMeasureAux[be.get<2>()][be.get<3>()] += be.get<1>();
            _total_negative_batches += be.get<1>();
            _sum_negative_batches_by_time[be.get<2>()] += be.get<1>();
        }
    }
    // set as cumulative with respect to time intervals
    for (int tTract = 0; tTract < RealSet.getLocationDimension(); ++tTract) {
        for (int i = RealSet.getIntervalDimension() - 2; i >= 0; --i) {
            ppSimCases[i][tTract] = ppSimCases[i + 1][tTract] + ppSimCases[i][tTract];
            ppMeasureAux[i][tTract] = ppMeasureAux[i + 1][tTract] + ppMeasureAux[i][tTract];
            ppMeasureAux2[i][tTract] = ppMeasureAux2[i + 1][tTract] + ppMeasureAux2[i][tTract];
            ppPositiveBatches[i][tTract] |= ppPositiveBatches[i + 1][tTract];
        }
    }
    for (int i = RealSet.getIntervalDimension() - 2; i >= 0; --i)
        _positive_batches_indexes_by_time[i] |= _positive_batches_indexes_by_time[i + 1];
}

/** Randomly assigns which batches are positive. */
boost::dynamic_bitset<>& BatchedRandomizer::randomizePositiveBatches(
    const BatchEntryContainer_t& batches, boost::dynamic_bitset<>& positiveBatches, 
    unsigned int totalPositive, unsigned int sumBatchSizes, boost::optional<int> interval
) {
    positiveBatches.reset();
    positiveBatches.resize(batches.size());
    boost::dynamic_bitset<> negativeBatches(batches.size());
    count_t UNEXT = 0, UNOW = totalPositive; // number of unassigned positive traps in current iteration 
    unsigned int SS = sumBatchSizes; // total size of traps that has not yet assigned as positive
    unsigned int AS = 0; // size of the traps that have been assigned as positive

    do {
        // Create vector of UNOW random numbers between 0 and SS, sorting from least to greatest.
        std::vector<double> random(UNOW, 0);
        for (auto& r : random) r = gRandomNumberGenerator.GetRandomDouble() * SS;
        std::sort(random.begin(), random.end(), std::less<double>());
        random.push_back(SS);
        UNEXT = 0; // number of unassigned positive traps for next iteration
        unsigned int CUMSIZE = 0; // cumulative trap sizes of unassigned traps when iterating through the loop
        negativeBatches = positiveBatches.operator~(); // create the collection of batches which are still negative
        auto itrRand = random.begin(), itrLast = random.begin();
        for (auto idx = negativeBatches.find_first(); idx != boost::dynamic_bitset<>::npos; idx = negativeBatches.find_next(idx)) {
            auto& be = batches[idx];
            // If calling this process for a specific time interval and current batch isn't for that interval, skip record.
            if (interval && interval.get() != be.get<2>())
                continue;
            CUMSIZE += be.get<1>();
            if (*itrRand >= CUMSIZE) continue; // skip to the next until random number is less than cumulation
            unsigned int X2 = 0;
            while (*itrRand < CUMSIZE) { //while (*itrRand <= CUMSIZE) {
                ++itrRand;
                ++X2;
                if (itrRand == random.end())
                    throw prg_error("Unexpectedly reached the end of random array.", "randomize()");
            }
            unsigned int X = X2; //std::distance(itrLast, itrRand) + 1; // count number of random numbers evaluated
            itrLast = itrRand;
            if (X == 1)
                positiveBatches.set(idx);
            else if (X > 1) {
                positiveBatches.set(idx);
                UNEXT += X - 1;
            }
            if (X > 0)
                AS += be.get<1>(); // add batch size to accumulation
        }
        if (UNEXT > 0) {
            UNOW = UNEXT;
            SS = sumBatchSizes - AS;
        }
    } while (UNEXT > 0);

    // check that the number of positve traps is correct
    if (positiveBatches.count() != static_cast<size_t>(totalPositive))
        throw prg_error("The number of positive batches is incorrect, expecting %i, got %u.", "randomize()", totalPositive, positiveBatches.count());
    return positiveBatches;
}

/** Creates randomized under the null hypothesis for Poisson model, assigning data to DataSet objects structures.
    Random number generator seed initialized based upon 'iSimulation' index. */
void BatchedRandomizer::RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation) {
    SetSeed(iSimulation, SimSet.getSetIndex());
    if (_data_hub.GetParameters().GetIsPurelyTemporalAnalysis())
        randomizePurelyTemporal(RealSet, SimSet);
    else if (_data_hub.GetParameters().GetIsSpaceTimeAnalysis() && _data_hub.GetParameters().GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION)
        randomizeStratified(RealSet, SimSet);
    else
        randomize(RealSet, SimSet);
}

/** Removes all batches associated with interval and location. */
void BatchedRandomizer::RemoveCase(int iTimeInterval, tract_t tTractIndex) {
    for (auto itr = _batch_entries.begin(); itr != _batch_entries.end();) {
        if (itr->get<2>() == iTimeInterval && itr->get<3>() == tTractIndex)
            itr = _batch_entries.erase(itr);
        else
            ++itr;
    }
}