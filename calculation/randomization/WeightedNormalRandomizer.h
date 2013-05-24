//******************************************************************************
#ifndef WeightedNormalRandomizerH
#define WeightedNormalRandomizerH
//******************************************************************************
#include "PermutationDataRandomizer.h"
#include "MultipleDimensionArrayHandler.h"


class CSaTScanData;  /** forward class declaration */
class ColumnVector;  /** forward class declaration */
class Matrix;        /** forward class declaration */

class WeightedNormalVariables {
    public:
        const double & first;
        const double & second;

    private:
        std::pair<double, double>    gtStandard;
        MinimalGrowthArray<double> * gpAdditional; 

        void init() {gpAdditional=0;gtStandard.first=0;gtStandard.second=0;}

    public:
        WeightedNormalVariables(double dWeight, double dRate)
            :first(gtStandard.first), second(gtStandard.second) {
            init();
            gtStandard.first = dWeight;
            gtStandard.second = dRate;
        }
        WeightedNormalVariables(double dWeight, double dRate, const std::vector<double>& additional)
            :first(gtStandard.first), second(gtStandard.second) {
            init();
            gtStandard.first = dWeight;
            gtStandard.second = dRate;
            gpAdditional = new MinimalGrowthArray<double>(additional);
        }
        WeightedNormalVariables(const WeightedNormalVariables& rhs)
            :first(gtStandard.first), second(gtStandard.second) {
            init();
            gtStandard = rhs.gtStandard;
            if (rhs.gpAdditional) gpAdditional = new MinimalGrowthArray<double>(*rhs.gpAdditional);
        }
        virtual ~WeightedNormalVariables() { try {delete gpAdditional;} catch(...){} }

        WeightedNormalVariables & operator=(const WeightedNormalVariables& rhs) {
            try {
                gtStandard = rhs.gtStandard;
                delete gpAdditional; gpAdditional = 0;
                if (rhs.gpAdditional) gpAdditional = new MinimalGrowthArray<double>(*rhs.gpAdditional);
            } catch (prg_exception& x) {
                x.addTrace("operator=()","WeightedNormalVariables");
                throw;
            }
            return *this;          
        }

        const MinimalGrowthArray<double> * getAdditional() const {return gpAdditional;}
};

typedef StationaryAttribute<std::pair<int, tract_t> >   WeightedNormalStationary_t;
typedef PermutedAttribute<WeightedNormalVariables >   WeightedNormalPermuted_t;

/** Function object used to compare WeightedNormalStationary_t. */
class CompareWeightedNormalStationary {
  public:
     bool operator() (const WeightedNormalStationary_t & lhs, const WeightedNormalStationary_t & rhs) {
         if (lhs.GetStationaryVariable().first == rhs.GetStationaryVariable().first)
             return lhs.GetStationaryVariable().second < rhs.GetStationaryVariable().second;
         return lhs.GetStationaryVariable().first == rhs.GetStationaryVariable().first;
     }
};

class CSaTScanData; // forward class declaration

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
     AbstractWeightedNormalRandomizer(const CSaTScanData& dataHub, long lInitialSeed);
     virtual ~AbstractWeightedNormalRandomizer() {}

    virtual void               AddCase(count_t tCount, Julian date, tract_t tTractIndex, measure_t tContinuousVariable, double dWeight);
    virtual void               AddCase(count_t tCount, Julian date, tract_t tTractIndex, measure_t tContinuousVariable, double dWeight, const std::vector<double>& covariates);
    virtual void               AssignFromAttributes(RealDataSet& RealSet);
    void                       get_wg_deltag(std::auto_ptr<ColumnVector>& wg, std::auto_ptr<ColumnVector>& deltag) const;
    void                       get_xg(std::auto_ptr<Matrix>& xp, bool bExcludeSelectColumn=false) const;
    ClusterStatistics          getClusterStatistics(int iIntervalStart, int iIntervalEnd, const std::vector<tract_t>& vTracts) const;
    ClusterLocationStatistics  getClusterLocationStatistics(int iIntervalStart, int iIntervalEnd, const std::vector<tract_t>& vTracts) const;
    bool                       getHasCovariates() const;
    DataSetStatistics          getDataSetStatistics() const;
    measure_t                  getFirstRatioConstant() const {return gtFirstRatioConstant;}
    RiskEstimateStatistics     getRiskEstimateStatistics(const CSaTScanData& DataHub) const;
    measure_t                  getSecondRatioConstant() const {return gtSecondRatioConstant;}
    virtual void               RemoveCase(int iTimeInterval, tract_t tTractIndex);
    bool                       hasUniqueLocationsCoverage(CSaTScanData& DataHub);
    //boost::dynamic_bitset<>    hasUniqueLocationsCoverage(const CSaTScanData& DataHub) const;
};

/** Randomizes data of dataset for a 'normal' probablility model.
    Instead of assigning data to simulation case structures, assigns
    randomized data to simulation measure structures. */
class WeightedNormalRandomizer : public AbstractWeightedNormalRandomizer {
  protected:
    virtual void               AssignRandomizedData(const RealDataSet& RealSet, DataSet& SimSet);

  public:
    WeightedNormalRandomizer(const CSaTScanData& dataHub, long lInitialSeed=RandomNumberGenerator::glDefaultSeed) : AbstractWeightedNormalRandomizer(dataHub, lInitialSeed) {}
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
    WeightedNormalPurelyTemporalRandomizer(const CSaTScanData& dataHub, long lInitialSeed=RandomNumberGenerator::glDefaultSeed) : AbstractWeightedNormalRandomizer(dataHub, lInitialSeed) {}
    virtual ~WeightedNormalPurelyTemporalRandomizer() {}

    virtual WeightedNormalPurelyTemporalRandomizer * Clone() const {return new WeightedNormalPurelyTemporalRandomizer(*this);}
};
//******************************************************************************
#endif

