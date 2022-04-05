//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ParametersValidate.h"
#include "RandomNumberGenerator.h"
#include "Randomizer.h"
#include "ParametersPrint.h"
#include "SSException.h"
#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_int.hpp>
#include "ObservableRegion.h"
#include "TimeStamp.h"
#include "DateStringParser.h"

const char * ParametersValidate::MSG_INVALID_PARAM = "Invalid Parameter Setting";

/** Validates that given current state of settings, parameters and their relationships
    with other parameters are correct. Errors are sent to print direction and*/
bool ParametersValidate::Validate(BasePrint& PrintDirection, bool excludeFileValidation) const {
    bool bValid=true;

    try {
        //before version 3, there were no restrictions for secondary clusters
        if (gParameters.GetCreationVersion().iMajor < 3)
            const_cast<CParameters&>(gParameters).SetCriteriaForReportingSecondaryClusters(NORESTRICTIONS);

        // prevent unintended conflicts
        if (gParameters.GetAnalysisType() != PURELYSPATIAL) {
            // reporting gini optimized clusters is disabled for all analyses except purely spatial
            const_cast<CParameters&>(gParameters).setReportGiniOptimizedClusters(false);
        }

        if ((gParameters.GetProbabilityModelType() == ORDINAL || gParameters.GetProbabilityModelType() == CATEGORICAL)
            && gParameters.getNumFileSets() > 1 && gParameters.GetMultipleDataSetPurposeType() == ADJUSTMENT) {
            bValid = false;
            PrintDirection.Printf("%s:\nAdjustment purpose for multiple data sets is not permitted with %s model in this version of SaTScan.\n",
                                  BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
        }
        //validate model parameters
        if (gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION) {
            if (!(gParameters.GetAnalysisType() == SPACETIME || gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)) {
                bValid = false;
                PrintDirection.Printf("%s:\nFor the %s model, the analysis type must be either Retrospective or Prospective Space-Time.\n",
                                      BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
            }
        }
        const_cast<CParameters&>(gParameters).setProspectiveFrequency(std::max(gParameters.getProspectiveFrequency(), 1u));

        bValid &= ValidateMonotoneRisk(PrintDirection);
        bValid &= ValidateSVTTAnalysisSettings(PrintDirection);
        bValid &= ValidateExecutionTypeParameters(PrintDirection);
        bValid &= ValidateDateParameters(PrintDirection);
        bValid &= ValidateSpatialParameters(PrintDirection);
        bValid &= ValidateContinuousPoissonParameters(PrintDirection);
        if (!excludeFileValidation)
            bValid &= ValidateFileParameters(PrintDirection);
        bValid &= ValidateLocationNetworkParameters(PrintDirection);
        bValid &= ValidateOutputOptionParameters(PrintDirection);
        bValid &= ValidateLinelistParameters(PrintDirection);
        bValid &= ValidateRangeParameters(PrintDirection);
        bValid &= ValidateIterativeScanParameters(PrintDirection);
        bValid &= ValidateInferenceParameters(PrintDirection);
        bValid &= ValidateDrilldownParameters(PrintDirection);
        bValid &= ValidateBorderAnalysisParameters(PrintDirection);
        bValid &= ValidateEllipseParameters(PrintDirection);
        bValid &= ValidateSimulationDataParameters(PrintDirection);
        bValid &= ValidateRandomizationSeed(PrintDirection);
        bValid &= ValidatePowerEvaluationsParameters(PrintDirection);
        bValid &= ValidateClosedLoopAnalysisParameters(PrintDirection);
    } catch (prg_exception& x) {
        x.addTrace("ValidateParameters()","ParametersValidate");
        throw;
    }
    return bValid;
}

bool ParametersValidate::ValidateBorderAnalysisParameters(BasePrint& printDirection) const {
    bool bValid=true;

    if (gParameters.getCalculateOliveirasF()) {
        if (!(gParameters.GetAnalysisType() == PURELYSPATIAL && gParameters.GetProbabilityModelType() == POISSON)) {
            bValid = false;
            printDirection.Printf("%s:\nOliveira's F is only implemented for purely spatial analyses using the Poisson model.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
        if (gParameters.GetNumReplicationsRequested() < 99) {
            bValid = false;
            printDirection.Printf("%s:\nThe Oliveira's F calculation requires a minimum of 99 monte carlo replications.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
        if (gParameters.getNumRequestedOliveiraSets() < 100 || gParameters.getNumRequestedOliveiraSets() % 100 > 0) {
            bValid = false;
            printDirection.Printf("%s:\nThe number of bootstrap replications for Oliveira's F must be at least 100 and a multiple of 100.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
        if (gParameters.getOliveiraPvalueCutoff() < 0 || gParameters.getOliveiraPvalueCutoff() > 1) {
            bValid = false;
            printDirection.Printf("%s:\nThe Oliveira cutoff p-value of '%2g' is not a decimal value between 0 and 1.\n",
                                  BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, gParameters.getOliveiraPvalueCutoff());
        }
        if (gParameters.GetExecutionType() == CENTRICALLY) {
            bValid = false;
            printDirection.Printf("%s:\nOliveira's F is not implemented with the alternative memory allocation algorithm.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
        // If oliveira is requested, the 'Risk Estimates for Each Location' files must be selected -- this is the preferred format for reporting this information.
        if (!gParameters.GetOutputRelativeRisksFiles()) {
            const_cast<CParameters&>(gParameters).SetOutputRelativeRisksAscii(true);
            return true;
        }
        /* We're disabling the gini portion for the time being: https://www.squishlist.com/ims/satscan/66323/ */
        if (gParameters.getReportGiniOptimizedClusters()) {
            bValid = false;
            printDirection.Printf("%s:\nOliveira's F is not implemented with the gini optimized clusters option.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
        // only permit non-overlapping clusters when reporting hierarchical clusters with border analysis option
        if (gParameters.getReportHierarchicalClusters() && gParameters.GetCriteriaSecondClustersType() != NOGEOOVERLAP) {
            bValid = false;
            printDirection.Printf("%s:\nOliveira's F can be performed in conjunction with hierarchical clusters only when secondary clusters are not overlapping.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
    }
    return bValid;
}

bool ParametersValidate::ValidateClosedLoopAnalysisParameters(BasePrint& printDirection) const {
    bool bValid=true;

    if (gParameters.GetAnalysisType() == SEASONALTEMPORAL) {
        if (gParameters.GetPrecisionOfTimesType() == NONE || gParameters.GetPrecisionOfTimesType() == YEAR) {
            bValid = false;
            printDirection.Printf("%s:\nA closed loop analysis cannot be run with a time precision of none or year. Precision must be either month, day or generic.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
        if (gParameters.GetTimeAggregationUnitsType() == NONE || gParameters.GetTimeAggregationUnitsType() == YEAR) {
            bValid = false;
            printDirection.Printf("%s:\nA closed loop analysis cannot be run with a time aggregation of none or year. Time aggregation must be either month, day or generic.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
        if (gParameters.GetProbabilityModelType() == POISSON && gParameters.getAdjustForWeeklyTrends()) {
            bValid = false;
            printDirection.Printf("%s:\nA closed loop analysis is not implemented with the Poisson model using the adjustment for weekly trends.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
        if (gParameters.GetIncludeClustersType() != ALLCLUSTERS) {
            bValid = false;
            printDirection.Printf("%s:\nA closed loop analysis requires all temporal clusters to be evaluated.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
    }
    return bValid;
}

/** Validates date parameters based upon current settings. Error messages
    sent to print direction object and indication of valid settings returned. */
bool ParametersValidate::ValidateDateParameters(BasePrint& PrintDirection) const {
    bool   bValid=true, bStartDateValid=true, bEndDateValid=true, bProspectiveDateValid=true;
    Julian StudyPeriodStartDate, StudyPeriodEndDate, ProspectiveStartDate;

    // continuous Poisson does not use date parameters
    if (gParameters.GetProbabilityModelType() == HOMOGENEOUSPOISSON) return true;

    try {
        //validate study period start date based upon 'precision of times' parameter setting
        if (!ValidateStudyPeriodStartDate(PrintDirection)) {
            bValid = false;
            bStartDateValid = false;
        }
        //validate study period end date based upon precision of times parameter setting
        if (!ValidateStudyPeriodEndDate(PrintDirection)) {
            bValid = false;
            bEndDateValid = false;
        }
        //validate prospective start date based upon precision of times parameter setting
        if (gParameters.GetIsProspectiveAnalysis() && gParameters.GetAdjustForEarlierAnalyses() &&
            !ValidateDateString(PrintDirection, START_PROSP_SURV, gParameters.GetProspectiveStartDate())) {
            bValid = false;
            bProspectiveDateValid = false;
        }

        if (bStartDateValid && bEndDateValid) {
            //check that study period start and end dates are chronologically correct
            StudyPeriodStartDate = DateStringParser::getDateAsJulian(gParameters.GetStudyPeriodStartDate().c_str(), gParameters.GetPrecisionOfTimesType());
            StudyPeriodEndDate = DateStringParser::getDateAsJulian(gParameters.GetStudyPeriodEndDate().c_str(), gParameters.GetPrecisionOfTimesType());
            if (StudyPeriodStartDate > StudyPeriodEndDate) {
                bValid = false;
                PrintDirection.Printf("%s:\nThe study period start date occurs after the end date.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
            }
            if (bValid && gParameters.GetIsProspectiveAnalysis() && gParameters.GetAdjustForEarlierAnalyses() && bProspectiveDateValid) {
                //validate prospective start date
                ProspectiveStartDate = DateStringParser::getDateAsJulian(gParameters.GetProspectiveStartDate().c_str(), gParameters.GetPrecisionOfTimesType());
                if (ProspectiveStartDate < StudyPeriodStartDate || ProspectiveStartDate > StudyPeriodEndDate) {
                    bValid = false;
                    PrintDirection.Printf("%s:\nThe start date of prospective surveillance does not occur within the study period.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
                }
            }

            if (bValid && gParameters.GetAnalysisType() == SEASONALTEMPORAL &&
                (gParameters.GetPrecisionOfTimesType() == DAY || gParameters.GetPrecisionOfTimesType() == MONTH) &&
                (StudyPeriodEndDate - StudyPeriodStartDate + 1) < 365) {
                bValid = false;
                PrintDirection.Printf("%s:\nThe study period length must be at least one year for the Seasonal analysis.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
            }

        }
    } catch (prg_exception& x) {
        x.addTrace("ValidateDateParameters()","ParametersValidate");
        throw;
    }
    // Validate temporal options only if date parameters are valid. Some temporal parameters can not be correctly validated if dates are not valid.
    return bValid ? ValidateTemporalParameters(PrintDirection) : false;
}

/** Validates that date parameter string is in correct format. */
bool ParametersValidate::ValidateDateString(BasePrint& PrintDirection, ParameterType eParameterType, const std::string& value) const {
    bool          bValid=true;
    const char  * sName = 0;
    UInt          nYear, nMonth, nDay;

    try {
        // determine label for date field - in the event that we have to throw an error
        switch (eParameterType) {
            case STARTDATE                : sName = "study period start date"; break;
            case ENDDATE                  : sName = "study period end date"; break;
            case START_PROSP_SURV         : sName = "prospective surveillance start date"; break;
            case INTERVAL_STARTRANGE      : sName = "flexible temporal window definition start range date"; break;
            case INTERVAL_ENDRANGE        : sName = "flexible temporal window definition end range date"; break;
            default : throw prg_error("Unknown parameter enumeration %d.","ValidateDateString()", eParameterType);
        }

        // validate date field conditionally based upon precision of times specification
        if (gParameters.GetPrecisionOfTimesType() == GENERIC) {
            try {
                relativeDateToJulian(value.c_str());
            } catch (resolvable_error& r) {
                PrintDirection.Printf("%s:\n%s", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, r.what());
                return false;
            }
        } else {
            //parse date in parts
            if (CharToMDY(&nMonth, &nDay, &nYear, value.c_str()) != 3) {
                PrintDirection.Printf("%s:\nThe %s '%s' is not valid. Please specify as YYYY/MM/DD.\n",
                                      BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, sName, value.c_str());
                return false;
            }
            //validate date
            if (!IsDateValid(nMonth, nDay, nYear)) {
                PrintDirection.Printf("%s:\nThe %s '%s' is not a valid date.\n",
                                      BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, sName, value.c_str());
                return false;
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("ValidateDateParameters()","ParametersValidate");
        throw;
    }
    return bValid;
}

bool ParametersValidate::ValidateDrilldownParameters(BasePrint & PrintDirection) const {
    bool bValid = true;

    if (!(gParameters.getPerformStandardDrilldown() || gParameters.getPerformBernoulliDrilldown()))
        return bValid;

    if (gParameters.getPerformStandardDrilldown() && !(gParameters.GetIsPurelySpatialAnalysis() || gParameters.GetIsSpaceTimeAnalysis() || gParameters.GetAnalysisType() == SPATIALVARTEMPTREND)) {
        bValid = false;
        PrintDirection.Printf("%s:\nThe standard cluster drilldown is not implemented for %s analysis.\n",
            BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, ParametersPrint(gParameters).GetAnalysisTypeAsString());
        return bValid;
    }
    if (gParameters.getPerformBernoulliDrilldown() && 
        !(gParameters.GetIsSpaceTimeAnalysis() && (gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION || gParameters.GetProbabilityModelType() == POISSON))) {
        bValid = false;
        PrintDirection.Printf("%s:\nThe Bernoulli cluster drilldown is only implemented for space-time permutation and Poisson space-time analyses.\n",
            BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, ParametersPrint(gParameters).GetAnalysisTypeAsString());
        return bValid;
    }
    if (gParameters.UseLocationNeighborsFile() && gParameters.UseMetaLocationsFile()) {
        bValid = false;
        PrintDirection.Printf(
            "%s:\nThe cluster drilldown is not implemented for analyses using the Non-Euclidean Neighbors file in conjunction with the Meta Location file.\n", 
            BasePrint::P_PARAMERROR, MSG_INVALID_PARAM
        );
    }
    if (gParameters.getDrilldownMinimumLocationsCluster() < 2) {
        bValid = false;
        PrintDirection.Printf("%s:\nThe minimum number of locations in detected cluster for drilldown cannot be less than 2.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
    }
    if (gParameters.getDrilldownMinimumCasesCluster() < 10) {
        bValid = false;
        PrintDirection.Printf("%s:\nThe minimum number of cases in detected cluster for drilldown cannot be less than 10.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
    }
    double cutoff = gParameters.getDrilldownPvalueCutoff();
    if (cutoff <= 0.0 || 1.0 <= cutoff) {
        bValid = false;
        PrintDirection.Printf("%s:\nThe p-value cutoff for a detected cluster on drilldown must be greater than zero and less than one.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
    }
    if (gParameters.getPerformBernoulliDrilldown() && gParameters.getDrilldownAdjustWeeklyTrends()) {
        if (gParameters.getNumFileSets() > 1) {
            bValid = false;
            PrintDirection.Printf("%s:\nThe adjustment for weekly trends, with the purely spatial Beroulli drilldown, cannot be performed with multiple data sets.\n",
                BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
        if (!(gParameters.GetTimeAggregationUnitsType() == DAY && gParameters.GetTimeAggregationLength() == 1)) {
            bValid = false;
            PrintDirection.Printf("%s:\nThe adjustment for weekly trends, in the purely spatial Beroulli drilldown, can only be performed with a time aggregation length of 1 day.\n",
                BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
        double dStudyPeriodLengthInUnits = ceil(
            CalculateNumberOfTimeIntervals(DateStringParser::getDateAsJulian(gParameters.GetStudyPeriodStartDate().c_str(), gParameters.GetPrecisionOfTimesType()),
            DateStringParser::getDateAsJulian(gParameters.GetStudyPeriodEndDate().c_str(), gParameters.GetPrecisionOfTimesType()), gParameters.GetTimeAggregationUnitsType(), 1)
        );
        // Primary analysis must adhere to study period restriction length.
        if (gParameters.GetTimeAggregationUnitsType() == DAY && dStudyPeriodLengthInUnits < 14.0) {
            PrintDirection.Printf("%s:\nThe adjustment for day of week, in the purely spatial Beroulli drilldown, cannot be performed on a period less than 14 days.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
            return false;
        }
    }

    return bValid;
}

/** Validates ellipse parameters if number of ellipses greater than zero.
    Errors printed to print direction and return whether parameters are valid. */
bool ParametersValidate::ValidateEllipseParameters(BasePrint & PrintDirection) const {
  bool          bValid=true;
  size_t        t;

  try {
    if (gParameters.GetIsPurelyTemporalAnalysis() || gParameters.UseLocationNeighborsFile() || gParameters.GetSpatialWindowType() != ELLIPTIC) return true;
    if (gParameters.GetSpatialWindowType() == ELLIPTIC && gParameters.getUseLocationsNetworkFile()) {
        bValid = false;
        PrintDirection.Printf("%s:\nThe option to use elliptic spatial windows cannot be used in conjuction with the locations network file.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
    }
    if (gParameters.GetNumRequestedEllipses() < 1 || gParameters.GetNumRequestedEllipses() > CParameters::MAXIMUM_ELLIPSOIDS) {
      bValid = false;
      PrintDirection.Printf("%s:\nThe number of requested ellipses '%d' is not within allowable range of 1 - %d.\n",
                            BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, gParameters.GetNumRequestedEllipses(), CParameters::MAXIMUM_ELLIPSOIDS);
    }
    //analyses with ellipses can not be performed with coordinates defiend in latitude/longitude system (currently)
    if (gParameters.GetCoordinatesType() == LATLON) {
      bValid = false;
      PrintDirection.Printf("%s:\nSaTScan does not support lat/long coordinates when ellipses are used. "
                            "Please use the Cartesian coordinate system if possible.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
    }
    if (gParameters.GetEllipseShapes().size() != gParameters.GetEllipseRotations().size()) {
      bValid = false;
      PrintDirection.Printf("%s:\nSettings indicate %i elliptic shapes but %i variables for respective angles of rotation.\n",
                            BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, gParameters.GetEllipseShapes().size(), gParameters.GetEllipseRotations().size());
    }
    for (t=0; t < gParameters.GetEllipseShapes().size(); ++t)
       if (gParameters.GetEllipseShapes()[t] < 1) {
         bValid = false;
         PrintDirection.Printf("%s:\nEllipse shape '%g' is invalid. The shape can not be less than one.\n",
                               BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, gParameters.GetEllipseShapes()[t]);
       }
    for (t=0; t < gParameters.GetEllipseRotations().size(); t++)
       if (gParameters.GetEllipseRotations()[t] < 1) {
         bValid = false;
         PrintDirection.Printf("%s:\nThe number of angles, '%d', is invalid. The number can not be less than one.\n",
                               BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, gParameters.GetEllipseRotations()[t]);
       }
    if (gParameters.GetExecutionType() == CENTRICALLY && gParameters.GetNonCompactnessPenaltyType() > NOPENALTY) {
      bValid = false;
      PrintDirection.Printf("%s:\nThe non-compactness penalty for elliptic scans can not be applied "
                            "with the centric analysis execution.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("ValidateEllipseParameters()","ParametersValidate");
    throw;
  }
  return bValid;
}

/** Validates execution type parameters. */
bool ParametersValidate::ValidateExecutionTypeParameters(BasePrint & PrintDirection) const {
   bool          bValid=true;

   try {
     if (gParameters.GetExecutionType() == CENTRICALLY) {
        if (gParameters.GetPValueReportingType() == TERMINATION_PVALUE && gParameters.GetNumReplicationsRequested() >= MIN_SIMULATION_RPT_PVALUE) {
           bValid = false;
           PrintDirection.Printf("%s:\nThe sequential Monte Carlo option can not be applied with the alternative memory allocation.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
        if (gParameters.GetIsPurelyTemporalAnalysis() ||
            (gParameters.GetAnalysisType() == PURELYSPATIAL && gParameters.GetRiskType() == MONOTONERISK) ||
             gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
           bValid = false;
           PrintDirection.Printf("%s:\nThe alternative memory allocation is not available for:\n"
                                 " purely temporal analyses\n purely spatial analyses with isotonic scan\n"
                                 " spatial variation in temporal trends analysis\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
        if (gParameters.UseLocationNeighborsFile()) {
           bValid = false;
           PrintDirection.Printf("%s:\nThe alternative memory allocation is not implemented with the non-Eucludian neighbors file.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
        if (gParameters.GetMultipleCoordinatesType() != ONEPERLOCATION) {
           bValid = false;
           PrintDirection.Printf("%s:\nThe alternative memory allocation is not implemented with the multiple coordinates per location id feature.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
     }
  } catch (prg_exception& x) {
    x.addTrace("ValidateExecutionTypeParameters()","ParametersValidate");
    throw;
  }
  return bValid;
}

/**/
bool ParametersValidate::checkFileExists(const std::string& filename, const std::string& filetype, BasePrint& PrintDirection, bool writeCheck) const {
    // Trim whitespace and apply any time formats.
    std::string buffer = getFilenameFormatTime(filename, gParameters.getTimestamp(), true);
    trimString(buffer);
    if (buffer.empty()) {
        PrintDirection.Printf("%s:\nThe %s file could not be opened. No filename was specified.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, filetype.c_str());
        return false;
    } else if (!ValidateFileAccess(buffer, writeCheck)) {
        PrintDirection.Printf("%s:\nThe %s file '%s' could not be opened for %s. "
                              "Please confirm that the path and/or file name are valid and that you "
                              "have permissions to %s from this directory and file.\n",
                              BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, filetype.c_str(), buffer.c_str(), 
                              (writeCheck ? "writing": "reading"), (writeCheck ? "write": "read"));
        return false;
    }
    return true;
}

bool ParametersValidate::ValidateInputSource(const CParameters::InputSource * source, const std::string& filename, const std::string& verbosename, BasePrint& PrintDirection) const {
    FileName file(filename.c_str());

    std::string extension(file.getExtension());
    lowerString(extension);
    // First exclude file types that are not readable - namely, Excel;
    if (extension == ".xls" || extension == ".xlsx") {
        PrintDirection.Printf("%s:\nThe Excel file '%s' cannot be read as an input file.\n.SaTScan cannot read directly from Excel files.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, filename.c_str());
        return false;
    }

    // If file type is dBase or shapefile, then require an input source setting which defines how to read the file.
    if ((extension == ".dbf" || extension == ".shp") && !source) {
        PrintDirection.Printf("%s:\nThe file '%s' cannot be read as an input source. Both dBase files (.dbf) and shapefiles (.shp) require an input source mapping definition.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, filename.c_str());
        return false;
    }

    if (source) {
        // Verify that the input source settings's source data file type matches extension.
        bool correct_filetype=true;
        switch (source->getSourceType()) {
            case CSV : {
                FieldMapContainer_t::const_iterator itrMap=source->getFieldsMap().begin();
                for (;itrMap != source->getFieldsMap().end(); ++itrMap) {
                     if (itrMap->type() == typeid(long) && boost::any_cast<long>(*itrMap) < 0) {
                        PrintDirection.Printf("%s:\nThe field mapping column indexes cannot be unless than zero, got value %ld.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, boost::any_cast<long>(*itrMap));
                        return false;
                     }
                }
                correct_filetype = !(extension == ".dbf" || extension == ".shp" || extension == ".xls"); break;
            }
            case DBASE : correct_filetype = extension == ".dbf"; break;
            case SHAPE : correct_filetype = extension == ".shp"; break;
            case EXCEL : correct_filetype = extension == ".xls" || extension == ".xlsx"; break;
            default : throw prg_error("Unknown  source type: %d.", "ValidateInputSource()", source->getSourceType());
        }
        if (!correct_filetype) {
            PrintDirection.Printf("%s:\nThe file '%s' cannot be read as an input source. The specified source type does not match the file type.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, filename.c_str());
            return false;
        }
    } return true;
}

/** Validates input/output file parameters. */
bool ParametersValidate::ValidateFileParameters(BasePrint& PrintDirection) const {
  bool          bValid=true;
  size_t        t;

  try {
    // case file is ignored when using the continuous Poisson model or power evaluations with specified total cases
    if (!(gParameters.GetProbabilityModelType() == HOMOGENEOUSPOISSON || (gParameters.getPerformPowerEvaluation() && gParameters.getPowerEvaluationMethod() == PE_ONLY_SPECIFIED_CASES))) {
        //validate case file
        if (!gParameters.GetCaseFileNames().size()) {
            bValid = false;
            PrintDirection.Printf("%s:\nNo case file was specified.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
        for (t=0; t < gParameters.GetCaseFileNames().size(); ++t) {
            bool exists = checkFileExists(gParameters.GetCaseFileNames()[t], "case", PrintDirection);
            bValid &= exists;
            if (exists) {
                bValid &= ValidateInputSource(gParameters.getInputSource(CASEFILE, t+1), gParameters.GetCaseFileNames()[t], "case", PrintDirection);
            }
        }
    }
    //validate population file for a poisson model.
    if (gParameters.GetProbabilityModelType() == POISSON) {
      //special processing for purely temporal analyses - population file is optional
      if (gParameters.GetIsPurelyTemporalAnalysis()) {
         //either all datasets omit the population file or specify a filename
         unsigned int iNumDataSetsWithoutPopFile=0;
         for (t=0; t < gParameters.GetPopulationFileNames().size(); ++t)
            if (gParameters.GetPopulationFileNames()[0].empty())
              ++iNumDataSetsWithoutPopFile;
         if (iNumDataSetsWithoutPopFile && iNumDataSetsWithoutPopFile != gParameters.GetPopulationFileNames().size()) {
            bValid = false;
            PrintDirection.Printf("%s:\nFor the Poisson model with purely temporal analyses, the population file "
                                  "is optional but all data sets must either specify a population file or omit it.\n",
                                  BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
         }
         else if (!iNumDataSetsWithoutPopFile) {
           const_cast<CParameters&>(gParameters).SetPopulationFile(true);
           for (t=0; t < gParameters.GetPopulationFileNames().size(); ++t) {
              bool exists = checkFileExists(gParameters.GetPopulationFileNames()[t], "population", PrintDirection);
              bValid &= exists;
              if (exists) {
                bValid &= ValidateInputSource(gParameters.getInputSource(POPFILE, t+1), gParameters.GetPopulationFileNames()[t], "population", PrintDirection);
              }
           }
         }
      }
      else {
        const_cast<CParameters&>(gParameters).SetPopulationFile(true);
        if (!gParameters.GetPopulationFileNames().size()) {
          bValid = false;
          PrintDirection.Printf("%s:\nFor the Poisson model, a population file must be specified unless analysis "
                                "is purely temporal. In which case the population file is optional.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
        for (t=0; t < gParameters.GetPopulationFileNames().size(); ++t) {
            bool exists = checkFileExists(gParameters.GetPopulationFileNames()[t], "population", PrintDirection);
            bValid &= exists;
            if (exists) {
              bValid &= ValidateInputSource(gParameters.getInputSource(POPFILE, t+1), gParameters.GetPopulationFileNames()[t], "population", PrintDirection);
            }
        }
      }
    }
    //validate control file for a bernoulli model.
    if (gParameters.GetProbabilityModelType() == BERNOULLI) {
      if (!gParameters.GetControlFileNames().size()) {
        PrintDirection.Printf("%s:\nFor the Bernoulli model, a control file must be specified.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        bValid = false;
      } else {
        for (t=0; t < gParameters.GetControlFileNames().size(); ++t) {
            bool exists = checkFileExists(gParameters.GetControlFileNames()[t], "control", PrintDirection);
            bValid &= exists;
            if (exists) {
              bValid &= ValidateInputSource(gParameters.getInputSource(CONTROLFILE, t+1), gParameters.GetControlFileNames()[t], "control", PrintDirection);
            }
        }
      }
    }
    //validate coordinates file
    if (gParameters.UseCoordinatesFile()) {
        bool exists = checkFileExists(gParameters.GetCoordinatesFileName(), "coordinates", PrintDirection);
        bValid &= exists;
        if (exists) {
            bValid &= ValidateInputSource(gParameters.getInputSource(COORDFILE), gParameters.GetCoordinatesFileName(), "coordinates", PrintDirection);
        }
    }
    //validate special grid file
    if (gParameters.GetIsPurelyTemporalAnalysis() || gParameters.UseLocationNeighborsFile() || gParameters.getUseLocationsNetworkFile())
      const_cast<CParameters&>(gParameters).SetUseSpecialGrid(false);
    else if (gParameters.UseSpecialGrid()) {
        bool exists = checkFileExists(gParameters.GetSpecialGridFileName(), "grid", PrintDirection);
        bValid &= exists;
        if (exists) {
            bValid &= ValidateInputSource(gParameters.getInputSource(GRIDFILE), gParameters.GetSpecialGridFileName(), "grid", PrintDirection);
        }
    }
    // validate adjustment for known relative risks file
    if (gParameters.UseAdjustmentForRelativeRisksFile()) {
        bool exists = checkFileExists(gParameters.GetAdjustmentsByRelativeRisksFilename(), "adjustments", PrintDirection);
        bValid &= exists;
        if (exists) {
            bValid &= ValidateInputSource(gParameters.getInputSource(ADJ_BY_RR_FILE), gParameters.GetAdjustmentsByRelativeRisksFilename(), "adjustments", PrintDirection);
        }
    } 
    //validate maximum circle population file for a prospective space-time analysis w/ maximum geographical cluster size
    //defined as a percentage of the population and adjusting for earlier analyses.
    if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME && gParameters.GetAdjustForEarlierAnalyses()) {
      if (gParameters.UseLocationNeighborsFile() && !gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFPOPULATION, false)) {
        bValid = false;
        PrintDirection.Printf("%s:\nFor a prospective space-time analysis adjusting for ealier analyses and defining "
                              "non-eucledian neighbors, the maximum spatial cluster size must be defined as a "
                              "percentage of the population as defined in a max circle size file.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
      }
      else if (!gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false) && !gParameters.GetRestrictMaxSpatialSizeForType(MAXDISTANCE, false)) {
        bValid = false;
        PrintDirection.Printf("%s:\nFor a prospective space-time analysis adjusting for ealier analyses, the maximum spatial "
                              "cluster size must be defined as a percentage of the population as defined in a max "
                              "circle size file. Alternatively you may choose to specify the maximum as a fixed radius, "
                              "in which case a max circle size file is not required.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
      }
    }
    if (gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false) ||
        (gParameters.GetRestrictingMaximumReportedGeoClusterSize() && gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true))) {
      if (gParameters.GetMaxCirclePopulationFileName().empty() && gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false)) {
        bValid = false;
        PrintDirection.Printf("%s:\nThe settings indicate to the use a max circle size file, but a file name was not specified.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
      } else if (gParameters.GetMaxCirclePopulationFileName().empty() &&
               gParameters.GetRestrictingMaximumReportedGeoClusterSize() && gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true)) {
        bValid = false;
        PrintDirection.Printf("%s:\nMaximum circle size file name was not specified. A maximum circle file is required "
                              "when restricting the maximum reported spatial cluster size by a population defined "
                              "in that maximum circle file.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
      } else {
          bool exists = checkFileExists(gParameters.GetMaxCirclePopulationFileName(), "max circle size", PrintDirection);
          bValid &= exists;
          if (exists) {
            bValid &= ValidateInputSource(gParameters.getInputSource(MAXGEOPOPFILE), gParameters.GetMaxCirclePopulationFileName(), "max circle size", PrintDirection);
          }
      }
    }
    //validate neighbor array file
    if (gParameters.UseLocationNeighborsFile()) {
       bValid &= checkFileExists(gParameters.GetLocationNeighborsFileName(), "location neighbors", PrintDirection);
    }
    //validate meta locations file
    if (gParameters.UseMetaLocationsFile()) {
       bValid &= checkFileExists(gParameters.getMetaLocationsFilename(), "meta locations", PrintDirection);
    }
    //validate locations network file
    if (gParameters.getUseLocationsNetworkFile()) {
        bValid &= checkFileExists(gParameters.getLocationsNetworkFilename(), "locations network", PrintDirection);
    }
    //validate output file
    bValid &= checkFileExists(gParameters.GetOutputFileName(), "results", PrintDirection, true);
  } catch (prg_exception& x) {
    x.addTrace("ValidateFileParameters()","ParametersValidate");
    throw;
  }
  return bValid;
}

/** Validates inference parameters.
    Prints errors to print direction and returns whether values are valid.*/
bool ParametersValidate::ValidateInferenceParameters(BasePrint & PrintDirection) const {
    if (gParameters.getPerformPowerEvaluation()) return true;

    bool bValid=true;

    try {
        if (gParameters.GetNumReplicationsRequested() > 0 && gParameters.GetPValueReportingType() == TERMINATION_PVALUE &&
            (gParameters.GetEarlyTermThreshold() < 1 || gParameters.GetEarlyTermThreshold() > gParameters.GetNumReplicationsRequested())) {
            bValid = false;
            PrintDirection.Printf("%s:\nThe threshold for early termination of simulations must be from 1 to "
                                  "number of replications.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }

        /* Validate any restrictions on clusters by relative risk. */
        if (gParameters.getRiskLimitHighClusters() || gParameters.getRiskLimitLowClusters()) {
            // Validate the risk thresholds by probability model.
            switch (gParameters.GetProbabilityModelType()) {
            case EXPONENTIAL:
            case SPACETIMEPERMUTATION:
            case POISSON:
            case HOMOGENEOUSPOISSON:
            case BERNOULLI:
            case UNIFORMTIME:
                if ((gParameters.GetAreaScanRateType() == HIGH || gParameters.GetAreaScanRateType() == HIGHANDLOW) && gParameters.getRiskLimitHighClusters()) {
                    if (gParameters.getRiskThresholdHighClusters() < 1.0) {
                        bValid = false;
                        PrintDirection.Printf("%s:\nThe risk threshold for %s clusters more be greater than or equal to 1.0.\n",
                                              BasePrint::P_PARAMERROR, MSG_INVALID_PARAM,
                                              (gParameters.GetProbabilityModelType() == EXPONENTIAL ? "short survival" : "high rate"));
                    }
                }
                if ((gParameters.GetAreaScanRateType() == LOW || gParameters.GetAreaScanRateType() == HIGHANDLOW) && gParameters.getRiskLimitLowClusters()) {
                    if (gParameters.getRiskThresholdLowClusters() < 0.0 || gParameters.getRiskThresholdLowClusters() > 1.0) {
                        bValid = false;
                        PrintDirection.Printf("%s:\nThe risk threshold for %s clusters more be greater than or equal to 0 and less than or equal to 1.0.\n",
                                              BasePrint::P_PARAMERROR, MSG_INVALID_PARAM,
                                              (gParameters.GetProbabilityModelType() == EXPONENTIAL ? "long survival" : "low rate"));
                    }

                } break;
            default:
                bValid = false;
                PrintDirection.Printf("%s:\nThe option to limit clusters by risk level is not implemented for the %s model.\n",
                    BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
            }
            // During development of version 10, Martin and I were unable able to determine how this should work with multiple data sets.
            // As a matter of fact, Martin was questioning the formulas for relative risk and LLR but other pressing matters made us put this down.
            if (gParameters.GetProbabilityModelType() == UNIFORMTIME && gParameters.getNumFileSets() > 1) {
                bValid = false;
                PrintDirection.Printf("%s:\nThe option to limit clusters by risk level is not implemented for the %s model and multiple data sets.\n",
                    BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
            }
        }
        /* Validate the settings for minimum number of cases in low rate and high rate clusters. This feature isn't available for ordinal / multinomial models. */
        if (!(gParameters.GetProbabilityModelType() == ORDINAL || gParameters.GetProbabilityModelType() == CATEGORICAL)) {
            /* Validate the minimum number of cases in low rate clusters */
            if (gParameters.GetProbabilityModelType() == NORMAL && gParameters.getMinimumCasesLowRateClusters() < 2 && (gParameters.GetAreaScanRateType() == LOW || gParameters.GetAreaScanRateType() == HIGHANDLOW)) {
                /* Since this settings is no present in GUI, just default to minimum value for normal model. */
                const_cast<CParameters&>(gParameters).setMinimumCasesLowRateClusters(2);
            }
            /* Validate minimum number of cases for high rate scans. */
            if (gParameters.getMinimumCasesHighRateClusters() < 2 && (gParameters.GetAreaScanRateType() == HIGH || gParameters.GetAreaScanRateType() == HIGHANDLOW)) {
                bValid = false;
                PrintDirection.Printf("%s:\nThe option to specify the minimum number of cases, when scanning for high rate clusters, cannot be less than 2.\n",
                    BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
            }

        } else {
            /* Default the values when these settings are not used -- the AbstractLikelihoodCalculator methods still reference the values in places. */
            const_cast<CParameters&>(gParameters).setMinimumCasesLowRateClusters(gParameters.GetProbabilityModelType() == NORMAL ? 2: 0);
            const_cast<CParameters&>(gParameters).setMinimumCasesHighRateClusters(2);
        }
    } catch (prg_exception& x) {
        x.addTrace("ValidateInferenceParameters()","ParametersValidate");
        throw;
    }
    return bValid;
}

/** Validates parameters used in optional iterative scan feature.
    Prints errors to print direction and returns whether values are vaild.*/
bool ParametersValidate::ValidateIterativeScanParameters(BasePrint & PrintDirection) const {
  if (gParameters.getPerformPowerEvaluation()) return true;

  bool  bValid=true;

  try {
    if (gParameters.GetIsIterativeScanning()) {
      if (gParameters.GetSimulationType() == FILESOURCE) {
        PrintDirection.Printf("%s:\nThe iterative scan feature can not be combined with the feature to read simulation data from file.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        return false;
      }
      if (gParameters.GetOutputSimulationData()) {
        PrintDirection.Printf("%s:\nThe iterative scan feature can not be combined with the feature to write simulation data to file.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        return false;
      }
      if (!(gParameters.GetAnalysisType() == PURELYSPATIAL || gParameters.GetAnalysisType() == SPATIALVARTEMPTREND || gParameters.GetIsPurelyTemporalAnalysis())) {
        PrintDirection.Printf("%s:\nThe iterative scan option is not implemented for the %s analysis.\n",
                              BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, ParametersPrint(gParameters).GetAnalysisTypeAsString());
        return false;
      }
      if (!(gParameters.GetProbabilityModelType() == POISSON || gParameters.GetProbabilityModelType() == BERNOULLI ||
            gParameters.GetProbabilityModelType() == ORDINAL || gParameters.GetProbabilityModelType() == CATEGORICAL ||
            gParameters.GetProbabilityModelType() == NORMAL || gParameters.GetProbabilityModelType() == EXPONENTIAL)) {
        PrintDirection.Printf("%s:\nThe iterative scan feature is not implemented for %s model.\n",
                              BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
        return false;
      }
      if (gParameters.GetNumIterativeScansRequested() > static_cast<unsigned int>(CParameters::MAXIMUM_ITERATIVE_ANALYSES)) {
        bValid = false;
        PrintDirection.Printf("%s:\n%d exceeds the maximum number of iterative analyses allowed (%d).\n",
                              BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, gParameters.GetNumIterativeScansRequested(), CParameters::MAXIMUM_ITERATIVE_ANALYSES);
      }
      if (gParameters.GetIterativeCutOffPValue() < 0 || gParameters.GetIterativeCutOffPValue() > 1) {
        bValid = false;
        PrintDirection.Printf("%s:\nThe iterative scan analysis cutoff p-value of '%2g' is not a decimal value between 0 and 1.\n",
                              BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, gParameters.GetIterativeCutOffPValue());
      }
    }
  } catch (prg_exception& x) {
    x.addTrace("ValidateIterativeScanParameters()","ParametersValidate");
    throw;
  }
  return bValid;
}

/* Validate line list parameters. */
bool ParametersValidate::ValidateLinelistParameters(BasePrint& PrintDirection) const {
    try {
        if (gParameters.getOutputKMLFile() && gParameters.getGroupLinelistEventsKML()) {
            if (gParameters.getKmlEventGroupAttribute().size() == 0) {
                PrintDirection.Printf(
                    "%s:\nThe option to output a KML file has been selected along with line list event grouping\n"
                    "yet no grouping characteristic has been specified.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM
                );
                return false;
            }
        }
        if (!gParameters.getEventCacheFileName().empty()) {
            if (boost::filesystem::exists(gParameters.getEventCacheFileName().c_str()) && 
                !checkFileExists(gParameters.getEventCacheFileName(), "event cache", PrintDirection, false))
                return false;
        }
    } catch (prg_exception& x) {
        x.addTrace("ValidateLinelistParameters()", "ParametersValidate");
        throw;
    }
    return true;
}

/** Validates the temporal cluster size parameters. */
bool ParametersValidate::ValidateTemporalClusterSize(BasePrint& PrintDirection) const {
  std::string   sPrecisionString;
  double        dStudyPeriodLengthInUnits, dMaxTemporalLengthInUnits;

  try {
    //Maximum temporal cluster size parameters not used for these analyses.
    if (gParameters.GetAnalysisType() == PURELYSPATIAL || gParameters.GetAnalysisType() == SPATIALVARTEMPTREND)
      return true;

    if (gParameters.GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE) {
      //validate for maximum specified as percentage of study period
      if (gParameters.GetMaximumTemporalClusterSize() <= 0) {
        PrintDirection.Printf("%s:\nThe maximum temporal cluster size of '%g' is invalid. "
                              "Specifying the maximum as a percentage of the study period "
                              "requires the value to be a decimal number that is greater than zero.\n",
                              BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, gParameters.GetMaximumTemporalClusterSize());
        return false;
      }
      //check maximum temporal cluster size(as percentage of population) is less than maximum for given probability model
      if (gParameters.GetMaximumTemporalClusterSize() > 100.0/*(gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION ? 50 : 90)*/) {
        PrintDirection.Printf("%s:\nFor the %s model, the maximum temporal cluster size as a percent "
                              "of the study period is %d percent.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, 
                              ParametersPrint(gParameters).GetProbabilityModelTypeAsString(),
                              100/*(gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION ? 50 : 90)*/);
        return false;
      }

      //validate the time aggregation length agree with the study period and maximum temporal cluster size
      dStudyPeriodLengthInUnits = std::ceil(CalculateNumberOfTimeIntervals(DateStringParser::getDateAsJulian(gParameters.GetStudyPeriodStartDate().c_str(), gParameters.GetPrecisionOfTimesType()),
                                                                           DateStringParser::getDateAsJulian(gParameters.GetStudyPeriodEndDate().c_str(), gParameters.GetPrecisionOfTimesType()),
                                                                           gParameters.GetTimeAggregationUnitsType(), 1));
      dMaxTemporalLengthInUnits = std::floor(dStudyPeriodLengthInUnits * gParameters.GetMaximumTemporalClusterSize() / 100.0);
      if (dMaxTemporalLengthInUnits < 1) {
          GetDatePrecisionAsString(gParameters.GetTimeAggregationUnitsType(), sPrecisionString, false, false);
          PrintDirection.Printf("%s:\nA maximum temporal cluster size as %g percent of a %d %s study period results in a maximum "
                                "temporal cluster size that is less than one time aggregation %s.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM,
                                gParameters.GetMaximumTemporalClusterSize(), static_cast<int>(dStudyPeriodLengthInUnits),
                                sPrecisionString.c_str(), sPrecisionString.c_str());
          return false;
      }
    } else if (gParameters.GetMaximumTemporalClusterSizeType() == TIMETYPE) {
      //validate for maximum specified as time aggregation unit
      if (gParameters.GetMaximumTemporalClusterSize() < 1) {
        PrintDirection.Printf("%s:\nThe maximum temporal cluster size of '%2g' is invalid. "
                              "Specifying the maximum in time aggregation units requires "
                              "the value to be a whole number that is greater than zero.\n",
                              BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, gParameters.GetMaximumTemporalClusterSize());
        return false;
      }
      GetDatePrecisionAsString(gParameters.GetTimeAggregationUnitsType(), sPrecisionString, false, false);
      dStudyPeriodLengthInUnits = std::ceil(CalculateNumberOfTimeIntervals(DateStringParser::getDateAsJulian(gParameters.GetStudyPeriodStartDate().c_str(), gParameters.GetPrecisionOfTimesType()),
                                                                           DateStringParser::getDateAsJulian(gParameters.GetStudyPeriodEndDate().c_str(), gParameters.GetPrecisionOfTimesType()),
                                                                           gParameters.GetTimeAggregationUnitsType(), 1));
      double maximum = (gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION || gParameters.GetAnalysisType() == SEASONALTEMPORAL ? 50.0 : 90.0);
      dMaxTemporalLengthInUnits = std::floor(dStudyPeriodLengthInUnits * maximum /100.0);
      if (gParameters.GetMaximumTemporalClusterSize() > dMaxTemporalLengthInUnits) {
        PrintDirection.Printf("%s:\nA maximum temporal cluster size of %d %s%s exceeds %d percent of a %d %s study period. "
                              "Note that current settings limit the maximum to %d %s%s.\n",
                              BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, static_cast<int>(gParameters.GetMaximumTemporalClusterSize()),
                              sPrecisionString.c_str(), (gParameters.GetMaximumTemporalClusterSize() == 1 ? "" : "s"), static_cast<int>(maximum),
                              static_cast<int>(dStudyPeriodLengthInUnits), sPrecisionString.c_str(),
                              static_cast<int>(dMaxTemporalLengthInUnits), sPrecisionString.c_str(),
                              (dMaxTemporalLengthInUnits == 1 ? "" : "s"));
        return false;
      }
      dMaxTemporalLengthInUnits = gParameters.GetMaximumTemporalClusterSize();
    } else
      throw prg_error("Unknown temporal percentage type: %d.", "ValidateTemporalClusterSize()", gParameters.GetMaximumTemporalClusterSizeType());

    if (gParameters.getMinimumTemporalClusterSize() < 1) {
        GetDatePrecisionAsString(gParameters.GetTimeAggregationUnitsType(), sPrecisionString, false, false);
        PrintDirection.Printf("%s:\nThe minimum temporal cluster size is 1 %s when time aggregating to %ss.\n",
                              BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, sPrecisionString.c_str(), sPrecisionString.c_str());
        return false;
    }
    // compare the maximum temporal cluster size to the minimum temporal cluster size
    if (gParameters.getMinimumTemporalClusterSize() > static_cast<unsigned int>(dMaxTemporalLengthInUnits)) {
        PrintDirection.Printf("%s:\nThe minimum temporal cluster size of %d %s%s is greater than the maximum temporal cluster size of %d %s%s.\n",
                              BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, 
                              gParameters.getMinimumTemporalClusterSize(),
                              sPrecisionString.c_str(), (gParameters.getMinimumTemporalClusterSize() == 1 ? "" : "s"),
                              static_cast<unsigned int>(dMaxTemporalLengthInUnits), 
                              sPrecisionString.c_str(), (dMaxTemporalLengthInUnits == 1 ? "" : "s"));
        return false;
    }

    if (gParameters.GetAnalysisType() == SEASONALTEMPORAL) {
        /* Check that time aggregration length is less than defined maximum for unit type. */
        switch (gParameters.GetTimeAggregationUnitsType()) {
        case DAY:
            if (gParameters.getMinimumTemporalClusterSize() > 90) {
                PrintDirection.Printf("%s:\nThe minimum temporal cluster size may not exceed 90 days for the Seasonal analysis.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
                return false;
            } break;
        case MONTH:
            if (gParameters.getMinimumTemporalClusterSize() > 3) {
                PrintDirection.Printf("%s:\nThe minimum temporal cluster size may not exceed 3 months for the Seasonal analysis.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
                return false;
            } break;
        case GENERIC:
            if (gParameters.getMinimumTemporalClusterSize() > static_cast<long>(std::floor(0.25 * dStudyPeriodLengthInUnits))) {
                PrintDirection.Printf("%s:\nThe minimum temporal cluster size may not exceed %ld units for the Seasonal analysis.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, static_cast<long>(std::floor(0.25 * dStudyPeriodLengthInUnits)));
                return false;
            } break;
        case YEAR:
            PrintDirection.Printf("%s:\nThe time aggregation in years is not implemented for the Seasonal analysis.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, static_cast<long>(std::floor(0.25 * dStudyPeriodLengthInUnits)));
            return false;
        default: throw prg_error("Unknown time aggregation type '%d'.", "ValidateTemporalClusterSize()", gParameters.GetTimeAggregationUnitsType());
        }
    }
  } catch (prg_exception& x) {
    x.addTrace("ValidateTemporalClusterSize()","ParametersValidate");
    throw;
  }
  return true;
}

bool ParametersValidate::ValidateSpatialOutputParameters(BasePrint & PrintDirection) const {
  bool  bReturn=true;

  //verify the index based cluster collection option with other settings.
  if (gParameters.getReportGiniOptimizedClusters()) {
      if (gParameters.GetProbabilityModelType() == ORDINAL || gParameters.GetProbabilityModelType() == CATEGORICAL) {
         bReturn = false;
         PrintDirection.Printf("%s:\nGini index based collection reporting is not implemented for the %s model.\n",
                               BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
      }
      if (gParameters.getNumFileSets() > 1 && gParameters.GetMultipleDataSetPurposeType() == MULTIVARIATE) {
          bReturn = false;
          PrintDirection.Printf("%s:\nGini index based collection reporting is not implemented with multiple data sets using the multivariate purpose.\n",
                                BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
      }
      if (gParameters.GetRiskType() == MONOTONERISK) {
        bReturn = false;
        PrintDirection.Printf("%s:\nGini index based collection reporting is not implemented for the isotonic scan.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
      }
      if (gParameters.GetAnalysisType() != PURELYSPATIAL) {
        bReturn = false;
        PrintDirection.Printf("%s:\nGini index based collection reporting is only implemented for purely spatial analyses.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
      }
      if (gParameters.GetIsIterativeScanning()) {
        bReturn = false;
        PrintDirection.Printf("%s:\nGini index based collection reporting is not implemented with the iterative scan statistic.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
      }
  }
  return bReturn;
}

bool ParametersValidate::ValidateMonotoneRisk(BasePrint& PrintDirection) const {
  bool  bReturn=true;
  try {
    if (gParameters.GetRiskType() == MONOTONERISK) {
      if (gParameters.GetAnalysisType() != PURELYSPATIAL) {
        bReturn = false;
        PrintDirection.Printf("%s:\nThe isotonic scan is implemented only for the purely spatial analysis.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
      }
      if (gParameters.getNumFileSets() > 1) {
        bReturn = false;
        PrintDirection.Printf("%s:\nThe isotonic scan statistic is not implemented with mulitple data sets.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
      }
      if (!(gParameters.GetProbabilityModelType() == POISSON || gParameters.GetProbabilityModelType() == BERNOULLI)) {
        bReturn = false;
        PrintDirection.Printf("%s:\nThe isotonic scan is implemented for only the Poisson and Bernoulli models.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
      }
      if (!gParameters.UseMetaLocationsFile() && gParameters.GetSpatialWindowType() == ELLIPTIC) {
        bReturn = false;
        PrintDirection.Printf("%s:\nThe isotonic scan is not implemented for elliptic shaped windows.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
      }
    }
  } catch (prg_exception& x) {
    x.addTrace("ValidateMonotoneRisk()","ParametersValidate");
    throw;
  }
  return bReturn;
}

bool ParametersValidate::ValidateLocationNetworkParameters(BasePrint& PrintDirection) const {
    bool  bReturn = true;
    try {
        if (!gParameters.getUseLocationsNetworkFile())
            return bReturn;

        if (gParameters.GetIsPurelyTemporalAnalysis()) {
            const_cast<CParameters&>(gParameters).setUseLocationsNetworkFile(false);
        }
        if (gParameters.UseLocationNeighborsFile()) {
            bReturn = false;
            PrintDirection.Printf("%s:\nThe non-euclidean neighbors file cannot be used in conjuction with the locations network file.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
        if (gParameters.UseSpecialGrid()) {
            bReturn = false;
            PrintDirection.Printf("%s:\nThe special grid file cannot be used in conjuction with the locations network file.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
        if (gParameters.requestsGeogrphaicalOutput() && gParameters.GetCoordinatesFileName().size() == 0) {
            const_cast<CParameters&>(gParameters).toggleGeogrphaicalOutput(false);
            PrintDirection.Printf("Parameter Setting Warning:\n"
                "Geographical output file(s) have been requested in conjuction with a network file yet a coordinates file has not been provided.\n"
                "A coordinates file is required to place locations geograghically. Geographical output files will not be created in this analysis.\n", BasePrint::P_WARNING
            );
        }
        if (gParameters.GetMultipleCoordinatesType() != ONEPERLOCATION) {
            bReturn = false;
            PrintDirection.Printf("%s:\nThe locations network file cannot be used in conjunction with the multiple coordinates per location id feature.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
        if (gParameters.getNetworkFilePurpose() == COORDINATES_OVERRIDE) {
            // There is a problem that has not been resolved yet -- see CentroidNeighborCalculator::CenterLocationDistancesAbout.
            bReturn = false;
            PrintDirection.Printf("%s:\nThe locations network file defined to override euclidean distances of the coordinates file is not currently implemented.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
        if (gParameters.getNetworkFilePurpose() == COORDINATES_OVERRIDE && gParameters.requestsGeogrphaicalOutput()) {
            const_cast<CParameters&>(gParameters).toggleGeogrphaicalOutput(false);
            PrintDirection.Printf("Parameter Setting Warning:\n"
                "Geographical output file(s) have been requested in conjuction with a network file which is defined to override euclidean distances of the coordinates file.\n"
                "File generation is not defined for this combination of settings and output files will not be created in this analysis.\n", BasePrint::P_WARNING
            );
        }
    } catch (prg_exception& x) {
        x.addTrace("ValidateLocationNetworkParameters()", "ParametersValidate");
        throw;
    }
    return bReturn;
}

/** Validates observable regions parameters. */
bool ParametersValidate::ValidateContinuousPoissonParameters(BasePrint & PrintDirection) const {
  bool  bReturn=true;
  if (gParameters.GetProbabilityModelType() != HOMOGENEOUSPOISSON) return true;
  try {
    const_cast<CParameters&>(gParameters).SetMultipleCoordinatesType(ONEPERLOCATION);
    if (gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false) ||
        gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true)) {
        bReturn = false;
        PrintDirection.Printf("%s:\nThe maximum spatial cluster size can not be specified interms of as percentage\n"
                              "of population defined in maximum circle file. This feature is not implemented for %s model.\n",
                              BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
    }
    if (gParameters.GetAnalysisType() != PURELYSPATIAL) {
      bReturn = false;
      PrintDirection.Printf("%s:\nThe %s model is not implemented for %s analysis.\n",
                            BasePrint::P_PARAMERROR, MSG_INVALID_PARAM,
                            ParametersPrint(gParameters).GetProbabilityModelTypeAsString(),
                            ParametersPrint(gParameters).GetAnalysisTypeAsString());
    }
    if (gParameters.GetExecutionType() == CENTRICALLY) {
      bReturn = false;
      PrintDirection.Printf("%s:\nCentric analysis execution is not a valid settings for %s model.\n",
                            BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
    }
    if (gParameters.GetSpatialWindowType() == ELLIPTIC) {
      bReturn = false;
      PrintDirection.Printf("%s:\nElliptical scans are not implemented for %s model.\n",
                            BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
    }
    if (gParameters.getNumFileSets() > 1) {
       bReturn = false;
       PrintDirection.Printf("%s:\nMultiple data sets are not permitted with %s model.\n",
                             BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
    }
    if (gParameters.GetCoordinatesType() == LATLON) {
      bReturn = false;
      PrintDirection.Printf("%s:\nLatitude/Longitude coordinates are not implemented for %s model.\n",
                            BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
    }

    std::vector<ConvexPolygonObservableRegion> polygons;
    // for each region defined, attempt to define region from specifications
    for (size_t t=0; t < gParameters.getObservableRegions().size(); ++t) {
       // Validate that regions define inequalities correctly ...
       InequalityContainer_t inequalities = ConvexPolygonBuilder::parse(gParameters.getObservableRegions()[t]);
       // Validate inequalities define a bound region ...
       polygons.push_back(ConvexPolygonBuilder::buildConvexPolygon(inequalities));
    }
    //test that polygons do not overlap
    if (polygons.size()) {
    for (size_t i=0; i < polygons.size() - 1; ++i) {
        for (size_t j=i+1; j < polygons.size(); ++j) {
            if (polygons[i].intersectsRegion(polygons[j]))
                throw region_exception("Inequalities define regions that overlap.\n"
                                       "Please check inequalities and/or redefine to not have overlap.");
            }
        }
    }
  } catch (region_exception& x) {
    PrintDirection.Printf("%s:\n%s\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, x.what());
    bReturn=false;
  } catch (prg_exception& x) {
    x.addTrace("ValidateContinuousPoissonParameters()","ParametersValidate");
    throw;
  }
  return bReturn;
}

/** Validates output options. */
bool ParametersValidate::ValidateOutputOptionParameters(BasePrint & PrintDirection) const {
  bool  bValid=true;
  try {
    // Just suppress this setting for situations that don't allow it.
    if (gParameters.GetOutputRelativeRisksFiles() &&
        (gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION || gParameters.GetProbabilityModelType() == HOMOGENEOUSPOISSON ||
         gParameters.GetProbabilityModelType() == ORDINAL || gParameters.GetProbabilityModelType() == CATEGORICAL|| gParameters.GetProbabilityModelType() == UNIFORMTIME)) {
      const_cast<CParameters&>(gParameters).SetOutputRelativeRisksAscii(false);
      const_cast<CParameters&>(gParameters).SetOutputRelativeRisksDBase(false);
      PrintDirection.Printf("Parameter Setting Warning:\n"
                            "The additional output file for risk estimates is not permitted for %s model (option was disabled).\n",
                            BasePrint::P_WARNING,
                            ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
    }
    if (gParameters.GetOutputRelativeRisksFiles() && gParameters.GetIsPurelyTemporalAnalysis()) {
      const_cast<CParameters&>(gParameters).SetOutputRelativeRisksAscii(false);
      const_cast<CParameters&>(gParameters).SetOutputRelativeRisksDBase(false);
      PrintDirection.Printf("Parameter Setting Warning:\n"
                            "The additional output file for risk estimates is not permitted for %s analysis.\nThe option was disabled.\n",
                            BasePrint::P_WARNING,
                            ParametersPrint(gParameters).GetAnalysisTypeAsString());
    }
    if (gParameters.getOutputKMLFile() && gParameters.GetCoordinatesType() != LATLON) {
        const_cast<CParameters&>(gParameters).setOutputKMLFile(false);
        PrintDirection.Printf("Parameter Setting Warning:\n"
                              "The Google Earth option is not available for Cartesian coordinates.\nThe option was disabled.\n", BasePrint::P_WARNING);
    }
    if (gParameters.getOutputKMLFile() && gParameters.GetIsPurelyTemporalAnalysis()) {
        const_cast<CParameters&>(gParameters).setOutputKMLFile(false);
        PrintDirection.Printf("Parameter Setting Warning:\n"
                              "The Google Earth option is not available for purely temporal analyses.\nThe option was disabled.\n", BasePrint::P_WARNING);
    }
    if (gParameters.getOutputKMLFile() && gParameters.UseLocationNeighborsFile()) {
        const_cast<CParameters&>(gParameters).setOutputKMLFile(false);
        PrintDirection.Printf("Parameter Setting Warning:\n"
                              "The Google Earth option is not available for non-euclidean neighbors file.\nThe option was disabled.\n", BasePrint::P_WARNING);
    }
    if (gParameters.getOutputShapeFiles() && gParameters.GetCoordinatesType() != LATLON) {
       const_cast<CParameters&>(gParameters).setOutputShapeFiles(false);
       PrintDirection.Printf("Parameter Setting Warning:\n"
                             "The shapefiles option is not available for Cartesian coordinates.\nThe option was disabled.\n", BasePrint::P_WARNING);
    }
    if (gParameters.getOutputShapeFiles() && gParameters.GetIsPurelyTemporalAnalysis()) {
        const_cast<CParameters&>(gParameters).setOutputShapeFiles(false);
        PrintDirection.Printf("Parameter Setting Warning:\n"
                              "The shapefiles option is not available for purely temporal analyses.\nThe option was disabled.\n", BasePrint::P_WARNING);
    }
    if (gParameters.getOutputShapeFiles() && gParameters.UseLocationNeighborsFile()) {
        const_cast<CParameters&>(gParameters).setOutputShapeFiles(false);
        PrintDirection.Printf("Parameter Setting Warning:\n"
                              "The shapefiles option is not available for non-euclidean neighbors file.\nThe option was disabled.\n", BasePrint::P_WARNING);
    }

    if (gParameters.getOutputGoogleMapsFile() && gParameters.GetCoordinatesType() != LATLON) {
        const_cast<CParameters&>(gParameters).setOutputGoogleMapsFile(false);
        PrintDirection.Printf("Parameter Setting Warning:\n"
            "The Google Maps option is not available for Cartesian coordinates.\nThe option was disabled.\n", BasePrint::P_WARNING);
    }
    if (gParameters.getOutputGoogleMapsFile() && gParameters.GetIsPurelyTemporalAnalysis()) {
        const_cast<CParameters&>(gParameters).setOutputGoogleMapsFile(false);
        PrintDirection.Printf("Parameter Setting Warning:\n"
            "The Google Maps option is not available for purely temporal analyses.\nThe option was disabled.\n", BasePrint::P_WARNING);
    }
    if (gParameters.getOutputGoogleMapsFile() && gParameters.UseLocationNeighborsFile()) {
        const_cast<CParameters&>(gParameters).setOutputGoogleMapsFile(false);
        PrintDirection.Printf("Parameter Setting Warning:\n"
            "The Google Maps option is not available for non-euclidean neighbors file.\nThe option was disabled.\n", BasePrint::P_WARNING);
    }


    if (gParameters.getOutputShapeFiles() && !(gParameters.GetOutputClusterLevelDBase() || gParameters.GetOutputAreaSpecificDBase())) {
      const_cast<CParameters&>(gParameters).SetOutputClusterLevelDBase(true);
      const_cast<CParameters&>(gParameters).SetOutputAreaSpecificDBase(true);
      PrintDirection.Printf("Parameter Setting Warning:\n"
                            "The shapefiles option requires that the 'Cluster Information' and 'Location Information' dBase files also be generated.\nThese options were enabled.\n",
                            BasePrint::P_WARNING);
    }
    if (gParameters.getOutputCartesianGraph() && gParameters.GetIsPurelyTemporalAnalysis()) {
        const_cast<CParameters&>(gParameters).setOutputCartesianGraph(false);
        PrintDirection.Printf("Parameter Setting Warning:\n"
            "The cartesian graph option is not available for purely temporal analyses.\nThe option was disabled.\n",
            BasePrint::P_WARNING);
    }

    if (gParameters.getOutputTemporalGraphFile() &&
        (!(gParameters.GetIsPurelyTemporalAnalysis() || gParameters.GetIsSpaceTimeAnalysis() || gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) ||
        !(gParameters.GetProbabilityModelType() == POISSON || gParameters.GetProbabilityModelType() == BERNOULLI || 
          gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION || gParameters.GetProbabilityModelType() == EXPONENTIAL || gParameters.GetProbabilityModelType() == UNIFORMTIME))) {
            const_cast<CParameters&>(gParameters).setOutputTemporalGraphFile(false);
      PrintDirection.Printf("Parameter Setting Warning:\n"
                            "The temporal graph option is only available for temporal analyses with Poisson, Bernoulli, STP, Exponential and Uniform Time models.\nThe option was disabled.\n",
                            BasePrint::P_WARNING);
    }
    if (!ValidateSpatialOutputParameters(PrintDirection))
        bValid = false;
  } catch (prg_exception& x) {
    x.addTrace("ValidateOutputOptionParameters()","ParametersValidate");
    throw;
  }
  return bValid;
}

/** Validates power calculation parameters.
   Prints errors to print direction and returns validity. */
bool ParametersValidate::ValidatePowerEvaluationsParameters(BasePrint & PrintDirection) const {
    bool bValid=true;
    if (gParameters.getPerformPowerEvaluation()) {
        if (gParameters.GetProbabilityModelType() != POISSON) {
            PrintDirection.Printf("%s:\nThe power evaluation is not available for the %s model.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
            bValid = false;
        }
        if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
            PrintDirection.Printf("%s:\nThe power evaluation is not available for the %s analysis.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, ParametersPrint(gParameters).GetAnalysisTypeAsString());
            bValid = false;
        }
        if (gParameters.GetRiskType() != STANDARDRISK) {
            PrintDirection.Printf("%s:\nThe power evaluation is not available for the isotonic scan statistic.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
            bValid = false;
        }
        if(gParameters.GetPValueReportingType() == TERMINATION_PVALUE) {
            PrintDirection.Printf("%s:\nThe power evaluation is not available for the Sequential Standard Monte Carlo p-value reporting.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
            bValid = false;
        }
        // if the user is specifying the total # of cases, then certain features are not valid
        if (gParameters.getPowerEvaluationMethod() == PE_ONLY_SPECIFIED_CASES) {
            // temporal adjustments are not available without case data
            if (gParameters.GetTimeTrendAdjustmentType() != TEMPORAL_NOTADJUSTED) {
                PrintDirection.Printf("%s:\nThe power evaluation can not perform temporal adjustments without a case file.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
                bValid = false;
            }
            // spatial adjustments are not available without case data
            if (gParameters.GetSpatialAdjustmentType() != SPATIAL_NOTADJUSTED) {
                PrintDirection.Printf("%s:\nThe power evaluation can not perform spatial adjustments without a case file.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
                bValid = false;
            }
            if (gParameters.UseAdjustmentForRelativeRisksFile()) {
                bValid = false;
                PrintDirection.Printf("%s:\nThe power evaluation perform adjustment for known relative risks without a case file.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
            }
            if (gParameters.GetOutputRelativeRisksFiles()) {
                bValid = false;
                PrintDirection.Printf("%s:\nThe power evaluation can not report the location relative risks file without a case file.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
            }
        }
        bValid &= checkFileExists(gParameters.getPowerEvaluationAltHypothesisFilename(), "power evaluations alternative hypothesis", PrintDirection);
        if (gParameters.GetNumReplicationsRequested() < 999) {
            PrintDirection.Printf("%s:\nThe minimum number of standard replications in the power evaluation is %u.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, 999);
            bValid = false;
        }
        if (gParameters.getNumPowerEvalReplicaPowerStep() < 100) {
            PrintDirection.Printf("%s:\nThe minimum number of power replications in the power evaluation is %u.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, 100);
            bValid = false;
        }
        if (gParameters.getNumPowerEvalReplicaPowerStep() % 100) {
            PrintDirection.Printf("%s:\nThe number of power replications in the power evaluation must be a multiple of 100.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
            bValid = false;
        }
        if (gParameters.getNumPowerEvalReplicaPowerStep() > gParameters.GetNumReplicationsRequested() + 1) {
            PrintDirection.Printf("%s:\nThe number of standard replications must be at most one less than the number of power replications (%u).\n", 
                                  BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, gParameters.getNumPowerEvalReplicaPowerStep());
            bValid = false;
        }
        if (gParameters.GetIsIterativeScanning()) {
           bValid = false;
           PrintDirection.Printf("%s:\nThe power evaluation can not be performed with the iterative scan statistic.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
        if (gParameters.getPowerEvaluationCriticalValueType() == CV_POWER_VALUES) {
            if (gParameters.getPowerEvaluationCriticalValue05() < 0.0) {
                bValid = false;
                PrintDirection.Printf("%s:\nThe power evaluation critical value at .05 '%lf' is invalid. Please use a value greater than zero.\n",
                                      BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, gParameters.getPowerEvaluationCriticalValue05());
            }
            if (gParameters.getPowerEvaluationCriticalValue01() < 0.0) {
                bValid = false;
                PrintDirection.Printf("%s:\nThe power evaluation critical value at .01 '%lf' is invalid. Please use a value greater than zero.\n",
                                      BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, gParameters.getPowerEvaluationCriticalValue01());
            }
            if (gParameters.getPowerEvaluationCriticalValue001() < 0.0) {
                bValid = false;
                PrintDirection.Printf("%s:\nThe power evaluation critical value at .001 '%lf' is invalid. Please use a value greater than zero.\n",
                                      BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, gParameters.getPowerEvaluationCriticalValue01());
            }
        }
        // validate power evaluations' power step simulation options
        if (gParameters.getOutputPowerEvaluationSimulationData()) {
            bValid &= checkFileExists(gParameters.getPowerEvaluationSimulationDataOutputFilename(), "power evaluations simulation output", PrintDirection, true);
        }
        switch (gParameters.GetPowerEvaluationSimulationType()) {
            case STANDARD         : 
                // validate the alternative hypothesis file 
                bValid &= checkFileExists(gParameters.getPowerEvaluationAltHypothesisFilename(), "alternative hypothesis", PrintDirection);
                break;
            case FILESOURCE       :
                if (gParameters.getNumFileSets() > 1){
                    bValid = false;
                    PrintDirection.Printf("%s:\nThe feature to read power evaluation simulated data from a file is not implemented "
                                          "for analyses that read data from multiple data sets.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
                }
                bValid &= checkFileExists(gParameters.getPowerEvaluationSimulationDataSourceFilename(), "power evaluation simulation data source", PrintDirection);
                break;
            case HA_RANDOMIZATION : 
            default : throw prg_error("Unknown power evaluation simulation type '%d'.","ValidatePowerEvaluationsParameters()", gParameters.GetPowerEvaluationSimulationType());
        };
        if (gParameters.GetExecutionType() == CENTRICALLY) {
            bValid = false;
            PrintDirection.Printf("%s:\nThe power evaluation can not be performed with the alternative memory allocation.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
    } return bValid;
}

/** Validates parameters parameters related to randomization seed.
    Prints errors to print direction and returns whether values are vaild. */
bool ParametersValidate::ValidateRandomizationSeed(BasePrint& PrintDirection) const {
  double        dMaxRandomizationSeed, dMaxReplications, dMaxSeed;

  if (gParameters.GetNumReplicationsRequested()) {
    if (gParameters.GetIsRandomlyGeneratingSeed()) {
      dMaxSeed = (double)RandomNumberGenerator::glM - (double)gParameters.GetNumReplicationsRequested() - (double)(gParameters.getNumFileSets() -1) * AbstractRandomizer::glDataSetSeedOffSet - 1;
      boost::minstd_rand generator(static_cast<int>(time(0)));
      const_cast<CParameters&>(gParameters).SetRandomizationSeed(boost::uniform_int<>(1,static_cast<int>(dMaxSeed))(generator));
      return true;
    }
    //validate hidden parameter which specifies randomization seed
    if (!(0 < gParameters.GetRandomizationSeed() && gParameters.GetRandomizationSeed() < RandomNumberGenerator::glM)) {
      PrintDirection.Printf("%s:\nRandomization seed out of range [1 - %ld].\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, RandomNumberGenerator::glM - 1);
      return false;
    }

    //validate that generated seeds during randomization will not exceed defined range
    // - note that though the number of data sets has a bearing on the maximum seed, but we
    //   will not indicate to user to reduce the number of data sets in order to correct issues.
    dMaxRandomizationSeed = (double)gParameters.GetRandomizationSeed() + (double)gParameters.GetNumReplicationsRequested() + (double)(gParameters.getNumFileSets() -1) * AbstractRandomizer::glDataSetSeedOffSet;

    if (dMaxRandomizationSeed >= static_cast<double>(RandomNumberGenerator::glM)) {
      //case #1 - glRandomizationSeed == RandomNumberGenerator::glDefaultSeed
      //    In this case, it is assumed that user accepted default - at this time
      //    changing the initial seed through parameter settings is a 'hidden' parameter;
      //    so no direction should be given regarding the alteration of seed value.
      if (gParameters.GetRandomizationSeed() == RandomNumberGenerator::glDefaultSeed) {
        dMaxReplications = (double)RandomNumberGenerator::glM - (double)gParameters.GetRandomizationSeed() - (double)(gParameters.getNumFileSets() -1) * AbstractRandomizer::glDataSetSeedOffSet;
        dMaxReplications = (floor((dMaxReplications)/1000) - 1)  * 1000 + 999;
        PrintDirection.Printf("%s:\nRequested number of replications causes randomization seed to exceed defined limit. "
                              "Maximum number of replications is %.0lf.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, dMaxReplications);
        return false;
      }
      //case #2 - user specified alternate randomization seed
      //    This alternate seed or the number of requested replications could be the problem.
      //    User has two options, either pick a lesser seed or request less replications.
      //calculate maximum seed for requested number of replications
      dMaxSeed = (double)RandomNumberGenerator::glM - (double)gParameters.GetNumReplicationsRequested() - (double)(gParameters.getNumFileSets() -1) * AbstractRandomizer::glDataSetSeedOffSet - 1;
      //calculate maximum number of replications for requested seed
      dMaxReplications = (double)RandomNumberGenerator::glM - (double)gParameters.GetRandomizationSeed() - (double)(gParameters.getNumFileSets() -1) * AbstractRandomizer::glDataSetSeedOffSet;
      dMaxReplications = (floor((dMaxReplications)/1000) - 1)  * 1000 + 999;
      //check whether specified combination of seed and requested number of replications fights each other
      if (dMaxReplications < 9 && (dMaxSeed <= 0 || dMaxSeed > RandomNumberGenerator::glM)) {
        PrintDirection.Printf("%s:\nRandomization seed will exceed defined limit. "
                              "The specified initial seed, in conjunction with the number of replications, "
                              "contend for numerical range in defined limits. Please modify the specified "
                              "initial seed and/or lessen the number of replications and try again.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
      }
      //check that randomization seed is not so large that we can't run any replications
      else if (dMaxReplications < 9) {
        PrintDirection.Printf("%s:\nRandomization seed will exceed defined limit. "
                              "The intial seed specified prevents any replications from being performed. "
                              "With %ld replications, the initial seed can be [0 - %.0lf].\n",
                              BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, gParameters.GetNumReplicationsRequested(), dMaxSeed);
      }
      //check that number of replications isn't too large
      else if (dMaxSeed <= 0 || dMaxSeed > RandomNumberGenerator::glM) {
        PrintDirection.Printf("%s:\nRequested number of replications causes randomization seed to exceed defined limit. "
                              "With initial seed of %i, maximum number of replications is %.0lf.\n",
                              BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, gParameters.GetRandomizationSeed(), dMaxReplications);
      }
      else {
        PrintDirection.Printf("%s:\nRandomization seed will exceed defined limit. "
                              "Either limit the number of replications to %.0lf or "
                              "define the initial seed to a value less than %.0lf.\n",
                              BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, dMaxReplications, dMaxSeed);
      }
      return false;
    }
  }

  return true;
}

/** Validates parameters used in optional start and end ranges for time windows.
    Prints errors to print direction and returns whether values are vaild. */
bool ParametersValidate::ValidateRangeParameters(BasePrint& PrintDirection) const {
  bool          bValid=true;
  UInt          uiYear, uiMonth, uiDay;
  Julian        StudyPeriodStartDate, StudyPeriodEndDate,
                StartRangeStartDate, StartRangeEndDate,
                EndRangeStartDate, EndRangeEndDate;

  try {
    if (gParameters.GetIncludeClustersType() == CLUSTERSINRANGE && (gParameters.GetAnalysisType() == PURELYTEMPORAL || gParameters.GetAnalysisType() == SPACETIME)) {
      //validate start range start date
      if (!ValidateDateString(PrintDirection, INTERVAL_STARTRANGE, gParameters.GetStartRangeStartDate()))
        return false;
      //validate start range end date
      if (!ValidateDateString(PrintDirection, INTERVAL_STARTRANGE, gParameters.GetStartRangeEndDate()))
        return false;
      //validate end range start date
      if (!ValidateDateString(PrintDirection, INTERVAL_ENDRANGE, gParameters.GetEndRangeStartDate()))
        return false;
      //validate end range end date
      if (!ValidateDateString(PrintDirection, INTERVAL_ENDRANGE, gParameters.GetEndRangeStartDate()))
        return false;
      //now valid that range dates are within study period start and end dates
      if (bValid) {
        StudyPeriodStartDate = DateStringParser::getDateAsJulian(gParameters.GetStudyPeriodStartDate().c_str(), gParameters.GetPrecisionOfTimesType());
        StudyPeriodEndDate = DateStringParser::getDateAsJulian(gParameters.GetStudyPeriodEndDate().c_str(), gParameters.GetPrecisionOfTimesType());

        EndRangeStartDate = DateStringParser::getDateAsJulian(gParameters.GetEndRangeStartDate().c_str(), gParameters.GetPrecisionOfTimesType());
        EndRangeEndDate = DateStringParser::getDateAsJulian(gParameters.GetEndRangeEndDate().c_str(), gParameters.GetPrecisionOfTimesType());
        if (EndRangeStartDate > EndRangeEndDate) {
          bValid = false;
          PrintDirection.Printf("%s:\nInvalid scanning window end range. Range date '%s' occurs after date '%s'.\n",
                                BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, gParameters.GetEndRangeStartDate().c_str(), gParameters.GetEndRangeEndDate().c_str());
        }
        else {
          if (EndRangeStartDate < StudyPeriodStartDate || EndRangeStartDate > StudyPeriodEndDate) {
            bValid = false;
            PrintDirection.Printf("%s:\nThe scanning window end range date '%s' is not within the study period (%s - %s).\n",
                                  BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, gParameters.GetEndRangeStartDate().c_str(),
                                  gParameters.GetStudyPeriodStartDate().c_str(), gParameters.GetStudyPeriodEndDate().c_str());
          }
          if (EndRangeEndDate < StudyPeriodStartDate || EndRangeEndDate > StudyPeriodEndDate) {
            bValid = false;
            PrintDirection.Printf("%s:\nThe scanning window end range date '%s' is not within the study period (%s - %s) \n",
                                  BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, gParameters.GetEndRangeEndDate().c_str(),
                                  gParameters.GetStudyPeriodStartDate().c_str(), gParameters.GetStudyPeriodEndDate().c_str());
          }
        }

        StartRangeStartDate = DateStringParser::getDateAsJulian(gParameters.GetStartRangeStartDate().c_str(), gParameters.GetPrecisionOfTimesType());
        StartRangeEndDate = DateStringParser::getDateAsJulian(gParameters.GetStartRangeEndDate().c_str(), gParameters.GetPrecisionOfTimesType());
        if (StartRangeStartDate > StartRangeEndDate) {
          bValid = false;
          PrintDirection.Printf("%s:\nInvalid scanning window start range. The range date '%s' occurs after date '%s'.\n",
                                BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, gParameters.GetStartRangeStartDate().c_str(),
                                gParameters.GetStartRangeEndDate().c_str());
        }
        else {
          if (StartRangeStartDate < StudyPeriodStartDate || StartRangeStartDate > StudyPeriodEndDate) {
            bValid = false;
            PrintDirection.Printf("%s:\nThe scanning window start range date '%s' is not within the study period (%s - %s).\n",
                                  BasePrint::P_PARAMERROR, gParameters.GetStartRangeStartDate().c_str(),
                                  gParameters.GetStudyPeriodStartDate().c_str(), gParameters.GetStudyPeriodEndDate().c_str());
          }
          if (StartRangeEndDate < StudyPeriodStartDate || StartRangeEndDate > StudyPeriodEndDate) {
            bValid = false;
            PrintDirection.Printf("%s:\nThe scanning window start range date '%s' is not within the study period (%s - %s) \n",
                                  BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, MSG_INVALID_PARAM, gParameters.GetStartRangeEndDate().c_str(),
                                  gParameters.GetStudyPeriodStartDate().c_str(), gParameters.GetStudyPeriodEndDate().c_str());
          }
        }
        if (StartRangeStartDate >= EndRangeEndDate) {
          bValid = false;
          PrintDirection.Printf("%s:\nThe scanning window start range does not occur before the end range.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
      }
    }
  } catch (prg_exception& x) {
    x.addTrace("ValidateRangeParameters()","ParametersValidate");
    throw;
  }
  return bValid;
}

/** Validates parameters used in making simulation data. */
bool ParametersValidate::ValidateSimulationDataParameters(BasePrint & PrintDirection) const {
  bool  bValid=true;

  try {
    if (gParameters.GetNumReplicationsRequested() == 0)
      const_cast<CParameters&>(gParameters).SetOutputSimulationData(false);
    if (gParameters.GetOutputSimulationData() && gParameters.GetSimulationDataOutputFilename().empty()) {
      bValid = false;
      PrintDirection.Printf("%s:\nSimulation data output file not specified.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
    }

    switch (gParameters.GetSimulationType()) {
      case STANDARD         : break;
      case FILESOURCE       :
        if (gParameters.GetProbabilityModelType() == EXPONENTIAL || gParameters.GetProbabilityModelType() == NORMAL ||
            gParameters.GetProbabilityModelType() == RANK) {
          bValid = false;
          PrintDirection.Printf("%s:\nThe feature to read simulated data from a file is not implemented for "
                                "the %s probability model.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM,
                                ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
        }
        if (gParameters.getNumFileSets() > 1){
          bValid = false;
          PrintDirection.Printf("%s:\nThe feature to read simulated data from a file is not implemented for analyses "
                                "that read data from multiple data sets.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
        if (gParameters.GetSimulationDataSourceFilename().empty()) {
          bValid = false;
          PrintDirection.Printf("%s:\nThe simulated data input file was not specified.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
        else if (!checkFileExists(gParameters.GetSimulationDataSourceFilename(), "simulation source", PrintDirection)) {
          bValid = false;
        }
        if (gParameters.GetOutputSimulationData() && gParameters.GetSimulationDataSourceFilename() == gParameters.GetSimulationDataOutputFilename()) {
          bValid = false;
          PrintDirection.Printf("%s:\nThe file '%s' is specified as both the input and the output file for simulated data.\n",
                                BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, gParameters.GetSimulationDataSourceFilename().c_str());
        }
        break;
      case HA_RANDOMIZATION : break;
      default : throw prg_error("Unknown simulation type '%d'.","ValidateSimulationDataParameters()", gParameters.GetSimulationType());
    };
  } catch (prg_exception& x) {
    x.addTrace("ValidateSimulationDataParameters()","ParametersValidate");
    throw;
  }
  return bValid;
}

/** Validates optional parameters particular to spatial analyses
    (i.e. purely spatial, retrospective space-time and prospective space-time).
    Prints errors to print direction and returns whether values are vaild. */
bool ParametersValidate::ValidateSpatialParameters(BasePrint & PrintDirection) const {
  bool bValid=true;

  try {
    //validate spatial options
    if (!gParameters.GetIsPurelyTemporalAnalysis()) {
      //validate maximum is specified as a pecentage of population at risk when using neighbors file
      if (gParameters.UseLocationNeighborsFile() && gParameters.GetRestrictMaxSpatialSizeForType(MAXDISTANCE, false)) {
        bValid = false;
        PrintDirection.Printf("%s:\nWhen specifying geographical neighbors through the non-Eucledian neighbors file, "
                              "the maximum spatial cluster size can not be defined as a fixed distance.", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
      }
      //validate reported maximum is specified as a pecentage of population at risk when using neighbors file
      if (gParameters.UseLocationNeighborsFile() && gParameters.GetRestrictMaxSpatialSizeForType(MAXDISTANCE, true)) {
        bValid = false;
        PrintDirection.Printf("%s:\nWhen specifying geographical neighbors through the non-Eucledian neighbors file, "
                              "the maximum reported spatial cluster size can not be defined as a fixed distance.", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
      }
      //validate maximum as pecentage of population at risk
      double dPercentPopValue = gParameters.GetMaxSpatialSizeForType(PERCENTOFPOPULATION, false);
      if (!(gParameters.GetAnalysisType() == PROSPECTIVESPACETIME && gParameters.GetAdjustForEarlierAnalyses()) && (dPercentPopValue <= 0.0 || dPercentPopValue > 100.0/*50.0*/)) {
        bValid = false;
        PrintDirection.Printf("%s:\nThe maximum spatial cluster size, defined as percentage of population at risk, is invalid. "
                              "The specified value is %2g. Must be greater than zero and <= %d.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, dPercentPopValue, 100/*50*/);
      }
      //validate maximum as pecentage of population defined in max circle file
      double dPercentMaxFileValue = gParameters.GetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false);
      if (gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false) && (dPercentMaxFileValue <= 0.0 || dPercentMaxFileValue > 100.0/*50.0*/)) {
        bValid = false;
        PrintDirection.Printf("%s:\nThe maximum spatial cluster size, defined as percentage of population in max circle file, is invalid. "
                              "The specified value is %2g. Must be greater than zero and <= %d.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, dPercentMaxFileValue, 100/*50*/);
      }
      //validate maximum as a fixed distance
      double dDistanceValue = gParameters.GetMaxSpatialSizeForType(MAXDISTANCE, false);
      if (gParameters.GetRestrictMaxSpatialSizeForType(MAXDISTANCE, false) && dDistanceValue <= 0.0) {
        bValid = false;
        PrintDirection.Printf("%s:\nThe maximum spatial cluster size of %2g units is invalid. "
                              "The specified value is %2g. Must be greater than zero.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, dDistanceValue);
      }
      //validate maximum as pecentage of population at risk -- reported
      if (!(gParameters.GetAnalysisType() == PROSPECTIVESPACETIME && gParameters.GetAdjustForEarlierAnalyses()) && gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFPOPULATION, true)) {
          double dRptPercentPopValue = gParameters.GetMaxSpatialSizeForType(PERCENTOFPOPULATION, true);
          if (dRptPercentPopValue <= 0.0 || dRptPercentPopValue > dPercentPopValue) {
            bValid = false;
            PrintDirection.Printf("%s:\nThe maximum reported spatial cluster size, defined as percentage of population at risk, is invalid. "
                                  "The specified value is %2g. Must be greater than zero and <= %2g.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, dRptPercentPopValue, dPercentPopValue);
          }
      }
      //validate maximum as pecentage of population defined in max circle file -- reported
      if (gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true)) {
          double dRptPercentMaxFileValue = gParameters.GetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true);
          if (dRptPercentMaxFileValue <= 0.0 || dRptPercentMaxFileValue > dPercentMaxFileValue) {
            bValid = false;
            PrintDirection.Printf("%s:\nThe maximum reported spatial cluster size, defined as percentage of population in max circle file, is invalid. "
                                  "The specified value is %2g. Must be greater than zero and <= %2g.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, dRptPercentMaxFileValue, dPercentMaxFileValue);
          }
      }
      //validate maximum as a fixed distance  -- reported
      if (gParameters.GetRestrictMaxSpatialSizeForType(MAXDISTANCE, true)) {
          double dRptDistanceValue = gParameters.GetMaxSpatialSizeForType(MAXDISTANCE, true);
          if (gParameters.GetRestrictMaxSpatialSizeForType(MAXDISTANCE, false) && (dRptDistanceValue <= 0.0 || dRptDistanceValue > dDistanceValue)) {
            bValid = false;
            PrintDirection.Printf("%s:\nThe maximum reported spatial cluster size of %2g units is invalid. "
                                  "Must be greater than zero and <= %2g.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, dRptDistanceValue, dDistanceValue);
          }
      }
    }

    if (gParameters.GetIncludePurelySpatialClusters()) {
      if (!gParameters.GetPermitsPurelySpatialCluster(gParameters.GetProbabilityModelType())) {
          bValid = false;
          PrintDirection.Printf("%s:\nA purely spatial cluster cannot be included for a %s model.\n",
                                BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
      }
      else if (!gParameters.GetPermitsPurelySpatialCluster()) {
        bValid = false;
        PrintDirection.Printf("%s:\nA purely spatial cluster can not be included in %s analyses.\n",
                              BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, ParametersPrint(gParameters).GetAnalysisTypeAsString());
      }
    }
    if (gParameters.GetSpatialAdjustmentType() == SPATIAL_STRATIFIED_RANDOMIZATION) {
      if (!(gParameters.GetAnalysisType() == SPACETIME || gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)) {
        bValid = false;
        PrintDirection.Printf("%s:\nThe nonparametric spatial adjustment by stratified randomization is valid for "
                              "either retrospective or prospective space-time analyses only.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
      }
      if (gParameters.GetProbabilityModelType() != POISSON) {
          bValid = false;
          PrintDirection.Printf(
              "%s:\nThe nonparametric spatial adjustment by stratified randomization is valid only for Poisson model currently.\n", 
              BasePrint::P_PARAMERROR, MSG_INVALID_PARAM
          );
      }
      if (gParameters.GetIncludePurelySpatialClusters()) {
        bValid = false;
        PrintDirection.Printf("%s:\nThe nonparametric spatial adjustment by stratified randomization does not permit "
                              "the inclusion of a purely spatial cluster.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
      }
    }
    if (gParameters.GetSpatialAdjustmentType() == SPATIAL_NONPARAMETRIC && 
        !(gParameters.GetTimeTrendAdjustmentType() == TEMPORAL_NOTADJUSTED || gParameters.GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION)) {
        bValid = false;
        PrintDirection.Printf("%s:\nThe nonparametric spatial adjustment can only be performed with non-parametric temporal time stratified adjustment.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
    }
  } catch (prg_exception& x) {
    x.addTrace("ValidateSpatialParameters()","ParametersValidate");
    throw;
  }
  return bValid;
}

/** Validates end date for the following attributes:
    - date contains year, month and day.
    - date is a real date.
    - checks that date agrees with precision of times settings
      ;precision year - month = 12 and day = 31
      ;precision month - day = last day of month
   Returns boolean indication of whether date is valid, printing relevant
   messages to BasePrint object.                                              */
bool ParametersValidate::ValidateStudyPeriodEndDate(BasePrint& PrintDirection) const {
  UInt                  nYear, nMonth, nDay;
  DatePrecisionType     ePrecision;

  try {
    if (!ValidateDateString(PrintDirection, ENDDATE, gParameters.GetStudyPeriodEndDate()))
        return false;

    // remainder of checks are for non-generic dates
    if (gParameters.GetPrecisionOfTimesType() == GENERIC)
        return true;

    //parse date in parts
    CharToMDY(&nMonth, &nDay, &nYear, gParameters.GetStudyPeriodEndDate().c_str());

    //validate against precision of times
    if (gParameters.GetCreationVersion().iMajor == 4)
      // no date precision validation needed for purely spatial
      ePrecision = (gParameters.GetAnalysisType() == PURELYSPATIAL ? NONE : gParameters.GetTimeAggregationUnitsType());
    else
      ePrecision = gParameters.GetPrecisionOfTimesType();

    switch (ePrecision) {
      case YEAR  :
        if (nMonth != 12 || nDay != 31) {
          PrintDirection.Printf("%s:\nThe study period end date '%s' is not valid. "
                                "With the setting for %s as years, the date  must be the last day of respective year.\n",
                                BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, gParameters.GetStudyPeriodEndDate().c_str(),
                                (gParameters.GetCreationVersion().iMajor == 4 ? "time aggregation" : "time precision"));
          return false;
        }
        break;
      case MONTH :
        if (nDay != DaysThisMonth(nYear, nMonth)) {
          PrintDirection.Printf("%s:\nThe study period end date '%s' is not valid. "
                                "With the setting for %s as months, the date must be the last day of respective month.\n",
                                BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, gParameters.GetStudyPeriodEndDate().c_str(),
                                (gParameters.GetCreationVersion().iMajor == 4 ? "time aggregation" : "time precision"));
          return false;
        }
      case DAY   :
      case GENERIC :
      case NONE  : break;
    };
  } catch (prg_exception& x) {
    x.addTrace("ValidateStudyPeriodEndDate()","ParametersValidate");
    throw;
  }
  return true;
}

/** Validates start date for the following attributes:
    - date contains year, month and day.
    - date is a real date.
    - checks that date agrees with precision of times settings
      ;precision year - month = 12 and day = 31
      ;precision month - day = last day of month
   Returns boolean indication of whether date is valid, printing relevant
   messages to BasePrint object.                                              */
bool ParametersValidate::ValidateStudyPeriodStartDate(BasePrint& PrintDirection) const {
  UInt                  nYear, nMonth, nDay;
  DatePrecisionType     ePrecision;

  try {
    if (!ValidateDateString(PrintDirection, STARTDATE, gParameters.GetStudyPeriodStartDate()))
        return false;

    // remainder of checks are for non-generic dates
    if (gParameters.GetPrecisionOfTimesType() == GENERIC)
        return true;

    if (gParameters.GetCreationVersion().iMajor == 4)
      // no date precision validation needed for purely spatial
      ePrecision = (gParameters.GetAnalysisType() == PURELYSPATIAL ? NONE : gParameters.GetTimeAggregationUnitsType());
    else
      ePrecision = gParameters.GetPrecisionOfTimesType();

    //parse date in parts
    CharToMDY(&nMonth, &nDay, &nYear, gParameters.GetStudyPeriodStartDate().c_str());
    //validate against precision of times
    switch (ePrecision) {
      case YEAR  :
         if (nMonth != 1 || nDay != 1) {
            PrintDirection.Printf("%s:\nThe study period start date, '%s', is not valid. "
                                  "With the setting for %s as years, the date must be the first day of respective year.\n",
                                  BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, gParameters.GetStudyPeriodStartDate().c_str(),
                                  (gParameters.GetCreationVersion().iMajor == 4 ? "time aggregation" : "time precision"));
            return false;
         }
         break;
      case MONTH :
         if (nDay != 1) {
             PrintDirection.Printf("%s:\nThe study period start date, '%s', is not valid. "
                                   "With the setting for %s as months, the date must be the first day of respective month.\n",
                                   BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, gParameters.GetStudyPeriodStartDate().c_str(),
                                   (gParameters.GetCreationVersion().iMajor == 4 ? "time aggregation" : "time precision"));
            return false;
         }
      case DAY   :
      case GENERIC :
      case NONE  : break;
    };
  } catch (prg_exception& x) {
    x.addTrace("ValidateStartDate()","ParametersValidate");
    throw;
  }
  return true;
}

/** Validates spatial variation in temporal trends analysis settings. */
bool ParametersValidate::ValidateSVTTAnalysisSettings(BasePrint& PrintDirection) const {
  bool          bValid=true;

  if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
    if (gParameters.getNumFileSets() > 1) {
      bValid = false;
      PrintDirection.Printf("%s:\nMultiple data sets is not implemented with spatial variation in temporal trends.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
    }
    if (gParameters.GetProbabilityModelType() != POISSON) {
      bValid = false;
      PrintDirection.Printf("%s:\nSpatial variation in temporal trends is implemented only for the Poisson model.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
    }
    if (gParameters.GetTimeTrendConvergence() < 0.0) {
      bValid = false;
      PrintDirection.Printf("%s:\nTime trend convergence value of '%2g' is less than zero.\n",
                            BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, gParameters.GetTimeTrendConvergence());
    }
    if (gParameters.getTimeTrendType() == QUADRATIC && gParameters.GetExecuteScanRateType() != HIGHANDLOW) {
      bValid = false;
      PrintDirection.Printf("%s:\nThe quadtratic time trend is only implemented for simulantienous high and low scanning.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
    }
  }
  return bValid;
}

/** Validates optional parameters particular to temporal analyses
    (i.e. purely temporal, retrospective space-time and prospective space-time).
    Prints errors to print direction and returns whether values are vaild.*/
bool ParametersValidate::ValidateTemporalParameters(BasePrint & PrintDirection) const {
  bool          bValid=true;

  try {
    //validate temporal options only for analyses that are temporal
    if (gParameters.GetAnalysisType() == PURELYSPATIAL || gParameters.GetProbabilityModelType() == HOMOGENEOUSPOISSON) {
      //default these options - Not sure why orignal programmers did this. When
      //there is more time, we want to examine code so that we don't need to.
      //Instead, code related to these variables just shouldn't be executed.
      const_cast<CParameters&>(gParameters).SetMaximumTemporalClusterSize(50.0);
      const_cast<CParameters&>(gParameters).SetMaximumTemporalClusterSizeType(PERCENTAGETYPE);
      const_cast<CParameters&>(gParameters).SetIncludeClustersType(ALLCLUSTERS);
      const_cast<CParameters&>(gParameters).SetTimeAggregationUnitsType(NONE);
      const_cast<CParameters&>(gParameters).SetTimeAggregationLength(0);
      const_cast<CParameters&>(gParameters).SetTimeTrendAdjustmentType(TEMPORAL_NOTADJUSTED);
      const_cast<CParameters&>(gParameters).SetTimeTrendAdjustmentPercentage(0);
      return true;
    }
    //validate temporal cluster sizes
    if (!ValidateTemporalClusterSize(PrintDirection))
      bValid = false;
    //Prospective analyses include only alive clusters - reset this parameter
    //instead of reporting error since this parameter has under gone changes
    //that would make any error message confusing to user. Prospective analyses
    //should have reported this error when they were introduced to SaTScan, but
    //I think it's too late to start enforcing this.
    if (gParameters.GetIsProspectiveAnalysis() && gParameters.GetIncludeClustersType() != ALIVECLUSTERS)
      const_cast<CParameters&>(gParameters).SetIncludeClustersType(ALIVECLUSTERS);
    //validate time aggregation units
    if (!ValidateTimeAggregationUnits(PrintDirection))
      bValid = false;
    //validate the adjustment for weekly trends
    if (gParameters.getAdjustForWeeklyTrends()) {
        if (!(gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION ||
              gParameters.GetProbabilityModelType() == POISSON ||
              gParameters.GetProbabilityModelType() == NORMAL ||
              gParameters.GetProbabilityModelType() == EXPONENTIAL ||
              gParameters.GetProbabilityModelType() == RANK ||
              gParameters.GetProbabilityModelType() == ORDINAL ||
              gParameters.GetProbabilityModelType() == CATEGORICAL ||
              gParameters.GetProbabilityModelType() == BERNOULLI)) {
          bValid = false;
          PrintDirection.Printf("%s:\nThe adjustment for weekly trends is not implemented for the %s model.\n",
                                BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
        }
        if (gParameters.GetAnalysisType() == PURELYSPATIAL) {
          bValid = false;
          PrintDirection.Printf("%s:\nThe adjustment for weekly trends is not implemented for the %s analysis.\n",
                                BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
        }
        if (!(gParameters.GetTimeAggregationUnitsType() == DAY && gParameters.GetTimeAggregationLength() == 1)) {
          bValid = false;
          PrintDirection.Printf("%s:\nThe adjustment for weekly trends can only be performed with a time aggregation length of 1 day.\n",
                                BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
    }
    //validate time trend adjustment
    switch (gParameters.GetProbabilityModelType()) {
      case BERNOULLI            :
        if (gParameters.GetTimeTrendAdjustmentType() != TEMPORAL_NOTADJUSTED) {
            if (gParameters.GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION) {
                if (gParameters.GetIsPurelyTemporalAnalysis()) {
                    bValid = false;
                    PrintDirection.Printf("%s:\nThe non-parametric temporal adjustment by stratified randomization is not valid "
                        "for purely temporal analyses.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
                } else if (!gParameters.GetIsSpaceTimeAnalysis()) {
                    bValid = false;
                    PrintDirection.Printf("%s:\nThe non-parametric temporal adjustment by stratified randomization is valid "
                        "only for space-time analyses.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
                } /* else if (gParameters.GetSpatialAdjustmentType() != SPATIAL_NOTADJUSTED) {
                    bValid = false;
                    PrintDirection.Printf("%s:\nThe Bernoulli model does not permit the nonparametric temporal trends adjustment\n"
                        "in conjunction with the nonparametric spatial adjustment.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
                }*/
                if (gParameters.GetAreaScanRateType() != HIGH) {
                    bValid = false;
                    PrintDirection.Printf("%s:\nThe Bernoulli model permits the nonparametric temporal trends adjustment\n"
                        "in conjunction with high scanning rates only.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
                }
            } else {
                bValid = false;
                PrintDirection.Printf("%s:\nThe Bernoulli model permits only the nonparametric temporal trends adjustment. Other temporal trend adjustments are not implemented.\n",
                    BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
            }
        }
        break;
      case CATEGORICAL          :
      case ORDINAL              :
      case EXPONENTIAL          :
      case NORMAL               :
      case RANK                 :
      case UNIFORMTIME          :
        if (gParameters.GetTimeTrendAdjustmentType() != TEMPORAL_NOTADJUSTED) {
          PrintDirection.Printf("Notice:\nFor the %s model, adjusting for temporal trends is not permitted."
                                "Temporal trends adjustment settings will be ignored.\n",
                                BasePrint::P_NOTICE, ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
          const_cast<CParameters&>(gParameters).SetTimeTrendAdjustmentType(TEMPORAL_NOTADJUSTED);
          const_cast<CParameters&>(gParameters).SetTimeTrendAdjustmentPercentage(0);
        }
        break;
      case SPACETIMEPERMUTATION :
        if (gParameters.GetTimeTrendAdjustmentType() != TEMPORAL_NOTADJUSTED) {
          PrintDirection.Printf("Notice:\nFor the space-time permutation model, adjusting for temporal trends "
                                "is not permitted nor needed, as this model automatically adjusts for "
                                "any temporal variation. Temporal trends adjustment settings will be ignored.\n",
                                BasePrint::P_NOTICE);
          const_cast<CParameters&>(gParameters).SetTimeTrendAdjustmentType(TEMPORAL_NOTADJUSTED);
          const_cast<CParameters&>(gParameters).SetTimeTrendAdjustmentPercentage(0);
        }
        break;
      case POISSON             :
        if (gParameters.GetTimeTrendAdjustmentType() != TEMPORAL_NOTADJUSTED && gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
          bValid = false;
          PrintDirection.Printf("%s:\nTemporal adjustments can not be performed for a spatial variation in temporal trends analysis.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
        if (gParameters.GetTimeTrendAdjustmentType() != TEMPORAL_NOTADJUSTED && gParameters.GetIsPurelyTemporalAnalysis() && gParameters.GetPopulationFileName().empty()) {
          bValid = false;
          PrintDirection.Printf("%s:\nTemporal adjustments can not be performed for a purely temporal analysis "
                                "using the Poisson model, when no population file has been specfied.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
        if (gParameters.GetTimeTrendAdjustmentType() == TEMPORAL_NONPARAMETRIC && gParameters.GetIsPurelyTemporalAnalysis()) {
          bValid = false;
          PrintDirection.Printf("%s:\nThe non-parametric time trend adjustment cannot be used with a purely temporal analysis.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
        if (gParameters.GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION && gParameters.GetIsPurelyTemporalAnalysis()) {
          bValid = false;
          PrintDirection.Printf("%s:\nThe non-parametric temporal adjustment by stratified randomization is not valid "
                                 "for purely temporal analyses.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
        if (gParameters.GetTimeTrendAdjustmentType() == LOGLINEAR_PERC && -100.0 >= gParameters.GetTimeTrendAdjustmentPercentage()) {
          bValid = false;
          PrintDirection.Printf("%s:\nThe time adjustment percentage is '%2g', but must greater than -100.\n",
                                BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, gParameters.GetTimeTrendAdjustmentPercentage());
        }
        if (gParameters.GetTimeTrendAdjustmentType() == TEMPORAL_NOTADJUSTED) {
          const_cast<CParameters&>(gParameters).SetTimeTrendAdjustmentPercentage(0);
          if (gParameters.GetAnalysisType() != SPATIALVARTEMPTREND) const_cast<CParameters&>(gParameters).SetTimeTrendConvergence(0);
        }
        if (gParameters.GetTimeTrendAdjustmentType() == CALCULATED_LOGLINEAR_PERC && gParameters.GetTimeTrendConvergence() < 0.0) {
           bValid = false;
           PrintDirection.Printf("%s:\nTime trend convergence value of '%2g' is less than zero.\n",
                                 BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, gParameters.GetTimeTrendConvergence());
        }
        if ((gParameters.GetTimeTrendAdjustmentType() == TEMPORAL_NONPARAMETRIC || gParameters.GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION) &&
            gParameters.getAdjustForWeeklyTrends()) {
            bValid = false;
            PrintDirection.Printf("%s:\nThe non-parametric time trend adjustment cannot be used with the adjustment for weekly trends.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
        break;
      default : throw prg_error("Unknown model type '%d'.", "ValidateTemporalParameters()", gParameters.GetProbabilityModelType());
    }
    //validate including purely temporal clusters
    if (gParameters.GetIncludePurelyTemporalClusters()) {
      if (!gParameters.GetPermitsPurelyTemporalCluster(gParameters.GetProbabilityModelType())) {
          bValid = false;
          PrintDirection.Printf("%s\nScanning for purely temporal clusters can not be included when the %s model is used.\n",
                                BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
      }
      else if (!gParameters.GetPermitsPurelyTemporalCluster()) {
        bValid = false;
        PrintDirection.Printf("%s:\nA purely temporal cluster can not be included with %s analyses.\n",
                              BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, ParametersPrint(gParameters).GetAnalysisTypeAsString());
      }
    }

    //the locations neighbor file is irrelevant when analysis type is purely temporal
    if (gParameters.GetIsPurelyTemporalAnalysis()) const_cast<CParameters&>(gParameters).UseLocationNeighborsFile(false);
    //since there is not location data, we can no report relative risk estimates per location
  } catch (prg_exception& x) {
    x.addTrace("ValidateTemporalParameters()","ParametersValidate");
    throw;
  }
  return bValid;
}

/** Validates the time aggregation units. */
bool ParametersValidate::ValidateTimeAggregationUnits(BasePrint& PrintDirection) const {
  std::string   sPrecisionString, sBuffer;
  double        dStudyPeriodLengthInUnits, dMaxTemporalLengthInUnits;

  if (gParameters.GetAnalysisType() == PURELYSPATIAL) //validate settings for temporal analyses
    return true;

  //get date precision string for error reporting
  GetDatePrecisionAsString(gParameters.GetTimeAggregationUnitsType(), sPrecisionString, false, false);
  if (gParameters.GetTimeAggregationUnitsType() == NONE) { //validate time aggregation units
    PrintDirection.Printf("%s:\nTime aggregation units can not be 'none' for a temporal analysis.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
    return false;
  }
  if (gParameters.GetTimeAggregationUnitsType() > gParameters.GetPrecisionOfTimesType()) {
    GetDatePrecisionAsString(gParameters.GetTimeAggregationUnitsType(), sPrecisionString, true, false);
    GetDatePrecisionAsString(gParameters.GetPrecisionOfTimesType(), sBuffer, true, false);
    PrintDirection.Printf("%s:\nA time aggregation unit in %s exceeds precision of input case data (unit is %s).\n",
                          BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, sPrecisionString.c_str(), sBuffer.c_str());
    return false;
  }
  if (gParameters.GetTimeAggregationLength() <= 0) {
    PrintDirection.Printf("%s:\nThe time aggregation length of '%d' is invalid. Length must be greater than zero.\n",
                          BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, gParameters.GetTimeAggregationLength());
    return false;
  }
  //validate that the time aggregation length agrees with the study period and maximum temporal cluster size
  dStudyPeriodLengthInUnits = ceil(CalculateNumberOfTimeIntervals(DateStringParser::getDateAsJulian(gParameters.GetStudyPeriodStartDate().c_str(), gParameters.GetPrecisionOfTimesType()),
                                                                  DateStringParser::getDateAsJulian(gParameters.GetStudyPeriodEndDate().c_str(), gParameters.GetPrecisionOfTimesType()),
                                                                  gParameters.GetTimeAggregationUnitsType(), 1));
  if (dStudyPeriodLengthInUnits < static_cast<double>(gParameters.GetTimeAggregationLength()))  {
    PrintDirection.Printf("%s:\nA time aggregation of %d %s%s is greater than the %d %s study period.\n",
                          BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, gParameters.GetTimeAggregationLength(), sPrecisionString.c_str(),
                          (gParameters.GetTimeAggregationLength() == 1 ? "" : "s"),
                          static_cast<int>(dStudyPeriodLengthInUnits), sPrecisionString.c_str());
    return false;
  }
  if (ceil(dStudyPeriodLengthInUnits/static_cast<double>(gParameters.GetTimeAggregationLength())) <= 1) {
    PrintDirection.Printf("%s:\nA time aggregation of %d %s%s with a %d %s study period results in only "
                          "one time period to analyze. Temporal and space-time analyses can not be performed "
                          "on less than two time periods.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM,
                          gParameters.GetTimeAggregationLength(), sPrecisionString.c_str(), (gParameters.GetTimeAggregationLength() == 1 ? "" : "s"),
                          static_cast<int>(dStudyPeriodLengthInUnits), sPrecisionString.c_str());
    return false;
  }

  if (gParameters.getAdjustForWeeklyTrends() && gParameters.GetTimeAggregationUnitsType() == DAY && dStudyPeriodLengthInUnits < 14.0) {
    PrintDirection.Printf("%s:\nThe adjustment for day of week cannot be performed on a period less than 14 days.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
    return false;
  }

  if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) //svtt does not have a maximum temporal cluster size
    return true;

  if (gParameters.GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE)
    dMaxTemporalLengthInUnits = floor(dStudyPeriodLengthInUnits * (double)gParameters.GetMaximumTemporalClusterSize()/100.0);
  else if (gParameters.GetMaximumTemporalClusterSizeType() == TIMETYPE)
    dMaxTemporalLengthInUnits = (double)gParameters.GetMaximumTemporalClusterSize();

  //validate the time aggregation agrees with maximum temporal cluster size
  if (static_cast<int>(floor(dMaxTemporalLengthInUnits /static_cast<double>(gParameters.GetTimeAggregationLength()))) == 0) {
    if (gParameters.GetMaximumTemporalClusterSizeType() == TIMETYPE)
      PrintDirection.Printf("%s:\nThe time aggregation of %d %s%s is greater than the maximum temporal "
                            "cluster size of %g %s%s.\n",
                            BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, gParameters.GetTimeAggregationLength(),
                            sPrecisionString.c_str(), (gParameters.GetTimeAggregationLength() == 1 ? "" : "s"),
                            gParameters.GetMaximumTemporalClusterSize(), sPrecisionString.c_str(),
                            (gParameters.GetMaximumTemporalClusterSize() == 1 ? "" : "s"));
    else if (gParameters.GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE)
      PrintDirection.Printf("%s:\nWith the maximum temporal cluster size as %g percent of a %d %s study period, "
                            "the time aggregation as %d %s%s is greater than the resulting maximum "
                            "temporal cluster size of %g %s%s.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM,
                            gParameters.GetMaximumTemporalClusterSize(), static_cast<int>(dStudyPeriodLengthInUnits),
                            sPrecisionString.c_str(), gParameters.GetTimeAggregationLength(),
                            sPrecisionString.c_str(), (gParameters.GetTimeAggregationLength() == 1 ? "" : "s"),
                            dMaxTemporalLengthInUnits, sPrecisionString.c_str(),
                            (dMaxTemporalLengthInUnits == 1 ? "" : "s"));
    return false;
  }

  if (gParameters.GetAnalysisType() == SEASONALTEMPORAL) {
      /* Check that time aggregration length is less than defined maximum for unit type. */
      switch (gParameters.GetTimeAggregationUnitsType()) {
        case DAY: 
            if (gParameters.GetTimeAggregationLength() > 90) {
                PrintDirection.Printf("%s:\nThe time aggregation may not exceed 90 days for the Seasonal analysis.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
                return false;
            } break;
        case MONTH:
            if (gParameters.GetTimeAggregationLength() > 3) {
                PrintDirection.Printf("%s:\nThe time aggregation may not exceed 3 months for the Seasonal analysis.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
                return false;
            } break;
        case GENERIC: 
            if (gParameters.GetTimeAggregationLength() > static_cast<long>(std::floor(0.25 * dStudyPeriodLengthInUnits))) {
                PrintDirection.Printf("%s:\nThe time aggregation may not exceed %ld units for the Seasonal analysis.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, static_cast<long>(std::floor(0.25 * dStudyPeriodLengthInUnits)));
                return false;
            } break;
        case YEAR :
            PrintDirection.Printf("%s:\nThe time aggregation in years is not implemented for the Seasonal analysis.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, static_cast<long>(std::floor(0.25 * dStudyPeriodLengthInUnits)));
            return false;
        default: throw prg_error("Unknown time aggregation type '%d'.", "ValidateTimeAggregationUnits()", gParameters.GetTimeAggregationUnitsType());
      }
  }

  return true;
}

