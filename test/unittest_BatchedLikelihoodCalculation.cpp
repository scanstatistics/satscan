
// project files
#include "fixture_sampledata.h"
#include "BatchedLikelihoodCalculation.h"
#include "PurelySpatialData.h"
#include "SpaceTimeData.h"
#include "DateStringParser.h"

struct batched_loglikelihood_fixture : prm_testset_fixture {
    batched_loglikelihood_fixture() : prm_testset_fixture("batched\\batched.prm") {}
    virtual ~batched_loglikelihood_fixture() {}

    void readAsPurelySpatial() {
        // create the data hub and read from specified files
        _parameters.SetAnalysisType(PURELYSPATIAL);
        _data_hub.reset(new CPurelySpatialData(_parameters, _print));
        _data_hub->ReadDataFromFiles();
    }

    void readAsProspectiveSpacetime() {
        // create the data hub and read from specified files
        _parameters.SetAnalysisType(PROSPECTIVESPACETIME);
        _parameters.SetTimeTrendAdjustmentType(TEMPORAL_STRATIFIED_RANDOMIZATION);
        _data_hub.reset(new CSpaceTimeData(_parameters, _print));
        _data_hub->ReadDataFromFiles();
    }

    std::auto_ptr<CSaTScanData> _data_hub;
};

size_t getBatchIndex(CSaTScanData * hub, BatchedRandomizer* randomizer, const std::string& id, const std::string& date) {
    auto loc_idx = hub->getIdentifierInfo().getIdentifierIndex(id.c_str());
    Julian jdate;
    DateStringParser(hub->GetParameters().GetPrecisionOfTimesType()).ParseCountDateString(
        date.c_str(), hub->GetParameters().GetPrecisionOfTimesType(),
        hub->GetStudyPeriodStartDate(), hub->GetStudyPeriodStartDate(), jdate
    );
    int intervalIdx = hub->GetTimeIntervalOfDate(jdate);
    for (size_t t = 0; t < randomizer->getBatches().size(); ++t) {
        auto& be = randomizer->getBatches()[t];
        if (be.get<2>() == intervalIdx && be.get<3>() == loc_idx.get())
            return t;
    }
    throw std::exception("Not found");
};

/* Test Suite for the Poisson loglikelihood calculation. */
BOOST_FIXTURE_TEST_SUITE(batched_loglikelihood_suite, batched_loglikelihood_fixture)

/** Test calculating log-likelihood value with all positive batches of data set. */
BOOST_AUTO_TEST_CASE( test_calcLoglikelihood ) {
    readAsPurelySpatial();
    BatchedLikelihoodCalculator batchedCalc(*_data_hub);
    BatchedRandomizer * randomizer = dynamic_cast<BatchedRandomizer*>(_data_hub->GetDataSetHandler().GetRandomizer(0));
    BatchIndexes_t positive_batches(randomizer->getBatches().size());
    count_t cases = 0;
    measure_t Sc = 0, Sn = 0;
    for (size_t t = 0; t < randomizer->getBatches().size(); ++t) {
        auto& be = randomizer->getBatches()[t];
        if (be.get<0>()) {
            positive_batches.set(t);
            ++cases;
            Sc += be.get<1>();
        } else
            Sn += be.get<1>();
    }
    ProbabilitiesAOI probabilities;
    batchedCalc.CalculateProbabilities(
        probabilities, cases, randomizer->getBatches().size(), Sc, Sn, positive_batches
    );
    double maximizing = batchedCalc.getLoglikelihoodRatio(probabilities);
    BOOST_CHECK(maximizing == 0);
    BOOST_REQUIRE_CLOSE(batchedCalc.CalculateFullStatistic(maximizing), 14.3537488, 0.001);
}

/** Test calculating log-likelihood value with only one of the positive batches. */
BOOST_AUTO_TEST_CASE(test_calcLoglikelihood_1_positive) {
    readAsPurelySpatial();
    BatchedLikelihoodCalculator batchedCalc(*_data_hub);
    BatchedRandomizer* randomizer = dynamic_cast<BatchedRandomizer*>(_data_hub->GetDataSetHandler().GetRandomizer(0));
    BatchIndexes_t positive_batches(randomizer->getBatches().size());
    count_t cases = 0;
    measure_t Sc = 0, Sn = 0, batches = 0;
    std::vector<size_t> selectedBatches = {
        getBatchIndex(_data_hub.get(), randomizer, "HudsonCoastalRawBar&Grill", "10/1/2024"),
        getBatchIndex(_data_hub.get(), randomizer, "GormanFarmsCSA", "10/2/2024")
    };
    for (auto t : selectedBatches) {
        auto& be = randomizer->getBatches()[t];
        ++batches;
        if (be.get<0>()) {
            positive_batches.set(t);
            ++cases;
            Sc += be.get<1>();
        } else
            Sn += be.get<1>();
    }
    ProbabilitiesAOI probabilities;
    batchedCalc.CalculateProbabilities(probabilities, cases, batches, Sc, Sn, positive_batches);
    double maximizing = batchedCalc.getLoglikelihoodRatio(probabilities);
    BOOST_REQUIRE_CLOSE(maximizing, 0.17447097, 0.001);
    BOOST_REQUIRE_CLOSE(batchedCalc.CalculateFullStatistic(maximizing), 14.52821987, 0.001);
}

