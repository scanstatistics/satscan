//******************************************************************************
#ifndef __RANDOMNERERER_H
#define __RANDOMNERERER_H
//******************************************************************************
#include "RandomNumberGenerator.h"
#include "RandomDistribution.h"
#include "DataSet.h"

/** abstract randomizer class to randomize data for replications */
class AbstractRandomizer {
  protected:
    static const long            glDataSetSeedOffSet;
    RandomNumberGenerator        gRandomNumberGenerator;  /** generates random numbers */

    void                         SetSeed(unsigned int iSimulationIndex, unsigned int iDataSetIndex);

  public:
    AbstractRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    virtual ~AbstractRandomizer();

    virtual AbstractRandomizer * Clone() const = 0;

    virtual void	         RandomizeData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet, unsigned int iSimulation) = 0;
};

typedef ZdPointerVector<AbstractRandomizer>     RandomizerContainer_t;

/** Reads simulation data from file.
    NOTE: This unit has note been thoughly tested, especially with multiple datasets. */
class FileSourceRandomizer : public AbstractRandomizer {
  protected:
    const CParameters            & gParameters;

  public:
    	    FileSourceRandomizer(const CParameters& Parameters, long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    	    FileSourceRandomizer(const FileSourceRandomizer& rhs);
    virtual ~FileSourceRandomizer();

    virtual FileSourceRandomizer * Clone() const;
    virtual void	           RandomizeData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet, unsigned int iSimulation);
};
//******************************************************************************
#endif
