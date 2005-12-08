//******************************************************************************
#ifndef __ContinuousVariableRandomizer_H
#define __ContinuousVariableRandomizer_H
//******************************************************************************
#include "PermutationDataRandomizer.h"

typedef StationaryAttribute<std::pair<int, tract_t> >   ContinuousVariableStationary_t;
typedef PermutedAttribute<double>                       ContinuousVariablePermuted_t;

/** Randomizer which has a stationary space-time attribute and a randomized continuous variable. */
class ContinuousVariableRandomizer : public AbstractPermutedDataRandomizer<ContinuousVariableStationary_t, ContinuousVariablePermuted_t> {
  protected:
    virtual void                SortPermutedAttribute();

  public:
    ContinuousVariableRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    virtual ~ContinuousVariableRandomizer();
};

/** constructor */
ContinuousVariableRandomizer::ContinuousVariableRandomizer(long lInitialSeed)
                             :AbstractPermutedDataRandomizer<ContinuousVariableStationary_t, ContinuousVariablePermuted_t>(lInitialSeed) {}

/** Randomizes data of dataset for a 'normal' probablility model.
    Instead of assigning data to simulation case structures, assigns
    randomized data to simulation meaure structures. */
class NormalRandomizer : public ContinuousVariableRandomizer {
  protected:
    virtual void               AssignRandomizedData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet);

  public:
    NormalRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    virtual ~NormalRandomizer();

    virtual NormalRandomizer * Clone() const;
    void                       AddCase(int iTimeInterval, tract_t tTractIndex, measure_t tContinuousVariable);
    void                       AssignMeasure(measure_t ** ppMeasure, measure_t ** ppSqMeasure, int iNumTimeIntervals, int iNumTracts);
};

/** Randomizes data of dataset for a 'normal' probablility model.
    Instead of assigning data to simulation case structures, assigns
    randomized data to simulation meaure structures. */
class RankRandomizer : public ContinuousVariableRandomizer {
  protected:
    virtual void             AssignRandomizedData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet);

  public:
    RankRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    virtual ~RankRandomizer();

    virtual RankRandomizer * Clone() const;
    void                     AddCase(int iTimeInterval, tract_t tTractIndex, measure_t tContinuousVariable);
    void                     AssignMeasure(measure_t ** ppMeasure, int iNumTimeIntervals, int iNumTracts);
};
//******************************************************************************
#endif

