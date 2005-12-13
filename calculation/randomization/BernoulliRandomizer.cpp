//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "BernoulliRandomizer.h"
#include "SaTScanData.h"

/** Creates randomized under the null hypothesis for Bernoulli model, assigning data to SimDataSet objects structures.
    Random number generator seed initialized based upon 'iSimulation' index. */
void BernoulliNullHypothesisRandomizer::RandomizeData(const RealDataSet& RealSet, SimDataSet& SimSet, unsigned int iSimulation) {
  //reset seed of random number generator
  SetSeed(iSimulation, SimSet.GetSetIndex());
  SimSet.GetCaseArrayHandler().Set(0);
  //randomize data
  RandomizeOrdinalData(RealSet.GetTotalCases(), RealSet.GetTotalControls(), SimSet.GetCaseArray(),
                       RealSet.GetMeasureArray(), RealSet.GetNumTracts(), RealSet.GetNumTimeIntervals());
}
//******************************************************************************

/** Creates randomized under the null hypothesis for Bernoulli model, assigning data to SimDataSet objects structures.
    Random number generator seed initialized based upon 'iSimulation' index. */
void BernoulliPurelyTemporalNullHypothesisRandomizer::RandomizeData(const RealDataSet& RealSet, SimDataSet& SimSet, unsigned int iSimulation) {
  //reset seed of random number generator
  SetSeed(iSimulation, SimSet.GetSetIndex());
  memset(SimSet.GetPTCasesArray(), 0, (RealSet.GetNumTimeIntervals()+1) * sizeof(count_t));
  //randomize data
  RandomizePurelyTemporalOrdinalData(RealSet.GetTotalCases(), RealSet.GetTotalControls(), SimSet.GetPTCasesArray(), RealSet.GetPTMeasureArray(), RealSet.GetNumTimeIntervals());
}

