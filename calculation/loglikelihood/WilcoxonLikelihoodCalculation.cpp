//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "WilcoxonLikelihoodCalculation.h"

/** constructor */
WilcoxonLikelihoodCalculator::WilcoxonLikelihoodCalculator(const CSaTScanData& DataHub)
                              :AbstractLikelihoodCalculator(DataHub) {}

/** destructor */
WilcoxonLikelihoodCalculator::~WilcoxonLikelihoodCalculator() {}

/**  */
double WilcoxonLikelihoodCalculator::CalcLogLikelihood(count_t n1, measure_t r1) const {
    count_t   n2 = gvDataSetTotals[0].first - n1;
    measure_t r2 = gvDataSetTotals[0].second - r1;
    double test = TestStatisticZ(std::min(MannWhitneyU(static_cast<double>(n1), n2, r1), MannWhitneyU(n2, static_cast<double>(n1), r2)), n1, n2);
    return TestStatisticZ(std::min(MannWhitneyU(static_cast<double>(n1), n2, r1), MannWhitneyU(n2, static_cast<double>(n1), r2)), n1, n2);
}

/**  */
double WilcoxonLikelihoodCalculator::CalcLogLikelihoodRatio(count_t n1, measure_t r1, size_t tSetIndex) const {
    count_t   n2 = gvDataSetTotals[0].first - n1;
    measure_t r2 = gvDataSetTotals[0].second - r1;
    double test = TestStatisticZ(std::min(MannWhitneyU(static_cast<double>(n1), n2, r1), MannWhitneyU(n2, static_cast<double>(n1), r2)), n1, n2);
    return TestStatisticZ(std::min(MannWhitneyU(static_cast<double>(n1), n2, r1), MannWhitneyU(n2, static_cast<double>(n1), r2)), n1, n2);
}

/**  */
double WilcoxonLikelihoodCalculator::CalculateFullStatistic(double dMaximizingValue, size_t tSetIndex) const {
    return dMaximizingValue;
}

/**  */
double WilcoxonLikelihoodCalculator::CalculateMaximizingValue(count_t n1, measure_t r1, size_t tDataSetIndex) const {
    count_t   n2 = gvDataSetTotals[0].first - n1;
    measure_t r2 = gvDataSetTotals[0].second - r1;
    double test = TestStatisticZ(std::min(MannWhitneyU(static_cast<double>(n1), n2, r1), MannWhitneyU(n2, static_cast<double>(n1), r2)), n1, n2);
    return TestStatisticZ(std::min(MannWhitneyU(static_cast<double>(n1), n2, r1), MannWhitneyU(n2, static_cast<double>(n1), r2)), n1, n2);
}

/**  */
double WilcoxonLikelihoodCalculator::GetLogLikelihoodForTotal(size_t tSetIndex) const {
    return 0.0;
}
