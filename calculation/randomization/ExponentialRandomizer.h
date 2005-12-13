//******************************************************************************
#ifndef __ExponentialRandomizer_H
#define __ExponentialRandomizer_H
//******************************************************************************
#include "PermutationDataRandomizer.h"

typedef StationaryAttribute<std::pair<int, tract_t> >         ExponentialStationary_t;
typedef PermutedAttribute<std::pair<double, unsigned short> > ExponentialPermuted_t;

/** Abstraction for Exponential model data randomization. Derives from class AbstractPermutedDataRandomizer,
    defining stationary attribute as pair (time interval index, tract index) and permuted attribute as
    pair (continuous variable, censoring). */
class AbstractExponentialRandomizer : public AbstractPermutedDataRandomizer<ExponentialStationary_t, ExponentialPermuted_t> {
   public:
     AbstractExponentialRandomizer(long lInitialSeed) : AbstractPermutedDataRandomizer<ExponentialStationary_t, ExponentialPermuted_t>(lInitialSeed) {}
     virtual ~AbstractExponentialRandomizer() {}

    void                       AddPatients(count_t tNumPatients, int iTimeInterval, tract_t tTractIndex, measure_t tContinuousVariable, count_t tCensored);
    void                       AssignFromAttributes(count_t tTotalCases, measure_t tTotalMeasure, RealDataSet& RealSet);
};

/** Exponential model data randomizer, derives from class AbstractExponentialRandomizer to define appropriate
    functionality needed to assigned randomized data to SimDataSet object. */
class ExponentialRandomizer : public AbstractExponentialRandomizer {
  protected:
    virtual void                        AssignRandomizedData(const RealDataSet& RealSet, SimDataSet& SimSet);

  public:
    ExponentialRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed) : AbstractExponentialRandomizer(lInitialSeed) {}
    virtual ~ExponentialRandomizer() {}

    virtual ExponentialRandomizer     * Clone() const {return new ExponentialRandomizer(*this);}

    std::vector<double>               & CalculateMaxCirclePopulationArray(std::vector<double>& vMaxCirclePopulation) const;
};

/** Exponential model data randomizer, derives from class AbstractExponentialRandomizer to define appropriate
    functionality needed to assigned randomized data to SimDataSet object. Optimized for purely temporal analyses. */
class ExponentialPurelyTemporalRandomizer : public AbstractExponentialRandomizer {
  protected:
    virtual void                        AssignRandomizedData(const RealDataSet& RealSet, SimDataSet& SimSet);

  public:
    ExponentialPurelyTemporalRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed) : AbstractExponentialRandomizer(lInitialSeed) {}
    virtual ~ExponentialPurelyTemporalRandomizer() {}

    virtual ExponentialPurelyTemporalRandomizer     * Clone() const {return new ExponentialPurelyTemporalRandomizer(*this);}
};
//******************************************************************************
#endif

