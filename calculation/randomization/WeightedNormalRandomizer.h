//******************************************************************************
#ifndef WeightedNormalRandomizerH
#define WeightedNormalRandomizerH
//******************************************************************************
#include "PermutationDataRandomizer.h"

typedef StationaryAttribute<std::pair<int, tract_t> >   WeightedNormalStationary_t;
typedef PermutedAttribute<std::pair<double, double> >   WeightedNormalPermuted_t;

/** */
class AbstractWeightedNormalRandomizer : public AbstractPermutedDataRandomizer<WeightedNormalStationary_t, WeightedNormalPermuted_t>{
   public:
      //typedef std::pair< std::pair<measure_t,measure_t>, std::pair<measure_t,measure_t> > mean_t;
      typedef std::pair<measure_t,measure_t> variance_t;

      struct DataSetStatistics {
          measure_t  gtMean;
          measure_t  gtWeightedMean;
          measure_t  gtVariance;
          measure_t  gtWeightedVariance;
          measure_t  gtTotalWeight;

          void init() {gtMean=0;gtWeightedMean=0;gtVariance=0;gtWeightedVariance=0;gtTotalWeight=0;}
      };

      struct ClusterStatistics {
          count_t    gtObservations;
          measure_t  gtMeanIn;
          measure_t  gtMeanOut;
          measure_t  gtWeightedMeanIn;
          measure_t  gtWeightedMeanOut;
          measure_t  gtVariance;
          measure_t  gtWeightedVariance;
          measure_t  gtWeight;

          void init() {gtObservations=0;gtMeanIn=0;gtMeanOut=0;gtWeightedMeanIn=0;gtWeightedMeanOut=0;gtVariance=0;gtWeightedVariance=0;gtWeight=0;}
      };

   protected:
     measure_t                 gtFirstRatioConstant;
     measure_t                 gtSecondRatioConstant;

   public:
     AbstractWeightedNormalRandomizer(long lInitialSeed) : AbstractPermutedDataRandomizer<WeightedNormalStationary_t, WeightedNormalPermuted_t>(lInitialSeed),
                                                           gtFirstRatioConstant(0), gtSecondRatioConstant(0) {}
     virtual ~AbstractWeightedNormalRandomizer() {}

    virtual void               AddCase(count_t tCount, int iTimeInterval, tract_t tTractIndex, measure_t tContinuousVariable, double dWeight);
    virtual void               AssignFromAttributes(RealDataSet& RealSet);
    ClusterStatistics          getClusterStatistics(int iIntervalStart, int iIntervalEnd, std::vector<tract_t>& vTracts) const;
    DataSetStatistics          getDataSetStatistics() const;
    measure_t                  getFirstRatioConstant() const {return gtFirstRatioConstant;}
    measure_t                  getSecondRatioConstant() const {return gtSecondRatioConstant;}
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

