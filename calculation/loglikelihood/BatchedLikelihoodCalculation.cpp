//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "BatchedLikelihoodCalculation.h"
#include "SaTScanData.h"
#include "cluster.h"
#include "BatchedClusterData.h"

/** constructor */
BatchedLikelihoodCalculator::BatchedLikelihoodCalculator(const CSaTScanData& Data):AbstractLikelihoodCalculator(Data) {
    // Calculate the log-likelihoods for each data set under the null.
    if (Data.GetParameters().GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION) {
        _probabilities_cache.resize(Data.GetDataSetHandler().GetNumDataSets());
        // we need the probability and log-likelihood by adjustment range
        if (Data.GetParameters().isTimeStratifiedWithLargerAdjustmentLength()) {
            _log_likelihoods_by_range.reset(new TwoDimensionArrayHandler<std::pair<double, double>>(
                Data.GetDataSetHandler().GetNumDataSets(), Data.getTimeStratifiedTemporalAdjustmentWindows().size()
            ));
        } else {// we need the probability and log-likelihood by time interval
            _log_likelihoods_by_time.reset(new TwoDimensionArrayHandler<std::pair<double, double>>(
                Data.GetDataSetHandler().GetNumDataSets(), Data.GetNumTimeIntervals() + 1
            ));
        }
    }
    for (size_t t = 0; t < Data.GetDataSetHandler().GetNumDataSets(); ++t) {
        const auto& dataset = Data.GetDataSetHandler().GetDataSet(t);
        const BatchedRandomizer* randomizer = dynamic_cast<const BatchedRandomizer*>(Data.GetDataSetHandler().GetRandomizer(t));
        boost::dynamic_bitset<> positiveBatchIndexes(randomizer->getBatches().size());
        positiveBatchIndexes.flip(); // toggle all on - we'll ensure only positive batches are included through randomizer call.
        BatchedRandomizer::BatchEntryContainer_t positiveBatches;
        _probabilities.push_back(probabilityPositive(
            dataset.getTotalCases(), dataset.getTotalMeasure(),
            dataset.getTotalMeasureAux2(),/* sum of the batch sizes for positive batches */
            dataset.getTotalMeasureAux(),/* sum of the batch sizes for negative batches */
            randomizer->getPositiveBatches(positiveBatches, positiveBatchIndexes)
        ));
        _log_likelihoods.push_back(LL(1 - _probabilities.back(), dataset.getTotalMeasureAux(), positiveBatches));
        if (Data.GetParameters().GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION) {
            if (Data.GetParameters().isTimeStratifiedWithLargerAdjustmentLength()) {
                for (size_t i = 0; i < Data.getTimeStratifiedTemporalAdjustmentWindows().size(); ++i) {
                    const auto& range = Data.getTimeStratifiedTemporalAdjustmentWindows()[i];
                    double ptime = probabilityPositive(
                        dataset.getCaseData_PT()[range.first] - dataset.getCaseData_PT()[range.second + 1],
                        dataset.getMeasureData_PT()[range.first] - dataset.getMeasureData_PT()[range.second + 1],
                        dataset.getMeasureData_PT_Aux2()[range.first] - dataset.getMeasureData_PT_Aux2()[range.second + 1],
                        dataset.getMeasureData_PT_Aux()[range.first] - dataset.getMeasureData_PT_Aux()[range.second + 1],
                        randomizer->getPositiveBatchesForRange(positiveBatches, positiveBatchIndexes, range.first, range.second, true)
                    );
                    _log_likelihoods_by_range->GetArray()[t][i] = std::make_pair(ptime,
                        LL(1 - ptime, dataset.getMeasureData_PT_Aux()[range.first] - dataset.getMeasureData_PT_Aux()[range.second + 1], positiveBatches)
                    );
                }
            } else {
                for (int i = 0; i < Data.GetNumTimeIntervals(); ++i) {
                    double ptime = probabilityPositive(
                        dataset.getCaseData_PT_NC()[i], dataset.getMeasureData_PT_NC()[i],
                        dataset.getMeasureData_PT_Aux2()[i] - dataset.getMeasureData_PT_Aux2()[i + 1],/* sum of the batch sizes for positive batches */
                        dataset.getMeasureData_PT_Aux()[i] - dataset.getMeasureData_PT_Aux()[i + 1],/* sum of the batch sizes for negative batches */
                        randomizer->getPositiveBatchesForInterval(positiveBatches, positiveBatchIndexes, i)
                    );
                    _log_likelihoods_by_time->GetArray()[t][i] = std::make_pair(ptime, LL(1 - ptime, dataset.getMeasureData_PT_Aux()[i] - dataset.getMeasureData_PT_Aux()[i + 1], positiveBatches));
                }
            }
        }
    }
    _always_full_statistic = Data.GetNumDataSets() > 1 || Data.GetParameters().GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION;
}

/** Associates collection of data set randomizers with this log-likelihood object. 
    The simulations alter which batches are positive so we need to access data through the randomizer objects. */
void BatchedLikelihoodCalculator::associateRandomizers(boost::shared_ptr<RandomizerContainer_t> rc) { 
    _randomizer_container = rc;
}

/* log-likelihood calculation
   q = probability of being negative
   Sn = the sum of the negative batch sizes
   positiveBatches = positive batches in area  of interest
*/
double BatchedLikelihoodCalculator::LL(double q, double Sn, const BatchedRandomizer::BatchEntryContainer_t& positiveBatches) const {
    if (q == 0.0) return 0.0; // special case
    if (q == 1.0) return 0.0; // special case
    double Sc = 0.0;
    for (const auto& be : positiveBatches)
        Sc += log(1.0 - std::pow(q, be.get<1>()));
    return log(q) * Sn + Sc;
}

