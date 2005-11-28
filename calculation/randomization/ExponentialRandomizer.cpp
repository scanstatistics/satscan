//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ExponentialRandomizer.h"

/** constructor */
ExponentialRandomizer::ExponentialRandomizer(long lInitialSeed) : AbstractPermutedDataRandomizer(lInitialSeed) {}

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
    gvOriginalPermutedAttribute.push_back(PermutedAttribute<ExponentialPermuted_t>(std::make_pair(tContinuousVariable, static_cast<unsigned short>(tCensored))));
  }
}


/** Assigns data in randomizer to case and measure structures.
    NOTE: Correctness of passed iNumTimeIntervals and iNumTracts, in relation
          to passed arrays is responsibility of caller. */
void ExponentialRandomizer::Assign(const PermutedContainer_t& vPermutedAttributes, count_t ** ppCases, measure_t ** ppMeasure, int iNumTimeIntervals, int iNumTracts) const {
  StationaryContainer_t::const_iterator itr_stationary=gvStationaryAttribute.begin();
  PermutedContainer_t::const_iterator   itr_permuted=vPermutedAttributes.begin();
  int                                   i, tTract;

  //assign randomized continuous data to measure
  for (; itr_stationary != gvStationaryAttribute.end(); ++itr_permuted, ++itr_stationary) {
     ppMeasure[itr_stationary->GetTimeInterval()][itr_stationary->GetTractIndex()] += (*itr_permuted).GetPermutedVariable().first;
     ppCases[itr_stationary->GetTimeInterval()][itr_stationary->GetTractIndex()] += ((*itr_permuted).GetPermutedVariable().second ? 0 : 1);
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
  StationaryContainer_t::const_iterator itr_stationary=gvStationaryAttribute.begin();
  PermutedContainer_t::const_iterator   itr_permuted=gvOriginalPermutedAttribute.begin();
  int                                   iIntervalIndex;
  count_t                            ** ppCases = tCensoredArray.GetArray();

  tCensoredArray.Set(0);
  //assign censored cases to array
  for (; itr_stationary != gvStationaryAttribute.end(); ++itr_permuted, ++itr_stationary)
     if ((*itr_permuted).GetPermutedVariable().second)
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
  Assign(gvPermutedAttribute, thisSimSet.GetCaseArray(), thisSimSet.GetMeasureArray(), thisRealSet.GetNumTimeIntervals(), thisRealSet.GetNumTracts());
}

/** Calculates the total populations for each location - both censored and uncensored data.
    Caller is responsible for sizing vector to number of locations in input data. */
std::vector<double>& ExponentialRandomizer::CalculateMaxCirclePopulationArray(std::vector<double>& vMaxCirclePopulation) const {
  StationaryContainer_t::const_iterator itr_stationary=gvStationaryAttribute.begin();

  // set all elements to zero
  std::fill(vMaxCirclePopulation.begin(), vMaxCirclePopulation.end(), 0);

  //assign population array for accumulated data
  for (; itr_stationary != gvStationaryAttribute.end();  ++itr_stationary)
     ++vMaxCirclePopulation[itr_stationary->GetTractIndex()];
     
  return vMaxCirclePopulation;
}

/** Calibrates accumulated survival times. Returns calibrated total measure. */
double ExponentialRandomizer::CalibrateAndAssign(measure_t tCalibration, RealDataSet& thisDataSet) {
  double                        dTotalMeasure=0;
  PermutedContainer_t::iterator itr_permuted=gvOriginalPermutedAttribute.begin();

  for (; itr_permuted != gvOriginalPermutedAttribute.end(); ++itr_permuted) {
    (*itr_permuted).ReferencePermutedVariable().first *= tCalibration;
    dTotalMeasure += (*itr_permuted).GetPermutedVariable().first;
  }

  // case and measure arrays
  Assign(gvOriginalPermutedAttribute, thisDataSet.GetCaseArray(), thisDataSet.GetMeasureArray(), thisDataSet.GetNumTimeIntervals(), thisDataSet.GetNumTracts());

  return dTotalMeasure;  
}

/** re-initializes and  sorts permutated attribute */
void ExponentialRandomizer::SortPermutedAttribute() {
  // Reset permuted attributes to original order - this is needed to maintain
  // consistancy of output when running in parallel.
  gvPermutedAttribute = gvOriginalPermutedAttribute;

  std::for_each(gvPermutedAttribute.begin(), gvPermutedAttribute.end(), AssignPermutedAttribute<ExponentialPermuted_t>(gRandomNumberGenerator));
  std::sort(gvPermutedAttribute.begin(), gvPermutedAttribute.end(), ComparePermutedAttribute<ExponentialPermuted_t>());
}



