//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "DataSetReader.h"
#include "SSException.h"

/** Returns new AbstractDataSetReader object. */
AbstractDataSetReader * AbstractDataSetReader::getNewDataSetReader(const CParameters& Parameters) {
    switch(Parameters.GetProbabilityModelType()) {
        case POISSON              :
        case BERNOULLI            :
        case SPACETIMEPERMUTATION : return new DefaultDataSetReader();
        case CATEGORICAL          :
        case ORDINAL              : return new OrdinalDataSetReader();
        case EXPONENTIAL          :
            throw prg_error("getNewDataSetReader() not implemented for Exponential model.","getNewDataSetReader()");
        case NORMAL               :
            throw prg_error("getNewDataSetReader() not implemented for Normal model.","getNewDataSetReader()");
        case RANK                 :
            throw prg_error("getNewDataSetReader() not implemented for Exponential model.","getNewDataSetReader()");
        default : throw prg_error("Unknown model type '%d'.","getNewDataSetReader()", Parameters.GetProbabilityModelType());
    }
}

/** Reads case data from file stream based upon analysis type. */
void DefaultDataSetReader::read(DataSet& Set, const CParameters& Parameters, const std::string& sourcename, unsigned int iSimulation) {
    std::ifstream  stream;
    if (!stream.is_open()) stream.open(sourcename.c_str());
    if (!stream) throw resolvable_error("Error: Could not open file '%s' to read the simulated data.\n", sourcename.c_str());

    //seek line offset for reading iSimulation'th simulation data
    unsigned int skip = iSimulation - 1;
    if (Parameters.GetIsSpaceTimeAnalysis() || Parameters.GetAnalysisType() == SPATIALVARTEMPTREND)
        skip *= Set.getLocationDimension();
    for (unsigned int i=0; i < skip; ++i)
        stream.ignore(std::numeric_limits<int>::max(), '\n');

    switch (Parameters.GetAnalysisType()) {
        case PURELYSPATIAL             :
        case SPACETIME                 :
        case PROSPECTIVESPACETIME      :
        case SPATIALVARTEMPTREND       : {
            count_t ** ppSimCases = Set.getCaseData().GetArray();
            for (unsigned int t=0; t < Set.getLocationDimension(); ++t)
                for (unsigned int i=0; i < Set.getIntervalDimension(); ++i)
                    stream >> ppSimCases[i][t];
        } break;
        case PURELYTEMPORAL            :
        case PROSPECTIVEPURELYTEMPORAL : {
            count_t * pSimCases = Set.getCaseData_PT();
            for (unsigned int i=0; i < Set.getIntervalDimension(); ++i)
                stream >> pSimCases[i];
        } break;
        default : throw prg_error("Unknown analysis type '%d'.","write()", Parameters.GetAnalysisType());
    }
    stream.close();
}

/** Reads category case data from file stream based upon analysis type. */
void OrdinalDataSetReader::read(DataSet& Set, const CParameters& Parameters, const std::string& sourcename, unsigned int iSimulation) {
    std::ifstream  stream;
    if (!stream.is_open()) stream.open(sourcename.c_str());
    if (!stream) throw resolvable_error("Error: Could not open file '%s' to read the simulated data.\n", sourcename.c_str());

    //seek line offset for reading iSimulation'th simulation data
    unsigned int skip = iSimulation - 1;
    skip *= (Parameters.GetIsPurelyTemporalAnalysis() ? Set.getCaseData_PT_Cat().Get1stDimension() : Set.getCaseData_Cat().size());
    if (Parameters.GetIsSpaceTimeAnalysis() || Parameters.GetAnalysisType() == SPATIALVARTEMPTREND)
        skip *= Set.getLocationDimension();
    for (unsigned int  i=0; i < skip; ++i)
        stream.ignore(std::numeric_limits<int>::max(), '\n');

    switch (Parameters.GetAnalysisType()) {
        case PURELYSPATIAL             :
        case SPACETIME                 :
        case PROSPECTIVESPACETIME      :
        case SPATIALVARTEMPTREND       : {
            for (CasesByCategory_t::const_iterator itr=Set.getCaseData_Cat().begin(); itr != Set.getCaseData_Cat().end(); ++itr) {
                count_t ** ppSimCases = (*itr)->GetArray();
                for (unsigned int  t=0; t < Set.getLocationDimension(); ++t)
                    for (unsigned int  i=0; i < Set.getIntervalDimension(); ++i)
                        stream >> ppSimCases[i][t];
            }
        } break;
        case PURELYTEMPORAL            :
        case PROSPECTIVEPURELYTEMPORAL : {
            for (unsigned int c=0; c < Set.getCaseData_PT_Cat().Get1stDimension(); ++c) {
                count_t * pSimCases = Set.getCaseData_PT_Cat().GetArray()[c];
                for (unsigned int i=0; i < Set.getIntervalDimension(); ++i)
                    stream >> pSimCases[i];
            }
        } break;
        default : throw prg_error("Unknown analysis type '%d'.","write()", Parameters.GetAnalysisType());
    }
    stream.close();
}
