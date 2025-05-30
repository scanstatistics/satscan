//******************************************************************************
#ifndef __RANDOMNERERER_H
#define __RANDOMNERERER_H
//******************************************************************************
#include "RandomNumberGenerator.h"
#include "RandomDistribution.h"
#include "DataSet.h"
#include "DataSetReader.h"

/** abstract randomizer class to randomize data for replications */
class AbstractRandomizer {
    protected:
        RandomNumberGenerator gRandomNumberGenerator;  /** generates random numbers */

    public:
        AbstractRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
        virtual ~AbstractRandomizer();

        static const long glDataSetSeedOffSet;
        virtual AbstractRandomizer * Clone() const = 0;
        virtual void RandomizeData(const RealDataSet& thisRealSet, DataSet& thisSimSet, unsigned int iSimulation) = 0;
        const RandomNumberGenerator getRandomNumberGenerator() const {return gRandomNumberGenerator;}
        void SetSeed(unsigned int iSimulationIndex, unsigned int iDataSetIndex);
};

typedef ptr_vector<AbstractRandomizer> RandomizerContainer_t;

/** Reads simulation data from file.
    NOTE: This unit has note been thoughly tested, especially with multiple datasets. */
class FileSourceRandomizer : public AbstractRandomizer {
    protected:
        const CParameters & gParameters;
        std::auto_ptr<AbstractDataSetReader> gReader;
        const std::string _source_filename;
        unsigned int _line_offset;

    public:
        FileSourceRandomizer(const CParameters& Parameters, const std::string& sourcename, long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
        FileSourceRandomizer(const FileSourceRandomizer& rhs);
        virtual ~FileSourceRandomizer() {}

        virtual FileSourceRandomizer * Clone() const;
        virtual void RandomizeData(const RealDataSet& thisRealSet, DataSet& thisSimSet, unsigned int iSimulation);
        void setLineOffset(unsigned int offset) {_line_offset = offset;}
};
//******************************************************************************
#endif
