//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ExponentialRandomizer.h"

/** Adds new randomization attributes with passed values. */
void AbstractExponentialRandomizer::AddPatients(count_t tNumPatients, int iTimeInterval, tract_t tTractIndex, measure_t tContinuousVariable, count_t tCensored) {
  for (count_t i=0; i < tNumPatients; ++i) {
    //add stationary values
    gvStationaryAttribute.push_back(ExponentialStationary_t(std::make_pair(iTimeInterval, tTractIndex)));
    //add permutated value
    gvOriginalPermutedAttribute.push_back(ExponentialPermuted_t(std::make_pair(tContinuousVariable, static_cast<unsigned short>(tCensored))));
  }
}

/** Calibrates accumulated continuous variables and assigns data to RealDataSet objects' structures. */
void AbstractExponentialRandomizer::AssignFromAttributes(RealDataSet& RealSet) {
  StationaryContainer_t::const_iterator itr_stationary=gvStationaryAttribute.begin();
  PermutedContainer_t::iterator         itr_permuted=gvOriginalPermutedAttribute.begin();
  int                                   i, tTract, iNumTracts = RealSet.GetNumTracts(), iNumTimeIntervals = RealSet.GetNumTimeIntervals();
  count_t                            ** ppCases, ** ppCensoredCases, tTotalCases=0;
  measure_t                          ** ppMeasure, tTotalMeasure=0, tCalibratedMeasure = 0, tCalibration;

  try {
    for (; itr_permuted != gvOriginalPermutedAttribute.end(); ++itr_permuted) {
       if ((*itr_permuted).GetPermutedVariable().second == 0) ++tTotalCases;
       tTotalMeasure += (*itr_permuted).GetPermutedVariable().first;
    }
    //calibrate permuted continuous variable
    if (tTotalMeasure == 0)
      return; //when performing iterative scan, it is possible that total measure has become zero
    tCalibration = (measure_t)tTotalCases/tTotalMeasure;
    itr_permuted=gvOriginalPermutedAttribute.begin();
    for (; itr_permuted != gvOriginalPermutedAttribute.end(); ++itr_permuted) {
      (*itr_permuted).ReferencePermutedVariable().first *= tCalibration;
      tCalibratedMeasure += (*itr_permuted).GetPermutedVariable().first;
    }
    //validate that calibration worked
    if (fabs((measure_t)tTotalCases - tCalibratedMeasure) > 0.0001)
      ZdGenerateException("The total measure '%8.6lf' is not equal to the total number of cases '%ld'.\n", "CalibrateAndAssign()", tCalibratedMeasure, tTotalCases);
    //assign totals for observed and expected in this data set
    RealSet.SetTotalCases(tTotalCases);
    RealSet.SetTotalMeasure(tCalibratedMeasure);
    RealSet.SetTotalPopulation(gvOriginalPermutedAttribute.size());
    RealSet.AllocateCasesArray(); ppCases = RealSet.GetCaseArray();
    RealSet.AllocateCensoredCasesArray(); ppCensoredCases = RealSet.GetCensoredCasesArrayHandler().GetArray();
    RealSet.AllocateMeasureArray(); ppMeasure = RealSet.GetMeasureArray();
    //assign randomized data to measure, cases, and censored cases arrays
    itr_permuted=gvOriginalPermutedAttribute.begin();
    for (; itr_stationary != gvStationaryAttribute.end(); ++itr_permuted, ++itr_stationary) {
       ppMeasure[itr_stationary->GetStationaryVariable().first][itr_stationary->GetStationaryVariable().second] += (*itr_permuted).GetPermutedVariable().first;
       ppCases[itr_stationary->GetStationaryVariable().first][itr_stationary->GetStationaryVariable().second] += ((*itr_permuted).GetPermutedVariable().second ? 0 : 1);
       if ((*itr_permuted).GetPermutedVariable().second)
         ++ppCensoredCases[itr_stationary->GetStationaryVariable().first][itr_stationary->GetStationaryVariable().second];
    }
   //now set as cumulative, in respect to time intervals
    for (tTract=0; tTract < iNumTracts; ++tTract)
       for (i=iNumTimeIntervals-2; i >= 0; --i) {
          ppMeasure[i][tTract] = ppMeasure[i+1][tTract] + ppMeasure[i][tTract];
          ppCases[i][tTract] = ppCases[i+1][tTract] + ppCases[i][tTract];
          ppCensoredCases[i][tTract] = ppCensoredCases[i+1][tTract] + ppCensoredCases[i][tTract];
       }
  }
  catch (ZdException& x) {
    x.AddCallpath("CalibrateAndAssign()","AbstractExponentialRandomizer");
    throw;
  }
}

