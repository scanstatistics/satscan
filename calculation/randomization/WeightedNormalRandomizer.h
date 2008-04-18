//******************************************************************************
#ifndef WeightedNormalRandomizerH
#define WeightedNormalRandomizerH
//******************************************************************************
#include "PermutationDataRandomizer.h"

typedef StationaryAttribute<std::pair<int, tract_t> >   WeightedNormalStationary_t;
typedef PermutedAttribute<std::pair<double, double> >   WeightedNormalPermuted_t;

/** */
class AbstractWeightedNormalRandomizer : public AbstractPermutedDataRandomizer<WeightedNormalStationary_t, WeightedNormalPermuted_t>{
   protected:
     measure_t                 gtUnweightedTotalMeasure;
     measure_t                 gtUnweightedTotalMeasureAux;
     measure_t                 gtFirstRatioConstant;
     measure_t                 gtSecondRatioConstant;

   public:
     AbstractWeightedNormalRandomizer(long lInitialSeed) : AbstractPermutedDataRandomizer<WeightedNormalStationary_t, WeightedNormalPermuted_t>(lInitialSeed),
                                                           gtUnweightedTotalMeasureAux(0), gtUnweightedTotalMeasure(0),
                                                           gtFirstRatioConstant(0), gtSecondRatioConstant(0) {}
     virtual ~AbstractWeightedNormalRandomizer() {}

    virtual void               AddCase(count_t tCount, int iTimeInterval, tract_t tTractIndex, measure_t tContinuousVariable, double dWeight);
    virtual void               AssignFromAttributes(RealDataSet& RealSet);
    measure_t                  getUnweightedTotalMeasure() const {return gtUnweightedTotalMeasure;}
    measure_t                  getUnweightedTotalMeasureAux() const {return gtUnweightedTotalMeasureAux;}
    measure_t                  getRateSquaredWeightedSummation(int iTimeInterval, tract_t tTractIndex) const;
    measure_t                  getFirstRatioConstant() const {return gtFirstRatioConstant;}
    measure_t                  getSecondRatioConstant() const {return gtSecondRatioConstant;}
    measure_t                  getSigma(int iIntervalStart, int iIntervalEnd, std::vector<tract_t>& vTracts, measure_t tMuInside, measure_t tMuOutside) const;
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

