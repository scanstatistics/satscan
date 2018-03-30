//******************************************************************************
#ifndef RankRandomizerH
#define RankRandomizerH
//******************************************************************************
#include "PermutationDataRandomizer.h"

class CSaTScanData; // forward class declaration

typedef StationaryAttribute<std::pair<int, tract_t> > RankStationary_t; // time interval / location index
typedef PermutedAttribute<std::pair<double, double> > RankPermuted_t; // attribute / rank


class RankRecord {
public:
    Julian _date;
    tract_t _tract;
    double _attribute;
    double _rank;
    RankRecord(Julian date, tract_t tract, double attribute): _date(date), _tract(tract), _attribute(attribute), _rank(0.0) {}
    bool  operator<(const RankRecord& rhs) const { return _attribute < rhs._attribute; }
    bool  operator>(const RankRecord& rhs) const { return _attribute > rhs._attribute; }
};
           
typedef std::vector<RankRecord> RankRecordCollection_t;

/** Function object used to compare Coordinates::gpCoordinates */
class CompareRankPermuted {
public:
    CompareRankPermuted() {}
    bool operator() (const RankPermuted_t& lhs, const RankPermuted_t& rhs) { return lhs.GetPermutedVariable().second > rhs.GetPermutedVariable().second; }
};

class CCluster;
/** Abstraction for Rank data randomizers */
class AbstractRankRandomizer :  public AbstractPermutedDataRandomizer<RankStationary_t, RankPermuted_t>{
   protected:
     double                    _average_attribute;
     virtual void              AddCase(Julian date, tract_t tract, measure_t variable, double rank);

   public:
     AbstractRankRandomizer(const CSaTScanData& dataHub, long lInitialSeed);
     virtual ~AbstractRankRandomizer() {}

    virtual void               AssignFromAttributes(RankRecordCollection_t& records, RealDataSet& RealSet);
    virtual double             getAverageAtributeValue() const { return _average_attribute; }
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
