//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ParametersPrint.h"
#include "DataSetHandler.h"
#include "SSException.h"
#include "ObservableRegion.h"
#include "ChartGenerator.h"
#include "LoglikelihoodRatioWriter.h"
#include "ClusterScatterChart.h"
#include "ClusterMap.h"
#include "AnalysisResultsWriter.h"

/** Returns analysis type as string. */
const char * ParametersPrint::GetAnalysisTypeAsString() const {
    const char * sAnalysisType;

    try {
        switch (_parameters.GetAnalysisType()) {
            case PURELYSPATIAL             : sAnalysisType = "Purely Spatial"; break;
            case PURELYTEMPORAL            : sAnalysisType = "Retrospective Purely Temporal"; break;
            case SPACETIME                 : sAnalysisType = "Retrospective Space-Time"; break;
            case PROSPECTIVESPACETIME      : sAnalysisType = "Prospective Space-Time"; break;
            case SPATIALVARTEMPTREND       : sAnalysisType = "Spatial Variation in Temporal Trends"; break;
            case PROSPECTIVEPURELYTEMPORAL : sAnalysisType = "Prospective Purely Temporal"; break;
            case SEASONALTEMPORAL          : sAnalysisType = "Seasonal Temporal"; break;
            default : throw prg_error("Unknown analysis type '%d'.\n", "GetAnalysisTypeAsString()", _parameters.GetAnalysisType());
        }
    } catch (prg_exception& x) {
        x.addTrace("GetAnalysisTypeAsString()","ParametersPrint");
        throw;
    }
    return sAnalysisType;
}

/* Returns time aggregation type as string. */
const char * ParametersPrint::GetTimeAggregationTypeAsString() const {
    switch (_parameters.GetTimeAggregationLength()) {
        case GENERIC:
        case YEAR  : return "annually";
        case MONTH : return "monthly";
        case DAY   : return "daily";
        case NONE  :
        default: throw prg_error("Unknown time aggregation type '%d'.\n", "GetTimeAggregationTypeAsString()", _parameters.GetTimeAggregationLength());
    }
}

/** Returns area scan type as string based upon probability model type. */
const char * ParametersPrint::GetAreaScanRateTypeAsString() const {
    try {
        switch (_parameters.GetProbabilityModelType()) {
            case POISSON :
                if (_parameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
                    switch (_parameters.GetAreaScanRateType()) {
                        case HIGH       : return "More Increasing or Less Decreasing Rates";
                        case LOW        : return "More Decreasing or Less Increasing Rates";
                        case HIGHANDLOW : return "Increasing or Decreasing Rates";
                        default : throw prg_error("Unknown area scan rate type '%d'.\n", "GetAreaScanRateTypeAsString()", _parameters.GetAreaScanRateType());
                    }
                }
            case RANK:
            case HOMOGENEOUSPOISSON :
            case BERNOULLI :
            case SPACETIMEPERMUTATION :
            case UNIFORMTIME :
            case BATCHED :
                switch (_parameters.GetAreaScanRateType()) {
                    case HIGH       : return "High Rates";
                    case LOW        : return "Low Rates";
                    case HIGHANDLOW : return "High or Low Rates";
                    default : throw prg_error("Unknown area scan rate type '%d'.\n", "GetAreaScanRateTypeAsString()", _parameters.GetAreaScanRateType());
                }
            case CATEGORICAL : return "All Values";
            case ORDINAL :
            case NORMAL :
                switch (_parameters.GetAreaScanRateType()) {
                    case HIGH       : return "High Values";
                    case LOW        : return "Low Values";
                    case HIGHANDLOW : return "High or Low Values";
                    default : throw prg_error("Unknown area scan rate type '%d'.\n", "GetAreaScanRateTypeAsString()", _parameters.GetAreaScanRateType());
                }
            case EXPONENTIAL :
                switch (_parameters.GetAreaScanRateType()) {
                    case HIGH       : return "Short Survival";
                    case LOW        : return "Long Survival";
                    case HIGHANDLOW : return "Short or Long Survival";
                    default : throw prg_error("Unknown area scan rate type '%d'.\n", "GetAreaScanRateTypeAsString()", _parameters.GetAreaScanRateType());
                }
            default : throw prg_error("Unknown probability model '%d'.", "GetAreaScanRateTypeAsString()", _parameters.GetProbabilityModelType());
        }
    } catch (prg_exception& x) {
        x.addTrace("GetAreaScanRateTypeAsString()","ParametersPrint");
        throw;
    }
}

/** Returns probability model type as a character array. */
const char * ParametersPrint::GetProbabilityModelTypeAsString() const {
    try {
        switch (_parameters.GetProbabilityModelType()) {
            case POISSON              : return "Discrete Poisson"; break;
            case BERNOULLI            : return "Bernoulli"; break;
            case SPACETIMEPERMUTATION : return "Space-Time Permutation"; break;
            case CATEGORICAL          : return "Multinomial"; break;
            case ORDINAL              : return "Ordinal"; break;
            case EXPONENTIAL          : return "Exponential"; break;
            case NORMAL               : return "Normal"; break;
            case RANK                 : return "Rank"; break;
            case UNIFORMTIME          : return "Uniform Time"; break;
            case BATCHED              : return "Batched"; break;
            case HOMOGENEOUSPOISSON   : return "Continuous Poisson"; break;
            default : throw prg_error("Unknown probability model type '%d'.\n", "GetProbabilityModelTypeAsString()", _parameters.GetProbabilityModelType());
        }
    } catch (prg_exception& x) {
        x.addTrace("GetProbabilityModelTypeAsString()","ParametersPrint");
        throw;
    }
}

const char * ParametersPrint::getPowerEvaluationMethodAsString() const {
    const char * sProbabilityModel;

    try {
        switch (_parameters.getPowerEvaluationMethod()) {
            case PE_WITH_ANALYSIS        : return "power evaluation with analysis";
            case PE_ONLY_CASEFILE        : return "only power evaluation using case file";
            case PE_ONLY_SPECIFIED_CASES : return "only power evaluation using specified total cases";
            default : throw prg_error("Unknown power evaluation method '%d'.\n", "getPowerEvaluationMethodAsString()", _parameters.getPowerEvaluationMethod());
        }
    } catch (prg_exception& x) {
        x.addTrace("getPowerEvaluationMethodAsString()","ParametersPrint");
        throw;
    }
    return sProbabilityModel;
}

void ParametersPrint::Print(AnalysisResultsWriter& resultsWriter, bool isDrilldown) const {
    Print(resultsWriter.getTextFile(), &resultsWriter.getHtmlFile(), isDrilldown);
}

/** Prints parameters, in a particular format, to passed ascii file. */
void ParametersPrint::Print(FILE* fp, std::ofstream* fstream, bool isDrilldown) const {
    AsciiPrintFormat::PrintSectionSeparatorString(fp, 0, 2, '_');
    if (fstream) *fstream << "<div class='hr' style='margin-top: 5px;'></div>" << std::endl;

    SettingContainer_t settingPairs;
    if (getAdditionalOutputFiles(settingPairs).size()) {
        //AsciiPrintFormat::PrintSectionSeparatorString(fp, 0, 2);
        fprintf(fp, "ADDITIONAL RESULTS FILES\n\n");
        writeSettingPairsTextFile(settingPairs, "", fp, 0);
        AsciiPrintFormat::PrintSectionSeparatorString(fp, 0, 2);
        if (fstream) {
            //AsciiPrintFormat::PrintSectionSeparatorString(*fstream, 0, 2); // TODO: Check this?
            *fstream << "<div id=\"parameter-settings\"><h4>Additional Results Files</h4>" << std::endl;
            writeSettingPairsHTML(settingPairs, "", "additional-results-section", *fstream);
            //AsciiPrintFormat::PrintSectionSeparatorString(*fstream, 0, 2); // TODO: Check this?
            *fstream << std::endl << "</div>" << std::endl;
            *fstream << "<div class='hr' style='margin-top: 5px;'></div>" << std::endl;
        }   
    }
    fprintf(fp, "PARAMETER SETTINGS\n");
    if (fstream) *fstream << "<div id=\"parameter-settings\"><h4>Parameter Settings</h4>" << std::endl;
    auto writeSettings = [&](const std::string& section, const SettingContainer_t& settings) {
        writeSettingPairsTextFile(settings, section, fp);
        if (fstream) writeSettingPairsHTML(settings, section, "parameter-section", *fstream);
    };
    writeSettings("Input", getInputParameters(settingPairs));
    writeSettings("Analysis", getAnalysisParameters(settingPairs));
    writeSettings("Output", getOutputParameters(settingPairs));
    writeSettings("Polygons", getPolygonParameters(settingPairs));
    writeSettings("Multiple Data Sets", getMultipleDataSetParameters(settingPairs));
    writeSettings("Data Checking", getDataCheckingParameters(settingPairs));
    writeSettings("Spatial Neighbors", getSpatialNeighborsParameters(settingPairs));
    writeSettings("Locations Network", getLocationNetworkParameters(settingPairs));
    writeSettings("Spatial Window", getSpatialWindowParameters(settingPairs));
    writeSettings("Temporal Window", getTemporalWindowParameters(settingPairs));
    writeSettings("Cluster Restrictions", getClusterRestrictionsParameters(settingPairs));
    writeSettings("Space And Time Adjustments", getSpaceAndTimeAdjustmentsParameters(settingPairs));
    writeSettings("Inference", getInferenceParameters(settingPairs));
    writeSettings("Drilldown", getDrilldownParameters(settingPairs, isDrilldown));
    writeSettings("Miscellaneous Analysis", getMiscellaneousAnalysisParameters(settingPairs));
    writeSettings("Power Evaluation", getPowerEvaluationsParameters(settingPairs));
    writeSettings("Spatial Output", getSpatialOutputParameters(settingPairs));
    writeSettings("Temporal Graphs", getTemporalOutputParameters(settingPairs));
    writeSettings("Other Output", getOtherOutputParameters(settingPairs));
    writeSettings("Notifications", getNotificationsParameters(settingPairs));
    writeSettings("Elliptic Scan", getEllipticScanParameters(settingPairs));
    writeSettings("Power Simulations", getPowerSimulationsParameters(settingPairs));
    writeSettings("Run Options", getRunOptionsParameters(settingPairs));
    writeSettings("System", getSystemParameters(settingPairs));
}

