//******************************************************************************
#ifndef __ExponentialRandomizer_H
#define __ExponentialRandomizer_H
//******************************************************************************
#include "ContinuousVariableRandomizer.h"

/** class representing a permuted attribute which is a continuous variable
    and censored atribute. */
class PermutedExponentialAttributes : public PermutedVariable {
  protected:
    unsigned short            guCensored;

  public:
    PermutedExponentialAttributes(double dVariable, unsigned short uCensored);
    virtual ~PermutedExponentialAttributes();

    virtual PermutedExponentialAttributes * Clone() const;
    inline unsigned short                   GetCensored() const {return guCensored;}
};

/** Randomizes data of dataset for a 'Exponetial' probablility model.
    Instead of assigning data to simulation case structures, assigns
    randomized data to simulation meaure structures. */
class ExponentialRandomizer : public AbstractPermutedDataRandomizer {
  protected:
    std::vector<SpaceTimeStationaryAttribute>	   gvStationaryAttribute;
    //$$ std::vector<PermutedExponentialAttributes>	  gvPermutedAttribute;
    ZdPointerVector<PermutedExponentialAttributes> gvPermutedAttribute;

    virtual void                                AssignRandomizedData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet);
    virtual void                                SortPermutedAttribute();

  public:
    ExponentialRandomizer();
    virtual ~ExponentialRandomizer();

    virtual ExponentialRandomizer     * Clone() const;

    void                                AddCase(int iTimeInterval, tract_t tTractIndex, measure_t tContinuosVariable, count_t tCensored);
    void                                Assign(count_t ** ppCases, measure_t ** ppMeasure, int iNumTimeIntervals, int iNumTracts);
};
//******************************************************************************
#endif

