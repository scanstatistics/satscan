//******************************************************************************
#ifndef __DenominatorDataRandomizer_H
#define __DenominatorDataRandomizer_H
//******************************************************************************
#include "Randomizer.h"

/** abstract denominator data randomizer class */
class AbstractDenominatorDataRandomizer : public AbstractRandomizer {
  protected:
    BinomialGenerator   gBinomialGenerator;

  public:
            AbstractDenominatorDataRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    virtual ~AbstractDenominatorDataRandomizer();
};

/** abstract ordinal denominator data randomizer class */
class AbstractOrdinalDenominatorDataRandomizer : public AbstractDenominatorDataRandomizer {
  protected:

    void                MakeDataB(count_t tTotalCounts, measure_t tTotalMeasure, std::vector<count_t>& RandCounts);
    void                RandomizeOrdinalData(count_t tTotalCases, count_t tTotalControls,
                                             count_t** ppSimCases, measure_t** ppMeasure,
                                             int tNumTracts, int tNumTimeIntervals);

  public:
            AbstractOrdinalDenominatorDataRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    virtual ~AbstractOrdinalDenominatorDataRandomizer();
};
//******************************************************************************
#endif

