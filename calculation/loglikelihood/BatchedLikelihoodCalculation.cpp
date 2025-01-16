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
        // If doing the time stratified adjustment, we need the log-likelihood by time interval.
        _log_likelihoods_by_time.reset(new TwoDimensionArrayHandler<std::pair<double, double>>(
            Data.GetDataSetHandler().GetNumDataSets(), Data.GetNumTimeIntervals() + 1
        ));
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
            for (int i = 0; i < Data.GetNumTimeIntervals(); ++i) {
                double ptime = probabilityPositive(
                    dataset.getCaseData_PT_NC()[i],
                    dataset.getMeasureData_PT_NC()[i],
                    dataset.getMeasureData_PT_Aux2()[i] - dataset.getMeasureData_PT_Aux2()[i + 1],/* sum of the batch sizes for positive batches */
                    dataset.getMeasureData_PT_Aux()[i] - dataset.getMeasureData_PT_Aux()[i + 1],/* sum of the batch sizes for negative batches */
                    randomizer->getPositiveBatchesForInterval(positiveBatches, positiveBatchIndexes, i)
                );
                _log_likelihoods_by_time->GetArray()[t][i] = std::make_pair(ptime, LL(1 - ptime, dataset.getMeasureData_PT_Aux()[i] - dataset.getMeasureData_PT_Aux()[i + 1], positiveBatches));
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

/** Returns the expected number of cases for cluster data when reporting. */
double BatchedLikelihoodCalculator::getClusterExpected(const CCluster& cluster, size_t tSetIndex) const {
    const auto& dataset = gDataHub.GetDataSetHandler().GetDataSet(tSetIndex);
    const BatchedRandomizer* randomizer = dynamic_cast<const BatchedRandomizer*>(gDataHub.GetDataSetHandler().GetRandomizer(tSetIndex));
    if (cluster.GetClusterType() == SPACETIMECLUSTER && gDataHub.GetParameters().GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION) {
        // The expected needs to be calculated by interval and added together.
        measure_t clusterExpected = 0.0, ** ppMeasure = dataset.getMeasureData().GetArray();
        std::vector<tract_t> indexes;
        cluster.getIdentifierIndexes(gDataHub, indexes, true);
        for (int i = cluster.m_nFirstInterval; i < cluster.m_nLastInterval; ++i) {
            measure_t measure = 0;
            for (tract_t t : indexes)
                measure += ppMeasure[i][t] - (i + 1 < dataset.getIntervalDimension() ? ppMeasure[i + 1][t] : 0.0);
            double Sk = 0.0, qall = 1.0 - _log_likelihoods_by_time->GetArray()[tSetIndex][i].first;
            for (auto& be : randomizer->getBatches()) {
                if (be.get<2>() == i && std::find(indexes.begin(), indexes.end(), be.get<3>()) != indexes.end()) {
                    Sk += std::pow(qall, be.get<1>());
                }
            }
            clusterExpected += measure - Sk;
        }
        return clusterExpected;
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
    const auto& dataset = gDataHub.GetDataSetHandler().GetDataSet(tSetIndex);
    const BatchedRandomizer* randomizer = dynamic_cast<const BatchedRandomizer*>(gDataHub.GetDataSetHandler().GetRandomizer(tSetIndex));
    measure_t totalExpected = 0.0, * pMeasure = dataset.getMeasureData_PT();
    for (int i = windowStart; i < windowEnd; ++i) {
        measure_t measure = pMeasure[i] - (i + 1 < dataset.getIntervalDimension() ? pMeasure[i + 1] : 0.0);
        double Sk = 0.0, qall = 0;
        // If time-stratified, the probability of being negative obtained by interval.
        if (gDataHub.GetParameters().GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION)
            qall = 1.0 - _log_likelihoods_by_time->GetArray()[tSetIndex][i].first;
        else // Otherwise the probability of being negative is that of the entire data set.
            qall = 1.0 - _probabilities[tSetIndex];
        for (auto& be : randomizer->getBatches()) {
            if (be.get<2>() == i)
                Sk += std::pow(qall, be.get<1>());
        }
        totalExpected += measure - Sk;
    }
    return totalExpected;
}

/** Returns the expected number of cases in window range for locations. This is a helper function used in TemporalChartGenerator class. */
double BatchedLikelihoodCalculator::getExpectedInWindow(int windowStart, int windowEnd, const std::vector<tract_t>& locationIdexes, size_t tSetIndex) const {
    const auto& dataset = gDataHub.GetDataSetHandler().GetDataSet(tSetIndex);
    const BatchedRandomizer* randomizer = dynamic_cast<const BatchedRandomizer*>(gDataHub.GetDataSetHandler().GetRandomizer(tSetIndex));
    measure_t totalExpected = 0.0, ** ppMeasure = dataset.getMeasureData().GetArray();
    for (int i = windowStart; i < windowEnd; ++i) {
        measure_t measure = 0;
        for (tract_t t : locationIdexes)
            measure += ppMeasure[i][t] - (i + 1 < dataset.getIntervalDimension() ? ppMeasure[i + 1][t] : 0.0);
        double Sk = 0.0, qall = 0;
        // If time-stratified, the probability of being negative obtained by interval.
        if (gDataHub.GetParameters().GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION)
            qall = 1.0 - _log_likelihoods_by_time->GetArray()[tSetIndex][i].first;
        else // Otherwise the probability of being negative is that of the entire data set.
            qall = 1.0 - _probabilities[tSetIndex];
        for (auto& be : randomizer->getBatches()) {
            if (be.get<2>() == i && std::find(locationIdexes.begin(), locationIdexes.end(), be.get<3>()) != locationIdexes.end())
                Sk += std::pow(qall, be.get<1>());
        }
        totalExpected += measure - Sk;
    }
    return totalExpected;
}

/** Returns the expected number of cases for cluster data at window. */
double BatchedLikelihoodCalculator::getClusterExpectedAtWindow(measure_t totalBatches, const boost::dynamic_bitset<>& BatchIndexes, int windowIndex, size_t tSetIndex) const {
    const BatchedRandomizer* randomizer = dynamic_cast<const BatchedRandomizer*>(gDataHub.GetDataSetHandler().GetRandomizer(tSetIndex));
    BatchedRandomizer::BatchEntryContainer_t batches;
    double Sk = 0.0, qall = 1.0 - _log_likelihoods_by_time->GetArray()[tSetIndex][windowIndex].first;
    for (const auto& be : randomizer->getBatchesInSet(batches, BatchIndexes))
        Sk += std::pow(qall, be.get<1>());
    return totalBatches - Sk;
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
ProbabilitiesAOI& BatchedLikelihoodCalculator::CalculateProbabilitiesByTimeInterval(ProbabilitiesAOI& probabilities, count_t n, measure_t u, measure_t Sc, measure_t Sn, const boost::dynamic_bitset<>& positiveBatchIndexes, int interval, size_t tSetIndex) const {
    double C = n; // number of positive batches
    double N = u; // number of batches
    const BatchedRandomizer* randomizer = dynamic_cast<const BatchedRandomizer*>(gDataHub.GetDataSetHandler().GetRandomizer(tSetIndex));
    probabilities._pinside = probabilityPositive(C, N, Sc, Sn, randomizer->getPositiveBatches(probabilities._positive_batches, positiveBatchIndexes));
    probabilities._sn_inside = Sn;
    boost::dynamic_bitset<> positiveBatchIndexesOutside = positiveBatchIndexes;
    // We need totals by time interval
    auto& dataset = gDataHub.GetDataSetHandler().GetDataSet(tSetIndex);
    probabilities._sn_outside = (dataset.getMeasureData_PT_Aux()[interval] - dataset.getMeasureData_PT_Aux()[interval + 1]) - Sn;
    probabilities._poutside = probabilityPositive(
        dataset.getCaseData_PT_NC()[interval] - C, dataset.getMeasureData_PT_NC()[interval] - N,
        (dataset.getMeasureData_PT_Aux2()[interval] - dataset.getMeasureData_PT_Aux2()[interval + 1]) - Sc,
        probabilities._sn_outside,
        randomizer->getPositiveBatchesForInterval(probabilities._positive_batches_outside, positiveBatchIndexesOutside.flip()/* flip to become outside */, interval)
    );
    return probabilities;
}

/** Returns whether ProbabilitiesAOI meets the scanning area requested. */
bool BatchedLikelihoodCalculator::isScanArea(const ProbabilitiesAOI& probabilities, count_t nCases) const {
    switch (gDataHub.GetParameters().GetExecuteScanRateType()) {
        case LOW: return probabilities._pinside < probabilities._poutside;
        case HIGHANDLOW:
            // When scanning for both high and low rates simultaneously, we can't shortcut minimum number
            // of cases w/o calculating the probabilities inside and outside - which then tells us if it's
            // a high or low rate cluster.
            if (probabilities._pinside < probabilities._poutside)
                return nCases >= _min_low_rate_cases;
            if (probabilities._pinside > probabilities._poutside)
                return nCases >= _min_high_rate_cases;
            return false;
        case HIGH:
        default: return probabilities._pinside > probabilities._poutside;
    }
}

/** Returns whether ProbabilitiesAOI meets the scanning area requested. */
bool BatchedLikelihoodCalculator::isScanArea(measure_t positiveCases, measure_t totalCases, const boost::dynamic_bitset<>& batchIndexes, size_t tSetIndex) const {
    const auto& dataset = gDataHub.GetDataSetHandler().GetDataSet(tSetIndex);
    const BatchedRandomizer* randomizer = dynamic_cast<const BatchedRandomizer*>(gDataHub.GetDataSetHandler().GetRandomizer(tSetIndex));
    BatchedRandomizer::BatchEntryContainer_t batches;
    double Sk = 0.0, qall = 1.0 - _probabilities[tSetIndex];
    for (const auto& be : randomizer->getBatchesInSet(batches, batchIndexes))
        Sk += std::pow(qall, be.get<1>());
    double clusterExpected = totalCases - Sk;
    switch (gDataHub.GetParameters().GetExecuteScanRateType()) {
        case LOW: return positiveCases < clusterExpected;
        // When scanning for both high and low rates simultaneously, we can't shortcut minimum number
        // of cases w/o calculating the expected number of cases - which then tells us if it's
        // a high or low rate cluster.
        case HIGHANDLOW: 
            if (positiveCases < clusterExpected)
                return positiveCases >= _min_low_rate_cases;
            if (positiveCases > clusterExpected)
                return positiveCases >= _min_high_rate_cases;
            return false;
        case HIGH:
        default: return positiveCases > clusterExpected;
    }
}

/** Returns loglikelihood ratio given probabilities in area of interest. */
double BatchedLikelihoodCalculator::getLoglikelihoodRatio(ProbabilitiesAOI& probabilities, size_t tSetIndex) const {
    return LL(1 - probabilities._pinside, probabilities._sn_inside, probabilities._positive_batches)
        + LL(1 - probabilities._poutside, probabilities._sn_outside, probabilities._positive_batches_outside)
        - _log_likelihoods[tSetIndex];
}

/** Returns loglikelihood ratio given probabilities in area of interest, for time interval. */
double BatchedLikelihoodCalculator::getLoglikelihoodRatioForInterval(ProbabilitiesAOI& probabilities, int interval, size_t tSetIndex) const {
    return LL(1 - probabilities._pinside, probabilities._sn_inside, probabilities._positive_batches)
        + LL(1 - probabilities._poutside, probabilities._sn_outside, probabilities._positive_batches_outside)
        - _log_likelihoods_by_time->GetArray()[tSetIndex][interval].second;
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
            for (int i = 0; i < gDataHub.GetNumTimeIntervals(); ++i) {
                randomizer->getBatchesInSetForInterval(positiveBatches, randomizer->getPositiveIndexesOfRandomization(), i);
                double ptime = probabilityPositive(
                    dataset.getCaseData_PT_NC()[i], dataset.getMeasureData_PT_NC()[i],
                    randomizer->getSumPositiveBatchesByTime()[i], randomizer->getSumNegativeBatchesByTime()[i], positiveBatches
                );
                _log_likelihoods_by_time->GetArray()[t][i] = std::make_pair(ptime, LL(1 - ptime, randomizer->getSumNegativeBatchesByTime()[i], positiveBatches));
            }
        } /*else {
            randomizer->getBatchesInSet(positiveBatches, randomizer->getPositiveIndexesOfRandomization());
            _log_likelihoods.push_back(LL(1 - probabilityPositive(
                dataset.getTotalCases(), dataset.getTotalMeasure(),
                randomizer->getSumPositiveBatches(), randomizer->getSumNegativeBatches(),
                positiveBatches), randomizer->getSumNegativeBatches(), positiveBatches)
            );
        }*/
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
ProbabilitiesAOI& BatchedLikelihoodCalculator::CalculateProbabilitiesForSimulationByTimeInterval(ProbabilitiesAOI& probabilities, count_t n, measure_t u, measure_t Sc, measure_t Sn, const boost::dynamic_bitset<>& positiveBatchIndexes, int interval, size_t tSetIndex) const {
    double C = n; // number of positive batches
    double N = u; // number of batches
    // Retrieve the randomizer for this simulation and data set, we need to obtain totals and batch information.
    const BatchedRandomizer* randomizer = dynamic_cast<const BatchedRandomizer*>(_randomizer_container->at(tSetIndex));
    boost::dynamic_bitset<> positiveBatchIndexesOutside = randomizer->getPositiveIndexesOfRandomization() - positiveBatchIndexes;
    probabilities._sn_outside = randomizer->getSumNegativeBatchesByTime()[interval] - Sn;
    auto& dataset = gDataHub.GetDataSetHandler().GetDataSet(tSetIndex);
    probabilities._poutside = probabilityPositive(
        dataset.getCaseData_PT_NC()[interval] - C, dataset.getMeasureData_PT_NC()[interval] - N,
        randomizer->getSumPositiveBatchesByTime()[interval] - Sc, probabilities._sn_outside,
        randomizer->getBatchesInSetForInterval(probabilities._positive_batches_outside, positiveBatchIndexesOutside, interval)
    );
    probabilities._pinside = probabilityPositive(C, N, Sc, Sn, randomizer->getBatchesInSet(probabilities._positive_batches, positiveBatchIndexes));
    probabilities._sn_inside = Sn;
    return probabilities;
}

/** Returns the maximizing value for simulation. */
double BatchedLikelihoodCalculator::getMaximizingValue(ProbabilitiesAOI& probabilities, size_t tSetIndex) const {
    double loglikelihood = LL(1 - probabilities._pinside, probabilities._sn_inside, probabilities._positive_batches)
        + LL(1 - probabilities._poutside, probabilities._sn_outside, probabilities._positive_batches_outside);
    return _always_full_statistic ? loglikelihood - _log_likelihoods[tSetIndex] : loglikelihood;
}
