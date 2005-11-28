//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "PermutationDataRandomizer.h"

/** constructor */
AbstractPermutedDataRandomizer::AbstractPermutedDataRandomizer(long lInitialSeed)
                               :AbstractRandomizer(lInitialSeed) {}

/** destructor */
AbstractPermutedDataRandomizer::~AbstractPermutedDataRandomizer() {}

/** randomizes data of dataset*/
void AbstractPermutedDataRandomizer::RandomizeData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet, unsigned int iSimulation) {
  //set seed for simulation number
  SetSeed(iSimulation, thisSimSet.GetSetIndex());
  //assign random numbers to permuted attribute and sort
  SortPermutedAttribute();
  //re-assign dataset's simulation data
  AssignRandomizedData(thisRealSet, thisSimSet);
};

