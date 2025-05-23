//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "Randomizer.h"
#include "SSException.h"

const long AbstractRandomizer::glDataSetSeedOffSet = 1000000;

/** constructor */
AbstractRandomizer::AbstractRandomizer(long lInitialSeed) : gRandomNumberGenerator(lInitialSeed) {}

/** destructor */
AbstractRandomizer::~AbstractRandomizer() {}

/** Reset seed of randomizer for particular simulation index and data set
    index combination. Note that neither parameter should be zero. This special
    seed creation is to ensure that, should a data set contain identical data
    sets; identical randomized data are not created. This situation would be
    unusual and unlikely but this implementation has no ill effects otherwise.
    Note: For requested simulations greater than 999999, the determined seed
          will not be unique when more than one data set is in input. This
          behavior has been OK'ed by Martin through reasoning that having
          identical data sets will be rare.*/
void AbstractRandomizer::SetSeed(unsigned int iSimulationIndex, unsigned int iDataSetIndex) {
    unsigned long ulSeed;

    try {
        //calculate seed as unsigned long
        ulSeed = gRandomNumberGenerator.GetInitialSeed() + iSimulationIndex +  ((iDataSetIndex - 1) * glDataSetSeedOffSet);
        //compare to max seed(declared as positive signed long)
        if (ulSeed >= static_cast<unsigned long>(gRandomNumberGenerator.GetMaxSeed()))
            throw prg_error("Calculated seed for simulation %u, data set %u, exceeds defined limit of %i.",
                            "SetSeed()", iSimulationIndex, iDataSetIndex, gRandomNumberGenerator.GetMaxSeed());

        gRandomNumberGenerator.SetSeed(static_cast<long>(ulSeed));
    } catch (prg_exception& x) {
        x.addTrace("SetSeed()","AbstractRandomizer");
        throw;
    }
}

/** constructor */
FileSourceRandomizer::FileSourceRandomizer(const CParameters& Parameters, const std::string& sourcename, long lInitialSeed)
                     :AbstractRandomizer(lInitialSeed), gParameters(Parameters), _source_filename(sourcename), _line_offset(0) {
    gReader.reset(AbstractDataSetReader::getNewDataSetReader(Parameters));
}

/** copy constructor */
FileSourceRandomizer::FileSourceRandomizer(const FileSourceRandomizer& rhs)
                     :AbstractRandomizer(rhs), gParameters(rhs.gParameters), _source_filename(rhs._source_filename), _line_offset(rhs._line_offset) {
    gReader.reset(AbstractDataSetReader::getNewDataSetReader(gParameters));
}

/** returns pointer to newly cloned FileSourceRandomizer */
FileSourceRandomizer * FileSourceRandomizer::Clone() const {
    //std::auto_ptr<FileSourceRandomizer> clone(new FileSourceRandomizer(gParameters, _source_filename, gRandomNumberGenerator.GetSeed()));
    //clone->setLineOffset(_line_offset);
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
void FileSourceRandomizer::RandomizeData(const RealDataSet&, DataSet& thisSimSet, unsigned int iSimulation) {
    gReader->read(thisSimSet, gParameters, _source_filename, _line_offset + iSimulation);
}
