//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "Randomizer.h"

const long AbstractRandomizer::glStreamSeedOffSet        = 1000000;

/** constructor */
AbstractRandomizer::AbstractRandomizer() {}

/** destructor */
AbstractRandomizer::~AbstractRandomizer() {}

/** Reset seed of randomizer for particular simulation index and data stream
    index combination. Note that neither parameter should be zero. This special
    seed creation is to ensure that, should a data set contain identical data
    streams; identical randomized data are not created. This situation would be
    unusual and unlikely but this implementation has no ill effects otherwise.
    Note: For requested simulations greater than 999999, the determined seed
          will not be unique when more than one data stream is in set. This
          behavior has been OK'ed by Martin through reasoning that having
          identical data streams will be rare.*/
void AbstractRandomizer::SetSeed(unsigned int iSimulationIndex, unsigned int iDataStreamIndex) {
  unsigned long ulSeed;

  try {
    //calculate seed as unsigned long
    ulSeed = gRandomNumberGenerator.GetDefaultSeed() + iSimulationIndex +  ((iDataStreamIndex - 1) * glStreamSeedOffSet);
    //compare to max seed(declared as positive signed long)
    if (ulSeed >= static_cast<unsigned long>(gRandomNumberGenerator.GetMaxSeed()))
      ZdGenerateException("Calculated seed for simulation %u, data stream %u, exceeds defined limit of %i.",
                          "SetSeed()", iSimulationIndex, iDataStreamIndex, gRandomNumberGenerator.GetMaxSeed());

    gRandomNumberGenerator.SetSeed(static_cast<long>(ulSeed));
  }
  catch (ZdException &x) {
    x.AddCallpath("SetSeed()","AbstractRandomizer");
    throw;
  }
}