/* Calculate p = probability that a mosquito is positive (in area of interest).
   C = number of positive traps 
   N = number of traps
   Sc = sum of the trap sizes for positive batches
   Sn = sum of the trap sizes for negative batches
   positiveBatches = positive batches
*/
double BatchedLikelihoodCalculator::probabilityPositive(measure_t C, measure_t N, measure_t Sc, measure_t Sn, const BatchedRandomizer::BatchEntryContainer_t& positiveBatches) const {
    if (C == 0) return 0.0; // special case
    if (C == N) return 1.0; // special case
    // Initial values for p1, p2, p3, p4
    double p1 = C / (Sn + Sc), p2 = 2.0 * C / (Sn + Sc), p3 = 3.0 * C / (Sn + Sc), p4 = 4.0 * C / (Sn + Sc);
    p1 *= 0.999; // Dealing with special case.
    if (p4 >= 1.0) { // Dealing with special case.
        p4 = 0.9999;
        p3 = (2.0 + p1) / 3.0;
        p2 = (1.0 + 2.0 * p1) / 3.0;
    }
    // LOOP #1:
    std::vector<double> pLL = { LL(1 - p1, Sn, positiveBatches), LL(1 - p2, Sn, positiveBatches), LL(1 - p3, Sn, positiveBatches), LL(1 - p4, Sn, positiveBatches) };
    auto it = std::max_element(pLL.begin(), pLL.end());
    while (it == (pLL.end() - 1)) { //LL(1 - p4) is the largest
        p1 = p3;
        p2 = p4;
        p3 = p4 + C / (Sn + Sc);
        p4 = p4 + 2.0 * C / (Sn + Sc);
        pLL = { LL(1 - p1, Sn, positiveBatches), LL(1 - p2, Sn, positiveBatches), LL(1 - p3, Sn, positiveBatches), LL(1 - p4, Sn, positiveBatches) };
        it = std::max_element(pLL.begin(), pLL.end());
    }
    // LOOP #2:
    pLL = { LL(1 - p1, Sn, positiveBatches), LL(1 - p2, Sn, positiveBatches), LL(1 - p3, Sn, positiveBatches), LL(1 - p4, Sn, positiveBatches) };
    it = std::max_element(pLL.begin(), pLL.end());
    while (it == pLL.begin()) { // LL(1-p1) is the largest
        p1 = p1;
        p4 = p2;
        p3 = p1 + 2.0 * (p2 - p1) / 3.0;
        p2 = p1 + (p2 - p1) / 3.0;
        pLL = { LL(1 - p1, Sn, positiveBatches), LL(1 - p2, Sn, positiveBatches), LL(1 - p3, Sn, positiveBatches), LL(1 - p4, Sn, positiveBatches) };
        it = std::max_element(pLL.begin(), pLL.end());
    }
    // LOOP #3:
    pLL = { LL(1 - p1, Sn, positiveBatches), LL(1 - p2, Sn, positiveBatches), LL(1 - p3, Sn, positiveBatches), LL(1 - p4, Sn, positiveBatches) };
    while (std::abs(pLL[1] - pLL[2]) > 0.0001) { // note: convergence near 15 decimal places cause round-off problems
        it = std::max_element(pLL.begin(), pLL.end());
        if (it == pLL.begin() + 1) { // LL(1-p2) is the largest
            p4 = p3;
            p3 = (p3 + p2) / 2.0;
            //p3 = p2 + (p3 - p2) / 2.0; // value of 2.0 can be optimized, ex. 3.0
            p2 = p1 + (p2 - p1) / 2.0; // value of 2.0 can be optimized, ex. 1.5
            pLL[1] = LL(1 - p2, Sn, positiveBatches);
            pLL[2] = LL(1 - p3, Sn, positiveBatches);
            pLL[3] = LL(1 - p4, Sn, positiveBatches);
        } else if (it == pLL.begin() + 2) { // LL(1-p3) is the largest
            p1 = p2;
            p2 = p2 + (p3 - p2) / 2.0; // value of 2.0 can be optimized, ex. 1.5
            p3 = p3 + (p4 - p3) / 2.0; // value of 2.0 can be optimized, ex. 3.0
            pLL[0] = LL(1 - p1, Sn, positiveBatches);
            pLL[1] = LL(1 - p2, Sn, positiveBatches);
            pLL[2] = LL(1 - p3, Sn, positiveBatches);
        } else
            throw prg_error(
                "Unexpected situation: p1=%g, LL=%g\n p2=%g, LL=%g\n p3=%g, LL=%g\n p4=%g, LL=%g", 
                "probabilityPositive()", p1, pLL[0], p2, pLL[1], p3, pLL[2], p4, pLL[3]
            );
    }
    // return the probability associated with the maximum log-likelihood
    it = std::max_element(pLL.begin(), pLL.end());
    if (it == pLL.begin()) return p1;
    else if (it == pLL.begin() + 1) return p2;
    else if (it == pLL.begin() + 2) return p3;
    else return p4;
}

/* Returns the probability under the null for time interval. */
double BatchedLikelihoodCalculator::getProbabilityForInterval(int interval, size_t tSetIndex) const {
    if (gDataHub.GetParameters().GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION) {
        if (gDataHub.GetParameters().isTimeStratifiedWithLargerAdjustmentLength()) {
            auto batchRange = std::find_if(
                gDataHub.getTimeStratifiedTemporalAdjustmentWindows().begin(),
                gDataHub.getTimeStratifiedTemporalAdjustmentWindows().end(),
                [interval](const WindowRange_t& range) { return range.first <= interval && interval <= range.second; }
            );
            auto pos = std::distance(gDataHub.getTimeStratifiedTemporalAdjustmentWindows().begin(), batchRange);
            return _log_likelihoods_by_range->GetArray()[tSetIndex][pos].first;
        }
        return _log_likelihoods_by_time->GetArray()[tSetIndex][interval].first;
    }
    return _probabilities[tSetIndex];
}

/** Returns the expected number of cases for cluster data when reporting. */
double BatchedLikelihoodCalculator::getClusterExpected(const CCluster& cluster, size_t tSetIndex) const {
    const BatchedRandomizer* randomizer = dynamic_cast<const BatchedRandomizer*>(gDataHub.GetDataSetHandler().GetRandomizer(tSetIndex));
    if (!randomizer) throw prg_error("Unable to cast to BatchedRandomizer", "getClusterExpected()");
    if (cluster.GetClusterType() == SPACETIMECLUSTER && gDataHub.GetParameters().GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION) {
        const BatchedRandomizer* randomizer = dynamic_cast<const BatchedRandomizer*>(gDataHub.GetDataSetHandler().GetRandomizer(tSetIndex));
        const AbstractBatchedClusterData* clusterData = dynamic_cast<const AbstractBatchedClusterData*>(cluster.GetClusterData());
        double Sk = 0.0;
        BatchedRandomizer::BatchEntryContainer_t batches;
        for (const auto& be : randomizer->getBatchesInSet(batches, clusterData->GetBatches(tSetIndex)))
            Sk += std::pow(1.0 - getProbabilityForInterval(be.get<2>(), tSetIndex), be.get<1>());
        return static_cast<double>(clusterData->GetBatches(tSetIndex).count()) - Sk;
    } else {
        BatchedRandomizer::BatchEntryContainer_t batches;
        double Sk = 0.0, qall = 1.0 - _probabilities[tSetIndex];
        for (const auto& be : randomizer->getClusterBatches(cluster, batches))
            Sk += std::pow(qall, be.get<1>());
        return cluster.GetClusterData()->GetMeasure(tSetIndex) - Sk;
    }
}

/** Returns the expected number of cases in window range. This is a helper function used in TemporalChartGenerator class. */
double BatchedLikelihoodCalculator::getExpectedInWindow(int windowStart, int windowEnd, size_t tSetIndex) const {
    const BatchedRandomizer* randomizer = dynamic_cast<const BatchedRandomizer*>(gDataHub.GetDataSetHandler().GetRandomizer(tSetIndex));
    if (!randomizer) throw prg_error("Unable to cast to BatchedRandomizer", "getExpectedInWindow()");
    double Sk = 0.0, batchCount = 0.0;
    for (auto& be : randomizer->getBatches()) {
        if (windowStart <= be.get<2>() && be.get<2>() < windowEnd) {
            Sk += std::pow(1.0 - getProbabilityForInterval(be.get<2>(), tSetIndex), be.get<1>());
            ++batchCount;
        }
    }
    return batchCount - Sk;
}

