//******************************************************************************
#ifndef __ExponentialRandomizer_H
#define __ExponentialRandomizer_H
//******************************************************************************
#include "ContinuousVariableRandomizer.h"

/** class representing a permuted attribute which is a continuous variable
    and censored atribute. */
class PermutedExponentialAttribute : public PermutedVariable {
  protected:
    unsigned short                          guCensoredAttribute;

  public:
    PermutedExponentialAttribute(double dVariable, unsigned short uCensoreddAttribute);
    virtual ~PermutedExponentialAttribute();

    virtual PermutedExponentialAttribute  * Clone() const;
    double                                  Calibrate(double dCalibration) {gdVariable *= dCalibration; return gdVariable; }
    inline unsigned short                   GetCensoredAttribute() const {return guCensoredAttribute;}
};

/** Randomizes data of dataset for a 'Exponetial' probablility model.
    Instead of assigning data to simulation case structures, assigns
    randomized data to simulation meaure structures. */
class ExponentialRandomizer : public AbstractPermutedDataRandomizer {
  public:
    typedef std::vector<SpaceTimeStationaryAttribute> StationaryContainer_t;
    typedef std::vector<PermutedExponentialAttribute> PermutedContainer_t;
     
  protected:
    StationaryContainer_t	        gvStationaryAttribute;
    PermutedContainer_t                 gvOriginalPermutedAttribute;
    PermutedContainer_t                 gvPermutedAttribute;

    void                                Assign(const PermutedContainer_t& vPermutedAttributes,
                                               count_t ** ppCases, measure_t ** ppMeasure,
                                               int iNumTimeIntervals, int iNumTracts) const;
    virtual void                        AssignRandomizedData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet);
    virtual void                        SortPermutedAttribute();

  public:
    ExponentialRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    virtual ~ExponentialRandomizer();

    virtual ExponentialRandomizer     * Clone() const;

    void                                AddPatients(count_t tNumPatients, int iTimeInterval, tract_t tTractIndex, measure_t tContinuousVariable, count_t tCensored);
    void                                AssignCensoredIndividuals(TwoDimCountArray_t& tCensoredArray) const;
    std::vector<double>               & CalculateMaxCirclePopulationArray(std::vector<double>& vMaxCirclePopulation) const;
    double                              CalibrateAndAssign(measure_t tCalibration, RealDataSet& thisDataSet);
};
//******************************************************************************
#endif

