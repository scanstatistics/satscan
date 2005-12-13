//******************************************************************************
#ifndef __OrdinalDataRandomizer_H
#define __OrdinalDataRandomizer_H
//******************************************************************************
#include "DenominatorDataRandomizer.h"
#include "PermutationDataRandomizer.h"

/** Ordinal denominator-data randomizer. */
class OrdinalDenominatorDataRandomizer : public AbstractOrdinalDenominatorDataRandomizer {
  public:
    OrdinalDenominatorDataRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed)
        : AbstractOrdinalDenominatorDataRandomizer(lInitialSeed) {}
    virtual ~OrdinalDenominatorDataRandomizer() {}
    
    virtual OrdinalDenominatorDataRandomizer * Clone() const {return new OrdinalDenominatorDataRandomizer(*this);}

    virtual void        RandomizeData(const RealDataSet& RealSet, SimDataSet& SimSet, unsigned int iSimulation);

    static const size_t gtMaximumCategories;
};

/** Ordinal denominator-data randomizer. Optimaized for purely temporal analyses. */
class OrdinalPurelyTemporalDenominatorDataRandomizer : public AbstractOrdinalDenominatorDataRandomizer {
  public:
    OrdinalPurelyTemporalDenominatorDataRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed)
        : AbstractOrdinalDenominatorDataRandomizer(lInitialSeed) {}
    virtual ~OrdinalPurelyTemporalDenominatorDataRandomizer() {}
    
    virtual OrdinalPurelyTemporalDenominatorDataRandomizer * Clone() const {return new OrdinalPurelyTemporalDenominatorDataRandomizer(*this);}

    virtual void        RandomizeData(const RealDataSet& RealSet, SimDataSet& SimSet, unsigned int iSimulation);
};

typedef StationaryAttribute<std::pair<int, tract_t> >   OrdinalStationary_t;
typedef PermutedAttribute<int>                          OrdinalPermuted_t;

/** Ordinal permuted-data randomizer.

    NOTE: Testing indicates that this randomizer does not perform faster that denominator
          randomizer in situations where it was expected (many categories, many locations,
          fewer cases). As such, it will not be used at this time. */
class OrdinalPermutedDataRandomizer : public AbstractPermutedDataRandomizer<OrdinalStationary_t, OrdinalPermuted_t> {
  protected:
    virtual void                AssignRandomizedData(const RealDataSet& RealSet, SimDataSet& SimSet);
    void                        Setup(const RealDataSet& RealSet);

  public:
    OrdinalPermutedDataRandomizer(const RealDataSet& RealSet, long lInitialSeed=RandomNumberGenerator::glDefaultSeed)
       : AbstractPermutedDataRandomizer<OrdinalStationary_t, OrdinalPermuted_t>(lInitialSeed) { Setup(RealSet); }
    virtual ~OrdinalPermutedDataRandomizer() {}
    
    virtual OrdinalPermutedDataRandomizer * Clone() const {return new OrdinalPermutedDataRandomizer(*this);}
};

typedef StationaryAttribute<int>   OrdinalPurelyTemporalStationary_t;

/** Ordinal permuted-data randomizer. Optimized for purely temporal analyses.

    NOTE: Testing indicates that this randomizer does not perform faster that denominator
          randomizer in situations where it was expected (many categories, many locations,
          fewer cases). As such, it will not be used at this time. */
class OrdinalPurelyTemporalPermutedDataRandomizer : public AbstractPermutedDataRandomizer<OrdinalPurelyTemporalStationary_t, OrdinalPermuted_t> {
  protected:
    virtual void                AssignRandomizedData(const RealDataSet& RealSet, SimDataSet& SimSet);
    void                        Setup(const RealDataSet& RealSet);

  public:
    OrdinalPurelyTemporalPermutedDataRandomizer(const RealDataSet& RealSet, long lInitialSeed=RandomNumberGenerator::glDefaultSeed)
       : AbstractPermutedDataRandomizer<OrdinalPurelyTemporalStationary_t, OrdinalPermuted_t>(lInitialSeed) { Setup(RealSet); }
    virtual ~OrdinalPurelyTemporalPermutedDataRandomizer() {}
    
    virtual OrdinalPurelyTemporalPermutedDataRandomizer * Clone() const {return new OrdinalPurelyTemporalPermutedDataRandomizer(*this);}
};
//******************************************************************************
#endif

