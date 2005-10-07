//******************************************************************************
#ifndef __ExponentialRandomizer_H
#define __ExponentialRandomizer_H
//******************************************************************************
#include "ContinuousVariableRandomizer.h"

/** class representing a permuted attribute which is a continuous variable
    and censored atribute. */
class PermutedExponentialAttributes : public PermutedVariable {
  protected:
    unsigned short            guCensoredAttribute;

  public:
    PermutedExponentialAttributes(double dVariable, unsigned short uCensoreddAttribute, unsigned int iOrderIndex);
    virtual ~PermutedExponentialAttributes();

    virtual PermutedExponentialAttributes * Clone() const;
    double                                  Calibrate(double dCalibration) {gdVariable *= dCalibration; return gdVariable; }
    inline unsigned short                   GetCensoredAttribute() const {return guCensoredAttribute;}
};

/** Randomizes data of dataset for a 'Exponetial' probablility model.
    Instead of assigning data to simulation case structures, assigns
    randomized data to simulation meaure structures. */
class ExponentialRandomizer : public AbstractPermutedDataRandomizer {
  protected:
    std::vector<SpaceTimeStationaryAttribute>	   gvStationaryAttribute;
    ZdPointerVector<PermutedExponentialAttributes> gvPermutedAttribute;
    unsigned int                                   giOrderIndex;

    virtual void                                AssignRandomizedData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet);
    virtual void                                SortPermutedAttribute();

  public:
    ExponentialRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    virtual ~ExponentialRandomizer();

    virtual ExponentialRandomizer     * Clone() const;

    void                                AddPatients(count_t tNumPatients, int iTimeInterval, tract_t tTractIndex, measure_t tContinuousVariable, count_t tCensored);
    void                                Assign(count_t ** ppCases, measure_t ** ppMeasure, int iNumTimeIntervals, int iNumTracts) const;
    void                                AssignCensoredIndividuals(TwoDimCountArray_t& tCensoredArray) const;
    std::vector<double>               & CalculateMaxCirclePopulationArray(std::vector<double>& vMaxCirclePopulation) const;
    double                              Calibrate(measure_t tCalibration);
};
//******************************************************************************
#endif

