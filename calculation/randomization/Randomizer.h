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
    RandomNumberGenerator	gRandomNumberGenerator;  /** generates random numbers */

  public:
    AbstractRandomizer();
    virtual ~AbstractRandomizer();

    virtual AbstractRandomizer* Clone() const = 0;

    virtual void	        RandomizeData(DataStream & thisStream, unsigned int iSimulation) = 0;
};
//---------------------------------------------------------------------------
#endif