/** Returns the expected number of cases in window range for locations in window. This is a helper function used in TemporalChartGenerator class. */
double BatchedLikelihoodCalculator::getExpectedInWindow(int windowStart, int windowEnd, const std::vector<tract_t>& locationIdexes, size_t tSetIndex) const {
    const BatchedRandomizer* randomizer = dynamic_cast<const BatchedRandomizer*>(gDataHub.GetDataSetHandler().GetRandomizer(tSetIndex));
    if (!randomizer) throw prg_error("Unable to cast to BatchedRandomizer", "getExpectedInWindow()");
    measure_t Sk = 0.0, batchCount = 0.0;
    for (auto& be : randomizer->getBatches()) {
        if (windowStart <= be.get<2>()  && be.get<2>() < windowEnd && std::find(locationIdexes.begin(), locationIdexes.end(), be.get<3>()) != locationIdexes.end()) {
            Sk += std::pow(1.0 - getProbabilityForInterval(be.get<2>(), tSetIndex), be.get<1>());
            ++batchCount;
        }
    }
    return batchCount - Sk;
}

/** Returns the expected number of cases for cluster data at window when performing the time stratified 
    temporal adjustment and the adjustment length equals the time aggregation length. */
double BatchedLikelihoodCalculator::getClusterExpectedAtWindow(const boost::dynamic_bitset<>& BatchIndexes, int windowIndex, size_t tSetIndex) const {
    const BatchedRandomizer* randomizer = dynamic_cast<const BatchedRandomizer*>(gDataHub.GetDataSetHandler().GetRandomizer(tSetIndex));
    if (!randomizer) throw prg_error("Unable to cast to BatchedRandomizer", "getExpectedInWindow()");
    BatchedRandomizer::BatchEntryContainer_t batches;
    double Sk = 0.0, qall = 1.0 - _log_likelihoods_by_time->GetArray()[tSetIndex][windowIndex].first;
    for (const auto& be : randomizer->getBatchesInSet(batches, BatchIndexes))
        Sk += std::pow(qall, be.get<1>());
    return static_cast<double>(batches.size()) - Sk;
}

/** Returns the expected number of cases for cluster data at window when performing the time stratified
    temporal adjustment and the adjustment length is longer than the time aggregation length. */
double BatchedLikelihoodCalculator::getExpectedForBatches(const boost::dynamic_bitset<>& BatchIndexes, size_t tSetIndex) const {
    const BatchedRandomizer* randomizer = dynamic_cast<const BatchedRandomizer*>(gDataHub.GetDataSetHandler().GetRandomizer(tSetIndex));
    if (!randomizer) throw prg_error("Unable to cast to BatchedRandomizer", "getExpectedForBatches()");
    BatchedRandomizer::BatchEntryContainer_t batches;
    double Sk = 0.0;
    for (const auto& be : randomizer->getBatchesInSet(batches, BatchIndexes))
        Sk += std::pow(1.0 - getProbabilityForInterval(be.get<2>(), tSetIndex), be.get<1>());
    return static_cast<double>(batches.size()) - Sk;
}

/** Returns the relative risk for cluster data. */
double BatchedLikelihoodCalculator::getClusterRelativeRisk(const CCluster& cluster, size_t tSetIndex) const {
    auto probabilities = getProbabilityPositive(cluster, tSetIndex);
    return probabilities.second ? probabilities.first / probabilities.second : -1;
}

/* Returns the probability of being positive inside and outside of cluster data set. */
std::pair<double, double> BatchedLikelihoodCalculator::getProbabilityPositive(const CCluster& cluster, size_t tSetIndex) const {
    auto pClusterData = dynamic_cast<const AbstractBatchedClusterData*>(cluster.GetClusterData());
    if (!pClusterData) throw prg_error("Data object could not casted to AbstractBatchedClusterData type.\n", "getClusterRelativeRisk()");
    double C = cluster.GetClusterData()->GetCaseCount(tSetIndex); // number of positive batches
    double N = cluster.GetClusterData()->GetMeasure(tSetIndex); // number of batches
    double Sc = pClusterData->GetMeasureAux2(tSetIndex); // sum of the trap sizes for positive batches
    double Sn = pClusterData->GetMeasureAux(tSetIndex); // sum of the trap sizes for negative batches
    auto& positiveBatchIndexes = pClusterData->GetPositiveBatches(tSetIndex);
    BatchedRandomizer::BatchEntryContainer_t positiveBatches;
    const BatchedRandomizer* randomizer = dynamic_cast<const BatchedRandomizer*>(gDataHub.GetDataSetHandler().GetRandomizer(tSetIndex));
    double pinside = probabilityPositive(C, N, Sc, Sn, randomizer->getPositiveBatches(positiveBatches, positiveBatchIndexes));
    boost::dynamic_bitset<> positiveBatchIndexesOutside = positiveBatchIndexes;
    double poutside = probabilityPositive(
        gvDataSetTotals[tSetIndex].first - C, gvDataSetTotals[tSetIndex].second - N, 
        gvDataSetMeasureAux2Totals[tSetIndex] - Sc, gvDataSetMeasureAuxTotals[tSetIndex] - Sn, 
        randomizer->getPositiveBatches(positiveBatches, positiveBatchIndexesOutside.flip())
    );
    return std::make_pair(pinside, poutside);
}

/* Calculate the probabilities of being positive inside and outside the area of interest.
   n = number of positive traps
   u = number of traps
   Sc = sum of the trap sizes for positive batches
   Sn = sum of the trap sizes for negative batches
*/
ProbabilitiesAOI& BatchedLikelihoodCalculator::CalculateProbabilities(ProbabilitiesAOI& probabilities, count_t n, measure_t u, measure_t Sc, measure_t Sn, const boost::dynamic_bitset<>& positiveBatchIndexes, size_t tSetIndex) const {
    double C = n; // number of positive batches
    double N = u; // number of batches
    const BatchedRandomizer* randomizer = dynamic_cast<const BatchedRandomizer*>(gDataHub.GetDataSetHandler().GetRandomizer(tSetIndex));
    probabilities._pinside = probabilityPositive(C, N, Sc, Sn, randomizer->getPositiveBatches(probabilities._positive_batches, positiveBatchIndexes));
    probabilities._sn_inside = Sn;
    boost::dynamic_bitset<> positiveBatchIndexesOutside = positiveBatchIndexes;
    probabilities._poutside = probabilityPositive(
        gvDataSetTotals[tSetIndex].first - C, gvDataSetTotals[tSetIndex].second - N, 
        gvDataSetMeasureAux2Totals[tSetIndex] - Sc, gvDataSetMeasureAuxTotals[tSetIndex] - Sn, 
        randomizer->getPositiveBatches(probabilities._positive_batches_outside, positiveBatchIndexesOutside.flip()/* flip to become outside */)
    );
    probabilities._sn_outside = gvDataSetMeasureAuxTotals[tSetIndex] - Sn;
    return probabilities;
}