/** Test calculating log-likelihood value with only half of the positive batches. */
BOOST_AUTO_TEST_CASE(test_calcLoglikelihood_6_positive) {
    readAsPurelySpatial();
    BatchedLikelihoodCalculator batchedCalc(*_data_hub);
    BatchedRandomizer* randomizer = dynamic_cast<BatchedRandomizer*>(_data_hub->GetDataSetHandler().GetRandomizer(0));
    BatchIndexes_t positive_batches(randomizer->getBatches().size());
    count_t cases = 0;
    measure_t Sc = 0, Sn = 0, batches = 0;
    for (size_t t = 0; t < randomizer->getBatches().size(); ++t) {
        auto& be = randomizer->getBatches()[t];
        ++batches;
        if (be.get<0>()) {
            positive_batches.set(t);
            ++cases;
            Sc += be.get<1>();
        } else {
            Sn += be.get<1>();
        }
        if (positive_batches.count() > 5) break;
    }
    ProbabilitiesAOI probabilities;
    batchedCalc.CalculateProbabilities(probabilities, cases, batches, Sc, Sn, positive_batches);
    double maximizing = batchedCalc.getLoglikelihoodRatio(probabilities);
    BOOST_REQUIRE_CLOSE(maximizing, 0.34995562, 0.001);
    BOOST_REQUIRE_CLOSE(batchedCalc.CalculateFullStatistic(maximizing), 14.70370452, 0.001);
}

/** Test calculating log-likelihood value with spacetime. not adjusted. */
BOOST_AUTO_TEST_CASE(test_calcLoglikelihood_spacetime) {
    readAsProspectiveSpacetime();
    _parameters.SetNonparametricAdjustmentSize(_parameters.GetTimeAggregationLength());
    BatchedLikelihoodCalculator batchedCalc(*_data_hub);
    BatchedRandomizer* randomizer = dynamic_cast<BatchedRandomizer*>(_data_hub->GetDataSetHandler().GetRandomizer(0));
    BatchIndexes_t positive_batches(randomizer->getBatches().size());
    count_t cases = 0;
    measure_t Sc = 0, Sn = 0, batches = 0;
    std::vector<size_t> selectedBatched = {
        getBatchIndex(_data_hub.get(), randomizer, "HighlandLocal", "10/7/2024"),
        getBatchIndex(_data_hub.get(), randomizer, "KonstantinesGreekTaverna", "10/12/2024")
    };
    for (auto t: selectedBatched) {
        auto& be = randomizer->getBatches()[t];
        ++batches;
        if (be.get<0>()) {
            positive_batches.set(t);
            ++cases;
            Sc += be.get<1>();
        } else
            Sn += be.get<1>();
    }
    ProbabilitiesAOI probabilities;
    batchedCalc.CalculateProbabilities(probabilities, cases, batches, Sc, Sn, positive_batches);
    double llr = batchedCalc.getLoglikelihoodRatio(probabilities);
    BOOST_REQUIRE_CLOSE(llr, 0.33693024, 0.001);
}

/** Test calculating log-likelihood value with adjusted spacetime. */
BOOST_AUTO_TEST_CASE(test_calcLoglikelihood_spacetime_adjusted) {
    readAsProspectiveSpacetime();
    _parameters.SetNonparametricAdjustmentSize(_parameters.GetTimeAggregationLength());
    BatchedLikelihoodCalculator batchedCalc(*_data_hub);
    BatchedRandomizer* randomizer = dynamic_cast<BatchedRandomizer*>(_data_hub->GetDataSetHandler().GetRandomizer(0));
    BatchIndexes_t positive_batches(randomizer->getBatches().size());
    int interval = 11;
    count_t cases = 0;
    measure_t Sc = 0, Sn = 0, batches = 0;
    for (size_t t = 0; t < randomizer->getBatches().size(); ++t) {
        auto& be = randomizer->getBatches()[t];
        if (be.get<2>() != interval || !be.get<0>())
            continue;
        ++batches;
        positive_batches.set(t);
        ++cases;
        Sc += be.get<1>();
    }
    ProbabilitiesRange_t probabilities;
    batchedCalc.CalculateProbabilitiesByTimeInterval(probabilities, cases, batches, Sc, Sn, positive_batches, interval);
    BOOST_REQUIRE_CLOSE(batchedCalc.getLoglikelihoodRatioForInterval(*probabilities, interval), 0.66829395, 0.001);
}

BOOST_AUTO_TEST_SUITE_END()
