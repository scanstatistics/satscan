//******************************************************************************
#ifndef __ExponentialRandomizer_H
#define __ExponentialRandomizer_H
//******************************************************************************
#include "PermutationDataRandomizer.h"

typedef StationaryAttribute<std::pair<int, tract_t> >         ExponentialStationary_t;
typedef PermutedAttribute<std::pair<double, unsigned short> > ExponentialPermuted_t;

class CSaTScanData; // forward class declaration

/** Abstraction for Exponential model data randomization. Derives from class AbstractPermutedDataRandomizer,
    defining stationary attribute as pair (time interval index, tract index) and permuted attribute as
    pair (continuous variable, censoring). */
class AbstractExponentialRandomizer : public AbstractPermutedDataRandomizer<ExponentialStationary_t, ExponentialPermuted_t> {
   public:
     AbstractExponentialRandomizer(const CSaTScanData& dataHub, long lInitialSeed);
     virtual ~AbstractExponentialRandomizer() {}

    void                       AddPatients(count_t tNumPatients, Julian date, tract_t tTractIndex, measure_t tContinuousVariable, count_t tCensored);
    void                       AssignFromAttributes(RealDataSet& RealSet);
    virtual void               RemoveCase(int iTimeInterval, tract_t tTractIndex);
};

/** Exponential model data randomizer, derives from class AbstractExponentialRandomizer to define appropriate
    functionality needed to assigned randomized data to DataSet object. */
class ExponentialRandomizer : public AbstractExponentialRandomizer {
  protected:
    virtual void                        AssignRandomizedData(const RealDataSet& RealSet, DataSet& SimSet);

  public:
    ExponentialRandomizer(const CSaTScanData& dataHub, long lInitialSeed=RandomNumberGenerator::glDefaultSeed) : AbstractExponentialRandomizer(dataHub, lInitialSeed) {}
    virtual ~ExponentialRandomizer() {}

    virtual ExponentialRandomizer     * Clone() const {return new ExponentialRandomizer(*this);}

    std::vector<double>               & CalculateMaxCirclePopulationArray(std::vector<double>& vMaxCirclePopulation, bool bZeroFirst=true) const;
};

/** Exponential model data randomizer, derives from class AbstractExponentialRandomizer to define appropriate
    functionality needed to assigned randomized data to DataSet object. Optimized for purely temporal analyses. */
class ExponentialPurelyTemporalRandomizer : public AbstractExponentialRandomizer {
  protected:
    virtual void                        AssignRandomizedData(const RealDataSet& RealSet, DataSet& SimSet);

  public:
    ExponentialPurelyTemporalRandomizer(const CSaTScanData& dataHub, long lInitialSeed=RandomNumberGenerator::glDefaultSeed) : AbstractExponentialRandomizer(dataHub, lInitialSeed) {}
    virtual ~ExponentialPurelyTemporalRandomizer() {}

    virtual ExponentialPurelyTemporalRandomizer     * Clone() const {return new ExponentialPurelyTemporalRandomizer(*this);}
};
//******************************************************************************
#endif