/* Calculate the probabilities of being positive inside and outside the area of interest, for time interval.
   n = number of positive traps
   u = number of traps
   Sc = sum of the trap sizes for positive batches
   Sn = sum of the trap sizes for negative batches
*/
ProbabilitiesRange_t& BatchedLikelihoodCalculator::CalculateProbabilitiesByTimeInterval(ProbabilitiesRange_t& probabilities, count_t n, measure_t u, measure_t Sc, measure_t Sn, const boost::dynamic_bitset<>& positiveBatchIndexes, int interval, size_t tSetIndex) const {
    // First check cache for already calculated probability for this interval.
    auto& probabilitiesCache = _probabilities_cache[tSetIndex];
    ProbabilitiesCache_t::iterator cachedProbability = probabilitiesCache.find(WindowRange_t(interval, interval));
    if (cachedProbability != probabilitiesCache.end()) {
        probabilities = cachedProbability->second;
        return probabilities;
    }
    // Calculate probabilities.
    probabilities = ProbabilitiesRange_t(new ProbabilitiesRange());
    double C = n; // number of positive batches
    double N = u; // number of batches
    const BatchedRandomizer* randomizer = dynamic_cast<const BatchedRandomizer*>(gDataHub.GetDataSetHandler().GetRandomizer(tSetIndex));
    probabilities->_paoi._pinside = probabilityPositive(C, N, Sc, Sn, randomizer->getPositiveBatches(probabilities->_paoi._positive_batches, positiveBatchIndexes));
    probabilities->_paoi._sn_inside = Sn;
    boost::dynamic_bitset<> positiveBatchIndexesOutside = positiveBatchIndexes;
    // We need totals by time interval
    auto& dataset = gDataHub.GetDataSetHandler().GetDataSet(tSetIndex);
    probabilities->_paoi._sn_outside = (dataset.getMeasureData_PT_Aux()[interval] - dataset.getMeasureData_PT_Aux()[interval + 1]) - Sn;
    probabilities->_paoi._poutside = probabilityPositive(
        dataset.getCaseData_PT_NC()[interval] - C, dataset.getMeasureData_PT_NC()[interval] - N,
        (dataset.getMeasureData_PT_Aux2()[interval] - dataset.getMeasureData_PT_Aux2()[interval + 1]) - Sc,
        probabilities->_paoi._sn_outside,
        randomizer->getPositiveBatchesForInterval(probabilities->_paoi._positive_batches_outside, positiveBatchIndexesOutside.flip()/* flip to become outside */, interval)
    );
    // Insert probability into cache.
    probabilitiesCache.insert(std::make_pair(WindowRange_t(interval, interval), probabilities));
    return probabilities;
}

