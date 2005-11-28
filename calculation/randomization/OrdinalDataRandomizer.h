//******************************************************************************
#ifndef __OrdinalDataRandomizer_H
#define __OrdinalDataRandomizer_H
//******************************************************************************
#include "DenominatorDataRandomizer.h"
#include "ContinuousVariableRandomizer.h"

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

/** Randomizer which has a stationary space-time attribute and a randomized continuous variable.

    NOTE: Testing indicates that this randomizer does not perform faster that denominator
          randomizer in situations where it was expected (many categories, many locations,
          fewer cases). As such, it will not be used at this time. */
class OrdinalPermutedDataRandomizer : public AbstractPermutedDataRandomizer {
  public:
    typedef std::vector<SpaceTimeStationaryAttribute> StationaryContainer_t;
    typedef std::vector<PermutedAttribute<int> >      PermutedContainer_t;

  protected:
    StationaryContainer_t       gvStationaryAttribute;
    PermutedContainer_t         gvOriginalPermutedAttribute;
    PermutedContainer_t         gvPermutedAttribute;

    virtual void                AssignRandomizedData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet);
    void                        CreateRandomizationData(const RealDataSet& thisRealSet);
    virtual void                SortPermutedAttribute();

  public:
    OrdinalPermutedDataRandomizer(const RealDataSet& thisRealSet, long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    virtual ~OrdinalPermutedDataRandomizer();
    virtual OrdinalPermutedDataRandomizer * Clone() const;
};
//******************************************************************************
#endif

