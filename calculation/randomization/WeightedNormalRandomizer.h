//******************************************************************************
#ifndef WeightedNormalRandomizerH
#define WeightedNormalRandomizerH
//******************************************************************************
#include "PermutationDataRandomizer.h"
#include "AbstractNormalDataRandomizer.h"

typedef StationaryAttribute<std::pair<int, tract_t> >   WeightedNormalStationary_t;
typedef PermutedAttribute<std::pair<double, double> >   WeightedNormalPermuted_t;

/** */
class AbstractWeightedNormalRandomizer :  public AbstractNormalDataRandomizer, public AbstractPermutedDataRandomizer<WeightedNormalStationary_t, WeightedNormalPermuted_t>{
   public:
     AbstractWeightedNormalRandomizer(long lInitialSeed) : AbstractPermutedDataRandomizer<WeightedNormalStationary_t, WeightedNormalPermuted_t>(lInitialSeed) {}
     virtual ~AbstractWeightedNormalRandomizer() {}

    virtual void               AddCase(count_t tCount, int iTimeInterval, tract_t tTractIndex, measure_t tContinuousVariable, double dWeight);
    virtual void               AssignFromAttributes(RealDataSet& RealSet);
    virtual void               RemoveCase(int iTimeInterval, tract_t tTractIndex);
};

/** Randomizes data of dataset for a 'normal' probablility model.
    Instead of assigning data to simulation case structures, assigns
    randomized data to simulation measure structures. */
class WeightedNormalRandomizer : public AbstractWeightedNormalRandomizer {
  protected:
    virtual void               AssignRandomizedData(const RealDataSet& RealSet, DataSet& SimSet);

  public:
    WeightedNormalRandomizer::WeightedNormalRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed) : AbstractWeightedNormalRandomizer(lInitialSeed) {}
    virtual ~WeightedNormalRandomizer() {}

    virtual WeightedNormalRandomizer * Clone() const {return new WeightedNormalRandomizer(*this);}
};

/** Randomizes data of dataset for a 'normal' probablility model.
    Instead of assigning data to simulation case structures, assigns
    randomized data to simulation meaure structures. */
class WeightedNormalPurelyTemporalRandomizer : public AbstractWeightedNormalRandomizer {
  protected:
    virtual void               AssignRandomizedData(const RealDataSet& RealSet, DataSet& SimSet);

  public:
    WeightedNormalPurelyTemporalRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed) : AbstractWeightedNormalRandomizer(lInitialSeed) {}
    virtual ~WeightedNormalPurelyTemporalRandomizer() {}

    virtual WeightedNormalPurelyTemporalRandomizer * Clone() const {return new WeightedNormalPurelyTemporalRandomizer(*this);}
};
//******************************************************************************
#endif