/** Calculates the probabilities the cluster data and window by time stratified adjustment lengths. */
void BatchedLikelihoodCalculator::CalculateProbabilitiesForWindow(
    BatchedSpaceTimeData& Data, int iWindowStart, int iWindowEnd, ProbabilitiesContainer_t& probabilities, size_t tSetIndex
) {
    const BatchedRandomizer* randomizer = dynamic_cast<const BatchedRandomizer*>(gDataHub.GetDataSetHandler().GetRandomizer(tSetIndex));
    auto& dataset = gDataHub.GetDataSetHandler().GetDataSet(tSetIndex);
    auto& probabilitiesCache = _probabilities_cache[tSetIndex];
    ProbabilitiesCache_t::iterator cachedProbability;
    std::vector<WindowRange_t> outRanges;
    double C, N, Sc, Sn, C_outside, N_outside, Sc_outside;
    boost::dynamic_bitset<> positiveBatchIndexes, positiveBatchIndexesOutside;
    auto const& adjustment_ranges = gDataHub.getTimeStratifiedTemporalAdjustmentWindows();
    probabilities.clear();
    // Find the adjustment range where iWindowEnd fits between the start and the end of the range.
    auto endRange = std::find_if(adjustment_ranges.begin(), adjustment_ranges.end(),
        [iWindowEnd](const WindowRange_t& range) { return range.first <= iWindowEnd && iWindowEnd <= range.second; }
    );
    // Create a range that expands to the iWindowStart -> iWindowEnd indexes, bound by this adjustment range.
    auto windowRange = WindowRange_t(std::max(endRange->first, iWindowStart), iWindowEnd);
    // search cache - we might have already calculated this window's probability
    if (windowRange == *endRange && (cachedProbability = probabilitiesCache.find(windowRange)) != probabilitiesCache.end()) {
        probabilities.push_back(cachedProbability->second);
    } else {
        probabilities.push_back(boost::shared_ptr<ProbabilitiesRange>(new ProbabilitiesRange(std::distance(adjustment_ranges.begin(), endRange))));
        // Calculate the values for the inside probability in the current adjustment range.
        C = Data.gpCases[windowRange.first] - Data.gpCases[windowRange.second + 1]; // number of positive batches
        N = Data.gpMeasure[windowRange.first] - Data.gpMeasure[windowRange.second + 1]; // number of batches
        Sc = Data.gpMeasureAux2[windowRange.first] - Data.gpMeasureAux2[windowRange.second + 1];
        Sn = Data.gpMeasureAux[windowRange.first] - Data.gpMeasureAux[windowRange.second + 1];
        positiveBatchIndexes = Data.gpPositiveBatches[windowRange.first] - Data.gpPositiveBatches[windowRange.second + 1];
        // Calculate the probability inside the cluster.
        probabilities.back()->_paoi._pinside = probabilityPositive(
            C, N, Sc, Sn, randomizer->getBatchesInSet(probabilities.back()->_paoi._positive_batches, positiveBatchIndexes)
        );
        probabilities.back()->_paoi._sn_inside = Sn;
        // Calculate the outside probability for the current adjustment range. First obtain values for area outside of the cluster.
        probabilities.back()->_paoi._sn_outside = dataset.getMeasureData_PT_Aux()[windowRange.first] - dataset.getMeasureData_PT_Aux()[windowRange.second + 1] - Sn;
        C_outside = dataset.getCaseData_PT()[windowRange.first] - dataset.getCaseData_PT()[windowRange.second + 1] - C;
        N_outside = dataset.getMeasureData_PT()[windowRange.first] - dataset.getMeasureData_PT()[windowRange.second + 1] - N;
        Sc_outside = (dataset.getMeasureData_PT_Aux2()[windowRange.first] - dataset.getMeasureData_PT_Aux2()[windowRange.second + 1]) - Sc;
        positiveBatchIndexesOutside = positiveBatchIndexes;
        randomizer->getPositiveBatchesForRange(
            probabilities.back()->_paoi._positive_batches_outside, positiveBatchIndexesOutside.flip()/* flip to exclude cluster area */,
            windowRange.first, windowRange.second, false
        );
        // Create range(s) on either side of the cluster window range within the current adjustment range.
        // There ranges are used to determine which batches are in the remainder of the current adjustment range.
        if (endRange->first < iWindowStart) outRanges.push_back(WindowRange_t(endRange->first, iWindowStart - 1)); // before
        if (iWindowEnd < endRange->second) outRanges.push_back(WindowRange_t(iWindowEnd + 1, endRange->second)); // after
        // Now add values from those ranges that potentially abutt the cluster window within this adjustment range.
        for (auto& range : outRanges) {
            probabilities.back()->_paoi._sn_outside += dataset.getMeasureData_PT_Aux()[range.first] - dataset.getMeasureData_PT_Aux()[range.second + 1];
            C_outside += dataset.getCaseData_PT()[range.first] - dataset.getCaseData_PT()[range.second + 1];
            N_outside += dataset.getMeasureData_PT()[range.first] - dataset.getMeasureData_PT()[range.second + 1];
            Sc_outside += (dataset.getMeasureData_PT_Aux2()[range.first] - dataset.getMeasureData_PT_Aux2()[range.second + 1]);
            // Obtain the batches in this range everywhere - not just the cluster area of interest.
            randomizer->getPositiveBatchesForRange(probabilities.back()->_paoi._positive_batches_outside, range.first, range.second, false);
        }
        // Now calculate the probability outside within this adjustment range.
        probabilities.back()->_paoi._poutside = probabilityPositive(
            C_outside, N_outside, Sc_outside, probabilities.back()->_paoi._sn_outside, probabilities.back()->_paoi._positive_batches_outside
        );
        // Add probability to cache if it fits the window.
        if (windowRange == *endRange)
            probabilitiesCache.insert(std::make_pair(windowRange, probabilities.back()));
    }
    // Are there other adjustment ranges before the initial window range?
    if (iWindowStart < endRange->first) {
        auto pos = std::distance(adjustment_ranges.begin(), endRange);
        auto irange = adjustment_ranges.rbegin() + (adjustment_ranges.size() - pos);
        for (; irange != adjustment_ranges.rend() && irange->second >= iWindowStart; ++irange) {
            windowRange = WindowRange_t(std::max(irange->first, iWindowStart), irange->second);
            // search cache - we might have already calculated this window's probability
            if (windowRange == *irange && (cachedProbability = probabilitiesCache.find(windowRange)) != probabilitiesCache.end()) {
                probabilities.push_back(cachedProbability->second);
            } else {
                pos = std::distance(adjustment_ranges.rbegin(), irange);
                probabilities.push_back(boost::shared_ptr<ProbabilitiesRange>(new ProbabilitiesRange(adjustment_ranges.size() - pos - 1)));
                // Calculate the values for the inside probability in the current adjustment range.
                C = Data.gpCases[windowRange.first] - Data.gpCases[windowRange.second + 1]; // number of positive batches
                N = Data.gpMeasure[windowRange.first] - Data.gpMeasure[windowRange.second + 1]; // number of batches
                Sc = Data.gpMeasureAux2[windowRange.first] - Data.gpMeasureAux2[windowRange.second + 1];
                Sn = Data.gpMeasureAux[windowRange.first] - Data.gpMeasureAux[windowRange.second + 1];
                positiveBatchIndexes = Data.gpPositiveBatches[windowRange.first] - Data.gpPositiveBatches[windowRange.second + 1];
                // Calculate the probability inside the cluster.
                probabilities.back()->_paoi._pinside = probabilityPositive(
                    C, N, Sc, Sn, randomizer->getBatchesInSet(probabilities.back()->_paoi._positive_batches, positiveBatchIndexes)
                );
                probabilities.back()->_paoi._sn_inside = Sn;
                // Calculate the outside probability for the current adjustment range. First obtain values for area outside of the cluster.
                probabilities.back()->_paoi._sn_outside = dataset.getMeasureData_PT_Aux()[windowRange.first] - dataset.getMeasureData_PT_Aux()[windowRange.second + 1] - Sn;
                C_outside = dataset.getCaseData_PT()[windowRange.first] - dataset.getCaseData_PT()[windowRange.second + 1] - C;
                N_outside = dataset.getMeasureData_PT()[windowRange.first] - dataset.getMeasureData_PT()[windowRange.second + 1] - N;
                Sc_outside = (dataset.getMeasureData_PT_Aux2()[windowRange.first] - dataset.getMeasureData_PT_Aux2()[windowRange.second + 1]) - Sc;
                positiveBatchIndexesOutside = positiveBatchIndexes;
                randomizer->getPositiveBatchesForRange(
                    probabilities.back()->_paoi._positive_batches_outside, positiveBatchIndexesOutside.flip()/* flip to exclude cluster area */,
                    windowRange.first, windowRange.second, false
                );
                // Create range(s) on either side of the cluster window range within the current adjustment range.
                // There ranges are used to determine which batches are in the remainder of the current adjustment range.
                outRanges.clear();
                if (irange->first < iWindowStart) outRanges.push_back(WindowRange_t(irange->first, iWindowStart - 1));
                // Now add values from those ranges that potentially abutt the cluster window within this adjustment range.
                for (auto& range : outRanges) {
                    probabilities.back()->_paoi._sn_outside += dataset.getMeasureData_PT_Aux()[range.first] - dataset.getMeasureData_PT_Aux()[range.second + 1];
                    C_outside += dataset.getCaseData_PT()[range.first] - dataset.getCaseData_PT()[range.second + 1];
                    N_outside += dataset.getMeasureData_PT()[range.first] - dataset.getMeasureData_PT()[range.second + 1];
                    Sc_outside += (dataset.getMeasureData_PT_Aux2()[range.first] - dataset.getMeasureData_PT_Aux2()[range.second + 1]);
                    // Obtain the batches in this range everywhere - not just the cluster area of interest.
                    randomizer->getPositiveBatchesForRange(probabilities.back()->_paoi._positive_batches_outside, range.first, range.second, false);
                }
                // Now calculate the probability outside within this adjustment range.
                probabilities.back()->_paoi._poutside = probabilityPositive(
                    C_outside, N_outside, Sc_outside, probabilities.back()->_paoi._sn_outside, probabilities.back()->_paoi._positive_batches_outside
                );
                if (windowRange == *irange)
                    probabilitiesCache.insert(std::make_pair(windowRange, probabilities.back()));
            }
        }
    }
}