ParametersPrint::SettingContainer_t& ParametersPrint::getAdditionalOutputFiles(SettingContainer_t& settings) const {
    FileName filename(_parameters.GetOutputFileName().c_str());
    std::string buffer;
    bool canReportClusterFiles = (!_parameters.getPerformPowerEvaluation() || (_parameters.getPerformPowerEvaluation() && _parameters.getPowerEvaluationMethod() == PE_WITH_ANALYSIS));
    try {
        settings.clear();
        auto addByExtension = [&](const std::string& file_label, const std::string& extension) {
            settings.push_back(std::make_pair(file_label, filename.setExtension(extension.c_str()).getFullPath(buffer)));
        };
        auto addByFullpath = [&](const std::string& file_label, const std::string& full_path) {
            settings.push_back(std::make_pair(file_label, full_path));
            filename.setFullPath(_parameters.GetOutputFileName().c_str()); // reset
        };
        addByExtension("Main Results File as HTML", ".html");
        if (_parameters.GetCoordinatesType() == LATLON && _parameters.getOutputKMLFile())
            addByExtension("Google Earth", _parameters.getCompressClusterKML() ? ".kmz" : ".kml");
        if (_parameters.GetCoordinatesType() == LATLON && _parameters.getOutputGoogleMapsFile())
            addByFullpath("Google Maps", ClusterMap::getFilename(filename).getFullPath(buffer));
        if (_parameters.GetCoordinatesType() == LATLON && _parameters.getOutputShapeFiles() && _parameters.getUseLocationsNetworkFile())
            addByExtension("Shapefile", ".col.edges.shp");
        if (_parameters.GetCoordinatesType() == LATLON && _parameters.getOutputShapeFiles())
            addByExtension("Shapefile", ".col.shp");
        if (_parameters.GetCoordinatesType() == LATLON && _parameters.getOutputShapeFiles())
            addByExtension("Shapefile", ".gis.shp");
        if (_parameters.getOutputCartesianGraph())
            addByFullpath("Cartesian Map", CartesianGraph::getFilename(filename).getFullPath(buffer));
        if (canReportClusterFiles && _parameters.GetOutputClusterLevelAscii())
            addByExtension("Cluster Information", ".col.txt");
        if (canReportClusterFiles && _parameters.GetOutputClusterLevelDBase())
            addByExtension("Cluster Information", ".col.dbf");
        if (canReportClusterFiles && _parameters.GetOutputClusterCaseAscii())
            addByExtension("Stratified Cluster Information", ".sci.txt");
        if (canReportClusterFiles && _parameters.GetOutputClusterCaseDBase())
            addByExtension("Stratified Cluster File", ".sci.dbf");
        if (canReportClusterFiles && _parameters.GetOutputAreaSpecificAscii())
            addByExtension("Location Information", ".gis.txt");
        if (canReportClusterFiles && _parameters.GetOutputAreaSpecificDBase())
            addByExtension("Location Information", ".gis.dbf");
        if (_parameters.GetOutputRelativeRisksAscii())
            addByExtension("Risk Estimates for Each Location", ".rr.txt");
        if (_parameters.GetOutputRelativeRisksDBase())
            addByExtension("Risk Estimates for Each Location", ".rr.dbf");
        if (_parameters.GetOutputSimLoglikeliRatiosAscii())
            addByExtension(_parameters.IsTestStatistic() ? "Simulated Test Statistics" : "Simulated Log Likelihood Ratios", ".llr.txt");
        if (_parameters.GetOutputSimLoglikeliRatiosDBase())
            addByExtension("Simulated Log Likelihood Ratios", ".llr.dbf");
        if (_parameters.getOutputTemporalGraphFile())
            addByFullpath("Temporal Graph", TemporalChartGenerator::getFilename(filename).getFullPath(buffer));
        if (_parameters.getReadingLineDataFromCasefile()) {
            for (unsigned int idx = 1; idx <= _parameters.getNumFileSets(); ++idx) {
                if (_parameters.getNumFileSets() == 1)
                    addByExtension("Cluster Linelist", ".linelist.csv");
                else
                    addByExtension("Cluster Linelist", printString(buffer, ".linelist.dataset%u.csv", idx));
            }
            if (_parameters.getLinelistIndividualsCacheFileName().size())
                addByFullpath("Individuals Cache", _parameters.getLinelistIndividualsCacheFileName());
        }
        for (auto const& file : _parameters.getDrilldownResultFilename())
            addByFullpath("Drilldown Results", file);
    } catch (prg_exception& x) {
        x.addTrace("getAdditionalOutputFiles()", "ParametersPrint");
        throw;
    }
    return settings;
}

