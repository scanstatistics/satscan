//---------------------------------------------------------------------------
#ifndef __RANDOMNERERER_H
#define __RANDOMNERERER_H
//---------------------------------------------------------------------------
#include "RandomNumberGenerator.h"
#include "RandomDistribution.h"
#include "DataStream.h"

/** abstract randomizer class to randomize data for replications */
class AbstractRandomizer {
  protected:
    static const long           glStreamSeedOffSet;
    RandomNumberGenerator	gRandomNumberGenerator;  /** generates random numbers */

    void                        SetSeed(unsigned int iSimulationIndex, unsigned int iDataStreamIndex);

  public:
    AbstractRandomizer();
    virtual ~AbstractRandomizer();

    virtual AbstractRandomizer* Clone() const = 0;

    virtual void	        RandomizeData(const RealDataStream& thisRealStream,
                                              SimulationDataStream& thisSimulationStream,
                                              unsigned int iSimulation) = 0;
};

typedef ZdPointerVector<AbstractRandomizer>     RandomizerContainer_t;
//---------------------------------------------------------------------------
#endif
