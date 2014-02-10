//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "ParameterProgramOptions.h"
#include "SSException.h"
#include "UtilityFunctions.h"

unsigned int ParameterProgramOptions::ADDITIONAL_DATASETS = 50;
unsigned int ParameterProgramOptions::POLYGON_REGIONS = 100;

ParameterProgramOptions::ParameterProgramOptions(CParameters& Parameters, CParameters::CreationVersion version, BasePrint& Print)
    :AbtractParameterFileAccess(Parameters, Print), _specifications(version, Parameters) {}

const char * ParameterProgramOptions::getOption(ParameterType e, bool withShortName) const {
    const char  * sSectionName, * sKey;

    if (!_specifications.GetParameterIniInfo(e, &sSectionName, &sKey))
        throw prg_error("Unknown parameter type '%d'.", "getOption()", e);
    return sKey;
}

/** Returns program options for CParameter class. All program options are defiend as std::string we that we can have reading and errors
    between program options and file based parameter reading. */
ParameterProgramOptions::ParamOptContainer_t & ParameterProgramOptions::getOptions(ParamOptContainer_t& opt_descriptions) {
    const char * OPT_FORMAT = "%s options";
    const int LINE_WIDTH = 150;
    std::string buffer, buffer2, buffer3;

    IniParameterSpecification::ParameterInfoCollection_t parameter_collection;
    _specifications.getParameterInfoCollection(parameter_collection);
    const char * curr_section = "";

    IniParameterSpecification::ParameterInfoCollection_t::iterator itr=parameter_collection.begin(), end=parameter_collection.end();
    for (;itr != end; ++itr) {
        if (!strcmp(itr->_section->_label, IniParameterSpecification::System))
            continue;

        if (!strcmp(itr->_section->_label, IniParameterSpecification::MultipleDataSets)) {
            buffer = itr->_label;
            printString(buffer3, "%d additional data sets specified via --%s2, --%s2, --%s2, etc.", ADDITIONAL_DATASETS, getOption(CASEFILE), getOption(CONTROLFILE), getOption(POPFILE));
        } else if (!strcmp(itr->_section->_label, IniParameterSpecification::Polygons)) {
            printString(buffer, "%s1", itr->_label);
            printString(buffer3, "%s\n  %d polygon regions specified via --Polygon1, --Polygon2, --Polygon3, etc.", GetParameterComment(OBSERVABLE_REGIONS), POLYGON_REGIONS);
        } else {
            buffer = itr->_label;
            buffer3 = "";
        }
        if (strcmp(curr_section, itr->_section->_label)) {
            opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer2, OPT_FORMAT, itr->_section->_label), LINE_WIDTH, LINE_WIDTH/2),true,buffer3)));
            curr_section = itr->_section->_label;
        }
        opt_descriptions.back()->get<0>().add_options()(buffer.c_str(), po::value<std::string>(), GetParameterComment(itr->_type));
    }

    ///* Polygons tab options */
    //printString(buffer2, "%s\n  %d polygon regions specified via --Polygon1, --Polygon2, --Polygon3, etc.", GetParameterComment(OBSERVABLE_REGIONS), POLYGON_REGIONS);
    //opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, IniParameterSpecification::Polygons), LINE_WIDTH, LINE_WIDTH/2),true,buffer2)));

    /* Polygons (additional files -- hidden) tab options */
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, "Polygon Regions"), LINE_WIDTH, LINE_WIDTH/2),false,std::string())));
    for (size_t t=1; t < POLYGON_REGIONS; ++t) {
        opt_descriptions.back()->get<0>().add_options()
            (printString(buffer, "Polygon%d", t+1).c_str(), po::value<std::string>(), GetParameterComment(OBSERVABLE_REGIONS));
    }

    /* Multiple Data Sets (additional files -- hidden) tab options */
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, "Additional File Sets"), LINE_WIDTH, LINE_WIDTH/2),false,std::string())));
    for (size_t t=0; t < ADDITIONAL_DATASETS; ++t) {
        opt_descriptions.back()->get<0>().add_options()
            (printString(buffer, "%s%d", getOption(CASEFILE), t+2).c_str(), po::value<std::string>(), GetParameterComment(CASEFILE))
            (printString(buffer2, "%s%d", getOption(CONTROLFILE), t+2).c_str(), po::value<std::string>(), GetParameterComment(CONTROLFILE))
            (printString(buffer3, "%s%d", getOption(POPFILE), t+2).c_str(), po::value<std::string>(), GetParameterComment(POPFILE));
    }
    return opt_descriptions;
}

