//******************************************************************************
#ifndef __OrdinalDataRandomizer_H
#define __OrdinalDataRandomizer_H
//******************************************************************************
#include "DenominatorDataRandomizer.h"
#include "PermutationDataRandomizer.h"

/** Randomizes data of dataset for a 'Ordinal' probablility model. */
class OrdinalDenominatorDataRandomizer : public AbstractOrdinalDenominatorDataRandomizer {
  protected:
    void                SetMeasure(const RealDataSet& thisRealSet, TwoDimensionArrayHandler<measure_t>& Measure);

  public:
    OrdinalDenominatorDataRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    virtual ~OrdinalDenominatorDataRandomizer();
    virtual OrdinalDenominatorDataRandomizer * Clone() const;

    virtual void        RandomizeData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet, unsigned int iSimulation);

    static const size_t gtMaximumCategories;
};

typedef StationaryAttribute<std::pair<int, count_t> >   OrdinalStationary_t;
typedef PermutedAttribute<int>                          OrdinalPermuted_t;

/** Randomizer which has a stationary space-time attribute and a randomized continuous variable.

    NOTE: Testing indicates that this randomizer does not perform faster that denominator
          randomizer in situations where it was expected (many categories, many locations,
          fewer cases). As such, it will not be used at this time. */
class OrdinalPermutedDataRandomizer : public AbstractPermutedDataRandomizer<OrdinalStationary_t, OrdinalPermuted_t> {
  protected:
    virtual void                AssignRandomizedData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet);
    void                        CreateRandomizationData(const RealDataSet& thisRealSet);
    virtual void                SortPermutedAttribute();

  public:
           OrdinalPermutedDataRandomizer::OrdinalPermutedDataRandomizer(const RealDataSet& thisRealSet, long lInitialSeed=RandomNumberGenerator::glDefaultSeed)
                                         :AbstractPermutedDataRandomizer<OrdinalStationary_t, OrdinalPermuted_t>(lInitialSeed) {
               CreateRandomizationData(thisRealSet);
           }
    virtual ~OrdinalPermutedDataRandomizer() {}
    virtual OrdinalPermutedDataRandomizer * Clone() const;
};

//******************************************************************************
#endif

