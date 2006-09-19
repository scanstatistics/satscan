//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "BernoulliRandomizer.h"
#include "SaTScanData.h"

/** Creates randomized under the null hypothesis for Bernoulli model, assigning data to DataSet objects structures.
    Random number generator seed initialized based upon 'iSimulation' index. */
void BernoulliNullHypothesisRandomizer::RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation) {
  //reset seed of random number generator
  SetSeed(iSimulation, SimSet.getSetIndex());
  SimSet.getCaseData().Set(0);
  //randomize data
  RandomizeOrdinalData(RealSet.getTotalCases(), RealSet.getTotalControls(), SimSet.getCaseData().GetArray(),
                       RealSet.getMeasureData().GetArray(), RealSet.getLocationDimension(), RealSet.getIntervalDimension());
}
//******************************************************************************

/** Creates randomized under the null hypothesis for Bernoulli model, assigning data to DataSet objects structures.
    Random number generator seed initialized based upon 'iSimulation' index. */
void BernoulliPurelyTemporalNullHypothesisRandomizer::RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation) {
  //reset seed of random number generator
  SetSeed(iSimulation, SimSet.getSetIndex());
  memset(SimSet.getCaseData_PT(), 0, (RealSet.getIntervalDimension()+1) * sizeof(count_t));
  //randomize data
  RandomizePurelyTemporalOrdinalData(RealSet.getTotalCases(), RealSet.getTotalControls(), SimSet.getCaseData_PT(), RealSet.getMeasureData_PT(), RealSet.getIntervalDimension());
}