/** Overrides settings  */
void ParameterProgramOptions::setParameterOverrides(const po::variables_map& vm) {
    IniParameterSpecification::ParameterInfoCollection_t parameter_collection;
    _specifications.getParameterInfoCollection(parameter_collection);
    IniParameterSpecification::ParameterInfoCollection_t::iterator itr=parameter_collection.begin(), end=parameter_collection.end();
    for (;itr != end; ++itr) {
        if (vm.count(itr->_label))
            SetParameter(itr->_type, vm[itr->_label].as<std::string>(), gPrintDirection);
    }

    std::string buffer;
    // manually scan for polygons
    for (size_t t=0; t < POLYGON_REGIONS; ++t) {
        printString(buffer, "Polygon%d", t+1);
        if (vm.count(buffer.c_str())) {
            gParameters.AddObservableRegion(vm[buffer.c_str()].as<std::string>().c_str(), t, false);
        }
    }
    // manually scan for additional data set file parameters
    size_t numSets = gParameters.GetNumDataSets();
    for (size_t t=0; t < ADDITIONAL_DATASETS; ++t) {
        printString(buffer, "%s%d", getOption(CASEFILE), t+2);
        if (vm.count(buffer.c_str())) {
            gParameters.SetCaseFileName(vm[buffer.c_str()].as<std::string>().c_str(), true, t + 2);
            gParameters.SetNumDataSets(std::max(t+2,numSets));
        }
        printString(buffer, "%s%d", getOption(CONTROLFILE), t+2);
        if (vm.count(buffer.c_str())) {
            gParameters.SetControlFileName(vm[buffer.c_str()].as<std::string>().c_str(), true, t + 2);
            gParameters.SetNumDataSets(std::max(t+2,numSets));
        }
        printString(buffer, "%s%d", getOption(POPFILE), t+2);
        if (vm.count(buffer.c_str())) {
            gParameters.SetPopulationFileName(vm[buffer.c_str()].as<std::string>().c_str(), true, t + 2);
            gParameters.SetNumDataSets(std::max(t+2,numSets));
        }
    }
}

void ParameterProgramOptions::listOptions(FILE * fp) {
    IniParameterSpecification::ParameterInfoCollection_t parameter_collection;
    _specifications.getParameterInfoCollection(parameter_collection);
    IniParameterSpecification::ParameterInfoCollection_t::iterator itr=parameter_collection.begin(), end=parameter_collection.end();
    std::string buffer;
    for (;itr != end; ++itr) {
        if (itr->_type != EARLY_SIM_TERMINATION && itr->_type != OBSERVABLE_REGIONS && itr->_type != CREATION_VERSION) { // skip certain parameters
            if (strcmp(itr->_section->_label, IniParameterSpecification::NotUsed)) // skip deprecated parameters
                fprintf(fp, " --%s \"%s\" ", itr->_label, GetParameterString(itr->_type, buffer).c_str());
        }
    }
}

bool ParameterProgramOptions::Read(const char* szFilename) {
    throw prg_error("ParameterProgramOptions::Read(const char*) not implemented.", "Read()");
    return false;
}
void ParameterProgramOptions::Write(const char * szFilename) {
    throw prg_error("ParameterProgramOptions::Write(const char*) not implemented.", "Write()");
}
