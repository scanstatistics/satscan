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

/** constructor */
FileSourceRandomizer::FileSourceRandomizer(const CParameters& Parameters)
                     :AbstractRandomizer(), gParameters(Parameters) {}

/** copy constructor */
FileSourceRandomizer::FileSourceRandomizer(const FileSourceRandomizer& rhs)
                     :AbstractRandomizer(rhs), gParameters(rhs.gParameters) {}

/** destructor */
FileSourceRandomizer::~FileSourceRandomizer() {}

/** returns pointer to newly cloned FileSourceRandomizer */
FileSourceRandomizer * FileSourceRandomizer::Clone() const {
  return new FileSourceRandomizer(*this);
}

/** Reads number of simulated cases from a text file rather than generating them randomly.
    NOTE: Data read from the file is not validated. This means that there is potential
          for the program to behave badly if:
          1) the data read from file does not match dimensions of ppSimCases
          2) the case counts read from file is inappropriate given real data -- probably access violations
          3) file does not actually contains numerical data
          Use of this feature should be discouraged except from someone who has
          detailed knowledge of how code works.                                                           */
void FileSourceRandomizer::RandomizeData(const RealDataStream&,
                                         SimulationDataStream& thisSimulationStream,
                                         unsigned int iSimulation) {

  thisSimulationStream.ReadSimulationData(gParameters, iSimulation);
}
