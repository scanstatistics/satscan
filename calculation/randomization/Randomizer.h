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
    AbstractRandomizer(long lInitialSeed);
    virtual ~AbstractRandomizer();

    virtual AbstractRandomizer* Clone() const = 0;

    virtual void	        RandomizeData(const RealDataStream& thisRealStream,
                                              SimulationDataStream& thisSimulationStream,
                                              unsigned int iSimulation) = 0;
};

typedef ZdPointerVector<AbstractRandomizer>     RandomizerContainer_t;

/** Reads simulation data from file.
    NOTE: This unit has note been thoughly tested, especially with multiple
          data streams. */
class FileSourceRandomizer : public AbstractRandomizer {
  protected:
    const CParameters                & gParameters;

  public:
    	    FileSourceRandomizer(long lInitialSeed, const CParameters& Parameters);
    	    FileSourceRandomizer(const FileSourceRandomizer & rhs);
    virtual ~FileSourceRandomizer();

    virtual FileSourceRandomizer     * Clone() const;
    virtual void	               RandomizeData(const RealDataStream& thisRealStream,
                                                     SimulationDataStream& thisSimulationStream,
                                                     unsigned int iSimulation);
};
//---------------------------------------------------------------------------
#endif
