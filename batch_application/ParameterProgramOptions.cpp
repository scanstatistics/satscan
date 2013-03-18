//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "ParameterProgramOptions.h"
#include "SSException.h"
#include "UtilityFunctions.h"

unsigned int ParameterProgramOptions::ADDITIONAL_DATASETS = 50;
unsigned int ParameterProgramOptions::POLYGON_REGIONS = 100;

const char * ParameterProgramOptions::getOption(ParameterType e, bool withShortName) const {
    const char  * sSectionName, * sKey;

    if (!_specifications.GetParameterIniInfo(e, &sSectionName, &sKey))
        throw prg_error("Unknown parameter type '%d'.", "getOption()", e);
    return sKey;
}

/** Returns program options for CParameter class. All program options are defiend as std::string we that we can have reading and errors
    between program options and file based parameter reading. */
ParameterProgramOptions::ParamOptContainer_t & ParameterProgramOptions::getOptions(ParamOptContainer_t& opt_descriptions) {
    /**
    TODO:
    1) How to implement multiple data sets?
    2) How to implement obserable regions? (polygons for continuous poisson model)
    **/

    const char * OPT_FORMAT = "%s options";
    const int LINE_WIDTH = 150;
    std::string buffer, buffer2, buffer3;

    /* Input tab options */
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, IniParameterSpecification::Input), LINE_WIDTH, LINE_WIDTH/2),true,std::string())));
    opt_descriptions.back()->get<0>().add_options()
        (getOption(CASEFILE, true), po::value<std::string>(), GetParameterComment(CASEFILE))
        (getOption(CONTROLFILE, true), po::value<std::string>(), GetParameterComment(CONTROLFILE))
        (getOption(PRECISION, true), po::value<std::string>(), GetParameterComment(PRECISION))
        (getOption(STARTDATE, true), po::value<std::string>(), GetParameterComment(STARTDATE))
        (getOption(ENDDATE, true), po::value<std::string>(), GetParameterComment(ENDDATE))
        (getOption(POPFILE, true), po::value<std::string>(), GetParameterComment(POPFILE))
        (getOption(COORDFILE, true), po::value<std::string>(), GetParameterComment(COORDFILE))
        (getOption(SPECIALGRID, true), po::value<std::string>(), GetParameterComment(SPECIALGRID))
        (getOption(GRIDFILE, true), po::value<std::string>(), GetParameterComment(GRIDFILE))
        (getOption(COORDTYPE, true), po::value<std::string>(), GetParameterComment(COORDTYPE));

    /* Analysis tab options */
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, IniParameterSpecification::Analysis), LINE_WIDTH, LINE_WIDTH/2),true,std::string())));
    opt_descriptions.back()->get<0>().add_options()
        (getOption(ANALYSISTYPE, true), po::value<std::string>(), GetParameterComment(ANALYSISTYPE))
        (getOption(MODEL, true), po::value<std::string>(), GetParameterComment(MODEL))
        (getOption(SCANAREAS, true), po::value<std::string>(), GetParameterComment(SCANAREAS))
        (getOption(TIME_AGGREGATION_UNITS, true), po::value<std::string>(), GetParameterComment(TIME_AGGREGATION_UNITS))
        (getOption(TIME_AGGREGATION, true), po::value<std::string>(), GetParameterComment(TIME_AGGREGATION));

    /* Output tab options */
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, IniParameterSpecification::Output), LINE_WIDTH, LINE_WIDTH/2),true,std::string())));
    opt_descriptions.back()->get<0>().add_options()
        (getOption(OUTPUTFILE, true), po::value<std::string>(), GetParameterComment(OUTPUTFILE))
        (getOption(OUTPUT_MLC_ASCII, true), po::value<std::string>(), GetParameterComment(OUTPUT_MLC_ASCII))
        (getOption(OUTPUT_MLC_DBASE, true), po::value<std::string>(), GetParameterComment(OUTPUT_MLC_DBASE))
        (getOption(OUTPUT_MLC_CASE_ASCII, true), po::value<std::string>(), GetParameterComment(OUTPUT_MLC_CASE_ASCII))
        (getOption(OUTPUT_MLC_CASE_DBASE, true), po::value<std::string>(), GetParameterComment(OUTPUT_MLC_CASE_DBASE))
        (getOption(OUTPUT_AREAS_ASCII, true), po::value<std::string>(), GetParameterComment(OUTPUT_AREAS_ASCII))
        (getOption(OUTPUT_AREAS_DBASE, true), po::value<std::string>(), GetParameterComment(OUTPUT_AREAS_DBASE))
        (getOption(OUTPUT_RR_ASCII, true), po::value<std::string>(), GetParameterComment(OUTPUT_RR_ASCII))
        (getOption(OUTPUT_RR_DBASE, true), po::value<std::string>(), GetParameterComment(OUTPUT_RR_DBASE))
        (getOption(OUTPUT_SIM_LLR_ASCII, true), po::value<std::string>(), GetParameterComment(OUTPUT_SIM_LLR_ASCII))
        (getOption(OUTPUT_SIM_LLR_DBASE, true), po::value<std::string>(), GetParameterComment(OUTPUT_SIM_LLR_DBASE))
        (getOption(OUTPUT_TEMPORAL_GRAPH, true), po::value<std::string>(), GetParameterComment(OUTPUT_TEMPORAL_GRAPH))
        (getOption(OUTPUT_KML, true), po::value<std::string>(), GetParameterComment(OUTPUT_KML));

    /* Polygons tab options */
    printString(buffer2, "%s\n  %d polygon regions specified via --Polygon1, --Polygon2, --Polygon3, etc.", GetParameterComment(OBSERVABLE_REGIONS), POLYGON_REGIONS);
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, IniParameterSpecification::Polygons), LINE_WIDTH, LINE_WIDTH/2),true,buffer2)));

    /* Polygons (additional files -- hidden) tab options */
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, "Polygon Regions"), LINE_WIDTH, LINE_WIDTH/2),false,std::string())));
    for (size_t t=0; t < POLYGON_REGIONS; ++t) {
        opt_descriptions.back()->get<0>().add_options()
            (printString(buffer, "Polygon%d", t+1).c_str(), po::value<std::string>(), GetParameterComment(OBSERVABLE_REGIONS));
    }

    /* Multiple Data Sets tab options */
    printString(buffer2, "%d additional data sets specified via --%s2, --%s2, --%s2, etc.", ADDITIONAL_DATASETS, getOption(CASEFILE), getOption(CONTROLFILE), getOption(POPFILE));
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, IniParameterSpecification::MultipleDataSets), LINE_WIDTH, LINE_WIDTH/2),true,buffer2)));
    opt_descriptions.back()->get<0>().add_options()
        //("case-file,c", "Case File")
        //("count-file,b", "Count File")
        //("population-file,p", "Population File")
        (getOption(MULTI_DATASET_PURPOSE_TYPE, true), po::value<std::string>(), GetParameterComment(MULTI_DATASET_PURPOSE_TYPE));

    /* Multiple Data Sets (additional files -- hidden) tab options */
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, "Additional File Sets"), LINE_WIDTH, LINE_WIDTH/2),false,std::string())));
    for (size_t t=0; t < ADDITIONAL_DATASETS; ++t) {
        opt_descriptions.back()->get<0>().add_options()
            (printString(buffer, "%s%d", getOption(CASEFILE), t+2).c_str(), po::value<std::string>(), GetParameterComment(CASEFILE))
            (printString(buffer2, "%s%d", getOption(CONTROLFILE), t+2).c_str(), po::value<std::string>(), GetParameterComment(CONTROLFILE))
            (printString(buffer3, "%s%d", getOption(POPFILE), t+2).c_str(), po::value<std::string>(), GetParameterComment(POPFILE));
    }

    /* Data Checking tab options */
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, IniParameterSpecification::DataChecking), LINE_WIDTH, LINE_WIDTH/2),true,std::string())));
    opt_descriptions.back()->get<0>().add_options()
        (getOption(STUDYPERIOD_DATACHECK, true), po::value<std::string>(), GetParameterComment(STUDYPERIOD_DATACHECK))
        (getOption(COORDINATES_DATACHECK, true), po::value<std::string>(), GetParameterComment(COORDINATES_DATACHECK));

    /* Spatial Neighbors tab options */
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, IniParameterSpecification::SpatialNeighbors), LINE_WIDTH, LINE_WIDTH/2),true,std::string())));
    opt_descriptions.back()->get<0>().add_options()
        (getOption(LOCATION_NEIGHBORS_FILE, true), po::value<std::string>(), GetParameterComment(LOCATION_NEIGHBORS_FILE))
        (getOption(USE_LOCATION_NEIGHBORS_FILE, true), po::value<std::string>(), GetParameterComment(USE_LOCATION_NEIGHBORS_FILE))
        (getOption(META_LOCATIONS_FILE, true), po::value<std::string>(), GetParameterComment(META_LOCATIONS_FILE))
        (getOption(USE_META_LOCATIONS_FILE, true), po::value<std::string>(), GetParameterComment(USE_META_LOCATIONS_FILE))
        (getOption(MULTIPLE_COORDINATES_TYPE, true), po::value<std::string>(), GetParameterComment(MULTIPLE_COORDINATES_TYPE));

    /* Spatial Window tab options */
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, IniParameterSpecification::SpatialWindow), LINE_WIDTH, LINE_WIDTH/2),true,std::string())));
    opt_descriptions.back()->get<0>().add_options()
        (getOption(MAXGEOPOPATRISK, true), po::value<std::string>(), GetParameterComment(MAXGEOPOPATRISK))
        (getOption(MAXGEOPOPFILE, true), po::value<std::string>(), GetParameterComment(MAXGEOPOPFILE))
        (getOption(MAXGEODISTANCE, true), po::value<std::string>(), GetParameterComment(MAXGEODISTANCE))
        (getOption(USE_MAXGEOPOPFILE, true), po::value<std::string>(), GetParameterComment(USE_MAXGEOPOPFILE))
        (getOption(USE_MAXGEODISTANCE, true), po::value<std::string>(), GetParameterComment(USE_MAXGEODISTANCE))
        (getOption(PURETEMPORAL, true), po::value<std::string>(), GetParameterComment(PURETEMPORAL))
        (getOption(MAXCIRCLEPOPFILE, true), po::value<std::string>(), GetParameterComment(MAXCIRCLEPOPFILE))
        (getOption(WINDOW_SHAPE, true), po::value<std::string>(), GetParameterComment(WINDOW_SHAPE))
        (getOption(NON_COMPACTNESS_PENALTY, true), po::value<std::string>(), GetParameterComment(NON_COMPACTNESS_PENALTY))
        (getOption(RISKFUNCTION, true), po::value<std::string>(), GetParameterComment(RISKFUNCTION));

    /* Temporal Window tab options */
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, IniParameterSpecification::TemporalWindow), LINE_WIDTH, LINE_WIDTH/2),true,std::string())));
    opt_descriptions.back()->get<0>().add_options()
        (getOption(TIMESIZE, true), po::value<std::string>(), GetParameterComment(TIMESIZE))
        (getOption(PURESPATIAL, true), po::value<std::string>(), GetParameterComment(PURESPATIAL))
        (getOption(MAX_TEMPORAL_TYPE, true), po::value<std::string>(), GetParameterComment(MAX_TEMPORAL_TYPE))
        (getOption(CLUSTERS, true), po::value<std::string>(), GetParameterComment(CLUSTERS))
        (getOption(INTERVAL_STARTRANGE, true), po::value<std::string>(), GetParameterComment(INTERVAL_STARTRANGE))
        (getOption(INTERVAL_ENDRANGE, true), po::value<std::string>(), GetParameterComment(INTERVAL_ENDRANGE));

    /* Space And Time Adjustments tab options */
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, IniParameterSpecification::SpaceAndTimeAdjustments), LINE_WIDTH, LINE_WIDTH/2),true,std::string())));
    opt_descriptions.back()->get<0>().add_options()
        (getOption(TIMETREND, true), po::value<std::string>(), GetParameterComment(TIMETREND))
        (getOption(TIMETRENDPERC, true), po::value<std::string>(), GetParameterComment(TIMETRENDPERC))
        (getOption(ADJ_BY_RR_FILE, true), po::value<std::string>(), GetParameterComment(ADJ_BY_RR_FILE))
        (getOption(USE_ADJ_BY_RR_FILE, true), po::value<std::string>(), GetParameterComment(USE_ADJ_BY_RR_FILE))
        (getOption(SPATIAL_ADJ_TYPE, true), po::value<std::string>(), GetParameterComment(SPATIAL_ADJ_TYPE))
        (getOption(TIME_TREND_TYPE, true), po::value<std::string>(), GetParameterComment(TIME_TREND_TYPE));

    /* Inference tab options */
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, IniParameterSpecification::Inference), LINE_WIDTH, LINE_WIDTH/2),true,std::string())));
    opt_descriptions.back()->get<0>().add_options()
        (getOption(START_PROSP_SURV, true), po::value<std::string>(), GetParameterComment(START_PROSP_SURV))
        (getOption(PVALUE_REPORT_TYPE, true), po::value<std::string>(), GetParameterComment(PVALUE_REPORT_TYPE))
        (getOption(REPORT_GUMBEL, true), po::value<std::string>(), GetParameterComment(REPORT_GUMBEL))
        (getOption(EARLY_TERM_THRESHOLD, true), po::value<std::string>(), GetParameterComment(EARLY_TERM_THRESHOLD))
        (getOption(ADJ_FOR_EALIER_ANALYSES, true), po::value<std::string>(), GetParameterComment(ADJ_FOR_EALIER_ANALYSES))
        (getOption(ITERATIVE, true), po::value<std::string>(), GetParameterComment(ITERATIVE))
        (getOption(ITERATIVE_NUM, true), po::value<std::string>(), GetParameterComment(ITERATIVE_NUM))
        (getOption(ITERATIVE_PVAL, true), po::value<std::string>(), GetParameterComment(ITERATIVE_PVAL))
        (getOption(REPLICAS, true), po::value<std::string>(), GetParameterComment(REPLICAS));

    /* Clusters Reported tab options */
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, IniParameterSpecification::ClustersReported), LINE_WIDTH, LINE_WIDTH/2),true,std::string())));
    opt_descriptions.back()->get<0>().add_options()
        (getOption(REPORT_HIERARCHICAL_CLUSTERS, true), po::value<std::string>(), GetParameterComment(REPORT_HIERARCHICAL_CLUSTERS))
        (getOption(REPORT_GINI_CLUSTERS, true), po::value<std::string>(), GetParameterComment(REPORT_GINI_CLUSTERS))
        (getOption(CRITERIA_SECOND_CLUSTERS, true), po::value<std::string>(), GetParameterComment(CRITERIA_SECOND_CLUSTERS))
        (getOption(SPATIAL_MAXIMA, true), po::value<std::string>(), GetParameterComment(SPATIAL_MAXIMA))
        (getOption(GINI_INDEX_REPORT_TYPE, true), po::value<std::string>(), GetParameterComment(GINI_INDEX_REPORT_TYPE))
        (getOption(GINI_INDEX_PVALUE_CUTOFF, true), po::value<std::string>(), GetParameterComment(GINI_INDEX_PVALUE_CUTOFF))
        (getOption(REPORT_GINI_COEFFICENTS, true), po::value<std::string>(), GetParameterComment(REPORT_GINI_COEFFICENTS))
        (getOption(USE_REPORTED_GEOSIZE, true), po::value<std::string>(), GetParameterComment(USE_REPORTED_GEOSIZE))
        (getOption(MAXGEOPOPATRISK_REPORTED, true), po::value<std::string>(), GetParameterComment(MAXGEOPOPATRISK_REPORTED))
        (getOption(MAXGEOPOPFILE_REPORTED, true), po::value<std::string>(), GetParameterComment(MAXGEOPOPFILE_REPORTED))
        (getOption(MAXGEODISTANCE_REPORTED, true), po::value<std::string>(), GetParameterComment(MAXGEODISTANCE_REPORTED))
        (getOption(USE_MAXGEOPOPFILE_REPORTED, true), po::value<std::string>(), GetParameterComment(USE_MAXGEOPOPFILE_REPORTED))
        (getOption(USE_MAXGEODISTANCE_REPORTED, true), po::value<std::string>(), GetParameterComment(USE_MAXGEODISTANCE_REPORTED));

    /* Additional Output tab options */
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, IniParameterSpecification::AdditionalOutput), LINE_WIDTH, LINE_WIDTH/2),true,std::string())));
    opt_descriptions.back()->get<0>().add_options()
        (getOption(REPORT_CRITICAL_VALUES, true), po::value<std::string>(), GetParameterComment(REPORT_CRITICAL_VALUES))
        (getOption(REPORT_RANK, true), po::value<std::string>(), GetParameterComment(REPORT_RANK))
        (getOption(PRINT_ASCII_HEADERS, true), po::value<std::string>(), GetParameterComment(PRINT_ASCII_HEADERS));

    /* Temporal Graph Output tab options */
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, IniParameterSpecification::TemporalGraphOutput), LINE_WIDTH, LINE_WIDTH/2),true,std::string())));
    opt_descriptions.back()->get<0>().add_options()
        (getOption(OUTPUT_TEMPORAL_GRAPH, true), po::value<std::string>(), GetParameterComment(OUTPUT_TEMPORAL_GRAPH));

    /* Elliptic Scan tab options */
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, IniParameterSpecification::EllipticScan), LINE_WIDTH, LINE_WIDTH/2),true,std::string())));
    opt_descriptions.back()->get<0>().add_options()
        (getOption(ESHAPES, true), po::value<std::string>(), GetParameterComment(ESHAPES))
        (getOption(ENUMBERS, true), po::value<std::string>(), GetParameterComment(ENUMBERS));

    /* Power Evaluations tab options */
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, IniParameterSpecification::PowerEvaluations), LINE_WIDTH, LINE_WIDTH/2),true,std::string())));
    opt_descriptions.back()->get<0>().add_options()
        (getOption(POWER_EVALUATION, true), po::value<std::string>(), GetParameterComment(POWER_EVALUATION))
        (getOption(PE_METHOD_TYPE, true), po::value<std::string>(), GetParameterComment(PE_METHOD_TYPE))
        (getOption(PE_COUNT, true), po::value<std::string>(), GetParameterComment(PE_COUNT))
        (getOption(PE_CRITICAL_TYPE, true), po::value<std::string>(), GetParameterComment(PE_CRITICAL_TYPE))
        (getOption(PE_ESTIMATION_TYPE, true), po::value<std::string>(), GetParameterComment(PE_ESTIMATION_TYPE))
        (getOption(PE_ALT_HYPOTHESIS_FILE, true), po::value<std::string>(), GetParameterComment(PE_ALT_HYPOTHESIS_FILE))
        (getOption(PE_POWER_REPLICAS, true), po::value<std::string>(), GetParameterComment(PE_POWER_REPLICAS))
        (getOption(POWER_05, true), po::value<std::string>(), GetParameterComment(POWER_05))
        (getOption(POWER_01, true), po::value<std::string>(), GetParameterComment(POWER_01))
        (getOption(POWER_001, true), po::value<std::string>(), GetParameterComment(POWER_001))
        (getOption(PE_OUTPUT_SIMUALTION_DATA, true), po::value<std::string>(), GetParameterComment(PE_OUTPUT_SIMUALTION_DATA))
        (getOption(PE_SIMUALTION_OUTPUTFILE, true), po::value<std::string>(), GetParameterComment(PE_SIMUALTION_OUTPUTFILE));

    /* Power Simulations tab options */
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, IniParameterSpecification::PowerSimulations), LINE_WIDTH, LINE_WIDTH/2),true,std::string())));
    opt_descriptions.back()->get<0>().add_options()
        (getOption(SIMULATION_TYPE, true), po::value<std::string>(), GetParameterComment(SIMULATION_TYPE))
        (getOption(SIMULATION_SOURCEFILE, true), po::value<std::string>(), GetParameterComment(SIMULATION_SOURCEFILE))
        (getOption(OUTPUT_SIMULATION_DATA, true), po::value<std::string>(), GetParameterComment(OUTPUT_SIMULATION_DATA))
        (getOption(SIMULATION_DATA_OUTFILE, true), po::value<std::string>(), GetParameterComment(SIMULATION_DATA_OUTFILE));

    /* Run Options tab options */
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, IniParameterSpecification::RunOptions), LINE_WIDTH, LINE_WIDTH/2),true,std::string())));
    opt_descriptions.back()->get<0>().add_options()
        (getOption(EXECUTION_TYPE, true), po::value<std::string>(), GetParameterComment(EXECUTION_TYPE))
        (getOption(NUM_PROCESSES, true), po::value<std::string>(), GetParameterComment(NUM_PROCESSES))
        (getOption(LOG_HISTORY, true), po::value<std::string>(), GetParameterComment(LOG_HISTORY))
        (getOption(SUPPRESS_WARNINGS, true), po::value<std::string>(), GetParameterComment(SUPPRESS_WARNINGS));

    return opt_descriptions;
}

/** Overrides settigs  */
void ParameterProgramOptions::setParameterOverrides(const po::variables_map& vm) {
    std::string buffer;
    for (ParameterType eType=ANALYSISTYPE; eType <= gParameters.giNumParameters; eType = ParameterType(eType + 1)) {
        if (vm.count(getOption(eType)))
            SetParameter(eType, vm[getOption(eType)].as<std::string>(), gPrintDirection);
    }
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
    std::string buffer;
    for (ParameterType e=ANALYSISTYPE; e <= gParameters.giNumParameters; e = ParameterType(e + 1)) {
        if (e != EARLY_SIM_TERMINATION && e != OBSERVABLE_REGIONS && e != CREATION_VERSION && e != OUTPUT_SHAPEFILES) { // skip certain parameters
            const char * option = getOption(e, true);
            if (option != IniParameterSpecification::NotUsed) // skip deprecated parameters
                fprintf(fp, " --%s \"%s\" ", option, GetParameterString(e, buffer).c_str());
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