/** Calculates the probabilities the cluster data and window by time stratified adjustment lengths within a simulation. */
void BatchedLikelihoodCalculator::CalculateProbabilitiesForWindowForSimulation(
    BatchedSpaceTimeData& Data, int iWindowStart, int iWindowEnd, ProbabilitiesContainer_t& probabilities, size_t tSetIndex
) {
    const BatchedRandomizer* randomizer = dynamic_cast<const BatchedRandomizer*>(_randomizer_container->at(tSetIndex));
    auto& dataset = gDataHub.GetDataSetHandler().GetDataSet(tSetIndex);
    auto& probabilitiesCache = _probabilities_cache[tSetIndex];
    ProbabilitiesCache_t::iterator cachedProbability;
    std::vector<WindowRange_t> outRanges;
    double C, N, Sc, Sn, C_outside, N_outside, Sc_outside;
    boost::dynamic_bitset<> positiveBatchIndexes, positiveBatchIndexesOutside;
    auto const& adjustment_ranges = gDataHub.getTimeStratifiedTemporalAdjustmentWindows();
    probabilities.clear();
    // Find the adjustment range where iWindowEnd fits between the start and the end of the range.
    auto endRange = std::find_if(adjustment_ranges.begin(), adjustment_ranges.end(),
        [iWindowEnd](const WindowRange_t& range) { return range.first <= iWindowEnd && iWindowEnd <= range.second; }
    );
    // Create a range that expands to the iWindowStart -> iWindowEnd indexes, bound by this adjustment range.
    auto windowRange = WindowRange_t(std::max(endRange->first, iWindowStart), iWindowEnd);
    // search cache - we might have already calculated this window's probability
    if (windowRange == *endRange && (cachedProbability = probabilitiesCache.find(windowRange)) != probabilitiesCache.end()) {
        probabilities.push_back(cachedProbability->second);
    } else {
        probabilities.push_back(boost::shared_ptr<ProbabilitiesRange>(new ProbabilitiesRange(std::distance(adjustment_ranges.begin(), endRange))));
        // Calculate the values for the inside probability in the current adjustment range.
        C = Data.gpCases[windowRange.first] - Data.gpCases[windowRange.second + 1]; // number of positive batches
        N = Data.gpMeasure[windowRange.first] - Data.gpMeasure[windowRange.second + 1]; // number of batches
        Sc = Data.gpMeasureAux2[windowRange.first] - Data.gpMeasureAux2[windowRange.second + 1];
        Sn = Data.gpMeasureAux[windowRange.first] - Data.gpMeasureAux[windowRange.second + 1];
        positiveBatchIndexes = Data.gpPositiveBatches[windowRange.first] - Data.gpPositiveBatches[windowRange.second + 1];
        // Calculate the probability inside the cluster.
        probabilities.back()->_paoi._pinside = probabilityPositive(
            C, N, Sc, Sn, randomizer->getBatchesInSet(probabilities.back()->_paoi._positive_batches, positiveBatchIndexes)
        );
        probabilities.back()->_paoi._sn_inside = Sn;
        // Calculate the outside probability for the current adjustment range. First obtain values for area outside of the cluster.
        probabilities.back()->_paoi._sn_outside = randomizer->getSumNegativeBatchesByTime()[windowRange.first] - randomizer->getSumNegativeBatchesByTime()[windowRange.second + 1] - Sn;
        C_outside = dataset.getCaseData_PT()[windowRange.first] - dataset.getCaseData_PT()[windowRange.second + 1] - C;
        N_outside = dataset.getMeasureData_PT()[windowRange.first] - dataset.getMeasureData_PT()[windowRange.second + 1] - N;
        Sc_outside = randomizer->getSumPositiveBatchesByTime()[windowRange.first] - randomizer->getSumPositiveBatchesByTime()[windowRange.second + 1] - Sc;
        positiveBatchIndexesOutside = randomizer->getPositiveIndexesOfRandomization() - positiveBatchIndexes;
        randomizer->getBatchesInSetForRange(
            probabilities.back()->_paoi._positive_batches_outside, positiveBatchIndexesOutside, windowRange.first, windowRange.second, true
        );
        // Create range(s) on either side of the cluster window range within the current adjustment range.
        // There ranges are used to determine which batches are in the remainder of the current adjustment range.
        if (endRange->first < iWindowStart) outRanges.push_back(WindowRange_t(endRange->first, iWindowStart - 1)); // before
        if (iWindowEnd < endRange->second) outRanges.push_back(WindowRange_t(iWindowEnd + 1, endRange->second)); // after
        // Now add values from those ranges that potentially abutt the cluster window within this adjustment range.
        for (auto& range : outRanges) {
            probabilities.back()->_paoi._sn_outside += randomizer->getSumPositiveBatchesByTime()[range.first] - randomizer->getSumPositiveBatchesByTime()[range.second + 1];
            C_outside += dataset.getCaseData_PT()[range.first] - dataset.getCaseData_PT()[range.second + 1];
            N_outside += dataset.getMeasureData_PT()[range.first] - dataset.getMeasureData_PT()[range.second + 1];
            Sc_outside += randomizer->getSumPositiveBatchesByTime()[range.first] - randomizer->getSumPositiveBatchesByTime()[range.second + 1];
            // Obtain the batches in this range everywhere - not just the cluster area of interest.
            randomizer->getBatchesInSetForRange(probabilities.back()->_paoi._positive_batches_outside, positiveBatchIndexesOutside, range.first, range.second, false);
        }
        // Now calculate the probability outside within this adjustment range.
        probabilities.back()->_paoi._poutside = probabilityPositive(
            C_outside, N_outside, Sc_outside, probabilities.back()->_paoi._sn_outside, probabilities.back()->_paoi._positive_batches_outside
        );
        // Add probability to cache if it fits the window.
        if (windowRange == *endRange)
            probabilitiesCache.insert(std::make_pair(windowRange, probabilities.back()));
    }
    // Are there other adjustment ranges before the initial window range?
    if (iWindowStart < endRange->first) {
        auto pos = std::distance(adjustment_ranges.begin(), endRange);
        auto irange = adjustment_ranges.rbegin() + (adjustment_ranges.size() - pos);
        for (; irange != adjustment_ranges.rend() && irange->second >= iWindowStart; ++irange) {
            windowRange = WindowRange_t(std::max(irange->first, iWindowStart), irange->second);
            // search cache - we might have already calculated this window's probability
            if (windowRange == *irange && (cachedProbability = probabilitiesCache.find(windowRange)) != probabilitiesCache.end()) {
                probabilities.push_back(cachedProbability->second);
            } else {
                probabilities.push_back(boost::shared_ptr<ProbabilitiesRange>(new ProbabilitiesRange(
                    adjustment_ranges.size() - std::distance(adjustment_ranges.rbegin(), irange) - 1
                )));
                // Calculate the values for the inside probability in the current adjustment range.
                C = Data.gpCases[windowRange.first] - Data.gpCases[windowRange.second + 1]; // number of positive batches
                N = Data.gpMeasure[windowRange.first] - Data.gpMeasure[windowRange.second + 1]; // number of batches
                Sc = Data.gpMeasureAux2[windowRange.first] - Data.gpMeasureAux2[windowRange.second + 1];
                Sn = Data.gpMeasureAux[windowRange.first] - Data.gpMeasureAux[windowRange.second + 1];
                positiveBatchIndexes = Data.gpPositiveBatches[windowRange.first] - Data.gpPositiveBatches[windowRange.second + 1];
                // Calculate the probability inside the cluster.
                probabilities.back()->_paoi._pinside = probabilityPositive(
                    C, N, Sc, Sn, randomizer->getBatchesInSet(probabilities.back()->_paoi._positive_batches, positiveBatchIndexes)
                );
                probabilities.back()->_paoi._sn_inside = Sn;
                // Calculate the outside probability for the current adjustment range. First obtain values for area outside of the cluster.
                probabilities.back()->_paoi._sn_outside = randomizer->getSumNegativeBatchesByTime()[windowRange.first] - randomizer->getSumNegativeBatchesByTime()[windowRange.second + 1] - Sn;
                C_outside = dataset.getCaseData_PT()[windowRange.first] - dataset.getCaseData_PT()[windowRange.second + 1] - C;
                N_outside = dataset.getMeasureData_PT()[windowRange.first] - dataset.getMeasureData_PT()[windowRange.second + 1] - N;
                Sc_outside = randomizer->getSumPositiveBatchesByTime()[windowRange.first] - randomizer->getSumPositiveBatchesByTime()[windowRange.second + 1] - Sc;
                positiveBatchIndexesOutside = randomizer->getPositiveIndexesOfRandomization() - positiveBatchIndexes;
                randomizer->getBatchesInSetForRange(
                    probabilities.back()->_paoi._positive_batches_outside, positiveBatchIndexesOutside, windowRange.first, windowRange.second, true
                );
                // Create range(s) on either side of the cluster window range within the current adjustment range.
                // There ranges are used to determine which batches are in the remainder of the current adjustment range.
                outRanges.clear();
                if (irange->first < iWindowStart) outRanges.push_back(WindowRange_t(irange->first, iWindowStart - 1));
                // Now add values from those ranges that potentially abutt the cluster window within this adjustment range.
                for (auto& range : outRanges) {
                    probabilities.back()->_paoi._sn_outside += randomizer->getSumNegativeBatchesByTime()[range.first] - randomizer->getSumNegativeBatchesByTime()[range.second + 1];
                    C_outside += dataset.getCaseData_PT()[range.first] - dataset.getCaseData_PT()[range.second + 1];
                    N_outside += dataset.getMeasureData_PT()[range.first] - dataset.getMeasureData_PT()[range.second + 1];
                    Sc_outside += randomizer->getSumPositiveBatchesByTime()[range.first] - randomizer->getSumPositiveBatchesByTime()[range.second + 1];
                    // Obtain the batches in this range everywhere - not just the cluster area of interest.
                    randomizer->getBatchesInSetForRange(probabilities.back()->_paoi._positive_batches_outside, positiveBatchIndexesOutside, range.first, range.second, false);
                }
                // Now calculate the probability outside within this adjustment range.
                probabilities.back()->_paoi._poutside = probabilityPositive(
                    C_outside, N_outside, Sc_outside, probabilities.back()->_paoi._sn_outside, probabilities.back()->_paoi._positive_batches_outside
                );
                if (windowRange == *irange)
                    probabilitiesCache.insert(std::make_pair(windowRange, probabilities.back()));
            }
        }
    }
}

