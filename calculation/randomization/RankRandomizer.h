//******************************************************************************
#ifndef RankRandomizerH
#define RankRandomizerH
//******************************************************************************
#include "PermutationDataRandomizer.h"

class CSaTScanData; // forward class declaration

typedef StationaryAttribute<std::pair<int, tract_t> >   RankStationary_t;
typedef PermutedAttribute<double>                       RankPermuted_t;

/** Abstraction for Rank data randomizers */
class AbstractRankRandomizer :  public AbstractPermutedDataRandomizer<RankStationary_t, RankPermuted_t>{
   public:
     AbstractRankRandomizer(const CSaTScanData& dataHub, long lInitialSeed);
     virtual ~AbstractRankRandomizer() {}

    virtual void               AddCase(count_t tCount, Julian date, tract_t tTractIndex, measure_t tContinuousVariable);
    virtual void               AssignFromAttributes(RealDataSet& RealSet);
};

/** Rank model randomizer for null hypothesis. */
class RankRandomizer : public AbstractRankRandomizer {
  protected:
    virtual void             AssignRandomizedData(const RealDataSet& RealSet, DataSet& SimSet);

  public:
    RankRandomizer(const CSaTScanData& dataHub, long lInitialSeed=RandomNumberGenerator::glDefaultSeed) : AbstractRankRandomizer(dataHub, lInitialSeed) {}
    virtual ~RankRandomizer() {}

    virtual RankRandomizer * Clone() const {return new RankRandomizer(*this);}
};

/** Rank model randomizer for null hypothesis. Optimized for purely temporal analyses. */
class RankPurelyTemporalRandomizer : public AbstractRankRandomizer {
  protected:
    virtual void             AssignRandomizedData(const RealDataSet& RealSet, DataSet& SimSet);

  public:
    RankPurelyTemporalRandomizer(const CSaTScanData& dataHub, long lInitialSeed=RandomNumberGenerator::glDefaultSeed) : AbstractRankRandomizer(dataHub, lInitialSeed) {}
    virtual ~RankPurelyTemporalRandomizer() {}

    virtual RankPurelyTemporalRandomizer * Clone() const {return new RankPurelyTemporalRandomizer(*this);}
};
//******************************************************************************
#endif
