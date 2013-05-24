//******************************************************************************
#ifndef __DenominatorDataRandomizer_H
#define __DenominatorDataRandomizer_H
//******************************************************************************
#include "Randomizer.h"

/** Abstraction for denominator data randomizer. */
class AbstractDenominatorDataRandomizer : public AbstractRandomizer {
  protected:
    BinomialGenerator   gBinomialGenerator;

  public:
    AbstractDenominatorDataRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed) : AbstractRandomizer(lInitialSeed) {}
    virtual ~AbstractDenominatorDataRandomizer() {}
};

/** Abstraction for ordinal denominator data randomizers. */
class AbstractOrdinalDenominatorDataRandomizer : public AbstractDenominatorDataRandomizer {
  protected:
    typedef std::deque<boost::gregorian::greg_weekday> WeekDaysContainer_t;
    WeekDaysContainer_t _weekdays;

    void                MakeDataB(count_t tTotalCounts, measure_t tTotalMeasure, std::vector<count_t>& RandCounts);
    void                RandomizeOrdinalData(count_t tNumCases, 
                                             count_t tNumControls, 
                                             count_t** ppSimCases,
                                             measure_t** ppMeasure, 
                                             int tNumTracts, 
                                             int tNumTimeIntervals);
    void                RandomizePurelyTemporalOrdinalData(count_t tNumCases, 
                                                           count_t tNumControls,
                                                           count_t* pSimCases, 
                                                           measure_t* pMeasure, 
                                                           int tNumTimeIntervals);
    void                RandomizeOrdinalDataForDayOfWeek(count_t totalCasesForDay, 
                                                         count_t totalControlForDay, 
                                                         count_t** ppSimCases,
                                                         measure_t** ppMeasure, 
                                                         int tNumTracts, 
                                                         int lastTimeInterval,
                                                         bool lastDay,
                                                         count_t tTotalCases, 
                                                         count_t tTotalControls,
                                                         int tNumTimeIntervals);
    void                RandomizePurelyTemporalOrdinalDataForDayOfWeek(count_t totalCasesForDay, 
                                                                       count_t totalControlForDay, 
                                                                       count_t* pSimCases,
                                                                       measure_t* pMeasure, 
                                                                       int tNumTracts, 
                                                                       int lastTimeInterval,
                                                                       bool lastDay,
                                                                       count_t tTotalCases, 
                                                                       count_t tTotalControls,
                                                                       int tNumTimeIntervals);

  public:
    AbstractOrdinalDenominatorDataRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed) : AbstractDenominatorDataRandomizer(lInitialSeed) {}
    AbstractOrdinalDenominatorDataRandomizer(boost::gregorian::greg_weekday last_weekday, long lInitialSeed=RandomNumberGenerator::glDefaultSeed);

    virtual ~AbstractOrdinalDenominatorDataRandomizer() {}
};
//******************************************************************************
#endif

