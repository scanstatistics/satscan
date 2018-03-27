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

    /* 
      Lower U1 implies high cluster, lower U2 implies low cluster. 
       I believe the pre-test will enforce either U1 or U2 are always lower than other. 
       e.g. The pre-test for high rate clusters will never allow this function to be called when U2 is lower than U1 -- I think. 
      
      The last I spoke with Martin, he was indicating that U shouldn't be used as the return test statistic. He mentioned a p-value, then
      maybe the normal distribution approximation. I need to clarify this ...
    */
    return std::min(U1, U2);
}

/**  */
double WilcoxonLikelihoodCalculator::CalcLogLikelihoodRatio(count_t n, measure_t u, size_t tSetIndex) const {
    count_t   N = gvDataSetTotals[tSetIndex].first;
    measure_t U = gvDataSetTotals[tSetIndex].second;

    double U1 = MannWhitneyU(static_cast<double>(n), static_cast<double>(N - n), u);
    double U2 = MannWhitneyU(static_cast<double>(N - n), static_cast<double>(n), (U - u));

    /*
    Lower U1 implies high cluster, lower U2 implies low cluster.
    I believe the pre-test will enforce either U1 or U2 are always lower than other.
    e.g. The pre-test for high rate clusters will never allow this function to be called when U2 is lower than U1 -- I think.

    The last I spoke with Martin, he was indicating that U shouldn't be used as the return test statistic. He mentioned a p-value, then
    maybe the normal distribution approximation. I need to clarify this ...
    */
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

    /*
    Lower U1 implies high cluster, lower U2 implies low cluster.
    I believe the pre-test will enforce either U1 or U2 are always lower than other.
    e.g. The pre-test for high rate clusters will never allow this function to be called when U2 is lower than U1 -- I think.

    The last I spoke with Martin, he was indicating that U shouldn't be used as the return test statistic. He mentioned a p-value, then
    maybe the normal distribution approximation. I need to clarify this ...
    */
    return std::min(U1, U2);
}

/**  */
double WilcoxonLikelihoodCalculator::GetLogLikelihoodForTotal(size_t tSetIndex) const {
    return 0.0;
}