/** Removes all stationary and permuted attributes associated with cases in interval and location. */
void AbstractExponentialRandomizer::RemoveCase(int iTimeInterval, tract_t tTractIndex) {
  ExponentialStationary_t               tAttribute(std::make_pair(iTimeInterval, tTractIndex));
  StationaryContainer_t::iterator       itr;

  while ((itr=std::find(gvStationaryAttribute.begin(), gvStationaryAttribute.end(), tAttribute)) != gvStationaryAttribute.end()) {
       size_t t = std::distance(gvStationaryAttribute.begin(), itr);
       gvOriginalPermutedAttribute.erase(gvOriginalPermutedAttribute.begin() + t);
       gvStationaryAttribute.erase(itr);
  }
}

//******************************************************************************

/** Assigns randomized data to SimDataSet objects' structures. */
void ExponentialRandomizer::AssignRandomizedData(const RealDataSet&, SimDataSet& SimSet) {
  StationaryContainer_t::const_iterator itr_stationary=gvStationaryAttribute.begin();
  PermutedContainer_t::const_iterator   itr_permuted=gvPermutedAttribute.begin();
  int                                   i, tTract, iNumTracts = SimSet.GetNumTracts(), iNumTimeIntervals = SimSet.GetNumTimeIntervals();
  count_t                            ** ppCases = SimSet.GetCaseArray();
  measure_t                          ** ppMeasure = SimSet.GetMeasureArray();   

  SimSet.GetCaseArrayHandler().Set(0);
  SimSet.GetMeasureArrayHandler().Set(0);
  //assign randomized continuous data to measure and case arrays
  for (; itr_stationary != gvStationaryAttribute.end(); ++itr_permuted, ++itr_stationary) {
     ppMeasure[itr_stationary->GetStationaryVariable().first][itr_stationary->GetStationaryVariable().second] += (*itr_permuted).GetPermutedVariable().first;
     ppCases[itr_stationary->GetStationaryVariable().first][itr_stationary->GetStationaryVariable().second] += ((*itr_permuted).GetPermutedVariable().second ? 0 : 1);
  }
  //now set as cumulative, in respect to time intervals
  for (tTract=0; tTract < iNumTracts; ++tTract)
     for (i=iNumTimeIntervals-2; i >= 0; --i) {
        ppMeasure[i][tTract] = ppMeasure[i+1][tTract] + ppMeasure[i][tTract];
        ppCases[i][tTract] = ppCases[i+1][tTract] + ppCases[i][tTract];
     }
}

/** Calculates the total populations for each location - both censored and uncensored data.
    Caller is responsible for sizing vector to number of locations in input data. */
std::vector<double>& ExponentialRandomizer::CalculateMaxCirclePopulationArray(std::vector<double>& vMaxCirclePopulation) const {
  StationaryContainer_t::const_iterator itr_stationary=gvStationaryAttribute.begin();

  std::fill(vMaxCirclePopulation.begin(), vMaxCirclePopulation.end(), 0);
  //assign population array for accumulated data
  for (; itr_stationary != gvStationaryAttribute.end();  ++itr_stationary)
     ++vMaxCirclePopulation[itr_stationary->GetStationaryVariable().second];
     
  return vMaxCirclePopulation;
}

//******************************************************************************

/** Assigns randomized data to SimDataSet objects' purely temporal structures. */
void ExponentialPurelyTemporalRandomizer::AssignRandomizedData(const RealDataSet&, SimDataSet& SimSet) {
  StationaryContainer_t::const_iterator itr_stationary=gvStationaryAttribute.begin();
  PermutedContainer_t::const_iterator   itr_permuted=gvPermutedAttribute.begin();
  int                                   i, tTract, iNumTracts = SimSet.GetNumTracts(), iNumTimeIntervals = SimSet.GetNumTimeIntervals();
  count_t                             * pCases = SimSet.GetPTCasesArray();
  measure_t                           * pMeasure = SimSet.GetPTMeasureArray();

  memset(pCases, 0, (iNumTimeIntervals+1) * sizeof(count_t));
  memset(pMeasure, 0, (iNumTimeIntervals+1) * sizeof(measure_t));
  //assign randomized continuous data to measure and case arrays
  for (; itr_stationary != gvStationaryAttribute.end(); ++itr_permuted, ++itr_stationary) {
     pMeasure[itr_stationary->GetStationaryVariable().first] += (*itr_permuted).GetPermutedVariable().first;
     pCases[itr_stationary->GetStationaryVariable().first] += ((*itr_permuted).GetPermutedVariable().second ? 0 : 1);
  }
  //now set as cumulative, in respect to time intervals
  for (i=iNumTimeIntervals-2; i >= 0; --i) {
     pMeasure[i] = pMeasure[i+1] + pMeasure[i];
     pCases[i] = pCases[i+1] + pCases[i];
  }
}

