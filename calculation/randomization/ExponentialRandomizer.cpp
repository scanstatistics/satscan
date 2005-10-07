//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ExponentialRandomizer.h"

/** constructor */
PermutedExponentialAttributes::PermutedExponentialAttributes(double dVariable, unsigned short uCensoreddAttribute, unsigned int iOrderIndex)
                              :PermutedVariable(dVariable, iOrderIndex), guCensoredAttribute(uCensoreddAttribute) {}

/** destructor */
PermutedExponentialAttributes::~PermutedExponentialAttributes() {}

/** returns pointer to newly cloned PermutatedVariable */
PermutedExponentialAttributes * PermutedExponentialAttributes::Clone() const {
  return new PermutedExponentialAttributes(*this);
}


/** constructor */
ExponentialRandomizer::ExponentialRandomizer(long lInitialSeed) : AbstractPermutedDataRandomizer(lInitialSeed), giOrderIndex(0) {}

/** destructor */
ExponentialRandomizer::~ExponentialRandomizer() {}

/** returns pointer to newly cloned ExponetialRandomizer */
ExponentialRandomizer * ExponentialRandomizer::Clone() const {
  return new ExponentialRandomizer(*this);
}

/** Adds new randomization entries with passed values. */
void ExponentialRandomizer::AddPatients(count_t tNumPatients, int iTimeInterval, tract_t tTractIndex, measure_t tContinuousVariable, count_t tCensored) {
  for (count_t i=0; i < tNumPatients; ++i) {
    //add stationary values
    gvStationaryAttribute.push_back(SpaceTimeStationaryAttribute(iTimeInterval, tTractIndex));
    //add permutated value
    gvPermutedAttribute.push_back(0);
    gvPermutedAttribute[gvPermutedAttribute.size() - 1] = new PermutedExponentialAttributes(tContinuousVariable, tCensored, ++giOrderIndex);
  }  
}


/** Assigns data in randomizer to case and measure structures.
    NOTE: Correctness of passed iNumTimeIntervals and iNumTracts, in relation
          to passed arrays is responsibility of caller. */
void ExponentialRandomizer::Assign(count_t ** ppCases, measure_t ** ppMeasure, int iNumTimeIntervals, int iNumTracts) const {
  std::vector<SpaceTimeStationaryAttribute>::const_iterator      itr_stationary=gvStationaryAttribute.begin();
  ZdPointerVector<PermutedExponentialAttributes>::const_iterator itr_permuted=gvPermutedAttribute.begin();
  int                                                            i, tTract;

  //assign randomized continuous data to measure
  for (; itr_stationary != gvStationaryAttribute.end(); ++itr_permuted, ++itr_stationary) {
     ppMeasure[itr_stationary->GetTimeInterval()][itr_stationary->GetTractIndex()] += (*itr_permuted)->GetVariable();
     ppCases[itr_stationary->GetTimeInterval()][itr_stationary->GetTractIndex()] += ((*itr_permuted)->GetCensoredAttribute() ? 0 : 1);
  }

  //now set as cumulative
  for (tTract=0; tTract < iNumTracts; ++tTract)
     for (i=iNumTimeIntervals-2; i >= 0; --i) {
        ppMeasure[i][tTract] = ppMeasure[i+1][tTract] + ppMeasure[i][tTract];
        ppCases[i][tTract] = ppCases[i+1][tTract] + ppCases[i][tTract];
     }
}

/** Assigns censored data in randomizer to 2-D array.
    NOTE: Correctness of passed 1st dimension and 2nd dimension, in relation
          to number of time intervals and locations in input data is responsibility
          of caller. */
void ExponentialRandomizer::AssignCensoredIndividuals(TwoDimCountArray_t& tCensoredArray) const {
  std::vector<SpaceTimeStationaryAttribute>::const_iterator      itr_stationary=gvStationaryAttribute.begin();
  ZdPointerVector<PermutedExponentialAttributes>::const_iterator itr_permuted=gvPermutedAttribute.begin();
  int                                                            iIntervalIndex;
  count_t                                                     ** ppCases = tCensoredArray.GetArray();

  tCensoredArray.Set(0);
  //assign censored cases to array
  for (; itr_stationary != gvStationaryAttribute.end(); ++itr_permuted, ++itr_stationary)
     if ((*itr_permuted)->GetCensoredAttribute())
       ++ppCases[itr_stationary->GetTimeInterval()][itr_stationary->GetTractIndex()];

  //now set as cumulative
  for (unsigned int t=0; t < tCensoredArray.Get2ndDimension(); ++t) {
     iIntervalIndex = static_cast<int>(tCensoredArray.Get1stDimension()) - 2;
     for (; iIntervalIndex >= 0; --iIntervalIndex)
       ppCases[iIntervalIndex][t] = ppCases[iIntervalIndex + 1][t] + ppCases[iIntervalIndex][t];
  }     
}

/** Assigns randomized data to dataset's simulation measure structures. */
void ExponentialRandomizer::AssignRandomizedData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet) {
  //reset simulation case structure to zero
  thisSimSet.ResetCumulativeCaseArray();
  //reset simulation measure array to zero
  thisSimSet.GetMeasureArrayHandler().Set(0);
  Assign(thisSimSet.GetCaseArray(), thisSimSet.GetMeasureArray(), thisRealSet.GetNumTimeIntervals(), thisRealSet.GetNumTracts());
}

/** Calculates the total populations for each location - both censored and uncensored data.
    Caller is responsible for sizing vector to number of locations in input data. */
std::vector<double>& ExponentialRandomizer::CalculateMaxCirclePopulationArray(std::vector<double>& vMaxCirclePopulation) const {
  std::vector<SpaceTimeStationaryAttribute>::const_iterator             itr_stationary=gvStationaryAttribute.begin();

  // set all elements to zero
  std::fill(vMaxCirclePopulation.begin(), vMaxCirclePopulation.end(), 0);

  //assign population array for accumulated data
  for (; itr_stationary != gvStationaryAttribute.end();  ++itr_stationary)
     ++vMaxCirclePopulation[itr_stationary->GetTractIndex()];
     
  return vMaxCirclePopulation;
}

/** Calibrates accumulated survival times. Returns calibrated total measure. */
double ExponentialRandomizer::Calibrate(measure_t tCalibration) {
  double                                                   dTotalMeasure=0;
  ZdPointerVector<PermutedExponentialAttributes>::iterator itr_permuted=gvPermutedAttribute.begin();

  for (; itr_permuted != gvPermutedAttribute.end(); ++itr_permuted)
    dTotalMeasure += (*itr_permuted)->Calibrate(tCalibration);

  return dTotalMeasure;  
}

/** re-initializes and  sorts permutated attribute */
void ExponentialRandomizer::SortPermutedAttribute() {
  // Sort permuted attributes to original order - this is needed to maintain
  // consistancy of output when running in parallel. 
  std::sort(gvPermutedAttribute.begin(), gvPermutedAttribute.end(), ComparePermutedOrderIndex());

  std::for_each(gvPermutedAttribute.begin(), gvPermutedAttribute.end(), AssignPermutedAttribute(gRandomNumberGenerator));
  std::sort(gvPermutedAttribute.begin(), gvPermutedAttribute.end(), ComparePermutedAttribute());
}

