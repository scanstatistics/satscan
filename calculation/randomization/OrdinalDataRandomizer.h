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

    virtual void        RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation);

    static const size_t gtMaximumCategories;
};

/** Ordinal denominator-data randomizer. Optimaized for purely temporal analyses. */
class OrdinalPurelyTemporalDenominatorDataRandomizer : public AbstractOrdinalDenominatorDataRandomizer {
  public:
    OrdinalPurelyTemporalDenominatorDataRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed)
        : AbstractOrdinalDenominatorDataRandomizer(lInitialSeed) {}
    virtual ~OrdinalPurelyTemporalDenominatorDataRandomizer() {}
    
    virtual OrdinalPurelyTemporalDenominatorDataRandomizer * Clone() const {return new OrdinalPurelyTemporalDenominatorDataRandomizer(*this);}

    virtual void        RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation);
};

/** Ordinal denominator-data randomizer with day of week adjustment. 
    This randomizer for the day of week adjustment is half-baked.
*/
//
//class OrdinalDenominatorDataDayOfWeekRandomizer : public AbstractOrdinalDenominatorDataRandomizer {
//  public:
//    OrdinalDenominatorDataDayOfWeekRandomizer(boost::gregorian::greg_weekday last_weekday, long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
//    virtual ~OrdinalDenominatorDataDayOfWeekRandomizer() {}
//
//    virtual OrdinalDenominatorDataDayOfWeekRandomizer * Clone() const {return new OrdinalDenominatorDataDayOfWeekRandomizer(*this);}
//
//    virtual void  RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation);
//};

class CSaTScanData; // forward class declaration

typedef StationaryAttribute<std::pair<int, tract_t> >   OrdinalStationary_t;
typedef PermutedAttribute<int>                          OrdinalPermuted_t;

class AbstractOrdinalPermutedDataRandomizer {
    public:
    virtual void setPermutedData(const RealDataSet& RealSet) = 0;
};

/** Ordinal permuted-data randomizer.

    NOTE: Testing indicates that this randomizer does not perform faster that denominator
          randomizer in situations where it was expected (many categories, many locations,
          fewer cases). As such, it will not be used at this time. */
class OrdinalPermutedDataRandomizer : public AbstractPermutedDataRandomizer<OrdinalStationary_t, OrdinalPermuted_t>, public AbstractOrdinalPermutedDataRandomizer {
  protected:
    virtual void                AssignRandomizedData(const RealDataSet& RealSet, DataSet& SimSet);

  public:
    OrdinalPermutedDataRandomizer(const CSaTScanData& dataHub, long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    virtual ~OrdinalPermutedDataRandomizer() {}
    
    virtual OrdinalPermutedDataRandomizer * Clone() const {return new OrdinalPermutedDataRandomizer(*this);}
    virtual void setPermutedData(const RealDataSet& RealSet);
};

typedef StationaryAttribute<int>   OrdinalPurelyTemporalStationary_t;

/** Ordinal permuted-data randomizer. Optimized for purely temporal analyses.

    NOTE: Testing indicates that this randomizer does not perform faster that denominator
          randomizer in situations where it was expected (many categories, many locations,
          fewer cases). As such, it will not be used at this time. */
class OrdinalPurelyTemporalPermutedDataRandomizer : public AbstractPermutedDataRandomizer<OrdinalPurelyTemporalStationary_t, OrdinalPermuted_t>, public AbstractOrdinalPermutedDataRandomizer {
  protected:
    virtual void                AssignRandomizedData(const RealDataSet& RealSet, DataSet& SimSet);

  public:
    OrdinalPurelyTemporalPermutedDataRandomizer(const CSaTScanData& dataHub, long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    virtual ~OrdinalPurelyTemporalPermutedDataRandomizer() {}
    
    virtual OrdinalPurelyTemporalPermutedDataRandomizer * Clone() const {return new OrdinalPurelyTemporalPermutedDataRandomizer(*this);}
    virtual void setPermutedData(const RealDataSet& RealSet);
};
//******************************************************************************
#endif

