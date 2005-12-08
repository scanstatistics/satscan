//******************************************************************************
#ifndef __ExponentialRandomizer_H
#define __ExponentialRandomizer_H
//******************************************************************************
#include "ContinuousVariableRandomizer.h"

typedef StationaryAttribute<std::pair<int, tract_t> >         ExponentialStationary_t;
typedef PermutedAttribute<std::pair<double, unsigned short> > ExponentialPermuted_t;

/** Randomizes data of dataset for a 'Exponetial' probablility model.
    Instead of assigning data to simulation case structures, assigns
    randomized data to simulation meaure structures. */
class ExponentialRandomizer : public AbstractPermutedDataRandomizer<ExponentialStationary_t, ExponentialPermuted_t> {
  protected:
    void                                Assign(const PermutedContainer_t& vPermutedAttributes,
                                               count_t ** ppCases, measure_t ** ppMeasure,
                                               int iNumTimeIntervals, int iNumTracts) const;
    virtual void                        AssignRandomizedData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet);
    virtual void                        SortPermutedAttribute();

  public:
    ExponentialRandomizer::ExponentialRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed)
                          :AbstractPermutedDataRandomizer<ExponentialStationary_t, ExponentialPermuted_t> (lInitialSeed) {}
    virtual ~ExponentialRandomizer();

    virtual ExponentialRandomizer     * Clone() const;

    void                                AddPatients(count_t tNumPatients, int iTimeInterval, tract_t tTractIndex, measure_t tContinuousVariable, count_t tCensored);
    void                                AssignCensoredIndividuals(TwoDimCountArray_t& tCensoredArray) const;
    std::vector<double>               & CalculateMaxCirclePopulationArray(std::vector<double>& vMaxCirclePopulation) const;
    double                              CalibrateAndAssign(measure_t tCalibration, RealDataSet& thisDataSet);
};
//******************************************************************************
#endif