/** Returns loglikelihood ratio given probabilities in area of interest. */
double BatchedLikelihoodCalculator::getLoglikelihoodRatio(ProbabilitiesAOI& probabilities, size_t tSetIndex) const {
    return LL(1 - probabilities._pinside, probabilities._sn_inside, probabilities._positive_batches)
        + LL(1 - probabilities._poutside, probabilities._sn_outside, probabilities._positive_batches_outside)
        - _log_likelihoods[tSetIndex];
}

/** Returns loglikelihood ratio given probabilities in area of interest, for time interval. */
double BatchedLikelihoodCalculator::getLoglikelihoodRatioForInterval(ProbabilitiesRange& probabilities, int interval, size_t tSetIndex) const {
    if (probabilities.llrIsSet()) return probabilities._llr;
    return (probabilities._llr = (LL(1 - probabilities._paoi._pinside, probabilities._paoi._sn_inside, probabilities._paoi._positive_batches)
        + LL(1 - probabilities._paoi._poutside, probabilities._paoi._sn_outside, probabilities._paoi._positive_batches_outside)
        - _log_likelihoods_by_time->GetArray()[tSetIndex][interval].second));
}

/** Returns loglikelihood ratio given probabilities in area of interest, for time range. */
double BatchedLikelihoodCalculator::getLoglikelihoodRatioForRange(ProbabilitiesRange& probabilities, size_t tSetIndex) const {
    if (probabilities.llrIsSet()) return probabilities._llr;
    return (probabilities._llr = (LL(1 - probabilities._paoi._pinside, probabilities._paoi._sn_inside, probabilities._paoi._positive_batches)
        + LL(1 - probabilities._paoi._poutside, probabilities._paoi._sn_outside, probabilities._paoi._positive_batches_outside)
        - _log_likelihoods_by_range->GetArray()[tSetIndex][probabilities._range_idx].second));
}

/** Calculates the log-likelihoods for each simulation data set under the null. */
void BatchedLikelihoodCalculator::calculateLoglikelihoodsForAll() const {
    _log_likelihoods.clear();
    for (size_t t = 0; t < gDataHub.GetDataSetHandler().GetNumDataSets(); ++t) {
        const auto& dataset = gDataHub.GetDataSetHandler().GetDataSet(t);
        const BatchedRandomizer* randomizer = dynamic_cast<const BatchedRandomizer*>(_randomizer_container->at(t));
        BatchedRandomizer::BatchEntryContainer_t positiveBatches;
        randomizer->getBatchesInSet(positiveBatches, randomizer->getPositiveIndexesOfRandomization());
        _log_likelihoods.push_back(LL(1 - probabilityPositive(
            dataset.getTotalCases(), dataset.getTotalMeasure(),
            randomizer->getSumPositiveBatches(), randomizer->getSumNegativeBatches(),
            positiveBatches), randomizer->getSumNegativeBatches(), positiveBatches)
        );
        if (gDataHub.GetParameters().GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION) {
            // When performing the time stratified adjustment at lengths greater than the time aggregation, we need to calculate
            // log-likelihoods under the null for each adjustment window.
            if (gDataHub.GetParameters().isTimeStratifiedWithLargerAdjustmentLength()) {
                auto& ranges = gDataHub.getTimeStratifiedTemporalAdjustmentWindows();
                for (size_t r = 0; r < ranges.size(); ++r) {
                    const auto& range = ranges[r];
                    randomizer->getBatchesInSetForRange(positiveBatches, randomizer->getPositiveIndexesOfRandomization(), range.first, range.second, true);
                    double ptime = probabilityPositive(
                        dataset.getCaseData_PT()[range.first] - dataset.getCaseData_PT()[range.second + 1],
                        dataset.getMeasureData_PT()[range.first] - dataset.getMeasureData_PT()[range.second + 1],
                        randomizer->getSumPositiveBatchesByTime()[range.first] - randomizer->getSumPositiveBatchesByTime()[range.second + 1],
                        randomizer->getSumNegativeBatchesByTime()[range.first] - randomizer->getSumNegativeBatchesByTime()[range.second + 1],
                        positiveBatches
                    );
                    _log_likelihoods_by_range->GetArray()[t][r] = std::make_pair(ptime,
                        LL(1 - ptime, randomizer->getSumNegativeBatchesByTime()[range.first] - randomizer->getSumNegativeBatchesByTime()[range.second + 1], positiveBatches)
                    );
                }
            } else {
                for (int i = 0; i < gDataHub.GetNumTimeIntervals(); ++i) {
                    randomizer->getBatchesInSetForInterval(positiveBatches, randomizer->getPositiveIndexesOfRandomization(), i);
                    double ptime = probabilityPositive(
                        dataset.getCaseData_PT_NC()[i], dataset.getMeasureData_PT_NC()[i],
                        randomizer->getSumPositiveBatchesByTime()[i], randomizer->getSumNegativeBatchesByTime()[i], positiveBatches
                    );
                    _log_likelihoods_by_time->GetArray()[t][i] = std::make_pair(ptime, LL(1 - ptime, randomizer->getSumNegativeBatchesByTime()[i], positiveBatches));
                }
            }
        }
    }
}

