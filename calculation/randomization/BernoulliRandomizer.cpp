//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "BernoulliRandomizer.h"
#include "SaTScanData.h"

/** constructor */
BernoulliNullHypothesisRandomizer::BernoulliNullHypothesisRandomizer(long lInitialSeed)
                                  :AbstractOrdinalDenominatorDataRandomizer(lInitialSeed) {}

/** destructor */
BernoulliNullHypothesisRandomizer::~BernoulliNullHypothesisRandomizer() {}

/** returns newly cloned BernoulliNullHypothesisRandomizer */
BernoulliNullHypothesisRandomizer * BernoulliNullHypothesisRandomizer::Clone() const {
  return new BernoulliNullHypothesisRandomizer(*this);
}

/** Randomizes dataset's data under null hypothesis for Bernoulli model. */
void BernoulliNullHypothesisRandomizer::RandomizeData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet, unsigned int iSimulation) {

  // reset seed of random number generator
  SetSeed(iSimulation, thisSimSet.GetSetIndex());
  // reset simulation cases to zero
  thisSimSet.ResetCumulativeCaseArray();
  //randomize data
  RandomizeOrdinalData(thisRealSet.GetTotalCases(), thisRealSet.GetTotalControls(),
                       thisSimSet.GetCaseArray(), thisRealSet.GetMeasureArray(),
                       thisRealSet.GetNumTracts(), thisRealSet.GetNumTimeIntervals());
}
