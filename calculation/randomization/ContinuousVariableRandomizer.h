//******************************************************************************
#ifndef __ContinuousVariableRandomizer_H
#define __ContinuousVariableRandomizer_H
//******************************************************************************
#include "PermutationDataRandomizer.h"

/** class representing a premuted attribute which is a continuous variable,
    or any other numberic attribute. */
class PermutedVariable : public PermutedAttribute {
  protected:
    double		      gdVariable;

  public:
    PermutedVariable(double dVariable);
    virtual ~PermutedVariable();

    virtual PermutedVariable* Clone() const;
    inline double	      GetVariable() const {return gdVariable;}
};

/** class representing the stationary space-time attributes in a permutated randomization. */
class SpaceTimeStationaryAttribute {
  protected:
    int		        giTimeIntervalIndex;
    count_t             gtTractIndex;

  public:
    SpaceTimeStationaryAttribute(int iTimeInterval, count_t tTractIndex);
    virtual ~SpaceTimeStationaryAttribute();
    virtual SpaceTimeStationaryAttribute * Clone() const;

    inline int		GetTimeInterval() const {return giTimeIntervalIndex;}
    inline int		GetTractIndex() const {return gtTractIndex;}
};

/** Randomizer which has a stationary space-time attribute
    and a randomized continuous variable. */
class ContinuousVariableRandomizer : public AbstractPermutedDataRandomizer {
  protected:
    std::vector<SpaceTimeStationaryAttribute>	gvStationaryAttribute;
    ZdPointerVector<PermutedVariable>           gvPermutedAttribute;

    virtual void                                SortPermutedAttribute();
    
  public:
    ContinuousVariableRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    virtual ~ContinuousVariableRandomizer();
};

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
    void                       AddCase(int iTimeInterval, tract_t tTractIndex, measure_t tContinuosVariable);
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
    void                     AddCase(int iTimeInterval, tract_t tTractIndex, measure_t tContinuosVariable);
    void                     AssignMeasure(measure_t ** ppMeasure, int iNumTimeIntervals, int iNumTracts);
};
//******************************************************************************
#endif