/** Calculates the full test statistic given passed maximizing value and data set index. */
double BatchedLikelihoodCalculator::CalculateFullStatistic(double dMaximizingValue, size_t tSetIndex) const {
    if (dMaximizingValue == -std::numeric_limits<double>::max()) return 0.0; // indicating no value set
    if (_always_full_statistic) return dMaximizingValue; // already is full statistic
    return dMaximizingValue - _log_likelihoods[tSetIndex];
}

/* Calculate the probabilities of being positive inside and outside the area of interest - in the context of simulations.
   n = number of positive traps
   u = number of traps
   Sc = sum of the trap sizes for positive batches
   Sn = sum of the trap sizes for negative batches
*/
ProbabilitiesAOI& BatchedLikelihoodCalculator::CalculateProbabilitiesForSimulation(ProbabilitiesAOI& probabilities, count_t n, measure_t u, measure_t Sc, measure_t Sn, const boost::dynamic_bitset<>& positiveBatchIndexes, size_t tSetIndex) const {
    double C = n; // number of positive batches
    double N = u; // number of batches
    // Retrieve the randomizer for this simulation and data set, we need to obtain totals and batch information.
    const BatchedRandomizer* randomizer = dynamic_cast<const BatchedRandomizer*>(_randomizer_container->at(tSetIndex));
    boost::dynamic_bitset<> positiveBatchIndexesOutside = randomizer->getPositiveIndexesOfRandomization() - positiveBatchIndexes;
    probabilities._sn_outside = randomizer->getSumNegativeBatches() - Sn;
    probabilities._poutside = probabilityPositive(
        gvDataSetTotals[tSetIndex].first - C, gvDataSetTotals[tSetIndex].second - N, 
        randomizer->getSumPositiveBatches() - Sc, probabilities._sn_outside,
        randomizer->getBatchesInSet(probabilities._positive_batches_outside, positiveBatchIndexesOutside)
    );
    probabilities._pinside = probabilityPositive(C, N, Sc, Sn, randomizer->getBatchesInSet(probabilities._positive_batches, positiveBatchIndexes));
    probabilities._sn_inside = Sn;
    return probabilities;
}

/* Calculate the probabilities of being positive inside and outside the area of interest - in the context of simulations.
   n = number of positive traps
   u = number of traps
   Sc = sum of the trap sizes for positive batches
   Sn = sum of the trap sizes for negative batches
*/
ProbabilitiesRange_t& BatchedLikelihoodCalculator::CalculateProbabilitiesForSimulationByTimeInterval(ProbabilitiesRange_t& probabilities, count_t n, measure_t u, measure_t Sc, measure_t Sn, const boost::dynamic_bitset<>& positiveBatchIndexes, int interval, size_t tSetIndex) const {
    // First check cache for already calculated probability for this interval.
    auto& probabilitiesCache = _probabilities_cache[tSetIndex];
    ProbabilitiesCache_t::iterator cachedProbability = probabilitiesCache.find(WindowRange_t(interval, interval));
    if (cachedProbability != probabilitiesCache.end()) {
        probabilities = cachedProbability->second;
        return probabilities;
    }
    // Calculate probabilities.
    probabilities = ProbabilitiesRange_t(new ProbabilitiesRange());
    double C = n; // number of positive batches
    double N = u; // number of batches
    // Retrieve the randomizer for this simulation and data set, we need to obtain totals and batch information.
    const BatchedRandomizer* randomizer = dynamic_cast<const BatchedRandomizer*>(_randomizer_container->at(tSetIndex));
    boost::dynamic_bitset<> positiveBatchIndexesOutside = randomizer->getPositiveIndexesOfRandomization() - positiveBatchIndexes;
    probabilities->_paoi._sn_outside = randomizer->getSumNegativeBatchesByTime()[interval] - Sn;
    auto& dataset = gDataHub.GetDataSetHandler().GetDataSet(tSetIndex);
    probabilities->_paoi._poutside = probabilityPositive(
        dataset.getCaseData_PT_NC()[interval] - C, dataset.getMeasureData_PT_NC()[interval] - N,
        randomizer->getSumPositiveBatchesByTime()[interval] - Sc, probabilities->_paoi._sn_outside,
        randomizer->getBatchesInSetForInterval(probabilities->_paoi._positive_batches_outside, positiveBatchIndexesOutside, interval)
    );
    // positiveBatchIndexes should already be limited to the time interval of interest for this cluster
    probabilities->_paoi._pinside = probabilityPositive(C, N, Sc, Sn, randomizer->getBatchesInSet(probabilities->_paoi._positive_batches, positiveBatchIndexes));
    probabilities->_paoi._sn_inside = Sn;
    // Insert probability into cache.
    probabilitiesCache.insert(std::make_pair(WindowRange_t(interval, interval), probabilities));
    return probabilities;
}

/** Returns the maximizing value for simulation. */
double BatchedLikelihoodCalculator::getMaximizingValue(ProbabilitiesAOI& probabilities, size_t tSetIndex) const {
    double loglikelihood = LL(1 - probabilities._pinside, probabilities._sn_inside, probabilities._positive_batches)
        + LL(1 - probabilities._poutside, probabilities._sn_outside, probabilities._positive_batches_outside);
    return _always_full_statistic ? loglikelihood - _log_likelihoods[tSetIndex] : loglikelihood;
}
