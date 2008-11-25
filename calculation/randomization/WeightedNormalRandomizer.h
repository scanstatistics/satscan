//******************************************************************************
#ifndef WeightedNormalRandomizerH
#define WeightedNormalRandomizerH
//******************************************************************************
#include "PermutationDataRandomizer.h"

class CSaTScanData;  /** forward class declaration */

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

      struct RiskEstimateStatistics {
          typedef std::map<tract_t,measure_t> container_t;

          container_t gtMean;
          container_t gtWeightedMean;
          container_t gtTotalObserved;
          container_t gtTotalWeight;

          void init(tract_t tNum) {
              gtMean.clear();
              for (tract_t t=0; t < tNum; ++t) {gtMean[t] = 0;}
              gtTotalWeight = gtTotalObserved = gtWeightedMean = gtMean;
          }
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

      struct ClusterLocationStatistics {
          typedef std::map<tract_t,measure_t> container_t;

          count_t     gtObservations;
          measure_t   gtMeanIn;
          measure_t   gtMeanOut;
          measure_t   gtWeightedMeanIn;
          measure_t   gtWeightedMeanOut;
          measure_t   gtWeight;
          container_t gtLocObserved;
          container_t gtLocMean;
          container_t gtLocWeightedMean;
          container_t gtLocTotalObserved;
          container_t gtLocTotalWeight;

          void init() {gtObservations=0;gtMeanIn=0;gtMeanOut=0;gtWeightedMeanIn=0;gtWeightedMeanOut=0;gtWeight=0;
                       gtLocObserved.clear();gtLocMean.clear();gtLocWeightedMean.clear();gtLocTotalObserved.clear();gtLocTotalWeight.clear();}
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
    ClusterLocationStatistics  getClusterLocationStatistics(int iIntervalStart, int iIntervalEnd, std::vector<tract_t>& vTracts) const;
    DataSetStatistics          getDataSetStatistics() const;
    measure_t                  getFirstRatioConstant() const {return gtFirstRatioConstant;}
    RiskEstimateStatistics     getRiskEstimateStatistics(const CSaTScanData& DataHub) const;
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

