//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "PermutationDataRandomizer.h"

/** constructor */
AbstractPermutedDataRandomizer::AbstractPermutedDataRandomizer() : AbstractRandomizer() {}

/** destructor */
AbstractPermutedDataRandomizer::~AbstractPermutedDataRandomizer() {}

/** randomizes data of data stream */
void AbstractPermutedDataRandomizer::RandomizeData(const RealDataStream& thisRealStream,
                                                   SimulationDataStream& thisSimulationStream,
                                                   unsigned int iSimulation) {
  //set seed for simulation number
  gRandomNumberGenerator.SetSeed(iSimulation + gRandomNumberGenerator.GetDefaultSeed());
  //assign random numbers to permuted attribute and sort
  SortPermutedAttribute();
  //re-assign data stream's simulation data
  AssignRandomizedData(thisRealStream, thisSimulationStream);
};


/** constructor */
PermutedAttribute::PermutedAttribute() : gfRandomNumber(0) {}

/** destructor */
PermutedAttribute::~PermutedAttribute() {}


/** constructor */
AssignPermutedAttribute::AssignPermutedAttribute(RandomNumberGenerator & Generator) : gGenerator(Generator) {}

/** destructor */
AssignPermutedAttribute::~AssignPermutedAttribute() {}

