//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "BernoulliRandomizer.h"
#include "SaTScanData.h"

/** constructor */
BernoulliNullHypothesisRandomizer::BernoulliNullHypothesisRandomizer() : AbstractOrdinalDenominatorDataRandomizer() {}

/** destructor */
BernoulliNullHypothesisRandomizer::~BernoulliNullHypothesisRandomizer() {}

/** returns newly cloned BernoulliNullHypothesisRandomizer */
BernoulliNullHypothesisRandomizer * BernoulliNullHypothesisRandomizer::Clone() const {
  return new BernoulliNullHypothesisRandomizer(*this);
}

/** Randomizes data stream's data under null hypothesis for Bernoulli model. */
void BernoulliNullHypothesisRandomizer::RandomizeData(const RealDataStream& thisRealStream,
                                              SimulationDataStream& thisSimulationStream,
                                              unsigned int iSimulation) {

  // reset seed of random number generator
  SetSeed(iSimulation, thisSimulationStream.GetStreamIndex());
  // reset simulation cases to zero
  thisSimulationStream.ResetCumulativeCaseArray();
  //randomize data
  RandomizeOrdinalData(thisRealStream.GetTotalCases(), thisRealStream.GetTotalControls(),
                       thisSimulationStream.GetCaseArray(), thisRealStream.GetMeasureArray(),
                       thisRealStream.GetNumTracts(), thisRealStream.GetNumTimeIntervals());
}