/** Prints 'Input' tab parameters to file stream. */
ParametersPrint::SettingContainer_t& ParametersPrint::getInputParameters(SettingContainer_t& settings) const {
    std::string buffer;
    try {
        settings.clear();
        if (_parameters.getNumFileSets() > 1)
            settings.push_back(std::make_pair("Data Set 1", _parameters.getDataSourceNames().front()));
        if (_parameters.GetProbabilityModelType() != HOMOGENEOUSPOISSON ||
            (_parameters.getPerformPowerEvaluation() && _parameters.getPowerEvaluationMethod() == PE_ONLY_SPECIFIED_CASES)) {
            settings.push_back(std::make_pair("Case File", getFilenameFormatTime(_parameters.GetCaseFileName(1), _parameters.getTimestamp())));
        }
        switch (_parameters.GetProbabilityModelType()) {
        case POISSON:
            if (!_parameters.UsePopulationFile()) break;
            settings.push_back(std::make_pair("Population File", getFilenameFormatTime(_parameters.GetPopulationFileName(1), _parameters.getTimestamp())));
            break;
        case BERNOULLI:
            settings.push_back(std::make_pair("Control File", getFilenameFormatTime(_parameters.GetControlFileName(1), _parameters.getTimestamp())));
            break;
        case SPACETIMEPERMUTATION:
        case CATEGORICAL:
        case ORDINAL:
        case EXPONENTIAL:
        case NORMAL:
        case RANK:
        case UNIFORMTIME:
        case BATCHED:
        case HOMOGENEOUSPOISSON:  break;
        default: throw prg_error("Unknown probability model type '%d'.\n", "PrintInputParameters()", _parameters.GetProbabilityModelType());
        }
        if (_parameters.GetProbabilityModelType() != HOMOGENEOUSPOISSON) {
            //Display precision, keeping in mind the v4 behavior.
            DatePrecisionType ePrecision;
            if (_parameters.GetPrecisionOfTimesType() == NONE)
                ePrecision = NONE;
            else if (_parameters.GetCreationVersion().iMajor == 4)
                ePrecision = (_parameters.GetAnalysisType() == PURELYSPATIAL ? YEAR : _parameters.GetTimeAggregationUnitsType());
            else
                ePrecision = _parameters.GetPrecisionOfTimesType();
            buffer = "Time Precision";
            switch (ePrecision) {
            case YEAR: settings.push_back(std::make_pair(buffer, "Year")); break;
            case MONTH: settings.push_back(std::make_pair(buffer, "Month")); break;
            case DAY: settings.push_back(std::make_pair(buffer, "Day")); break;
            case GENERIC: settings.push_back(std::make_pair(buffer, "Generic")); break;
            default: settings.push_back(std::make_pair(buffer, "None")); break;
            }
            settings.push_back(std::make_pair("Start Date", _parameters.GetStudyPeriodStartDate()));
            settings.push_back(std::make_pair("End Date", _parameters.GetStudyPeriodEndDate()));
        }
        if (_parameters.UseCoordinatesFile())
            settings.push_back(std::make_pair("Coordinates File", getFilenameFormatTime(_parameters.GetCoordinatesFileName(), _parameters.getTimestamp())));
        if (_parameters.UseSpecialGrid())
            settings.push_back(std::make_pair("Grid File", getFilenameFormatTime(_parameters.GetSpecialGridFileName(), _parameters.getTimestamp())));
        if (_parameters.GetSimulationType() == FILESOURCE)
            settings.push_back(std::make_pair("Simulated Data Import File", getFilenameFormatTime(_parameters.GetSimulationDataSourceFilename(), _parameters.getTimestamp())));
        if ((_parameters.UseCoordinatesFile() || _parameters.UseSpecialGrid())) {
            buffer = "Coordinates";
            switch (_parameters.GetCoordinatesType()) {
            case CARTESIAN: settings.push_back(std::make_pair(buffer, "Cartesian")); break;
            case LATLON: settings.push_back(std::make_pair(buffer, "Latitude/Longitude")); break;
            default: throw prg_error("Unknown coordinated type '%d'.\n", "PrintInputParameters()", _parameters.GetCoordinatesType());
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("getInputParameters()", "ParametersPrint");
        throw;
    }
    return settings;
}

/** Prints 'Analysis' tab parameters to file stream. */
ParametersPrint::SettingContainer_t& ParametersPrint::getAnalysisParameters(SettingContainer_t& settings) const {
    AnalysisType eAnalysisType = _parameters.GetAnalysisType();
    std::string buffer;
    try {
        settings.clear();
        settings.push_back(std::make_pair("Type of Analysis", GetAnalysisTypeAsString()));
        settings.push_back(std::make_pair("Probability Model", GetProbabilityModelTypeAsString()));
        if (_parameters.getSTPasHypergeometric())
            settings.back().second += " (Hypergeometric)";
        settings.push_back(std::make_pair("Scan For Areas With", GetAreaScanRateTypeAsString()));
        if (eAnalysisType != PURELYSPATIAL) {
            buffer = "Time Aggregation Units";
            switch (_parameters.GetTimeAggregationUnitsType()) {
            case YEAR: settings.push_back(std::make_pair(buffer, "Year")); break;
            case MONTH: settings.push_back(std::make_pair(buffer, "Month")); break;
            case DAY: settings.push_back(std::make_pair(buffer, "Day")); break;
            case GENERIC: settings.push_back(std::make_pair(buffer, "Generic"));  break;
            default: throw prg_error("Unknown date precision type '%d'.\n", "PrintAnalysisParameters()", _parameters.GetTimeAggregationUnitsType());
            }
            printString(buffer, "%i", _parameters.GetTimeAggregationLength());
            settings.push_back(std::make_pair("Time Aggregation Length", buffer));
        }
    } catch (prg_exception& x) {
        x.addTrace("getAnalysisParameters()", "ParametersPrint");
        throw;
    }
    return settings;
}

/** Prints 'Output' tab parameters to file stream. */
ParametersPrint::SettingContainer_t& ParametersPrint::getOutputParameters(SettingContainer_t& settings) const {
    std::string buffer;
    bool canReportClusterFiles = (
        !_parameters.getPerformPowerEvaluation() ||
        (_parameters.getPerformPowerEvaluation() && _parameters.getPowerEvaluationMethod() == PE_WITH_ANALYSIS)
    );
    try {
        settings.clear();
        settings.push_back(std::make_pair("Main Results File", _parameters.GetOutputFileName()));
        settings.push_back(std::make_pair("HTML file for Google Map", _parameters.GetCoordinatesType() == LATLON && _parameters.getOutputKMLFile() ? "Yes" : "No"));
        settings.push_back(std::make_pair("KML file for Google Earth", _parameters.GetCoordinatesType() == LATLON && _parameters.getOutputGoogleMapsFile() ? "Yes" : "No"));
        settings.push_back(std::make_pair("Shapefile for GIS software", _parameters.GetCoordinatesType() == LATLON && _parameters.getOutputShapeFiles() ? "Yes" : "No"));
        settings.push_back(std::make_pair("HTML file for Cartesian map", _parameters.getOutputCartesianGraph() ? "Yes" : "No"));
        settings.push_back(std::make_pair("Cluster Information", printString(buffer, "%s (ASCII), %s (dBase)",
            canReportClusterFiles && _parameters.GetOutputClusterLevelAscii() ? "Yes" : "No",
            canReportClusterFiles && _parameters.GetOutputClusterLevelDBase() ? "Yes" : "No"
        )));
        settings.push_back(std::make_pair("Stratified Cluster Information", printString(buffer, "%s (ASCII), %s (dBase)",
            canReportClusterFiles && _parameters.GetOutputClusterCaseAscii() ? "Yes" : "No",
            canReportClusterFiles && _parameters.GetOutputClusterCaseDBase() ? "Yes" : "No"
        )));
        settings.push_back(std::make_pair("Location Information", printString(buffer, "%s (ASCII), %s (dBase)",
            canReportClusterFiles && _parameters.GetOutputAreaSpecificAscii() ? "Yes" : "No",
            canReportClusterFiles && _parameters.GetOutputAreaSpecificDBase() ? "Yes" : "No"
        )));
        settings.push_back(std::make_pair("Risk Estimates for Each Location", printString(buffer, "%s (ASCII), %s (dBase)",
            _parameters.GetOutputRelativeRisksAscii() ? "Yes" : "No",
            _parameters.GetOutputRelativeRisksDBase() ? "Yes" : "No"
        )));
        settings.push_back(std::make_pair(_parameters.IsTestStatistic() ? "Simulated Test Statistics" : "Simulated Log Likelihood Ratios",
            printString(buffer, "%s (ASCII), %s (dBase)",
                _parameters.GetOutputSimLoglikeliRatiosAscii() ? "Yes" : "No",
                _parameters.GetOutputSimLoglikeliRatiosDBase() ? "Yes" : "No"
            )));
    } catch (prg_exception& x) {
        x.addTrace("PrintOutputParameters()", "ParametersPrint");
        throw;
    }
    return settings;
}

/** Prints polygon paramters to file stream. */
ParametersPrint::SettingContainer_t& ParametersPrint::getPolygonParameters(SettingContainer_t& settings) const {
    std::string buffer, worker;
    try {
        settings.clear();
        if (_parameters.GetProbabilityModelType() == HOMOGENEOUSPOISSON) {
            for (size_t t = 0; t < _parameters.getObservableRegions().size(); ++t) {
                printString(buffer, "Polygon %i", t + 1);
                settings.push_back(std::make_pair(buffer, ""));
                InequalityContainer_t list = ConvexPolygonBuilder::parse(_parameters.getObservableRegions().at(t));
                for (size_t y = 0; y < list.size(); ++y) {
                    printString(worker, "%s%s", list.at(y).toString().c_str(), (y < list.size() - 1 ? ", " : ""));
                    settings.back().second += worker;
                }
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("PrintPolygonParameters()", "ParametersPrint");
        throw;
    }
    return settings;
}

/** Prints 'Multiple Data Set' tab parameters to file stream. */
ParametersPrint::SettingContainer_t& ParametersPrint::getMultipleDataSetParameters(SettingContainer_t& settings) const {
    std::string buffer;
    try {
        settings.clear();
        if (_parameters.getNumFileSets() == 1) return settings;
        for (unsigned int t = 1; t < _parameters.getNumFileSets(); ++t) {
            settings.push_back(std::make_pair(printString(buffer, "Data Set %i", t + 1), _parameters.getDataSourceNames()[t]));
            settings.push_back(std::make_pair("Case File", getFilenameFormatTime(_parameters.GetCaseFileName(t + 1), _parameters.getTimestamp())));
            switch (_parameters.GetProbabilityModelType()) {
            case POISSON:
                if (!_parameters.UsePopulationFile()) break;
                settings.push_back(std::make_pair("Population File", getFilenameFormatTime(_parameters.GetPopulationFileName(t + 1), _parameters.getTimestamp()))); break;
            case BERNOULLI:
                settings.push_back(std::make_pair("Control File", getFilenameFormatTime(_parameters.GetControlFileName(t + 1), _parameters.getTimestamp()))); break;
            case SPACETIMEPERMUTATION:
            case CATEGORICAL:
            case ORDINAL:
            case EXPONENTIAL:
            case NORMAL:
            case RANK:
            case UNIFORMTIME:
            case BATCHED:
            case HOMOGENEOUSPOISSON: break;
            default: throw prg_error("Unknown probability model type '%d'.\n", "PrintMultipleDataSetParameters()", _parameters.GetProbabilityModelType());
            }
        }
        switch (_parameters.GetMultipleDataSetPurposeType()) {
        case MULTIVARIATE: settings.push_back(std::make_pair("Purpose of Multiple Data Sets", "Multivariate Analysis")); break;
        case ADJUSTMENT: settings.push_back(std::make_pair("Purpose of Multiple Data Sets", "Adjustment")); break;
        default: throw prg_error("Unknown purpose for multiple data sets type '%d'.\n", "PrintMultipleDataSetParameters()", _parameters.GetMultipleDataSetPurposeType());
        }
    } catch (prg_exception& x) {
        x.addTrace("PrintMultipleDataSetParameters()", "ParametersPrint");
        throw;
    }
    return settings;
}

/** Prints 'Data Checking' parameters to file stream. */
ParametersPrint::SettingContainer_t& ParametersPrint::getDataCheckingParameters(SettingContainer_t& settings) const {
    std::string buffer;
    try {
        settings.clear();
        if (_parameters.GetProbabilityModelType() != HOMOGENEOUSPOISSON) {
            buffer = "Temporal Data Check";
            switch (_parameters.GetStudyPeriodDataCheckingType()) {
            case STRICTBOUNDS:
                settings.push_back(std::make_pair(buffer, "Check to ensure that all cases and controls are within the specified temporal study period."));
                break;
            case RELAXEDBOUNDS: settings.push_back(std::make_pair(buffer, "Ignore cases and controls that are outside the specified temporal study period."));
                break;
            default: throw prg_error("Unknown study period check type '%d'.\n", "PrintDataCheckingParameters()", _parameters.GetStudyPeriodDataCheckingType());
            }
        }
        if (!_parameters.GetIsPurelyTemporalAnalysis()) {
            buffer = "Geographical Data Check";
            switch (_parameters.GetCoordinatesDataCheckingType()) {
            case STRICTCOORDINATES:
                settings.push_back(std::make_pair(buffer, "Check to ensure that all observations (cases, controls and populations) are within the specified geographical area."));
                break;
            case RELAXEDCOORDINATES:
                settings.push_back(std::make_pair(buffer, "Ignore observations that are outside the specified geographical area."));
                break;
            default: throw prg_error("Unknown geographical coordinates check type '%d'.\n", "PrintDataCheckingParameters()", _parameters.GetCoordinatesDataCheckingType());
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("PrintDataCheckingParameters()", "ParametersPrint");
        throw;
    }
    return settings;
}

/** Prints 'Spatial Neighbors' tab parameters to file stream. */
ParametersPrint::SettingContainer_t& ParametersPrint::getSpatialNeighborsParameters(SettingContainer_t& settings) const {
    std::string buffer;
    try {
        settings.clear();
        if (!(_parameters.GetIsPurelyTemporalAnalysis() || _parameters.GetProbabilityModelType() == HOMOGENEOUSPOISSON)) {
            settings.push_back(std::make_pair("Specify neighbors through a non-Euclidean neighbors file", (_parameters.UseLocationNeighborsFile() ? "Yes" : "No")));
            if (_parameters.UseLocationNeighborsFile())
                settings.push_back(std::make_pair("Non-Euclidean Neighbors file", getFilenameFormatTime(_parameters.GetLocationNeighborsFileName(), _parameters.getTimestamp())));
            settings.push_back(std::make_pair("Specify a meta location file", (_parameters.UseLocationNeighborsFile() ? "Yes" : "No")));
            if (_parameters.UseMetaLocationsFile())
                settings.push_back(std::make_pair("Meta Locations File", getFilenameFormatTime(_parameters.getMetaLocationsFilename(), _parameters.getTimestamp())));
        }
        if (!(_parameters.GetIsPurelyTemporalAnalysis() ||
            _parameters.UseLocationNeighborsFile() ||
            _parameters.GetProbabilityModelType() == HOMOGENEOUSPOISSON)) {
            buffer = "Observations with Multiple Locations";
            switch (_parameters.GetMultipleCoordinatesType()) {
            case ONEPERLOCATION:
                settings.push_back(std::make_pair(buffer, "One location per observation."));
                break;
            case ATLEASTONELOCATION:
                settings.push_back(std::make_pair(buffer, "Include observation if at least one of its locations is in the window."));
                settings.push_back(std::make_pair("Multiple Locations Per Observation File", getFilenameFormatTime(_parameters.getMultipleLocationsFile(), _parameters.getTimestamp())));
                break;
            case ALLLOCATIONS:
                settings.push_back(std::make_pair(buffer, "Include observation only if all its locations are in the window."));
                settings.push_back(std::make_pair("Multiple Locations Per Observation File", getFilenameFormatTime(_parameters.getMultipleLocationsFile(), _parameters.getTimestamp())));
                break;
            default: throw prg_error("Unknown multiple coordinates type %d.\n", "PrintSpatialNeighborsParameters()", _parameters.GetMultipleCoordinatesType());
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("PrintSpatialNeighborsParameters()", "ParametersPrint");
        throw;
    }
    return settings;
}

/** Prints 'Temporal Window' tab parameters to file stream. */
ParametersPrint::SettingContainer_t& ParametersPrint::getTemporalWindowParameters(SettingContainer_t& settings) const {
    std::string buffer, worker;
    try {
        settings.clear();
        // skip this section if purely spatial or svtt analyses, these settings are not relevant
        if (_parameters.GetAnalysisType() == PURELYSPATIAL || _parameters.GetAnalysisType() == SPATIALVARTEMPTREND) 
            return settings;
        printString(
            buffer, "%i %s", _parameters.getMinimumTemporalClusterSize(),
            GetDatePrecisionAsString(_parameters.GetTimeAggregationUnitsType(), worker, _parameters.getMinimumTemporalClusterSize() != 1, true)
        );
        settings.push_back(std::make_pair("Minimum Temporal Cluster Size", buffer));
        switch (_parameters.GetMaximumTemporalClusterSizeType()) {
        case PERCENTAGETYPE:
            printString(buffer, "%g percent of study period", _parameters.GetMaximumTemporalClusterSize());
            settings.push_back(std::make_pair("Maximum Temporal Cluster Size", buffer));
            break;
        case TIMETYPE:
            printString(buffer, "%g %s",
                _parameters.GetMaximumTemporalClusterSize(),
                GetDatePrecisionAsString(_parameters.GetTimeAggregationUnitsType(), worker, _parameters.GetMaximumTemporalClusterSize() != 1, true));
            settings.push_back(std::make_pair("Maximum Temporal Cluster Size", buffer)); break;
            break;
        default: throw prg_error("Unknown maximum temporal cluster size type '%d'.\n",
            "PrintTemporalWindowParameters()", _parameters.GetMaximumTemporalClusterSizeType());
        }
        if (_parameters.GetProbabilityModelType() != SPACETIMEPERMUTATION && _parameters.GetIsSpaceTimeAnalysis()) {
            settings.push_back(std::make_pair("Include Purely Spatial Clusters", (_parameters.GetIncludePurelySpatialClusters() ? "Yes" : "No")));
        }
        if (_parameters.GetAnalysisType() == PURELYTEMPORAL || _parameters.GetAnalysisType() == SPACETIME) {
            switch (_parameters.GetIncludeClustersType()) {
            case ALIVECLUSTERS:
                settings.push_back(std::make_pair("Clusters to Include", "Only those including the study end date"));
                break;
            case ALLCLUSTERS: /*fprintf(fp, "All\n");
                              -- geIncludeClustersType parameter no longer visible in GUI,
                                 defaulted to ALLCLUSTERS, so don't print setting */ break;
            case CLUSTERSINRANGE:
                printString(buffer, "%s to %s", _parameters.GetStartRangeStartDate().c_str(), _parameters.GetStartRangeEndDate().c_str());
                settings.push_back(std::make_pair("Flexible Temporal Window Start Range", buffer));
                printString(buffer, "%s to %s", _parameters.GetEndRangeStartDate().c_str(), _parameters.GetEndRangeEndDate().c_str());
                settings.push_back(std::make_pair("Flexible Temporal Window End Range", buffer));
                break;
            default: throw prg_error("Unknown inclusion cluster type '%d'.\n", "PrintTemporalWindowParameters()", _parameters.GetIncludeClustersType());
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("getTemporalWindowParameters()", "ParametersPrint");
        throw;
    }
    return settings;
}

/** Print parameters of 'Other Output' tab/section. */
ParametersPrint::SettingContainer_t& ParametersPrint::getOtherOutputParameters(SettingContainer_t& settings) const {
    std::string buffer;
    try {
        settings.clear();
        if (!_parameters.getPerformPowerEvaluation() || (_parameters.getPerformPowerEvaluation() && _parameters.getPowerEvaluationMethod() == PE_WITH_ANALYSIS)) {
            settings.push_back(std::make_pair("Report Critical Values",(_parameters.GetReportCriticalValues() ? "Yes" : "No")));
            settings.push_back(std::make_pair("Report Monte Carlo Rank",(_parameters.getReportClusterRank() ? "Yes" : "No")));
        }
        if (_parameters.GetOutputAreaSpecificAscii() || _parameters.GetOutputClusterCaseAscii() ||
            _parameters.GetOutputClusterLevelAscii() || _parameters.GetOutputRelativeRisksAscii() || _parameters.GetOutputSimLoglikeliRatiosAscii())
            settings.push_back(std::make_pair("Print ASCII Column Headers",(_parameters.getPrintAsciiHeaders() ? "Yes" : "No")));
        settings.push_back(std::make_pair("User Defined Title",_parameters.GetTitleName()));
    } catch (prg_exception& x) {
        x.addTrace("getOtherOutputParameters()","ParametersPrint");
        throw;
    }
    return settings;
}

/** Prints 'Locations Network' tab parameters to file stream. */
ParametersPrint::SettingContainer_t& ParametersPrint::getLocationNetworkParameters(SettingContainer_t& settings) const {
    std::string buffer;
    try {
        settings.clear();
        if (!(_parameters.GetIsPurelyTemporalAnalysis() || _parameters.GetProbabilityModelType() == HOMOGENEOUSPOISSON)) {
            settings.push_back(std::make_pair("Use Locations Network File", (_parameters.getUseLocationsNetworkFile() ? "Yes" : "No")));
            if (_parameters.getUseLocationsNetworkFile())
                settings.push_back(std::make_pair("Locations Network File", getFilenameFormatTime(_parameters.getLocationsNetworkFilename(), _parameters.getTimestamp())));
        }
    } catch (prg_exception& x) {
        x.addTrace("getLocationNetworkParameters()", "ParametersPrint");
        throw;
    }
    return settings;
}

/** Print parameters of 'Miscellaneous Analysis' tab/section. */
ParametersPrint::SettingContainer_t& ParametersPrint::getMiscellaneousAnalysisParameters(SettingContainer_t& settings) const {
    std::string buffer;
    settings.clear();
    if (_parameters.GetProbabilityModelType() == POISSON && _parameters.GetAnalysisType() == PURELYSPATIAL) {
        settings.push_back(std::make_pair("Report Oliveira's F", (_parameters.getCalculateOliveirasF() ? "Yes" : "No")));
        if (_parameters.getCalculateOliveirasF())
            settings.push_back(std::make_pair("Number of bootstrap replications", printString(buffer, "%u", _parameters.getNumRequestedOliveiraSets())));
    }
    if (_parameters.GetIsProspectiveAnalysis() && _parameters.GetTimeAggregationUnitsType() != GENERIC) {
        switch (_parameters.getProspectiveFrequencyType()) {
        case SAME_TIMEAGGREGATION: buffer = "Same As Time Aggregation"; break;
        case DAILY:
            if (_parameters.getProspectiveFrequency() > 1) printString(buffer, "Daily (every %u days)", _parameters.getProspectiveFrequency());
            else buffer = "Daily";
            break;
        case WEEKLY:
            if (_parameters.getProspectiveFrequency() > 1) printString(buffer, "Weekly (every %u weeks)", _parameters.getProspectiveFrequency());
            else buffer = "Weekly";
            break;
        case MONTHLY:
            if (_parameters.getProspectiveFrequency() > 1) printString(buffer, "Monthly (every %u months)", _parameters.getProspectiveFrequency());
            else buffer = "Monthly";
            break;
        case QUARTERLY:
            if (_parameters.getProspectiveFrequency() > 1) printString(buffer, "Quarterly (every %u quarters)", _parameters.getProspectiveFrequency());
            else buffer = "Quarterly";
            break;
        case YEARLY:
            if (_parameters.getProspectiveFrequency() > 1) printString(buffer, "Yearly (every %u years)", _parameters.getProspectiveFrequency());
            else buffer = "Yearly";
            break;
        default: throw prg_error("Unknown prospective frequency type '%d'.\n", "PrintMiscellaneousAnalysisParameters()", _parameters.getProspectiveFrequencyType());
        }
        settings.push_back(std::make_pair("Prospective Analysis Frequency", buffer));
    }
    return settings;
}


std::vector<std::string>& ParametersPrint::getAnalysisSummaryStatements(std::vector<std::string>& statements, const DataSetHandler& SetHandler) const {
    try {
        std::string buffer;
        switch (_parameters.GetAnalysisType()) {
        case PURELYSPATIAL: statements.push_back("Purely Spatial"); break;
        case PURELYTEMPORAL: statements.push_back("Retrospective Purely Temporal"); break;
        case SPACETIME: statements.push_back("Retrospective Space-Time"); break;
        case PROSPECTIVESPACETIME: statements.push_back("Prospective Space-Time"); break;
        case SPATIALVARTEMPTREND: statements.push_back("Spatial Variation in Temporal Trends"); break;
        case PROSPECTIVEPURELYTEMPORAL: statements.push_back("Prospective Purely Temporal"); break;
        case SEASONALTEMPORAL: statements.push_back("Seasonal Temporal"); break;
        default: throw prg_error("Unknown analysis type '%d'.\n", "PrintAnalysisSummary()", _parameters.GetAnalysisType());
        }
        if (_parameters.getPerformPowerEvaluation() && _parameters.getPowerEvaluationMethod() == PE_WITH_ANALYSIS)
            statements.back() += " analysis and power evaluation";
        else
            statements.back() += _parameters.getPerformPowerEvaluation() ? " power evaluation" : " analysis";
        statements.push_back("scanning for ");
        if (_parameters.GetAnalysisType() == PURELYSPATIAL && _parameters.GetRiskType() == MONOTONERISK)
            statements.back() += "monotone ";
        std::string s(GetAreaScanRateTypeAsString());
        std::transform(s.begin(), s.end(), s.begin(), (int(*)(int)) tolower);
        statements.back() += printString(buffer, "clusters with %s", s.c_str());
        switch (_parameters.GetProbabilityModelType()) {
        case POISSON: statements.push_back("using the Discrete Poisson model"); break;
        case BERNOULLI: statements.push_back("using the Bernoulli model"); break;
        case SPACETIMEPERMUTATION: statements.push_back("using the Space-Time Permutation model"); break;
        case CATEGORICAL: statements.push_back("using the Multinomial model"); break;
        case ORDINAL: statements.push_back("using the Ordinal model"); break;
        case EXPONENTIAL: statements.push_back("using the Exponential model"); break;
        case NORMAL: statements.push_back("using the Normal model"); break;
        case RANK: statements.push_back("using the Rank model"); break;
        case UNIFORMTIME: statements.push_back("using the Uniform Time model"); break;
        case BATCHED: statements.push_back("using the Batched model"); break;
        case HOMOGENEOUSPOISSON: statements.push_back("using the Continuous Poisson model"); break;
        default: throw prg_error("Unknown probability model type '%d'.\n", "PrintAnalysisSummary()", _parameters.GetProbabilityModelType());
        }
        if (_parameters.getNumFileSets() > 1) {
            if (_parameters.getIsBernoulliIterativeDrilldownAsDOW()) {
                statements.push_back(printString(buffer, "with day of week adjustment"));
            }
            else {
                switch (_parameters.GetMultipleDataSetPurposeType()) {
                case MULTIVARIATE: statements.push_back(printString(buffer, "with multivariate scan using %u data sets", _parameters.getNumFileSets())); break;
                case ADJUSTMENT: statements.push_back(printString(buffer, "with adjustment using %u data sets", _parameters.getNumFileSets())); break;
                default: throw prg_error("Unknown purpose for multiple data sets type '%d'.\n", "PrintAnalysisSummary()", _parameters.GetMultipleDataSetPurposeType());
                }
            }
        }
        if (_parameters.GetIsSpaceTimeAnalysis()) {
            if (_parameters.GetIncludePurelySpatialClusters() && _parameters.GetIncludePurelyTemporalClusters())
                statements.push_back("including purely spatial and purely temporal clusters");
            else if (_parameters.GetIncludePurelySpatialClusters())
                statements.push_back("including purely spatial clusters");
            else if (_parameters.GetIncludePurelyTemporalClusters())
                statements.push_back("including purely temporal clusters");
        }
        if (_parameters.GetIsIterativeScanning() && _parameters.GetProbabilityModelType() != HOMOGENEOUSPOISSON)
            statements.push_back("with iterative analyses for secondary clusters");
        switch (_parameters.GetTimeTrendAdjustmentType()) {
        case TEMPORAL_NOTADJUSTED: break;
        case TEMPORAL_NONPARAMETRIC: statements.push_back("adjusting for time nonparametrically"); break;
        case LOGLINEAR_PERC: statements.push_back(getCalculatedTimeTrendAsString(SetHandler)); break;
        case CALCULATED_LOGLINEAR_PERC: statements.push_back(getCalculatedTimeTrendAsString(SetHandler)); break;
        case TEMPORAL_STRATIFIED_RANDOMIZATION: statements.push_back("adjusting for time with stratified randomization"); break;
        case CALCULATED_QUADRATIC:statements.push_back(getCalculatedTimeTrendAsString(SetHandler)); break;
        default: throw prg_error("Unknown time trend adjustment type '%d'.\n", "PrintAdjustments()", _parameters.GetTimeTrendAdjustmentType());
        }
        switch (_parameters.GetSpatialAdjustmentType()) {
        case SPATIAL_NOTADJUSTED: break;
        case SPATIAL_STRATIFIED_RANDOMIZATION:
            statements.push_back("adjusting for purely spatial clusters with stratified randomization"); break;
        case SPATIAL_NONPARAMETRIC:
            statements.push_back("adjusting for purely spatial clusters nonparametrically"); break;
        default: throw prg_error("Unknown time trend adjustment type '%d'.\n", "PrintAdjustments()", _parameters.GetSpatialAdjustmentType());
        }
        if (_parameters.getAdjustForWeeklyTrends())
            statements.push_back("adjusting for weekly trends nonparametrically");
        if (_parameters.UseAdjustmentForRelativeRisksFile())
            statements.push_back("adjusting for known relative risks");
        statements.back() += ".";
    } catch (prg_exception& x) {
        x.addTrace("getAnalysisSummary()", "ParametersPrint");
        throw;
    }
    return statements;
}

/** Prints analysis type related information, in a particular format, to passed ascii file. */
void ParametersPrint::PrintAnalysisSummary(FILE* fp, const DataSetHandler& SetHandler) const {
    try {
        std::string buffer;
        std::vector<std::string> statements;
        switch (_parameters.GetAnalysisType()) {
            case PURELYSPATIAL             : statements.push_back("Purely Spatial"); break;
            case PURELYTEMPORAL            : statements.push_back("Retrospective Purely Temporal"); break;
            case SPACETIME                 : statements.push_back("Retrospective Space-Time"); break;
            case PROSPECTIVESPACETIME      : statements.push_back("Prospective Space-Time"); break;
            case SPATIALVARTEMPTREND       : statements.push_back("Spatial Variation in Temporal Trends"); break;
            case PROSPECTIVEPURELYTEMPORAL : statements.push_back("Prospective Purely Temporal"); break;
            case SEASONALTEMPORAL          : statements.push_back("Seasonal Temporal"); break;
            default : throw prg_error("Unknown analysis type '%d'.\n","PrintAnalysisSummary()", _parameters.GetAnalysisType());
        }
        if (_parameters.getPerformPowerEvaluation() && _parameters.getPowerEvaluationMethod() == PE_WITH_ANALYSIS)
            statements.back() += " analysis and power evaluation";
        else
            statements.back() += _parameters.getPerformPowerEvaluation() ? " power evaluation" : " analysis";
        statements.push_back("scanning for ");
        if (_parameters.GetAnalysisType() == PURELYSPATIAL && _parameters.GetRiskType() == MONOTONERISK)
            statements.back() += "monotone ";
        std::string s(GetAreaScanRateTypeAsString());
        std::transform(s.begin(), s.end(), s.begin(), (int(*)(int)) tolower);
        statements.back() += printString(buffer, "clusters with %s", s.c_str());
        switch (_parameters.GetProbabilityModelType()) {
            case POISSON              : statements.push_back("using the Discrete Poisson model"); break;
            case BERNOULLI            : statements.push_back("using the Bernoulli model"); break;
            case SPACETIMEPERMUTATION : statements.push_back("using the Space-Time Permutation model"); break;
            case CATEGORICAL          : statements.push_back("using the Multinomial model"); break;
            case ORDINAL              : statements.push_back("using the Ordinal model"); break;
            case EXPONENTIAL          : statements.push_back("using the Exponential model"); break;
            case NORMAL               : statements.push_back("using the Normal model"); break;
            case RANK                 : statements.push_back("using the Rank model"); break;
            case UNIFORMTIME          : statements.push_back("using the Uniform Time model"); break;
            case BATCHED              : statements.push_back("using the Batched model"); break;
            case HOMOGENEOUSPOISSON   : statements.push_back("using the Continuous Poisson model"); break;
            default : throw prg_error("Unknown probability model type '%d'.\n", "PrintAnalysisSummary()", _parameters.GetProbabilityModelType());
        }
        if (_parameters.getNumFileSets() > 1) {
            if (_parameters.getIsBernoulliIterativeDrilldownAsDOW()) {
                statements.push_back(printString(buffer, "with day of week adjustment"));
            } else {
                switch (_parameters.GetMultipleDataSetPurposeType()) {
                    case MULTIVARIATE: statements.push_back(printString(buffer, "with multivariate scan using %u data sets", _parameters.getNumFileSets())); break;
                    case ADJUSTMENT: statements.push_back(printString(buffer, "with adjustment using %u data sets", _parameters.getNumFileSets())); break;
                    default: throw prg_error("Unknown purpose for multiple data sets type '%d'.\n", "PrintAnalysisSummary()", _parameters.GetMultipleDataSetPurposeType());
                }
            }
        }
        if (_parameters.GetIsSpaceTimeAnalysis()) {
            if (_parameters.GetIncludePurelySpatialClusters() && _parameters.GetIncludePurelyTemporalClusters())
                statements.push_back("including purely spatial and purely temporal clusters");
            else if (_parameters.GetIncludePurelySpatialClusters())
                statements.push_back("including purely spatial clusters");
            else if (_parameters.GetIncludePurelyTemporalClusters())
                statements.push_back("including purely temporal clusters");
        }
        if (_parameters.GetIsIterativeScanning() && _parameters.GetProbabilityModelType() != HOMOGENEOUSPOISSON)
            statements.push_back("with iterative analyses for secondary clusters");
        switch (_parameters.GetTimeTrendAdjustmentType()) {
            case TEMPORAL_NOTADJUSTED: break;
            case TEMPORAL_NONPARAMETRIC: statements.push_back("adjusting for time nonparametrically"); break;
            case LOGLINEAR_PERC: statements.push_back(getCalculatedTimeTrendAsString(SetHandler)); break;
            case CALCULATED_LOGLINEAR_PERC: statements.push_back(getCalculatedTimeTrendAsString(SetHandler)); break;
            case TEMPORAL_STRATIFIED_RANDOMIZATION: statements.push_back("adjusting for time with stratified randomization"); break;
            case CALCULATED_QUADRATIC:statements.push_back(getCalculatedTimeTrendAsString(SetHandler)); break;
            default: throw prg_error("Unknown time trend adjustment type '%d'.\n", "PrintAdjustments()", _parameters.GetTimeTrendAdjustmentType());
        }
        switch (_parameters.GetSpatialAdjustmentType()) {
            case SPATIAL_NOTADJUSTED: break;
            case SPATIAL_STRATIFIED_RANDOMIZATION:
                statements.push_back("adjusting for purely spatial clusters with stratified randomization"); break;
            case SPATIAL_NONPARAMETRIC:
                statements.push_back("adjusting for purely spatial clusters nonparametrically"); break;
            default: throw prg_error("Unknown time trend adjustment type '%d'.\n", "PrintAdjustments()", _parameters.GetSpatialAdjustmentType());
        }
        if (_parameters.getAdjustForWeeklyTrends())
            statements.push_back("adjusting for weekly trends nonparametrically");
        if (_parameters.UseAdjustmentForRelativeRisksFile()) 
            statements.push_back("adjusting for known relative risks");
        statements.back() += ".";

        AsciiPrintFormat PrintFormat;
        PrintFormat.SetMarginsAsOverviewSection();
        for (auto& line: statements)
            PrintFormat.PrintAlignedMarginsDataString(fp, line);
    } catch (prg_exception& x) {
        x.addTrace("PrintAnalysisSummary()","ParametersPrint");
        throw;
    }
}

/** Creates formatted string of calculated time trend adjustment parameters. */
std::string ParametersPrint::getCalculatedTimeTrendAsString(const DataSetHandler& SetHandler) const {
    unsigned int             t;
    std::string              work, trend_label;
    std::stringstream        strBuffer;
    std::deque<unsigned int> TrendIncrease, TrendDecrease;

    if (!(_parameters.GetTimeTrendAdjustmentType() == LOGLINEAR_PERC ||
          _parameters.GetTimeTrendAdjustmentType() == CALCULATED_LOGLINEAR_PERC || 
          _parameters.GetTimeTrendAdjustmentType() == CALCULATED_QUADRATIC))
        return work;

    //NOTE: Each dataset has own calculated time trend.
    if (_parameters.GetTimeTrendAdjustmentType() == CALCULATED_QUADRATIC) {
        strBuffer << "adjusting for log quadratic time trend with:";
        for (t=0; t < SetHandler.GetNumDataSets(); ++t) {
            strBuffer << std::endl << SetHandler.GetDataSet(t).getCalculatedQuadraticTimeTrend().c_str();
            if (SetHandler.GetNumDataSets() > 1) strBuffer << " for data set " << (t + 1);
        }
    } else {
        switch (_parameters.GetTimeAggregationUnitsType()) {
            case YEAR: trend_label = "an annual"; break;
            case MONTH: trend_label = "a monthly"; break;
            case DAY: trend_label = "a daily"; break;
            case GENERIC: trend_label = "a unit"; break;
            case NONE:
            default: throw prg_error("Unknown time aggregation type '%d'.\n", "getCalculatedTimeTrendAsString()", _parameters.GetTimeAggregationUnitsType());
         }
         if (SetHandler.GetNumDataSets() == 1) {
             strBuffer << "adjusting for time trend with " << trend_label.c_str();
             strBuffer << (SetHandler.GetDataSet(0).getCalculatedTimeTrendPercentage() < 0 ? " decrease " : " increase ");
             strBuffer << printString(work, "of %g%%", fabs(SetHandler.GetDataSet(0).getCalculatedTimeTrendPercentage()));
         } else {//multiple datasets print
            //count number of increasing and decreasing trends
            for (t=0; t < SetHandler.GetNumDataSets(); ++t) {
                if (SetHandler.GetDataSet(t).getCalculatedTimeTrendPercentage() < 0)
                    TrendDecrease.push_back(t);
                else
                    TrendIncrease.push_back(t);
            }
            //now print
            strBuffer << "adjusting for time trend with " << trend_label.c_str() << " ";
            //print increasing trends first
            if (TrendIncrease.size()) {
                strBuffer << printString(work, "increase of %0.2f%%", fabs(SetHandler.GetDataSet(TrendIncrease.front()).getCalculatedTimeTrendPercentage())).c_str();
                for (t=1; t < TrendIncrease.size(); ++t) {
                    strBuffer << printString(work, (t < TrendIncrease.size() - 1) ? ", %g%%" : " and %g%%", fabs(SetHandler.GetDataSet(TrendIncrease[t]).getCalculatedTimeTrendPercentage())).c_str();
                }
                strBuffer << printString(work, " for data set%s %u", (TrendIncrease.size() == 1 ? "" : "s"), TrendIncrease.front() + 1).c_str();
                for (t=1; t < TrendIncrease.size(); ++t) {
                    strBuffer << printString(work, (t < TrendIncrease.size() - 1 ? ", %u" : " and %u"), TrendIncrease[t] + 1).c_str();
                }
                strBuffer << printString(work, (TrendIncrease.size() > 1 ? " respectively" : "")).c_str();
                if (TrendDecrease.size() > 0) {
                    strBuffer << printString(work, " and %s ", trend_label.c_str()).c_str();
                }
            }
            //print decreasing trends
            if (TrendDecrease.size()) {
                strBuffer << printString(work, "decrease of %0.2f%%", fabs(SetHandler.GetDataSet(TrendDecrease.front()).getCalculatedTimeTrendPercentage())).c_str();
                for (t=1; t < TrendDecrease.size(); ++t) {
                    strBuffer << printString(work, (t < TrendDecrease.size() - 1) ? ", %g%%" : " and %0.2f%%", fabs(SetHandler.GetDataSet(TrendDecrease[t]).getCalculatedTimeTrendPercentage())).c_str();
                }
                strBuffer << printString(work, " for data set%s %u", (TrendDecrease.size() == 1 ? "" : "s"), TrendDecrease.front() + 1).c_str();
                for (t=1; t < TrendDecrease.size(); ++t) {
                    strBuffer << printString(work, (t < TrendDecrease.size() - 1 ? ", %u" : " and %u"), TrendDecrease[t] + 1).c_str();
                }
                strBuffer << printString(work, (TrendDecrease.size() > 1 ? " respectively" : "")).c_str();
            }
        }
    }
    work = strBuffer.str();
    return work;
}

/** Prints 'Spatial Output' tab parameters to file stream. */
ParametersPrint::SettingContainer_t& ParametersPrint::getSpatialOutputParameters(SettingContainer_t& settings) const {
    std::string buffer, worker;
    try {
        settings.clear();
        // skip these settings for purely temporal analysis
        if (_parameters.GetIsPurelyTemporalAnalysis()) return settings;
        // skip these settings when performing power evaluations without running an analysis
        if ((_parameters.getPerformPowerEvaluation() && _parameters.getPowerEvaluationMethod() != PE_WITH_ANALYSIS)) return settings;
        if (_parameters.getOutputKMLFile() || _parameters.getOutputCartesianGraph() || _parameters.getOutputGoogleMapsFile())
            settings.push_back(std::make_pair("Automatically Launch Map", (_parameters.getLaunchMapViewer() ? "Yes" : "No")));
        if (_parameters.getOutputKMLFile()) {
            settings.push_back(std::make_pair("Compress KML File into KMZ File",(_parameters.getCompressClusterKML() ? "Yes" : "No")));
            settings.push_back(std::make_pair("Include All Location IDs in the Clusters",(_parameters.getIncludeLocationsKML() ? "Yes" : "No")));
            printString(buffer, "%u", _parameters.getLocationsThresholdKML());
            settings.push_back(std::make_pair("Cluster Location Threshold - Separate KML",buffer));
        }
        settings.push_back(std::make_pair("Report Hierarchical Clusters", (_parameters.getReportHierarchicalClusters() ? "Yes" : "No")));
        if (_parameters.getReportHierarchicalClusters()) {
            buffer = "Criteria for Reporting Secondary Clusters";
            switch (_parameters.GetCriteriaSecondClustersType()) {
            case NOGEOOVERLAP:
                settings.push_back(std::make_pair(buffer, "No Geographical Overlap")); break;
            case NOCENTROIDSINOTHER:
                settings.push_back(std::make_pair(buffer, "No Cluster Centroids in Other Clusters")); break;
            case NOCENTROIDSINMORELIKE:
                settings.push_back(std::make_pair(buffer, "No Cluster Centroids in More Likely Clusters")); break;
            case NOCENTROIDSINLESSLIKE:
                settings.push_back(std::make_pair(buffer, "No Cluster Centroids in Less Likely Clusters")); break;
            case NOPAIRSINEACHOTHERS:
                settings.push_back(std::make_pair(buffer, "No Pairs of Centroids Both in Each Others Clusters")); break;
            case NORESTRICTIONS:
                settings.push_back(std::make_pair(buffer, "No Restrictions = Most Likely Cluster for Each Centroid")); break;
            default: throw prg_error("Unknown secondary clusters type '%d'.\n", "getSpatialOutputParameters()", _parameters.GetCriteriaSecondClustersType());
            }
        }
        if (_parameters.GetAnalysisType() == PURELYSPATIAL) {
            settings.push_back(std::make_pair("Report Gini Optimized Cluster Collection", (_parameters.getReportGiniOptimizedClusters() ? "Yes" : "No")));
            if (_parameters.getReportGiniOptimizedClusters()) {
                buffer = "Gini Index Based Collection Reporting";
                switch (_parameters.getGiniIndexReportType()) {
                case OPTIMAL_ONLY: settings.push_back(std::make_pair(buffer, "Optimal Only")); break;
                case ALL_VALUES: settings.push_back(std::make_pair(buffer, "All Values")); break;
                default: throw prg_error("Unknown index based cluster reporting type '%d'.\n", "getSpatialOutputParameters()", _parameters.getGiniIndexReportType());
                }
                settings.push_back(std::make_pair("Report Gini Index Cluster Coefficents", (_parameters.getReportGiniIndexCoefficents() ? "Yes" : "No")));
                /*printString(buffer, "%g", _parameters.getExecuteSpatialWindowStops()[0]);
                for (size_t i=1; i < _parameters.getExecuteSpatialWindowStops().size(); ++i) {
                printString(worker, ", %g", _parameters.getExecuteSpatialWindowStops()[i]);
                buffer += worker;
                }
                settings.push_back(std::make_pair("Spatial Cluster Maxima",buffer));*/
            }
        }
        settings.push_back(std::make_pair("Restrict Reporting to Smaller Clusters", (_parameters.GetRestrictingMaximumReportedGeoClusterSize() ? "Yes" : "No")));
        if (_parameters.GetRestrictingMaximumReportedGeoClusterSize()) {
            if (!(_parameters.GetAnalysisType() == PROSPECTIVESPACETIME && _parameters.GetAdjustForEarlierAnalyses())) {
                if (_parameters.GetProbabilityModelType() == SPACETIMEPERMUTATION)
                    settings.push_back(std::make_pair("Reported Clusters", "Only clusters smaller < 100% of population at risk"));
                else
                    settings.push_back(std::make_pair("Reported Clusters", 
                        printString(buffer, "Only clusters smaller than %g percent of population at risk reported.", 
                                _parameters.GetMaxSpatialSizeForType(PERCENTOFPOPULATION, true))
                        )
                    );
            }
            if (_parameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true)) {
                printString(buffer, "Only clusters smaller than %g percent of population defined in max circle file reported.", _parameters.GetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true));
                settings.push_back(std::make_pair("Reported Clusters", buffer));
            }
            if (_parameters.GetRestrictMaxSpatialSizeForType(MAXDISTANCE, true)) {
                printString(buffer, "Only clusters smaller than %g%s reported.",
                    _parameters.GetMaxSpatialSizeForType(MAXDISTANCE, true), (_parameters.GetCoordinatesType() == CARTESIAN ? " Cartesian units" : " km"));
                settings.push_back(std::make_pair("Reported Clusters", buffer));
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("getSpatialOutputParameters()","ParametersPrint");
        throw;
    }
    return settings;
}

ParametersPrint::SettingContainer_t& ParametersPrint::getDrilldownParameters(SettingContainer_t& settings, bool isDrilldown) const {
    std::string buffer;
    try {
        settings.clear();
        bool permitsStandard = _parameters.GetIsSpaceTimeAnalysis() || _parameters.GetAnalysisType() == PURELYSPATIAL || _parameters.GetAnalysisType() == SPATIALVARTEMPTREND;
        permitsStandard &= !_parameters.UseMetaLocationsFile();
        bool permitsBernoulli = (
           (_parameters.GetIsSpaceTimeAnalysis() && (_parameters.GetProbabilityModelType() == POISSON || _parameters.GetProbabilityModelType() == SPACETIMEPERMUTATION)) ||
           (isDrilldown && _parameters.GetIsPurelySpatialAnalysis() && _parameters.GetProbabilityModelType() == BERNOULLI && _parameters.GetIsIterativeScanning())
        );
        permitsBernoulli &= !_parameters.UseMetaLocationsFile();
        if (permitsStandard || permitsBernoulli) {
            if (permitsStandard)
                settings.push_back(std::make_pair("Same Design as Main Analysis", (_parameters.getPerformStandardDrilldown() ? "Yes" : "No")));
            if (permitsBernoulli)
                settings.push_back(std::make_pair("Purely Spatial Bernoulli", (_parameters.getPerformBernoulliDrilldown() ? "Yes" : "No")));
            if (_parameters.getPerformStandardDrilldown() || _parameters.getPerformBernoulliDrilldown()) {
                settings.push_back(std::make_pair("Cutoff of Detected Cluster", printString(buffer, "%g", _parameters.getDrilldownCutoff())));
                settings.push_back(std::make_pair("Minimum Locations in Deteted Cluster", printString(buffer, "%u", _parameters.getDrilldownMinimumLocationsCluster())));
                settings.push_back(std::make_pair("Minimum Cases in Deteted Cluster", printString(buffer, "%u", _parameters.getDrilldownMinimumCasesCluster())));
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("getDrilldownParameters()", "ParametersPrint");
        throw;
    }
    return settings;
}

/** Prints 'Elliptic Scan' parameters to file stream. */
ParametersPrint::SettingContainer_t& ParametersPrint::getEllipticScanParameters(SettingContainer_t& settings) const {
    std::string buffer, worker;
    try {
        settings.clear();
        if (!_parameters.GetIsPurelyTemporalAnalysis() && !_parameters.UseLocationNeighborsFile() && _parameters.GetSpatialWindowType() == ELLIPTIC) {
            printString(buffer, "%g", _parameters.GetEllipseShapes()[0]);
            for (size_t i=1; i < _parameters.GetEllipseShapes().size(); ++i) {
                printString(worker, ", %g", _parameters.GetEllipseShapes()[i]);
                buffer += worker;
            }
            settings.push_back(std::make_pair("Ellipse Shapes",buffer));
            printString(buffer, "%i", _parameters.GetEllipseRotations()[0]);
            for (size_t i=1; i < _parameters.GetEllipseRotations().size(); ++i) {
                printString(worker, ", %i", _parameters.GetEllipseRotations()[i]);
                buffer += worker;
            }
            settings.push_back(std::make_pair("Number of Angles for Each Ellipse Shape",buffer));
        }
    } catch (prg_exception& x) {
        x.addTrace("getEllipticScanParameters()","ParametersPrint");
        throw;
    }
    return settings;
}

/** Prints 'Notifications' tab parameters to file stream. */
ParametersPrint::SettingContainer_t& ParametersPrint::getNotificationsParameters(SettingContainer_t& settings) const {
    std::string buffer;
    try {
        settings.clear();
        settings.push_back(std::make_pair("Always Send Email", (_parameters.getAlwaysEmailSummary() ? "Yes" : "No")));
        if (_parameters.getAlwaysEmailSummary())
            settings.push_back(std::make_pair("Always Notify Recipients", _parameters.getEmailAlwaysRecipients()));
        settings.push_back(std::make_pair("Send Email with Results Meeting Cutoff", (_parameters.getCutoffEmailSummary() ? "Yes" : "No")));
        if (_parameters.getCutoffEmailSummary()) {
            settings.push_back(std::make_pair("Cutoff Notify Recipients", _parameters.getEmailCutoffRecipients()));
            settings.push_back(std::make_pair("Email Cutoff Value", printString(buffer, "%g", _parameters.getCutoffEmailValue())));
        }
        if (_parameters.getAlwaysEmailSummary() || _parameters.getCutoffEmailSummary()) {
            settings.push_back(std::make_pair("Attach Primary Results File", (_parameters.getEmailAttachResults() ? "Yes" : "No")));
            settings.push_back(std::make_pair("Include Results File and Directory", (_parameters.getEmailIncludeResultsDirectory() ? "Yes" : "No")));
            settings.push_back(std::make_pair("Create Custom Email", (_parameters.getEmailCustom() ? "Yes" : "No")));
        }
    } catch (prg_exception& x) {
        x.addTrace("getNotificationsParameters()", "ParametersPrint");
        throw;
    }
    return settings;
}

/** Prints 'Inference' tab parameters to file stream. */
ParametersPrint::SettingContainer_t& ParametersPrint::getClusterRestrictionsParameters(SettingContainer_t& settings) const {
    std::string buffer;
    try {
        settings.clear();
        // skip these settings when performing power evaluations without running an analysis
        if (!(_parameters.GetProbabilityModelType() == ORDINAL || _parameters.GetProbabilityModelType() == CATEGORICAL)) {
            if (_parameters.getIsWeightedNormalCovariates() && _parameters.GetAreaScanRateType() == HIGHANDLOW) {
                // There is a special situation with the weighted normal model with covariates and scanning for low and high rates together.
                unsigned int minimum = std::max(_parameters.getMinimumCasesLowRateClusters(), _parameters.getMinimumCasesHighRateClusters());
                settings.push_back(std::make_pair("Minimum Cases in Cluster for Low Rates", printString(buffer, "%u", minimum)));
                settings.push_back(std::make_pair("Minimum Cases in Cluster for High Rates", printString(buffer, "%u", minimum)));
            } else {
                if ((_parameters.GetAreaScanRateType() == LOW || _parameters.GetAreaScanRateType() == HIGHANDLOW) && _parameters.getMinimumCasesLowRateClusters() != 0)
                    settings.push_back(std::make_pair("Minimum Cases in Cluster for Low Rates", printString(buffer, "%u", _parameters.getMinimumCasesLowRateClusters())));
                if (_parameters.GetAreaScanRateType() == HIGH || _parameters.GetAreaScanRateType() == HIGHANDLOW)
                    settings.push_back(std::make_pair("Minimum Cases in Cluster for High Rates", printString(buffer, "%u", _parameters.getMinimumCasesHighRateClusters())));
            }
        }
        switch (_parameters.GetProbabilityModelType()) {
        case EXPONENTIAL:
            if (_parameters.GetAreaScanRateType() == HIGH || _parameters.GetAreaScanRateType() == HIGHANDLOW) {
                settings.push_back(std::make_pair("Restrict Short Survival Clusters", (_parameters.getRiskLimitHighClusters() ? "Yes" : "No")));
                if (_parameters.getRiskLimitHighClusters())
                    settings.push_back(std::make_pair("Risk Threshold Short Survival Clusters", printString(buffer, "%g", _parameters.getRiskThresholdHighClusters())));
            }
            if (_parameters.GetAreaScanRateType() == LOW || _parameters.GetAreaScanRateType() == HIGHANDLOW) {
                settings.push_back(std::make_pair("Restrict Long Survival Clusters", (_parameters.getRiskLimitLowClusters() ? "Yes" : "No")));
                if (_parameters.getRiskLimitLowClusters())
                    settings.push_back(std::make_pair("Risk Threshold Long Survival Clusters", printString(buffer, "%g", _parameters.getRiskThresholdLowClusters())));
            } break;
        case BATCHED:
        case SPACETIMEPERMUTATION:
        case POISSON:
        case HOMOGENEOUSPOISSON:
        case BERNOULLI:
            if (_parameters.GetAreaScanRateType() == HIGH || _parameters.GetAreaScanRateType() == HIGHANDLOW) {
                settings.push_back(std::make_pair("Restrict High Rate Clusters", (_parameters.getRiskLimitHighClusters() ? "Yes" : "No")));
                if (_parameters.getRiskLimitHighClusters())
                    settings.push_back(std::make_pair("Risk Threshold High Rate Clusters", printString(buffer, "%g", _parameters.getRiskThresholdHighClusters())));
            }
            if (_parameters.GetAreaScanRateType() == LOW || _parameters.GetAreaScanRateType() == HIGHANDLOW) {
                settings.push_back(std::make_pair("Restrict Low Rate Clusters", (_parameters.getRiskLimitLowClusters() ? "Yes" : "No")));
                if (_parameters.getRiskLimitLowClusters())
                    settings.push_back(std::make_pair("Risk Threshold Low Rate Clusters", printString(buffer, "%g", _parameters.getRiskThresholdLowClusters())));
            } break;
        default: break;
        }
    } catch (prg_exception& x) {
        x.addTrace("getClusterRestrictionsParameters()", "ParametersPrint");
        throw;
    }
    return settings;
}

/** Prints 'Inference' tab parameters to file stream. */
ParametersPrint::SettingContainer_t& ParametersPrint::getInferenceParameters(SettingContainer_t& settings) const {
    std::string buffer;
    try {
        settings.clear();
        buffer = "P-Value Reporting";
        switch (_parameters.GetPValueReportingType()) {
            case DEFAULT_PVALUE :
                settings.push_back(std::make_pair(buffer,"Default Combination"));
                break;
            case STANDARD_PVALUE :
                settings.push_back(std::make_pair(buffer,"Standard Monte Carlo"));
                if (_parameters.GetAreaScanRateType() == HIGH)
                    settings.push_back(std::make_pair("Report Gumbel Based P-Values",(_parameters.GetReportGumbelPValue() ? "Yes" : "No")));
                break;
            case TERMINATION_PVALUE :
                settings.push_back(std::make_pair(buffer,"Sequential Monte Carlo Early Termination"));
                printString(buffer, "%u percent of replications", _parameters.GetEarlyTermThreshold());
                settings.push_back(std::make_pair("Termination Cutoff",buffer));
                if (_parameters.GetAreaScanRateType() == HIGH)
                    settings.push_back(std::make_pair("Report Gumbel Based P-Values", (_parameters.GetReportGumbelPValue() ? "Yes" : "No")));
                break;
            case GUMBEL_PVALUE :
                settings.push_back(std::make_pair(buffer,"Gumbel Approximation"));
                break;
        }
        printString(buffer, "%u", _parameters.GetNumReplicationsRequested());
        settings.push_back(std::make_pair("Number of Replications",buffer));
        if (_parameters.GetIsProspectiveAnalysis() && _parameters.GetAdjustForEarlierAnalyses()) {
            settings.push_back(std::make_pair("Adjusted for Earlier Analyses", "Yes"));
            settings.push_back(std::make_pair("Prospective Start Date",_parameters.GetProspectiveStartDate()));
        }
        if (_parameters.GetProbabilityModelType() != HOMOGENEOUSPOISSON && !_parameters.getPerformPowerEvaluation()) {
            settings.push_back(std::make_pair("Adjusting for More Likely Clusters",(_parameters.GetIsIterativeScanning() ? "Yes" : "No")));
            if (_parameters.GetIsIterativeScanning()) {
                printString(buffer, "%u", _parameters.GetNumIterativeScansRequested());
                settings.push_back(std::make_pair("Maximum Number of Iterations",buffer));
                printString(buffer, "%g", _parameters.GetIterativeCutOffPValue());
                settings.push_back(std::make_pair("Iterative Scan Cluster Cutoff",buffer));
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("getInferenceParameters()","ParametersPrint");
        throw;
    }
    return settings;
}

/** Prints 'Power Simulations' parameters to file stream. */
ParametersPrint::SettingContainer_t& ParametersPrint::getPowerEvaluationsParameters(SettingContainer_t& settings) const {
    std::string buffer;
    FileName AdditionalOutputFile(_parameters.GetOutputFileName().c_str());
    try {
        settings.clear();
        if (_parameters.GetProbabilityModelType() == POISSON && _parameters.GetAnalysisType() != SPATIALVARTEMPTREND) {
            settings.push_back(std::make_pair("Perform Power Evaluations", (_parameters.getPerformPowerEvaluation() ? "Yes" : "No")));
            if (!_parameters.getPerformPowerEvaluation()) return settings;
            buffer = "Power Method";
            switch (_parameters.getPowerEvaluationMethod()) {
                case PE_WITH_ANALYSIS: 
                    settings.push_back(std::make_pair(buffer,"Standard Analysis and Power Evaluation Together")); break;
                case PE_ONLY_CASEFILE: 
                    settings.push_back(std::make_pair(buffer,"Only Power Evaluation, Using Total Cases from Case File")); break;
                case PE_ONLY_SPECIFIED_CASES: 
                    settings.push_back(std::make_pair(buffer,"Only Power Evaluation, Using Defined Total Cases")); 
                    printString(buffer, "%i", _parameters.getPowerEvaluationCaseCount());
                    settings.push_back(std::make_pair("Power Evaluation Total Cases",buffer)); 
                    break;
                default: throw prg_error("Unknown power evaluation method type '%d'.\n", "getPowerEvaluationsParameters()", _parameters.getPowerEvaluationMethod());
            }
            buffer = "Critical Values";
            switch (_parameters.getPowerEvaluationCriticalValueType()) {
                case CV_MONTECARLO: 
                    settings.push_back(std::make_pair(buffer,"Monte Carlo")); break;
                case CV_GUMBEL: 
                    settings.push_back(std::make_pair(buffer,"Gumbel")); break;
                case CV_POWER_VALUES: 
                    settings.push_back(std::make_pair(buffer,"User Defined"));
                    printString(buffer, "%lf", _parameters.getPowerEvaluationCriticalValue05());
                    settings.push_back(std::make_pair("Critical Value .05",buffer));
                    printString(buffer, "%lf", _parameters.getPowerEvaluationCriticalValue01());
                    settings.push_back(std::make_pair("Critical Value .01",buffer));
                    printString(buffer, "%lf", _parameters.getPowerEvaluationCriticalValue001());
                    settings.push_back(std::make_pair("Critical Value .001",buffer));
                    break;
                default: throw prg_error("Unknown critical values type '%d'.\n", "getPowerEvaluationsParameters()", _parameters.getPowerEvaluationCriticalValueType());
            }
            buffer = "Power Estimation";
            switch (_parameters.getPowerEstimationType()) {
                case PE_MONTECARLO: 
                    settings.push_back(std::make_pair(buffer,"Monte Carlo")); break;
                case PE_GUMBEL: 
                    settings.push_back(std::make_pair(buffer,"Gumbel")); break;
                default: throw prg_error("Unknown critical values type '%d'.\n", "getPowerEvaluationsParameters()", _parameters.getPowerEstimationType());
            }
            printString(buffer, "%u", _parameters.getNumPowerEvalReplicaPowerStep());
            settings.push_back(std::make_pair("Number of Replications",buffer));
            switch (_parameters.GetPowerEvaluationSimulationType()) {
                case STANDARD         : 
                    settings.push_back(std::make_pair("Alternative Hypothesis File", getFilenameFormatTime(_parameters.getPowerEvaluationAltHypothesisFilename(), _parameters.getTimestamp())));
                    break;
                case FILESOURCE       :
                    settings.push_back(std::make_pair("Power Step Randomization Method","File Source"));
                    settings.push_back(std::make_pair("Randomization Source File", getFilenameFormatTime(_parameters.getPowerEvaluationSimulationDataSourceFilename(), _parameters.getTimestamp()))); break;
                case HA_RANDOMIZATION :
                default : throw prg_error("Unknown simulation type '%d'.\n", "getPowerEvaluationsParameters()", _parameters.GetPowerEvaluationSimulationType());
            }
            // Since reporting the power evaluations' simulation data is not settable in the gui, only report if toggled on.
            if (_parameters.getOutputPowerEvaluationSimulationData()) {
                settings.push_back(std::make_pair("Output Power Step Simulation Data","Yes"));
                if (_parameters.getOutputPowerEvaluationSimulationData()) {
                    settings.push_back(std::make_pair("Power Step Simulation Data Filename", getFilenameFormatTime(_parameters.getPowerEvaluationSimulationDataOutputFilename(), _parameters.getTimestamp())));
                }
            }
            // loglikelihood ratio files for power evaluations
            if (_parameters.GetOutputSimLoglikeliRatiosAscii()) {
                AdditionalOutputFile.setExtension(printString(buffer, "%s%s", LoglikelihoodRatioWriter::LOG_LIKELIHOOD_FILE_HA_EXT, ASCIIDataFileWriter::ASCII_FILE_EXT).c_str());
                settings.push_back(std::make_pair("Power Evaluations Simulated LLRs File", AdditionalOutputFile.getFullPath(buffer)));
            }
            if (_parameters.GetOutputSimLoglikeliRatiosDBase()) {
                AdditionalOutputFile.setExtension(printString(buffer, "%s%s", LoglikelihoodRatioWriter::LOG_LIKELIHOOD_FILE_HA_EXT, DBaseDataFileWriter::DBASE_FILE_EXT).c_str());
                settings.push_back(std::make_pair("Power Evaluations Simulated LLRs File", AdditionalOutputFile.getFullPath(buffer)));
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("getPowerEvaluationsParameters()","ParametersPrint");
        throw;
    }
    return settings;
}

/** Prints 'Power Simulations' parameters to file stream. */
ParametersPrint::SettingContainer_t& ParametersPrint::getPowerSimulationsParameters(SettingContainer_t& settings) const {
    std::string buffer;
    try {
        settings.clear();
        buffer = "Simulation Method";
        switch (_parameters.GetSimulationType()) {
            case STANDARD         : break;
            case FILESOURCE       :
                settings.push_back(std::make_pair(buffer,"File Source")); break;
                settings.push_back(std::make_pair("Randomization File", getFilenameFormatTime(_parameters.GetSimulationDataSourceFilename(), _parameters.getTimestamp()))); break;
                break;
            case HA_RANDOMIZATION :
            default : throw prg_error("Unknown simulation type '%d'.\n", "getPowerSimulationsParameters()", _parameters.GetSimulationType());
        };
        if (_parameters.GetOutputSimulationData()) {
            settings.push_back(std::make_pair("Output Simulation Data","Yes"));
            settings.push_back(std::make_pair("Simulation Data Output", getFilenameFormatTime(_parameters.GetSimulationDataOutputFilename(), _parameters.getTimestamp())));
        }
    } catch (prg_exception& x) {
        x.addTrace("getPowerSimulationsParameters()","ParametersPrint");
        throw;
    }
    return settings;
}

/** Prints 'Run Options' parameters to file stream. */
ParametersPrint::SettingContainer_t& ParametersPrint::getRunOptionsParameters(SettingContainer_t& settings) const {
    std::string buffer;
    try {
        settings.clear();
        if (_parameters.GetNumRequestedParallelProcesses() == 0)
            settings.push_back(std::make_pair("Processor Usage","All Available Processors"));
        else {
            printString(buffer, "At Most %u Processors", _parameters.GetNumRequestedParallelProcesses());
            settings.push_back(std::make_pair("Processor Usage",buffer));
        }
        settings.push_back(std::make_pair("Suppress Warnings", (_parameters.GetSuppressingWarnings() ? "Yes" : "No")));
        settings.push_back(std::make_pair("Logging Analysis", (_parameters.GetIsLoggingHistory() ? "Yes" : "No")));
        if (_parameters.GetIsRandomlyGeneratingSeed())
            settings.push_back(std::make_pair("Use Random Seed",(_parameters.GetIsRandomlyGeneratingSeed() ? "Yes" : "No")));
        if (_parameters.GetRandomizationSeed() != RandomNumberGenerator::glDefaultSeed) {
            printString(buffer, "%ld\n", _parameters.GetRandomizationSeed());
            settings.push_back(std::make_pair("Randomization Seed",buffer));
        }
        if (_parameters.GetExecutionType() != AUTOMATIC) {
            buffer = "Execution Type";
            switch (_parameters.GetExecutionType()) {
                case AUTOMATIC    : settings.push_back(std::make_pair(buffer,"Automatic Determination")); break;
                case SUCCESSIVELY : settings.push_back(std::make_pair(buffer,"Successively")); break;
                case CENTRICALLY  : settings.push_back(std::make_pair(buffer,"Centrically")); break;
                default : throw prg_error("Unknown execution type '%d'.\n", "getRunOptionsParameters()", _parameters.GetExecutionType());
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("getRunOptionsParameters()","ParametersPrint");
        throw;
    }
    return settings;
}

/** Prints 'Space And Time Adjustments' tab parameters to file stream. */
ParametersPrint::SettingContainer_t& ParametersPrint::getSpaceAndTimeAdjustmentsParameters(SettingContainer_t& settings) const {
    bool bPrintingTemporalAdjustment = (
        _parameters.GetAnalysisType() == PURELYTEMPORAL || _parameters.GetAnalysisType() == SPACETIME ||
        _parameters.GetAnalysisType() == PROSPECTIVEPURELYTEMPORAL || _parameters.GetAnalysisType() == PROSPECTIVESPACETIME ||
        _parameters.GetAnalysisType() == SPATIALVARTEMPTREND) && (
        _parameters.GetProbabilityModelType() == POISSON || _parameters.GetProbabilityModelType() == BERNOULLI ||
        _parameters.GetProbabilityModelType() == BATCHED
    );
    bool bPrintingSpatialAdjustment = (
        _parameters.GetAnalysisType() == SPACETIME || _parameters.GetAnalysisType() == PROSPECTIVESPACETIME
    ) && _parameters.GetProbabilityModelType() == POISSON;
    std::string buffer, worker;
    try {
        settings.clear();
        if (bPrintingTemporalAdjustment) {
            switch (_parameters.GetTimeTrendAdjustmentType()) {
                case TEMPORAL_NOTADJUSTED:
                    settings.push_back(std::make_pair("Temporal Adjustment","None"));break;
                case TEMPORAL_NONPARAMETRIC:
                    settings.push_back(std::make_pair("Temporal Adjustment","Nonparametric"));break;
                case LOGLINEAR_PERC            :
                    printString(buffer, "Log linear trend Of %g%%", _parameters.GetTimeTrendAdjustmentPercentage());
                    switch (_parameters.getLogLinearTimeTrendAdjUnits()) {
                    case YEAR: buffer += " per year"; break;
                    case MONTH: buffer += " per month"; break;
                    case DAY: buffer += " per day"; break;
                    case GENERIC: buffer += " per unit"; break;
                    default: 
                        throw prg_error("Unknown date precision type '%d'.\n", 
                            "getSpaceAndTimeAdjustmentsParameters()", _parameters.getLogLinearTimeTrendAdjUnits()
                        );
                    }
                    settings.push_back(std::make_pair("Temporal Adjustment",buffer));break;
                case CALCULATED_LOGLINEAR_PERC :
                    settings.push_back(std::make_pair("Temporal Adjustment","Log Linear with automatically calculated trend"));break;
                case TEMPORAL_STRATIFIED_RANDOMIZATION:
                    settings.push_back(std::make_pair("Temporal Adjustment","Nonparametric, with time stratified randomization"));
                    if (_parameters.GetProbabilityModelType() == BATCHED) {
                        settings.push_back(std::make_pair("Adjustment Length",
                            printString(buffer, "%u %s",
                                _parameters.GetNonparametricAdjustmentSize(),
                                GetDatePrecisionAsString(_parameters.GetTimeAggregationUnitsType(), worker, _parameters.GetNonparametricAdjustmentSize() != 1, false)
                            )
                        ));
                    }
                    break;
                case CALCULATED_QUADRATIC:
                    settings.push_back(std::make_pair("Temporal Adjustment", "Log quadratic with automatically calculated trend")); break;
                default : throw prg_error(
                    "Unknown time trend adjustment type '%d'.\n", "getSpaceAndTimeAdjustmentsParameters()", _parameters.GetTimeTrendAdjustmentType()
                );
            }
        }
        if (_parameters.GetAnalysisType() != PURELYSPATIAL && (_parameters.GetProbabilityModelType() == POISSON || _parameters.GetProbabilityModelType() == SPACETIMEPERMUTATION)) {
            settings.push_back(std::make_pair("Adjust for Weekly Trends, Nonparametric",(_parameters.getAdjustForWeeklyTrends() ? "Yes" : "No")));
        }
        if (bPrintingSpatialAdjustment) {
            switch (_parameters.GetSpatialAdjustmentType()) {
                case SPATIAL_NOTADJUSTED               :
                    settings.push_back(std::make_pair("Spatial Adjustment","None")); break;
                case SPATIAL_STRATIFIED_RANDOMIZATION:
                    settings.push_back(std::make_pair("Spatial Adjustment","Nonparametric, with spatial stratified randomization")); break;
                case SPATIAL_NONPARAMETRIC:
                    settings.push_back(std::make_pair("Spatial Adjustment", "Nonparametric")); break;
                default : throw prg_error("Unknown spatial adjustment type '%d'.\n", "getSpaceAndTimeAdjustmentsParameters()", _parameters.GetSpatialAdjustmentType());
            }
        }
        if (_parameters.GetProbabilityModelType() == POISSON) {
            settings.push_back(std::make_pair("Adjust for known relative risks",(_parameters.UseAdjustmentForRelativeRisksFile() ? "Yes" : "No")));
            if (_parameters.UseAdjustmentForRelativeRisksFile())
                settings.push_back(std::make_pair("Adjustments File", getFilenameFormatTime(_parameters.GetAdjustmentsByRelativeRisksFilename(), _parameters.getTimestamp())));
            //since SVTT time trend type is defaulted to Linear and not GUI, only report as quadratic when set
            if (_parameters.GetAnalysisType() == SPATIALVARTEMPTREND && _parameters.getTimeTrendType() == QUADRATIC)
                settings.push_back(std::make_pair("Time Trend Type (SVTT)","Quadratic"));
        }
    } catch (prg_exception& x) {
        x.addTrace("getSpaceAndTimeAdjustmentsParameters()","ParametersPrint");
        throw;
    }
    return settings;
}

/** Prints 'Spatial Window' tab parameters to file stream. */
ParametersPrint::SettingContainer_t& ParametersPrint::getSpatialWindowParameters(SettingContainer_t& settings) const {
    std::string buffer, worker;
    try {
        settings.clear();
        if (_parameters.GetIsPurelyTemporalAnalysis()) return settings;
        if (!(_parameters.GetAnalysisType() == PROSPECTIVESPACETIME && _parameters.GetAdjustForEarlierAnalyses())) {
            if (_parameters.GetProbabilityModelType() == SPACETIMEPERMUTATION)
                settings.push_back(std::make_pair("Maximum Spatial Cluster Size", "< 100% of population at risk"));
            else {
                settings.push_back(std::make_pair("Maximum Spatial Cluster Size", 
                    printString(buffer, "%g percent of population at risk", _parameters.GetMaxSpatialSizeForType(PERCENTOFPOPULATION, false)))
                );
            }
        }
        if (_parameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false)) {
            printString(buffer, "%g percent of population defined in max circle file", _parameters.GetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false));
            settings.push_back(std::make_pair("Maximum Spatial Cluster Size", buffer));
        }
        if (_parameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false) || _parameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true))
            settings.push_back(std::make_pair("Max Circle Size File", getFilenameFormatTime(_parameters.GetMaxCirclePopulationFileName(), _parameters.getTimestamp())));
        if (_parameters.GetRestrictMaxSpatialSizeForType(MAXDISTANCE, false)) {
            printString(buffer, "%g%s", _parameters.GetMaxSpatialSizeForType(MAXDISTANCE, false), (_parameters.GetCoordinatesType() == CARTESIAN ? " Cartesian units" : " km"));
            settings.push_back(std::make_pair("Maximum Spatial Cluster Size", buffer));
        }
        if (_parameters.GetProbabilityModelType() != SPACETIMEPERMUTATION && _parameters.GetIsSpaceTimeAnalysis()) {
            settings.push_back(std::make_pair("Include Purely Temporal Clusters", (_parameters.GetIncludePurelyTemporalClusters() ? "Yes" : "No")));
        }
        if (!_parameters.UseLocationNeighborsFile()) {
            switch (_parameters.GetSpatialWindowType()) {
            case CIRCULAR: settings.push_back(std::make_pair("Window Shape", "Circular")); break;
            case ELLIPTIC:
                settings.push_back(std::make_pair("Window Shape", "Elliptic"));
                switch (_parameters.GetNonCompactnessPenaltyType()) {
                case NOPENALTY: settings.push_back(std::make_pair("Non-Compactness Penalty", "None")); break;
                case MEDIUMPENALTY: settings.push_back(std::make_pair("Non-Compactness Penalty", "Meduim")); break;
                case STRONGPENALTY: settings.push_back(std::make_pair("Non-Compactness Penalty", "Strong")); break;
                default:
                    throw prg_error("Unknown non-compactness penalty type '%d'.\n", "PrintSpatialWindowParameters()", _parameters.GetNonCompactnessPenaltyType());
                }
                break;
            default: throw prg_error("Unknown window shape type %d.\n", "PrintSpatialWindowParameters()", _parameters.GetSpatialWindowType());
            }
        }
        if (!_parameters.getPerformPowerEvaluation() && _parameters.GetAnalysisType() == PURELYSPATIAL &&
            (_parameters.GetProbabilityModelType() == POISSON || _parameters.GetProbabilityModelType() == BERNOULLI)) {
            settings.push_back(std::make_pair("Isotonic Scan", (_parameters.GetRiskType() == MONOTONERISK ? "Yes" : "No")));
        }
    } catch (prg_exception& x) {
        x.addTrace("gettSpatialWindowParameters()", "ParametersPrint");
        throw;
    }
    return settings;
}


/** Prints 'System' parameters to file stream. */
ParametersPrint::SettingContainer_t& ParametersPrint::getSystemParameters(SettingContainer_t& settings) const {
    settings.clear();
    const CParameters::CreationVersion & IniVersion = _parameters.GetCreationVersion();
    CParameters::CreationVersion Current;
    if (IniVersion != Current) {
        std::string buffer;
        printString(buffer, "%u.%u.%u", IniVersion.iMajor, IniVersion.iMinor, IniVersion.iRelease);
        settings.push_back(std::make_pair("Parameters Version",buffer));
    }
    return settings;
}

/** Prints 'Temporal Ouput' tab parameters to file stream. */
ParametersPrint::SettingContainer_t& ParametersPrint::getTemporalOutputParameters(SettingContainer_t& settings) const {
    std::string buffer, buffer2;
    try {
        settings.clear();
        // The temporal graph is option for purely temporal/space-time analyses with Poisson, Bernoulli, STP and Exponential.
        if (!(_parameters.GetIsPurelyTemporalAnalysis() || _parameters.GetIsSpaceTimeAnalysis()) ||
            !(_parameters.GetProbabilityModelType() == POISSON || _parameters.GetProbabilityModelType() == BERNOULLI || 
              _parameters.GetProbabilityModelType() == SPACETIMEPERMUTATION || _parameters.GetProbabilityModelType() == EXPONENTIAL ||
                _parameters.GetProbabilityModelType() == BATCHED || _parameters.GetProbabilityModelType() == UNIFORMTIME)) return settings;
        settings.push_back(std::make_pair("Produce Temporal Graphs",(_parameters.getOutputTemporalGraphFile() ? "Yes" : "No")));
        if (_parameters.getOutputTemporalGraphFile()) {
            buffer = "Cluster Graphing";
            switch (_parameters.getTemporalGraphReportType()) {
                case MLC_ONLY: settings.push_back(std::make_pair(buffer, "Most likely cluster only")); break;
                case X_MCL_ONLY: 
                    printString(buffer2, "%d most likely clusters, one graph for each", _parameters.getTemporalGraphMostLikelyCount());
                    settings.push_back(std::make_pair(buffer, buffer2)); break;
                case SIGNIFICANT_ONLY:
                    printString(buffer2, "All clusters, one graph for each, meeting cutoff %g", _parameters.getTemporalGraphSignificantCutoff());
                    settings.push_back(std::make_pair(buffer, buffer2)); break;
                default : throw prg_error("Unknown temporal graph type %d.\n", "getTemporalOutputParameters()", _parameters.getOutputTemporalGraphFile());
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("getTemporalOutputParameters()","ParametersPrint");
        throw;
    }
    return settings;
}

/** Writes settings container to file stream. */
void ParametersPrint::writeSettingPairsTextFile(const SettingContainer_t& settings, const std::string& section, FILE* fp, unsigned int margin) const {
    try {
        if (!settings.size()) return;
        if (section.size()) { //print section label
            fprintf(fp, "\n%s\n", section.c_str());
            for (size_t t = 0; t < section.size(); ++t)
                fprintf(fp, "-");
            fprintf(fp, "\n");
        }
        size_t tMaxLabel=0; //first calculate maximum label length
        for (auto s: settings)
            tMaxLabel = std::max(tMaxLabel, s.first.size());
        for (const auto& s : settings) { //print settings
            for (unsigned int i=0; i < margin; ++i) fprintf(fp, " ");
            fprintf(fp, "%s", s.first.c_str());
            for (size_t t=s.first.size(); t < tMaxLabel; ++t)
                fprintf(fp, " ");
            fprintf(fp, " : %s\n", s.second.c_str());
        }
    } catch (prg_exception& x) {
        x.addTrace("WriteSettingsContainer()","ParametersPrint");
        throw;
    }
}

void ParametersPrint::writeSettingPairsHTML(const SettingContainer_t& settings, const std::string& section, const std::string& sectionClass, std::ostream& out) const {
    try {
        if (!settings.size()) return;
        out << "<div class='" << sectionClass << "'>" << std::endl;
        if (section.size()) {// print section label
            out << "<h4>" << section << "</h4>" << std::endl;
        }
        out << "<table><tbody>" << std::endl;
        // print settings
        for (auto& setting : settings) {
            if (setting.first == "Temporal Graph File")
                out << "<tr><th>" << setting.first << " :</th><td><a target=\"_blank\" href=\"file:///" << setting.second << "\">" << setting.second << "</a></td></tr>" << std::endl;
            else
                out << "<tr><th>" << setting.first << " :</th><td>" << setting.second << "</td></tr>" << std::endl;
        }
        out << std::endl << "</tbody></table></div>";
    }
    catch (prg_exception& x) {
        x.addTrace("WriteSettingsContainerHTML()", "ParametersPrint");
        throw;
    }
}
