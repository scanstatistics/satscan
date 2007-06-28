//******************************************************************************
#ifndef NormalRandomizerH
#define NormalRandomizerH
//******************************************************************************
#include "PermutationDataRandomizer.h"
#include "AbstractNormalDataRandomizer.h"

typedef StationaryAttribute<std::pair<int, tract_t> >   NormalStationary_t;
typedef PermutedAttribute<double>                       NormalPermuted_t;

/** */
class AbstractNormalRandomizer :  public AbstractNormalDataRandomizer, public AbstractPermutedDataRandomizer<NormalStationary_t, NormalPermuted_t>{
   public:
     AbstractNormalRandomizer(long lInitialSeed) : AbstractPermutedDataRandomizer<NormalStationary_t, NormalPermuted_t>(lInitialSeed) {}
     virtual ~AbstractNormalRandomizer() {}

    virtual void               AddCase(count_t tCount, int iTimeInterval, tract_t tTractIndex, measure_t tContinuousVariable);
    virtual void               AssignFromAttributes(RealDataSet& RealSet);
    virtual void               RemoveCase(int iTimeInterval, tract_t tTractIndex);
};

/** Randomizes data of dataset for a 'normal' probablility model.
    Instead of assigning data to simulation case structures, assigns
    randomized data to simulation measure structures. */
class NormalRandomizer : public AbstractNormalRandomizer {
  protected:
    virtual void               AssignRandomizedData(const RealDataSet& RealSet, DataSet& SimSet);

  public:
    NormalRandomizer::NormalRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed) : AbstractNormalRandomizer(lInitialSeed) {}
    virtual ~NormalRandomizer() {}

    virtual NormalRandomizer * Clone() const {return new NormalRandomizer(*this);}
};

/** Randomizes data of dataset for a 'normal' probablility model.
    Instead of assigning data to simulation case structures, assigns
    randomized data to simulation meaure structures. */
class NormalPurelyTemporalRandomizer : public AbstractNormalRandomizer {
  protected:
    virtual void               AssignRandomizedData(const RealDataSet& RealSet, DataSet& SimSet);

  public:
    NormalPurelyTemporalRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed) : AbstractNormalRandomizer(lInitialSeed) {}
    virtual ~NormalPurelyTemporalRandomizer() {}

    virtual NormalPurelyTemporalRandomizer * Clone() const {return new NormalPurelyTemporalRandomizer(*this);}
};
//******************************************************************************
#endif

