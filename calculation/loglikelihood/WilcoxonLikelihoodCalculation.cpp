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
double WilcoxonLikelihoodCalculator::CalcLogLikelihood(count_t n, measure_t u) const {
    count_t   N = gvDataSetTotals[0].first;
    measure_t U = gvDataSetTotals[0].second;

    double U1 = MannWhitneyU(static_cast<double>(n), static_cast<double>(N - n), u);
    double U2 = MannWhitneyU(static_cast<double>(N - n), static_cast<double>(n), (U - u));

    return std::min(U1, U2);
}

/**  */
double WilcoxonLikelihoodCalculator::CalcLogLikelihoodRatio(count_t n, measure_t u, size_t tSetIndex) const {
    count_t   N = gvDataSetTotals[tSetIndex].first;
    measure_t U = gvDataSetTotals[tSetIndex].second;

    double U1 = MannWhitneyU(static_cast<double>(n), static_cast<double>(N - n), u);
    double U2 = MannWhitneyU(static_cast<double>(N - n), static_cast<double>(n), (U - u));

    return std::min(U1, U2);
}

/**  */
double WilcoxonLikelihoodCalculator::CalculateFullStatistic(double dMaximizingValue, size_t tSetIndex) const {
    return dMaximizingValue;
}

/**  */
double WilcoxonLikelihoodCalculator::CalculateMaximizingValue(count_t n, measure_t u, size_t tDataSetIndex) const {
    count_t   N = gvDataSetTotals[tDataSetIndex].first;
    measure_t U = gvDataSetTotals[tDataSetIndex].second;

    double U1 = MannWhitneyU(static_cast<double>(n), static_cast<double>(N - n), u);
    double U2 = MannWhitneyU(static_cast<double>(N - n), static_cast<double>(n), (U - u));

    return std::min(U1, U2);
}

/**  */
double WilcoxonLikelihoodCalculator::GetLogLikelihoodForTotal(size_t tSetIndex) const {
    return 0.0;
}
