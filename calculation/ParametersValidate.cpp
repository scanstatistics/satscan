//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ParametersValidate.h"
#include "RandomNumberGenerator.h"
#include "Randomizer.h"

/** constructor */
ParametersValidate::ParametersValidate(const CParameters& Parameters) : gParameters(Parameters) {}

/** destructor */
ParametersValidate::~ParametersValidate() {}

/** Validates that given current state of settings, parameters and their relationships
    with other parameters are correct. Errors are sent to print direction and*/
bool ParametersValidate::Validate(BasePrint& PrintDirection) const {
  bool         bValid=true;

  try {
    //before version 3, there were no restrictions for secondary clusters
    if (gParameters.GetCreationVersion().iMajor < 3)
      const_cast<CParameters&>(gParameters).SetCriteriaForReportingSecondaryClusters(NORESTRICTIONS);
    //before version 6, critical values were always reported
    if (gParameters.GetCreationVersion().iMajor < 6)
      const_cast<CParameters&>(gParameters).SetReportCriticalValues(true);

    if (gParameters.GetValidatingParameters()) {
      //prevent access to Spatial Variation and Temporal Trends analysis -- still in development
      if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
        bValid = false;
        PrintDirection.Printf("Error: Please note that spatial variation in temporal trends analysis is not implemented\n"
                             "       in this version of SaTScan.\n", BasePrint::P_ERROR);
      }
      if (!ValidateMonotoneRisk(PrintDirection))
        bValid = false;
      if (gParameters.GetProbabilityModelType() == ORDINAL && gParameters.GetNumDataSets() > 1 && gParameters.GetMultipleDataSetPurposeType() == ADJUSTMENT) {
        bValid = false;
        PrintDirection.Printf("Error: Adjustment purpose for multiple data sets is not permitted\n"
                              "       with ordinal probability model in this version of SaTScan.\n", BasePrint::P_ERROR);
      }
      if (gParameters.GetExecutionType() == CENTRICALLY && gParameters.GetTerminateSimulationsEarly()) {
        bValid = false;
        PrintDirection.Printf("Error: The early termination of simulations option can not be applied\n"
                              "       with the centric analysis execution.\n", BasePrint::P_ERROR);
      }
      if (gParameters.GetExecutionType() == CENTRICALLY &&
          (gParameters.GetIsPurelyTemporalAnalysis() ||
           gParameters.GetAnalysisType() == SPATIALVARTEMPTREND ||
           (gParameters.GetAnalysisType() == PURELYSPATIAL && gParameters.GetRiskType() == MONOTONERISK))) {
        bValid = false;
        PrintDirection.Printf("Error: The centric analysis execution is not available for:\n"
                              "       purely temporal analyses\n"
                              "       purely spatial analyses with isotonic scan\n"
                              "       spatial variation of temporal trends analysis\n", BasePrint::P_ERROR);
      }

      //validate dates
      if (! ValidateDateParameters(PrintDirection))
        bValid = false;
      else {
        //Validate temporal options only if date parameters are valid. Some
        //temporal parameters can not be correctly validated if dates are not valid.
        if (! ValidateTemporalParameters(PrintDirection))
           bValid = false;
      }

      //validate spatial options
      if (! ValidateSpatialParameters(PrintDirection))
        bValid = false;

      //validate number of replications requested
      if (!(gParameters.GetNumReplicationsRequested() == 0 ||
            gParameters.GetNumReplicationsRequested() == 9 ||
            gParameters.GetNumReplicationsRequested() == 19 ||
            fmod(gParameters.GetNumReplicationsRequested() + 1, 1000) == 0.0)) {
        bValid = false;
        PrintDirection.Printf("Error: Invalid number of replications '%u'. The value must be 0, 9, 999, or n999.\n",
                              BasePrint::P_ERROR, gParameters.GetNumReplicationsRequested());
      }

      //validate input/oupt files
      if (! ValidateFileParameters(PrintDirection))
        bValid = false;

      //validate model parameters
      if (gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION) {
        if (!(gParameters.GetAnalysisType() == SPACETIME || gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)) {
          bValid = false;
          PrintDirection.Printf("Error: For the %s model, the analysis type must be either Retrospective or Prospective Space-Time.\n",
                                BasePrint::P_ERROR, gParameters.GetProbabilityModelTypeAsString(gParameters.GetProbabilityModelType()));
        }
        if (gParameters.GetOutputRelativeRisksAscii() || gParameters.GetOutputRelativeRisksDBase()) {
          bValid = false;
          PrintDirection.Printf("Error: The relative risks output files can not be produced for the %s model.\n",
                                BasePrint::P_ERROR, gParameters.GetProbabilityModelTypeAsString(gParameters.GetProbabilityModelType()));
        }
      }
      //validate range parameters
      if (! ValidateRangeParameters(PrintDirection))
        bValid = false;

      //validate sequential scan parameters
      if (! ValidateSequentialScanParameters(PrintDirection))
        bValid = false;

      //validate power calculation parameters
      if (! ValidatePowerCalculationParameters(PrintDirection))
        bValid = false;

      //validate ellipse parameters
      if (! ValidateEllipseParameters(PrintDirection))
        bValid = false;

      //validate simulation options
      if (! ValidateSimulationDataParameters(PrintDirection))
        bValid = false;

      if (! ValidateRandomizationSeed(PrintDirection))
        bValid = false;
    }
    else {
      PrintDirection.Printf("Notice: Parameters will not be validated, in accordance with the setting of the validation\n"
                            "        parameter in the parameters file.\n"
                            "        This may have adverse effects on analysis results and/or program operation.\n\n",
                            BasePrint::P_NOTICE);
    }

  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateParameters()","ParametersValidate");
    throw;
  }
  return bValid;
}

/** Validates date parameters based upon current settings. Error messages
    sent to print direction object and indication of valid settings returned. */
bool ParametersValidate::ValidateDateParameters(BasePrint& PrintDirection) const {
  bool          bValid=true, bStartDateValid=true, bEndDateValid=true, bProspectiveDateValid=true;
  Julian        StudyPeriodStartDate, StudyPeriodEndDate, ProspectiveStartDate;

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
    if (gParameters.GetIsProspectiveAnalysis() && gParameters.GetAdjustForEarlierAnalyses() && !ValidateProspectiveDate(PrintDirection)) {
      bValid = false;
      bProspectiveDateValid = false;
    }

    if (bStartDateValid && bEndDateValid) {
      //check that study period start and end dates are chronologically correct
      StudyPeriodStartDate = CharToJulian(gParameters.GetStudyPeriodStartDate().c_str());
      StudyPeriodEndDate = CharToJulian(gParameters.GetStudyPeriodEndDate().c_str());
      if (StudyPeriodStartDate > StudyPeriodEndDate) {
        bValid = false;
        PrintDirection.Printf("Error: The study period start date occurs after the end date.\n", BasePrint::P_ERROR);
      }
      if (bValid && gParameters.GetIsProspectiveAnalysis() && gParameters.GetAdjustForEarlierAnalyses() && bProspectiveDateValid) {
        //validate prospective start date
        ProspectiveStartDate = CharToJulian(gParameters.GetProspectiveStartDate().c_str());
        if (ProspectiveStartDate < StudyPeriodStartDate || ProspectiveStartDate > StudyPeriodEndDate) {
          bValid = false;
          PrintDirection.Printf("Error: The start date of prospective surveillance does not occur within\n"
                                "       the study period.\n", BasePrint::P_ERROR);
        }
      }
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateDateParameters()","ParametersValidate");
    throw;
  }                                                         
  return bValid;
}

/** Validates ellipse parameters if number of ellipses greater than zero.
    Errors printed to print direction and return whether parameters are valid. */
bool ParametersValidate::ValidateEllipseParameters(BasePrint & PrintDirection) const {
  bool          bValid=true;
  size_t        t;

  try {
    if (gParameters.GetSpatialWindowType() != ELLIPTIC) return true;
    
    if (gParameters.GetNumRequestedEllipses() < 1 || gParameters.GetNumRequestedEllipses() > CParameters::MAXIMUM_ELLIPSOIDS) {
      bValid = false;
      PrintDirection.Printf("Error: The number of requested ellipses '%d' is not within allowable range of 1 - %d.\n",
                            BasePrint::P_ERROR, gParameters.GetNumRequestedEllipses(), CParameters::MAXIMUM_ELLIPSOIDS);
    }
    //analyses with ellipses can not be performed with coordinates defiend in latitude/longitude system (currently)
    if (gParameters.GetCoordinatesType() == LATLON) {
      bValid = false;
      PrintDirection.Printf("Error: Invalid parameter setting for ellipses.\n"
                            "       SaTScan does not support lat/long coordinates when ellipses are used.\n"
                            "       Please use the Cartesian coordinate system.\n", BasePrint::P_ERROR);
    }
    if (gParameters.GetEllipseShapes().size() != gParameters.GetEllipseRotations().size()) {
      bValid = false;
      PrintDirection.Printf("Error: Invalid parameters; settings indicate %i elliptic shapes\n"
                            "       but %i variables for respective angles of rotation.\n",
                            BasePrint::P_ERROR, gParameters.GetEllipseShapes().size(), gParameters.GetEllipseRotations().size());
    }
    for (t=0; t < gParameters.GetEllipseShapes().size(); ++t)
       if (gParameters.GetEllipseShapes()[t] < 1) {
         bValid = false;
         PrintDirection.Printf("Error: Invalid parameter setting, ellipse shape '%g' is invalid.\n"
                               "       The shape can not be less than one.\n",
                               BasePrint::P_ERROR, gParameters.GetEllipseShapes()[t]);
       }
    for (t=0; t < gParameters.GetEllipseRotations().size(); t++)
       if (gParameters.GetEllipseRotations()[t] < 1) {
         bValid = false;
         PrintDirection.Printf("Error: Invalid parameter setting. The number of angles, '%d', is invalid.\n"
                               "       The number can not be less than one.\n",
                               BasePrint::P_ERROR, gParameters.GetEllipseRotations()[t]);
       }
    if (gParameters.GetExecutionType() == CENTRICALLY && gParameters.GetNonCompactnessPenaltyType() > NOPENALTY) {
      bValid = false;
      PrintDirection.Printf("Error: The non-compactness penalty for elliptic scans can not be applied\n"
                            "       with the centric analysis execution.\n", BasePrint::P_ERROR);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateEllipseParameters()","ParametersValidate");
    throw;
  }
  return bValid;
}

/** Validates input/output file parameters. */
bool ParametersValidate::ValidateFileParameters(BasePrint& PrintDirection) const {
  bool          bValid=true;
  size_t        t;

  try {
    //validate case file
    if (!gParameters.GetCaseFileNames().size()) {
      bValid = false;
      PrintDirection.Printf("Error: No case file was specified.\n", BasePrint::P_ERROR);
    }
    for (t=0; t < gParameters.GetCaseFileNames().size(); ++t) {
       if (access(gParameters.GetCaseFileNames()[t].c_str(), 00)) {
         bValid = false;
         PrintDirection.Printf("Error: The case file '%s' does not exist.\n"
                               "       Please ensure the path is correct.\n",
                               BasePrint::P_ERROR, gParameters.GetCaseFileNames()[t].c_str());
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
            PrintDirection.Printf("Error: For the Poisson model with purely temporal analyses, the population file\n"
                                  "       is optional but all data sets must either specify a population file or omit it.\n",
                                  BasePrint::P_ERROR);
         }
         else if (!iNumDataSetsWithoutPopFile) {
           const_cast<CParameters&>(gParameters).SetPopulationFile(true);
           for (t=0; t < gParameters.GetPopulationFileNames().size(); ++t) {
              if (access(gParameters.GetPopulationFileNames()[t].c_str(), 00)) {
                bValid = false;
                PrintDirection.Printf("Error: The population file '%s' does not exist.\n"
                                      "       Please ensure the path is correct.\n",
                                      BasePrint::P_ERROR, gParameters.GetPopulationFileNames()[t].c_str());
              }
           }
         }
      }
      else {
        const_cast<CParameters&>(gParameters).SetPopulationFile(true);
        if (!gParameters.GetPopulationFileNames().size()) {
          bValid = false;
          PrintDirection.Printf("Error: For the Poisson model, a population file must be specified unless analysis\n"
                                "       is purely temporal. In which case the population file is optional.\n", BasePrint::P_ERROR);
        }
        for (t=0; t < gParameters.GetPopulationFileNames().size(); ++t) {
          if (access(gParameters.GetPopulationFileNames()[t].c_str(), 00)) {
            bValid = false;
            PrintDirection.Printf("Error: The population file '%s' does not exist.\n"
                                  "       Please ensure the path is correct.\n",
                                  BasePrint::P_ERROR, gParameters.GetPopulationFileNames()[t].c_str());
          }
        }
      }
    }
    //validate control file for a bernoulli model.
    if (gParameters.GetProbabilityModelType() == BERNOULLI) {
      if (!gParameters.GetControlFileNames().size()) {
        bValid = false;
        PrintDirection.Printf("Error: For the Bernoulli model, a Control file must be specified.\n", BasePrint::P_ERROR);
      }
      for (t=0; t < gParameters.GetControlFileNames().size(); ++t) {
        if (access(gParameters.GetControlFileNames()[t].c_str(), 00)) {
          bValid = false;
          PrintDirection.Printf("Error: The control file '%s' does not exist.\n"
                                "       Please ensure the path is correct.\n",
                                BasePrint::P_ERROR, gParameters.GetControlFileNames()[t].c_str());
        }
      }
    }
    //validate coordinates file
    if (gParameters.GetCoordinatesFileName().empty()) {
      if (!gParameters.GetIsPurelyTemporalAnalysis()) {
        bValid = false;
        PrintDirection.Printf("Error: No coordinates file specified.\n", BasePrint::P_ERROR);
      }
    }  
    else if (access(gParameters.GetCoordinatesFileName().c_str(), 00)) {
      bValid = false;
      PrintDirection.Printf("Error: The coordinates file '%s' does not exist.\n"
                             "       Please ensure the path is correct.\n",
                             BasePrint::P_ERROR, gParameters.GetCoordinatesFileName().c_str());
    }
    //validate special grid file
    if (gParameters.GetIsPurelyTemporalAnalysis())
      const_cast<CParameters&>(gParameters).SetUseSpecialGrid(false);
    else if (gParameters.UseSpecialGrid() && gParameters.GetSpecialGridFileName().empty()) {
      bValid = false;
      PrintDirection.Printf("Error: The settings indicate to the use a grid file, but a grid file name is not specified.\n", BasePrint::P_ERROR);
    }
    else if (gParameters.UseSpecialGrid() && access(gParameters.GetSpecialGridFileName().c_str(), 00)) {
      bValid = false;
      PrintDirection.Printf("Error: The grid file '%s' does not exist.\n"
                            "       Please ensure the path is correct.\n",
                            BasePrint::P_ERROR, gParameters.GetSpecialGridFileName().c_str());
    }
    //validate adjustment for known relative risks file
    if (gParameters.GetProbabilityModelType() == POISSON) {
      if (gParameters.UseAdjustmentForRelativeRisksFile() && gParameters.GetAdjustmentsByRelativeRisksFilename().empty()) {
        bValid = false;
        PrintDirection.Printf("Error: The settings indicate to the use the adjustments file, but a file name not specified.\n", BasePrint::P_ERROR);
      }
      else if (gParameters.UseAdjustmentForRelativeRisksFile() && access(gParameters.GetAdjustmentsByRelativeRisksFilename().c_str(), 00)) {
        bValid = false;
        PrintDirection.Printf("Error: The adjustments file '%s' does not exist.\n"
                              "       Please ensure the path is correct.\n",
                              BasePrint::P_ERROR, gParameters.GetAdjustmentsByRelativeRisksFilename().c_str());
      }
    }
    else
      const_cast<CParameters&>(gParameters).SetUseAdjustmentForRelativeRisksFile(false);

    //validate maximum circle population file for a prospective space-time analysis w/ maximum geographical cluster size
    //defined as a percentage of the population and adjusting for earlier analyses.
    if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME &&
        gParameters.GetAdjustForEarlierAnalyses() &&
        gParameters.GetMaxGeographicClusterSizeType() == PERCENTOFPOPULATION) {
      bValid = false;
      PrintDirection.Printf("Error: For a prospective space-time analysis adjusting for ealier analyses, the maximum spatial\n"
                            "       cluster size must be defined as a percentage of the population as defined in a max\n"
                            "       circle size file.\n"
                            "       Alternatively you may choose to specify the maximum as a fixed radius, in which case a\n"
                            "       max circle size file is not required.\n", BasePrint::P_ERROR);
    }
    if (gParameters.GetMaxGeographicClusterSizeType() == PERCENTOFMAXCIRCLEFILE ||
        (gParameters.GetRestrictingMaximumReportedGeoClusterSize() && gParameters.GetMaxReportedGeographicClusterSizeType() == PERCENTOFMAXCIRCLEFILE)) {
      if (gParameters.GetMaxCirclePopulationFileName().empty() && gParameters.GetMaxGeographicClusterSizeType() == PERCENTOFMAXCIRCLEFILE) {
        bValid = false;
        PrintDirection.Printf("Error: The settings indicate to the use a max circle size file, but a file name was not specified.\n", BasePrint::P_ERROR);
      }
      else if (gParameters.GetMaxCirclePopulationFileName().empty() &&
               gParameters.GetRestrictingMaximumReportedGeoClusterSize() && gParameters.GetMaxReportedGeographicClusterSizeType() == PERCENTOFMAXCIRCLEFILE) {
        bValid = false;
        PrintDirection.Printf("Error: Maximum circle size file name was not specified.\n"
                              "       A maximum circle file is required when restricting the maximum\n"
                              "       reported spatial cluster size by a population defined in that\n"
                              "       maximum circle file.\n", BasePrint::P_ERROR);
      }
      else if (access(gParameters.GetMaxCirclePopulationFileName().c_str(), 00)) {
        bValid = false;
        PrintDirection.Printf("Error: The max circle size file '%s' does not exist.\n"
                              "       Please ensure the path is correct.\n",
                              BasePrint::P_ERROR, gParameters.GetMaxCirclePopulationFileName().c_str());
      }
    }
    //validate output file
    if (gParameters.GetOutputFileName().empty()) {
      bValid = false;
      PrintDirection.Printf("Error: No results file specified.\n", BasePrint::P_ERROR);
    }
    else if (access(ZdFileName(gParameters.GetOutputFileName().c_str()).GetLocation(), 00)) {
      bValid = false;
      PrintDirection.Printf("Error: Results file '%s' have an invalid path.\n", BasePrint::P_ERROR, gParameters.GetOutputFileName().c_str());
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateFileParameters()","ParametersValidate");
    throw;
  }
  return bValid;
}

/** Validates the maximum temporal cluster size parameters. */
bool ParametersValidate::ValidateMaximumTemporalClusterSize(BasePrint& PrintDirection) const {
  ZdString      sPrecisionString;
  double        dStudyPeriodLengthInUnits, dMaxTemporalLengthInUnits;

  try {
    //Maximum temporal cluster size is parameters not used for these analyses.
    if (gParameters.GetAnalysisType() == PURELYSPATIAL || gParameters.GetAnalysisType() == SPATIALVARTEMPTREND)
      return true;

    if (gParameters.GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE) {
      //validate for maximum specified as percentage of study period
      if (gParameters.GetMaximumTemporalClusterSize() <= 0) {
        PrintDirection.Printf("Error: The maximum temporal cluster size of '%g' is invalid.\n"
                              "       Specifying the maximum as a percentage of the study period\n"
                              "       requires the value to be a decimal number that is greater than zero.\n",
                              BasePrint::P_ERROR, gParameters.GetMaximumTemporalClusterSize());
        return false;
      }
      //check maximum temporal cluster size(as percentage of population) is less than maximum for given probability model
      if (gParameters.GetMaximumTemporalClusterSize() > (gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION ? 50 : 90)) {
        PrintDirection.Printf("Error: For the %s model, the maximum temporal cluster size as a percent\n"
                              "       of the study period is %d percent.\n", BasePrint::P_ERROR,
                              gParameters.GetProbabilityModelTypeAsString(gParameters.GetProbabilityModelType()),
                              (gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION ? 50 : 90));
        return false;
      }
      //validate that the time aggregation length agrees with the study period and maximum temporal cluster size
      dStudyPeriodLengthInUnits = ceil(CalculateNumberOfTimeIntervals(CharToJulian(gParameters.GetStudyPeriodStartDate().c_str()),
                                                                      CharToJulian(gParameters.GetStudyPeriodEndDate().c_str()),
                                                                      gParameters.GetTimeAggregationUnitsType(), 1));
      dMaxTemporalLengthInUnits = floor(dStudyPeriodLengthInUnits * gParameters.GetMaximumTemporalClusterSize()/100.0);
      if (dMaxTemporalLengthInUnits < 1) {
        GetDatePrecisionAsString(gParameters.GetTimeAggregationUnitsType(), sPrecisionString, false, false);
        PrintDirection.Printf("Error: A maximum temporal cluster size as %g percent of a %d %s study period\n"
                              "       results in a maximum temporal cluster size that is less than one time\n"
                              "       aggregation %s.\n", BasePrint::P_ERROR,
                              gParameters.GetMaximumTemporalClusterSize(), static_cast<int>(dStudyPeriodLengthInUnits),
                              sPrecisionString.GetCString(), sPrecisionString.GetCString());
        return false;
      }
    }
    else if (gParameters.GetMaximumTemporalClusterSizeType() == TIMETYPE) {
      //validate for maximum specified as time aggregation unit 
      if (gParameters.GetMaximumTemporalClusterSize() < 1) {
        PrintDirection.Printf("Error: The maximum temporal cluster size of '%2g' is invalid.\n"
                              "       Specifying the maximum in time aggregation units requires\n"
                              "       the value to be a whole number that is greater than zero.\n",
                              BasePrint::P_ERROR, gParameters.GetMaximumTemporalClusterSize());
        return false;
      }
      GetDatePrecisionAsString(gParameters.GetTimeAggregationUnitsType(), sPrecisionString, false, false);
      dStudyPeriodLengthInUnits = ceil(CalculateNumberOfTimeIntervals(CharToJulian(gParameters.GetStudyPeriodStartDate().c_str()),
                                                                      CharToJulian(gParameters.GetStudyPeriodEndDate().c_str()),
                                                                      gParameters.GetTimeAggregationUnitsType(), 1));
      dMaxTemporalLengthInUnits = floor(dStudyPeriodLengthInUnits * (gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION ? 50 : 90)/100.0);
      if (gParameters.GetMaximumTemporalClusterSize() > dMaxTemporalLengthInUnits) {
        PrintDirection.Printf("Error: A maximum temporal cluster size of %d %s%s exceeds\n"
                              "       %d percent of a %d %s study period.\n"
                              "       Note that current settings limit the maximum to %d %s%s.\n",
                              BasePrint::P_ERROR, static_cast<int>(gParameters.GetMaximumTemporalClusterSize()),
                              sPrecisionString.GetCString(), (gParameters.GetMaximumTemporalClusterSize() == 1 ? "" : "s"),
                              (gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION ? 50 : 90),
                              static_cast<int>(dStudyPeriodLengthInUnits), sPrecisionString.GetCString(),
                              static_cast<int>(dMaxTemporalLengthInUnits), sPrecisionString.GetCString(),
                              (dMaxTemporalLengthInUnits == 1 ? "" : "s"));
        return false;
      }
    }
    else
      ZdException::GenerateNotification("Unknown temporal percentage type: %d.",
                                        "ValidateMaximumTemporalClusterSize()", gParameters.GetMaximumTemporalClusterSizeType());
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateMaximumTemporalClusterSize()","ParametersValidate");
    throw;
  }
  return true;
}

bool ParametersValidate::ValidateMonotoneRisk(BasePrint& PrintDirection) const {
  bool  bReturn=true;

  try {
    if (gParameters.GetRiskType() == MONOTONERISK) {
      if (gParameters.GetAnalysisType() != PURELYSPATIAL) {
        bReturn = false;
        PrintDirection.Printf("Error: The isotonic scan is implemented only for the purely spatial analysis.\n", BasePrint::P_ERROR);
      }
      if (gParameters.GetNumDataSets() > 1) {
        bReturn = false;
        PrintDirection.Printf("Error: Multiple data sets are not permitted with isotonic scan in this version of SaTScan.\n", BasePrint::P_ERROR);
      }
      if (!(gParameters.GetProbabilityModelType() == POISSON || gParameters.GetProbabilityModelType() == BERNOULLI)) {
        bReturn = false;
        PrintDirection.Printf("Error: The isotonic scan is implemented for only the Poisson and Bernoulli models.\n", BasePrint::P_ERROR);
      }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateMonotoneRisk()","ParametersValidate");
    throw;
  }
  return bReturn;
}

/** Validates power calculation parameters.
   Prints errors to print direction and returns validity. */
bool ParametersValidate::ValidatePowerCalculationParameters(BasePrint& PrintDirection) const {
  bool  bValid=true;

  try {
    if (gParameters.GetIsPowerCalculated()) {
      if (gParameters.GetNumReplicationsRequested() == 0) {
        PrintDirection.Printf("Error: The power calculation feature requires that Monte Carlo simulations be performed.\n", BasePrint::P_ERROR);
        return false;
      }
      if (0.0 > gParameters.GetPowerCalculationX() || gParameters.GetPowerCalculationX() > std::numeric_limits<double>::max()) {
        bValid = false;
        PrintDirection.Printf("Error: Invalid parameter setting for the power calculation value X.\n"
                              "       Please use a value between 0 and %12.4f\n",
                              BasePrint::P_ERROR, std::numeric_limits<double>::max());
      }
      if (0.0 > gParameters.GetPowerCalculationY() || gParameters.GetPowerCalculationY() > std::numeric_limits<double>::max()) {
        bValid = false;
        PrintDirection.Printf("Error: Invalid parameter setting for the power calculation value Y.\n"
                              "       Please use a value between 0 and %12.4f\n",
                              BasePrint::P_ERROR, std::numeric_limits<double>::max());
      }
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidatePowerCalculationParameters()","ParametersValidate");
    throw;
  }
  return bValid;
}

/** Validates prospective surveillance start date by:
    - if not adjusting for earlier analyses, ensures that prospective start
      date equals the study period end date.
    - calls method ValidateEndDate(...)                                       */
bool ParametersValidate::ValidateProspectiveDate(BasePrint& PrintDirection) const {
  UInt          uiYear, uiMonth, uiDay;
  bool          bReturnValue=true;
  ZdString      sDate;

  try {
    //validate study period end date based upon precision of times parameter setting
    //parse date in parts
    if (CharToMDY(&uiMonth, &uiDay, &uiYear, gParameters.GetProspectiveStartDate().c_str()) != 3) {
      PrintDirection.Printf("Error: The specified prospective surveillance start date, '%s', is not valid.\n"
                            "       Please specify as YYYY/MM/DD.\n", BasePrint::P_ERROR, gParameters.GetProspectiveStartDate().c_str());
      return false;
    }
    //validate date
    if (!IsDateValid(uiMonth, uiDay, uiYear)) {
      PrintDirection.Printf("Error: The specified prospective surveillance start date, %s, is not a valid date.\n",
                            BasePrint::P_ERROR, gParameters.GetProspectiveStartDate().c_str());
      bReturnValue = false;
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateProspectiveDate()","ParametersValidate");
    throw;
  }
  return bReturnValue;
}

/** Validates parameters parameters related to randomization seed.
    Prints errors to print direction and returns whether values are vaild. */
bool ParametersValidate::ValidateRandomizationSeed(BasePrint& PrintDirection) const {
  double        dMaxRandomizationSeed, dMaxReplications, dMaxSeed;

  if (gParameters.GetNumReplicationsRequested()) {
    //validate hidden parameter which specifies randomization seed
    if (!(0 < gParameters.GetRandomizationSeed() && gParameters.GetRandomizationSeed() < RandomNumberGenerator::glM)) {
      PrintDirection.Printf("Error: Randomization seed out of range [1 - %ld].\n",
                            BasePrint::P_ERROR, RandomNumberGenerator::glM - 1);
      return false;
    }

    //validate that generated seeds during randomization will not exceed defined range
    // - note that though the number of data sets has a bearing on the maximum seed, but we
    //   will not indicate to user to reduce the number of data sets in order to correct issues.
    dMaxRandomizationSeed = (double)gParameters.GetRandomizationSeed() + (double)gParameters.GetNumReplicationsRequested() + (double)(gParameters.GetNumDataSets() -1) * AbstractRandomizer::glDataSetSeedOffSet;

    if (dMaxRandomizationSeed >= static_cast<double>(RandomNumberGenerator::glM)) {
      //case #1 - glRandomizationSeed == RandomNumberGenerator::glDefaultSeed
      //    In this case, it is assumed that user accepted default - at this time
      //    changing the initial seed through parameter settings is a 'hidden' parameter;
      //    so no direction should be given regarding the alteration of seed value.
      if (gParameters.GetRandomizationSeed() == RandomNumberGenerator::glDefaultSeed) {
        dMaxReplications = (double)RandomNumberGenerator::glM - (double)gParameters.GetRandomizationSeed() - (double)(gParameters.GetNumDataSets() -1) * AbstractRandomizer::glDataSetSeedOffSet;
        dMaxReplications = (floor((dMaxReplications)/1000) - 1)  * 1000 + 999;
        PrintDirection.Printf("Error: Requested number of replications causes randomization seed to exceed defined limit.\n"
                              "       Maximum number of replications is %.0lf.\n", BasePrint::P_ERROR, dMaxReplications);
        return false;
      }
      //case #2 - user specified alternate randomization seed
      //    This alternate seed or the number of requested replications could be the problem.
      //    User has two options, either pick a lesser seed or request less replications.
      //calculate maximum seed for requested number of replications
      dMaxSeed = (double)RandomNumberGenerator::glM - (double)gParameters.GetNumReplicationsRequested() - (double)(gParameters.GetNumDataSets() -1) * AbstractRandomizer::glDataSetSeedOffSet - 1;
      //calculate maximum number of replications for requested seed
      dMaxReplications = (double)RandomNumberGenerator::glM - (double)gParameters.GetRandomizationSeed() - (double)(gParameters.GetNumDataSets() -1) * AbstractRandomizer::glDataSetSeedOffSet;
      dMaxReplications = (floor((dMaxReplications)/1000) - 1)  * 1000 + 999;
      //check whether specified combination of seed and requested number of replications fights each other 
      if (dMaxReplications < 9 && (dMaxSeed <= 0 || dMaxSeed > RandomNumberGenerator::glM)) {
        PrintDirection.Printf("Error: Randomization seed will exceed defined limit.\n"
                              "       The specified initial seed, in conjunction with the number of replications,\n"
                              "       contend for numerical range in defined limits. Please modify the specified\n"
                              "       initial seed and/or lessen the number of replications and try again.\n", BasePrint::P_ERROR);
      }
      //check that randomization seed is not so large that we can't run any replications
      else if (dMaxReplications < 9) {
        PrintDirection.Printf("Error: Randomization seed will exceed defined limit.\n"
                              "       The intial seed specified prevents any replications from being performed.\n"
                              "       With %ld replications, the initial seed can be [0 - %.0lf].\n",
                              BasePrint::P_ERROR, gParameters.GetNumReplicationsRequested(), dMaxSeed);
      }
      //check that number of replications isn't too large
      else if (dMaxSeed <= 0 || dMaxSeed > RandomNumberGenerator::glM) {
        PrintDirection.Printf("Error: Requested number of replications causes randomization seed to exceed defined limit.\n"
                              "       With initial seed of %i, maximum number of replications is %.0lf.\n",
                              BasePrint::P_ERROR, gParameters.GetRandomizationSeed(), dMaxReplications);
      }
      else {
        PrintDirection.Printf("Error: Randomization seed will exceed defined limit.\n"
                              "       Either limit the number of replications to %.0lf or\n"
                              "       define the initial seed to a value less than %.0lf.\n",
                              BasePrint::P_ERROR, dMaxReplications, dMaxSeed);
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
      if (CharToMDY(&uiMonth, &uiDay, &uiYear, gParameters.GetStartRangeStartDate().c_str()) != 3) {
        PrintDirection.Printf("Error: The start date of start range in flexible temporal window definition,\n"
                              "       '%s', is not valid. Please specify as YYYY/MM/DD.\n",
                              BasePrint::P_ERROR, gParameters.GetStartRangeStartDate().c_str());
        bValid = false;
      }
      if (!IsDateValid(uiMonth, uiDay, uiYear)) {
        PrintDirection.Printf("Error: The start date of start range in flexible temporal window definition,\n"
                              "       %s, is not a valid date.\n", BasePrint::P_ERROR, gParameters.GetStartRangeStartDate().c_str());
        bValid = false;
      }
      //validate start range end date
      if (CharToMDY(&uiMonth, &uiDay, &uiYear, gParameters.GetStartRangeEndDate().c_str()) != 3) {
        PrintDirection.Printf("Error: The end date of start range in flexible temporal window definition,\n"
                              "       '%s', is not valid. Please specify as YYYY/MM/DD.\n",
                              BasePrint::P_ERROR, gParameters.GetStartRangeEndDate().c_str());
        bValid = false;
      }
      if (!IsDateValid(uiMonth, uiDay, uiYear)) {
        PrintDirection.Printf("Error: The end date of start range in flexible temporal window definition,\n"
                              "       %s, is not a valid date.\n", BasePrint::P_ERROR, gParameters.GetStartRangeEndDate().c_str());
        bValid = false;
      }
      //validate end range start date
      if (CharToMDY(&uiMonth, &uiDay, &uiYear, gParameters.GetEndRangeStartDate().c_str()) != 3) {
        PrintDirection.Printf("Error: The start date of end range in flexible temporal window definition,\n"
                              "       '%s', is not valid. Please specify as YYYY/MM/DD.\n",
                              BasePrint::P_ERROR, gParameters.GetEndRangeStartDate().c_str());
        bValid = false;
      }
      if (!IsDateValid(uiMonth, uiDay, uiYear)) {
        PrintDirection.Printf("Error: The start date of end range in flexible temporal window definition,\n"
                              "       %s, is not a valid date.\n", BasePrint::P_ERROR, gParameters.GetEndRangeStartDate().c_str());
        bValid = false;
      }
      //validate end range end date
      if (CharToMDY(&uiMonth, &uiDay, &uiYear, gParameters.GetEndRangeStartDate().c_str()) != 3) {
        PrintDirection.Printf("Error: The end date of end range in flexible temporal window definition,\n"
                              "       '%s', is not valid. Please specify as YYYY/MM/DD.\n",
                              BasePrint::P_ERROR, gParameters.GetEndRangeStartDate().c_str());
        bValid = false;
      }
      if (!IsDateValid(uiMonth, uiDay, uiYear)) {
        PrintDirection.Printf("Error: The end date of end range in flexible temporal window definition,\n"
                              "       %s, is not a valid date.\n", BasePrint::P_ERROR, gParameters.GetEndRangeStartDate().c_str());
        bValid = false;
      }
      //now valid that range dates are within study period start and end dates
      if (bValid) {
        StudyPeriodStartDate = CharToJulian(gParameters.GetStudyPeriodStartDate().c_str());
        StudyPeriodEndDate = CharToJulian(gParameters.GetStudyPeriodEndDate().c_str());

        EndRangeStartDate = CharToJulian(gParameters.GetEndRangeStartDate().c_str());
        EndRangeEndDate = CharToJulian(gParameters.GetEndRangeEndDate().c_str());
        if (EndRangeStartDate > EndRangeEndDate) {
          bValid = false;
          PrintDirection.Printf("Error: Invalid scanning window end range.\n"
                                "       Range date '%s' occurs after date '%s'.\n",
                                BasePrint::P_ERROR, gParameters.GetEndRangeStartDate().c_str(), gParameters.GetEndRangeEndDate().c_str());
        }
        else {
          if (EndRangeStartDate < StudyPeriodStartDate || EndRangeStartDate > StudyPeriodEndDate) {
            bValid = false;
            PrintDirection.Printf("Error: The scanning window end range date '%s',\n"
                                  "       is not within the study period (%s - %s).\n",
                                  BasePrint::P_ERROR, gParameters.GetEndRangeStartDate().c_str(),
                                  gParameters.GetStudyPeriodStartDate().c_str(), gParameters.GetStudyPeriodEndDate().c_str());
          }
          if (EndRangeEndDate < StudyPeriodStartDate || EndRangeEndDate > StudyPeriodEndDate) {
            bValid = false;
            PrintDirection.Printf("Error: The scanning window end range date '%s',\n"
                                  "       is not within the study period (%s - %s) \n",
                                  BasePrint::P_ERROR, gParameters.GetEndRangeEndDate().c_str(),
                                  gParameters.GetStudyPeriodStartDate().c_str(), gParameters.GetStudyPeriodEndDate().c_str());
          }
        }

        StartRangeStartDate = CharToJulian(gParameters.GetStartRangeStartDate().c_str());
        StartRangeEndDate = CharToJulian(gParameters.GetStartRangeEndDate().c_str());
        if (StartRangeStartDate > StartRangeEndDate) {
          bValid = false;
          PrintDirection.Printf("Error: Invalid scanning window start range.\n"
                                "       The range date '%s' occurs after date '%s'.\n",
                                BasePrint::P_ERROR, gParameters.GetStartRangeStartDate().c_str(),
                                gParameters.GetStartRangeEndDate().c_str());
        }
        else {
          if (StartRangeStartDate < StudyPeriodStartDate || StartRangeStartDate > StudyPeriodEndDate) {
            bValid = false;
            PrintDirection.Printf("Error: The scanning window start range date '%s',\n"
                                  "       is not within the study period (%s - %s).\n",
                                  BasePrint::P_ERROR, gParameters.GetStartRangeStartDate().c_str(),
                                  gParameters.GetStudyPeriodStartDate().c_str(), gParameters.GetStudyPeriodEndDate().c_str());
          }
          if (StartRangeEndDate < StudyPeriodStartDate || StartRangeEndDate > StudyPeriodEndDate) {
            bValid = false;
            PrintDirection.Printf("Error: The scanning window start range date '%s',\n"
                                  "       is not within the study period (%s - %s) \n",
                                  BasePrint::P_ERROR, gParameters.GetStartRangeEndDate().c_str(),
                                  gParameters.GetStudyPeriodStartDate().c_str(), gParameters.GetStudyPeriodEndDate().c_str());
          }
        }
        if (StartRangeStartDate >= EndRangeEndDate) {
          bValid = false;
          PrintDirection.Printf("Error: The scanning window start range does not occur before the end range.\n", BasePrint::P_ERROR);
        }
      }
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateRangeParameters()","ParametersValidate");
    throw;
  }
  return bValid;
}

/** Validates parameters used in optional sequenatial scan feature.
    Prints errors to print direction and returns whether values are vaild.*/
bool ParametersValidate::ValidateSequentialScanParameters(BasePrint & PrintDirection) const {
  bool  bValid=true;

  try {
    if (gParameters.GetIsSequentialScanning()) {
      if (gParameters.GetSimulationType() == FILESOURCE) {
        PrintDirection.Printf("Error: The sequential scan feature can not be combined with the feature to read simulation data from file.\n", BasePrint::P_ERROR);
        return false;
      }
      if (gParameters.GetOutputSimulationData()) {
        PrintDirection.Printf("Error: The sequential scan feature can not be combined with the feature to write simulation data to file.\n", BasePrint::P_ERROR);
        return false;
      }
      if (!(gParameters.GetProbabilityModelType() == POISSON || gParameters.GetProbabilityModelType() == BERNOULLI ||
            gParameters.GetProbabilityModelType() == ORDINAL || gParameters.GetProbabilityModelType() == NORMAL)) {
        //code only implemented for Poisson or Bernoulli models
        PrintDirection.Printf("Error: The sequential scan feature is implemented for Poisson, Bernoulli and Ordinal models only.\n", BasePrint::P_ERROR);
        return false;
      }
      if (gParameters.GetNumSequentialScansRequested() > static_cast<unsigned int>(CParameters::MAXIMUM_SEQUENTIAL_ANALYSES)) {
        bValid = false;
        PrintDirection.Printf("Error: %d exceeds the maximum number of sequential analyses allowed (%d).\n",
                              BasePrint::P_ERROR, gParameters.GetNumSequentialScansRequested(), CParameters::MAXIMUM_SEQUENTIAL_ANALYSES);
      }
      if (gParameters.GetSequentialCutOffPValue() < 0 || gParameters.GetSequentialCutOffPValue() > 1) {
        bValid = false;
        PrintDirection.Printf("Error: The sequential scan analysis cutoff p-value of '%2g' is not a decimal value between 0 and 1.\n",
                              BasePrint::P_ERROR, gParameters.GetSequentialCutOffPValue());
      }
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateSequentialScanParameters()","ParametersValidate");
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
      PrintDirection.Printf("Error: Simulation data output file not specified.\n", BasePrint::P_ERROR);
    }

    switch (gParameters.GetSimulationType()) {
      case STANDARD         : break;
      case HA_RANDOMIZATION :
        if (gParameters.GetProbabilityModelType() == POISSON) {
          if (gParameters.GetAdjustmentsByRelativeRisksFilename().empty()) {
            bValid = false;
            PrintDirection.Printf("Error: No adjustments file specified.\n", BasePrint::P_ERROR);
          }
          else if (access(gParameters.GetAdjustmentsByRelativeRisksFilename().c_str(), 00)) {
            bValid = false;
            PrintDirection.Printf("Error: The adjustments file '%s' does not exist.\n"
                                  "       Please ensure the path is correct.\n",
                                  BasePrint::P_ERROR, gParameters.GetAdjustmentsByRelativeRisksFilename().c_str());
          }
        }
        else {
          bValid = false;
          PrintDirection.Printf("Error: The alternative hypothesis method of creating simulated data\n"
                                "       is only implemented for the Poisson model.\n", BasePrint::P_ERROR);
        }
        break;
      case FILESOURCE       :
        if (gParameters.GetProbabilityModelType() == EXPONENTIAL || gParameters.GetProbabilityModelType() == NORMAL ||
            gParameters.GetProbabilityModelType() == RANK) {
          bValid = false;
          PrintDirection.Printf("Error: The feature to read simulated data from a file is not implemented for\n"
                                "       the %s probability model.\n", BasePrint::P_ERROR,
                                gParameters.GetProbabilityModelTypeAsString(gParameters.GetProbabilityModelType()));
        }
        if (gParameters.GetNumDataSets() > 1){
          bValid = false;
          PrintDirection.Printf("Error: The feature to read simulated data from a file is not implemented for analyses\n"
                                "       that read data from multiple data sets.\n", BasePrint::P_ERROR);
        }
        if (gParameters.GetSimulationDataSourceFilename().empty()) {
          bValid = false;
          PrintDirection.Printf("Error: The simulated data input file was not specified.\n", BasePrint::P_ERROR);
        }
        else if (access(gParameters.GetSimulationDataSourceFilename().c_str(), 00)) {
          bValid = false;
          PrintDirection.Printf("Error: The simulated data input file '%s' does not exist.\n"
                                "       Please ensure the path is correct.\n",
                                BasePrint::P_ERROR, gParameters.GetSimulationDataSourceFilename().c_str());
        }
        if (gParameters.GetOutputSimulationData() && gParameters.GetSimulationDataSourceFilename() == gParameters.GetSimulationDataOutputFilename()) {
          bValid = false;
          PrintDirection.Printf("Error: The file '%s' is specified as both\n"
                                "       the input and the output file for simulated data.\n",
                                BasePrint::P_ERROR, gParameters.GetSimulationDataSourceFilename().c_str());
        }
        break;
      default : ZdGenerateException("Unknown simulation type '%d'.","ValidateSimulationDataParameters()", gParameters.GetSimulationType());
    };
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateSimulationDataParameters()","ParametersValidate");
    throw;
  }
  return bValid;
}

/** Validates optional parameters particular to spatial analyses
    (i.e. purely spatial, retrospective space-time and prospective space-time).
    Prints errors to print direction and returns whether values are vaild. */
bool ParametersValidate::ValidateSpatialParameters(BasePrint & PrintDirection) const {
  bool  bValid=true;

  try {
    //validate spatial options
    if (gParameters.GetAnalysisType() == PURELYSPATIAL || gParameters.GetAnalysisType() == SPACETIME ||
        gParameters.GetAnalysisType() == PROSPECTIVESPACETIME || gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
      if (gParameters.GetMaximumGeographicClusterSize() <= 0) {
        bValid = false;
        PrintDirection.Printf("Error: The maximum spatial cluster size of '%2g%%' is invalid. The value must be greater than zero.\n",
                              BasePrint::P_ERROR, gParameters.GetMaximumGeographicClusterSize());
      }
      if (gParameters.GetMaxGeoClusterSizeTypeIsPopulationBased()  && gParameters.GetMaximumGeographicClusterSize() > 50.0) {
        bValid = false;
        PrintDirection.Printf("Error: Invalid parameter setting of '%2g%%' for the maximum spatial cluster size.\n"
                              "       When defined as a percentage of the population at risk, the maximum spatial cluster size is 50%%.\n",
                              BasePrint::P_ERROR, gParameters.GetMaximumGeographicClusterSize());
      }
      if (gParameters.GetRestrictingMaximumReportedGeoClusterSize() && gParameters.GetMaximumReportedGeoClusterSize() <= 0) {
        bValid = false;
        PrintDirection.Printf("Error: The maximum spatial cluster size of '%2g%%' for reported clusters is invalid. It must be greater than zero.\n",
                              BasePrint::P_ERROR, gParameters.GetMaximumGeographicClusterSize());
      }
    }
    else {
      //Purely temporal clusters should default maximum geographical clusters size to 50 of population.
      //This actually has no bearing on analysis results. These variables are used primarly for
      //finding neighbors which purely temporal analyses don't utilize. The finding neighbors
      //routine should really be skipped for this analysis type.
      const_cast<CParameters&>(gParameters).SetMaximumGeographicClusterSize(50.0); //KR980707 0 GG980716;
      const_cast<CParameters&>(gParameters).SetMaximumSpatialClusterSizeType(PERCENTOFPOPULATION);
      const_cast<CParameters&>(gParameters).SetRestrictReportedClusters(false);
      const_cast<CParameters&>(gParameters).SetMaximumReportedGeographicalClusterSize(50.0);
      const_cast<CParameters&>(gParameters).SetMaximumReportedSpatialClusterSizeType(PERCENTOFPOPULATION);
    }

    if (gParameters.GetIncludePurelySpatialClusters()) {
      if (!gParameters.GetPermitsPurelySpatialCluster(gParameters.GetProbabilityModelType())) {
          bValid = false;
          PrintDirection.Printf("Error: A purely spatial cluster cannot be included for a %s model.\n",
                                BasePrint::P_ERROR, gParameters.GetProbabilityModelTypeAsString(gParameters.GetProbabilityModelType()));
      }
      else if (!gParameters.GetPermitsPurelySpatialCluster()) {
        bValid = false;
        PrintDirection.Printf("Error: A purely spatial cluster can only be included for spatial based analyses.\n", BasePrint::P_ERROR);
      }
    }
    if (gParameters.GetSpatialAdjustmentType() == SPATIALLY_STRATIFIED_RANDOMIZATION) {
      if (!(gParameters.GetAnalysisType() == SPACETIME || gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)) {
        bValid = false;
        PrintDirection.Printf("Error: Spatial adjustment by stratified randomization is valid for\n"
                              "       either retrospective or prospective space-time analyses only.\n", BasePrint::P_ERROR);
      }
      if (gParameters.GetTimeTrendAdjustmentType() == STRATIFIED_RANDOMIZATION) {
        bValid = false;
        PrintDirection.Printf("Error: Spatial adjustment by stratified randomization can not be performed\n"
                              "       in conjunction with the temporal adjustment by stratified randomization.\n", BasePrint::P_ERROR);
      }
      if (gParameters.GetIncludePurelySpatialClusters()) {
        bValid = false;
        PrintDirection.Printf("Error: Spatial adjustment by stratified randomization does not permit\n"
                              "       the inclusion of a purely spatial cluster.\n", BasePrint::P_ERROR);
      }
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateSpatialParameters()","ParametersValidate");
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
    //parse date in parts
    if (CharToMDY(&nMonth, &nDay, &nYear, gParameters.GetStudyPeriodEndDate().c_str()) != 3) {
      PrintDirection.Printf("Error: The study period end date, '%s', is not valid.\n"
                            "       Please specify as YYYY/MM/DD.\n",
                            BasePrint::P_ERROR, gParameters.GetStudyPeriodEndDate().c_str());
      return false;
    }
    //validate date
    if (!IsDateValid(nMonth, nDay, nYear)) {
      PrintDirection.Printf("Error: The study period end date, '%s', is not a valid date.\n",
                            BasePrint::P_ERROR, gParameters.GetStudyPeriodEndDate().c_str());
      return false;
    }

    //validate against precision of times
    if (gParameters.GetCreationVersion().iMajor == 4)
      // no date precision validation needed for purely spatial
      ePrecision = (gParameters.GetAnalysisType() == PURELYSPATIAL ? NONE : gParameters.GetTimeAggregationUnitsType());
    else
      ePrecision = gParameters.GetPrecisionOfTimesType();
    switch (ePrecision) {
      case YEAR  :
        if (nMonth != 12 || nDay != 31) {
          PrintDirection.Printf("Error: The study period end date, '%s', is not valid.\n"
                                "       With the setting for %s as years, the date\n"
                                "       must be the last day of respective year.\n",
                                BasePrint::P_ERROR, gParameters.GetStudyPeriodEndDate().c_str(),
                                (gParameters.GetCreationVersion().iMajor == 4 ? "time aggregation" : "time precision"));
          return false;
        }
        break;
      case MONTH :
        if (nDay != DaysThisMonth(nYear, nMonth)) {
          PrintDirection.Printf("Error: The study period end date, '%s', is not valid.\n"
                                "       With the setting for %s as months, the date\n"
                                "       must be the last day of respective month.\n",
                                BasePrint::P_ERROR, gParameters.GetStudyPeriodEndDate().c_str(),
                                (gParameters.GetCreationVersion().iMajor == 4 ? "time aggregation" : "time precision"));
          return false;
        }
      case DAY   :
      case NONE  : break;
    };
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateEndDate()","ParametersValidate");
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
    //parse date in parts
    if (CharToMDY(&nMonth, &nDay, &nYear, gParameters.GetStudyPeriodStartDate().c_str()) != 3) {
      PrintDirection.Printf("Error: The study period start date, '%s', is not valid.\n"
                            "       Please specify as YYYY/MM/DD.\n",
                            BasePrint::P_ERROR, gParameters.GetStudyPeriodStartDate().c_str());
      return false;
    }
    //validate date
    if (!IsDateValid(nMonth, nDay, nYear)) {
      PrintDirection.Printf("Error: The study period start date, '%s', is not valid date.\n",
                            BasePrint::P_ERROR, gParameters.GetStudyPeriodStartDate().c_str());
      return false;
    }
    //validate against precision of times
    if (gParameters.GetCreationVersion().iMajor == 4)
      // no date precision validation needed for purely spatial
      ePrecision = (gParameters.GetAnalysisType() == PURELYSPATIAL ? NONE : gParameters.GetTimeAggregationUnitsType());
    else
      ePrecision = gParameters.GetPrecisionOfTimesType();
    switch (ePrecision) {
      case YEAR  :
        if (nMonth != 1 || nDay != 1) {
          PrintDirection.Printf("Error: The study period start date, '%s', is not valid.\n"
                                "       With the setting for %s as years, the date\n"
                                "       must be the first day of respective year.\n",
                                BasePrint::P_ERROR, gParameters.GetStudyPeriodStartDate().c_str(),
                                (gParameters.GetCreationVersion().iMajor == 4 ? "time aggregation" : "time precision"));
          return false;
        }
        break;
      case MONTH :
        if (nDay != 1) {
          PrintDirection.Printf("Error: The study period start date, '%s', is not valid.\n"
                                "       With the setting for %s as months, the date\n"
                                "       must be the first day of respective month.\n",
                                BasePrint::P_ERROR, gParameters.GetStudyPeriodStartDate().c_str(),
                                (gParameters.GetCreationVersion().iMajor == 4 ? "time aggregation" : "time precision"));
          return false;
        }
      case DAY   :
      case NONE  : break;
    };
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateStartDate()","ParametersValidate");
    throw;
  }
  return true;
}

/** Validates optional parameters particular to temporal analyses
    (i.e. purely temporal, retrospective space-time and prospective space-time).
    Prints errors to print direction and returns whether values are vaild.*/
bool ParametersValidate::ValidateTemporalParameters(BasePrint & PrintDirection) const {
  bool          bValid=true;

  try {
    //validate temporal options only for analyses that are temporal
    if (gParameters.GetAnalysisType() == PURELYSPATIAL) {
      //default these options - Not sure why orignal programmers did this. When
      //there is more time, we want to examine code so that we don't need to.
      //Instead, code related to these variables just shouldn't be executed.
      const_cast<CParameters&>(gParameters).SetMaximumTemporalClusterSize(50.0);
      const_cast<CParameters&>(gParameters).SetMaximumTemporalClusterSizeType(PERCENTAGETYPE);
      const_cast<CParameters&>(gParameters).SetIncludeClustersType(ALLCLUSTERS);
      const_cast<CParameters&>(gParameters).SetTimeAggregationUnitsType(NONE);
      const_cast<CParameters&>(gParameters).SetTimeAggregationLength(0);
      const_cast<CParameters&>(gParameters).SetTimeTrendAdjustmentType(NOTADJUSTED);
      const_cast<CParameters&>(gParameters).SetTimeTrendAdjustmentPercentage(0);
      return true;
    }
    //validate maximum temporal cluster size
    if (!ValidateMaximumTemporalClusterSize(PrintDirection))
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
    //validate time trend adjustment
    switch (gParameters.GetProbabilityModelType()) {
      case BERNOULLI            :
        //The SVTT analysis has hooks for temporal adjustments, but that code needs
        //much closer examination before it can be used, even experimentally.
        if (gParameters.GetTimeTrendAdjustmentType() != NOTADJUSTED) {
          PrintDirection.Printf("Notice: For the Bernoulli model, adjusting for temporal trends is not permitted.\n"
                                "        Temporal trends adjustment settings will be ignored.", BasePrint::P_NOTICE);
          const_cast<CParameters&>(gParameters).SetTimeTrendAdjustmentType(NOTADJUSTED);
          const_cast<CParameters&>(gParameters).SetTimeTrendAdjustmentPercentage(0);
        }
        break;
      case ORDINAL              :
      case EXPONENTIAL          :
      case NORMAL               :
      case RANK                 :
        if (gParameters.GetTimeTrendAdjustmentType() != NOTADJUSTED) {
          PrintDirection.Printf("Notice: For the %s model, adjusting for temporal trends is not permitted.\n"
                                "        Temporal trends adjustment settings will be ignored.",
                                BasePrint::P_NOTICE, gParameters.GetProbabilityModelTypeAsString(gParameters.GetProbabilityModelType()));
          const_cast<CParameters&>(gParameters).SetTimeTrendAdjustmentType(NOTADJUSTED);
          const_cast<CParameters&>(gParameters).SetTimeTrendAdjustmentPercentage(0);
        }
        break;
      case SPACETIMEPERMUTATION :
        if (gParameters.GetTimeTrendAdjustmentType() != NOTADJUSTED) {
          PrintDirection.Printf("Notice: For the space-time permutation model, adjusting for temporal trends\n"
                                "        is not permitted nor needed, as this model automatically adjusts for\n"
                                "        any temporal variation. Temporal trends adjustment settings will be ignored.\n",
                                BasePrint::P_NOTICE);
          const_cast<CParameters&>(gParameters).SetTimeTrendAdjustmentType(NOTADJUSTED);
          const_cast<CParameters&>(gParameters).SetTimeTrendAdjustmentPercentage(0);
        }
        break;
      case POISSON             :
        if (gParameters.GetTimeTrendAdjustmentType() != NOTADJUSTED && (gParameters.GetAnalysisType() == PURELYTEMPORAL || gParameters.GetAnalysisType() == PROSPECTIVEPURELYTEMPORAL)
            && gParameters.GetPopulationFileName().empty()) {
           bValid = false; 
          PrintDirection.Printf("Error: Temporal adjustments can not be performed for a purely temporal analysis\n"
                                "       using the Poisson model, when no population file has been specfied.\n", BasePrint::P_ERROR);
    }

        if (gParameters.GetTimeTrendAdjustmentType() == NONPARAMETRIC && (gParameters.GetAnalysisType() == PURELYTEMPORAL ||gParameters.GetAnalysisType() == PROSPECTIVEPURELYTEMPORAL)) {
          bValid = false;
          PrintDirection.Printf("Error: Invalid parameter setting for time trend adjustment.\n"
                                "       You may not use non-parametric time in a purely temporal analysis.\n", BasePrint::P_ERROR);
        }
        if (gParameters.GetTimeTrendAdjustmentType() == STRATIFIED_RANDOMIZATION && (gParameters.GetAnalysisType() == PURELYTEMPORAL || gParameters.GetAnalysisType() == PROSPECTIVEPURELYTEMPORAL)) {
          bValid = false;
          PrintDirection.Printf("Error: Temporal adjustment by stratified randomization is not valid\n"
                                 "       for purely temporal analyses.\n", BasePrint::P_ERROR);
        }
        if (gParameters.GetTimeTrendAdjustmentType() == LOGLINEAR_PERC && -100.0 >= gParameters.GetTimeTrendAdjustmentPercentage()) {
          bValid = false;
          PrintDirection.Printf("Error: The time adjustment percentage is '%2g', but must greater than -100.\n",
                                BasePrint::P_ERROR, gParameters.GetTimeTrendAdjustmentPercentage());
        }
        if (gParameters.GetTimeTrendAdjustmentType() == NOTADJUSTED) {
          const_cast<CParameters&>(gParameters).SetTimeTrendAdjustmentPercentage(0);
          if (gParameters.GetAnalysisType() != SPATIALVARTEMPTREND)
            const_cast<CParameters&>(gParameters).SetTimeTrendConvergence(0);
        }
        if (gParameters.GetTimeTrendAdjustmentType() == CALCULATED_LOGLINEAR_PERC || gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
          if (gParameters.GetTimeTrendConvergence() < 0.0) {
            bValid = false;
            PrintDirection.Printf("Error: Time trend convergence value of '%2g' is less than zero.\n",
                                  BasePrint::P_ERROR, gParameters.GetTimeTrendConvergence());
          }
        }
        break;
      default : ZdException::Generate("Unknown model type '%d'.",
                                      "ValidateTemporalParameters()", gParameters.GetProbabilityModelType());
    }
    //validate including purely temporal clusters
    if (gParameters.GetIncludePurelyTemporalClusters()) {
      if (!gParameters.GetPermitsPurelyTemporalCluster(gParameters.GetProbabilityModelType())) {
          bValid = false;
          PrintDirection.Printf("Error: Scanning for purely temporal clusters can not be included when the %s model is used.\n",
                                BasePrint::P_ERROR, gParameters.GetProbabilityModelTypeAsString(gParameters.GetProbabilityModelType()));
      }
      else if (!gParameters.GetPermitsPurelyTemporalCluster()) {
        bValid = false;
        PrintDirection.Printf("Error: A purely temporal cluster can only be included for time based analyses.\n", BasePrint::P_ERROR);
      }
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateTemporalParameters()","ParametersValidate");
    throw;
  }
  return bValid;
}

/** Validates the time aggregation units. */
bool ParametersValidate::ValidateTimeAggregationUnits(BasePrint& PrintDirection) const {
  ZdString      sPrecisionString;
  double        dStudyPeriodLengthInUnits, dMaxTemporalLengthInUnits;

  if (gParameters.GetAnalysisType() == PURELYSPATIAL) //validate settings for temporal analyses
    return true;
    
  //get date precision string for error reporting
  GetDatePrecisionAsString(gParameters.GetTimeAggregationUnitsType(), sPrecisionString, false, false);
  if (gParameters.GetTimeAggregationUnitsType() == NONE) { //validate time aggregation units
    PrintDirection.Printf("Error: Time aggregation units can not be 'none' for a temporal analysis.\n", BasePrint::P_ERROR);
    return false;
  }
  if (gParameters.GetTimeAggregationLength() <= 0) {
    PrintDirection.Printf("Error: The time aggregation length of '%d' is invalid. Length must be greater than zero.\n",
                          BasePrint::P_ERROR, gParameters.GetTimeAggregationLength());
    return false;
  }
  //validate that the time aggregation length agrees with the study period and maximum temporal cluster size
  dStudyPeriodLengthInUnits = ceil(CalculateNumberOfTimeIntervals(CharToJulian(gParameters.GetStudyPeriodStartDate().c_str()),
                                                                  CharToJulian(gParameters.GetStudyPeriodEndDate().c_str()),
                                                                  gParameters.GetTimeAggregationUnitsType(), 1));
  if (dStudyPeriodLengthInUnits < static_cast<double>(gParameters.GetTimeAggregationLength()))  {
    PrintDirection.Printf("Error: A time aggregation of %d %s%s is greater than the %d %s study period.\n",
                          BasePrint::P_ERROR, gParameters.GetTimeAggregationLength(), sPrecisionString.GetCString(),
                          (gParameters.GetTimeAggregationLength() == 1 ? "" : "s"),
                          static_cast<int>(dStudyPeriodLengthInUnits), sPrecisionString.GetCString());
    return false;
  }
  if (ceil(dStudyPeriodLengthInUnits/static_cast<double>(gParameters.GetTimeAggregationLength())) <= 1) {
    PrintDirection.Printf("Error: A time aggregation of %d %s%s with a %d %s study period results in only\n"
                          "       one time period to analyze. Temporal and space-time analyses can not be performed\n"
                          "       on less than two time periods.\n", BasePrint::P_ERROR,
                          gParameters.GetTimeAggregationLength(), sPrecisionString.GetCString(), (gParameters.GetTimeAggregationLength() == 1 ? "" : "s"),
                          static_cast<int>(dStudyPeriodLengthInUnits), sPrecisionString.GetCString());
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
      PrintDirection.Printf("Error: The time aggregation of %d %s%s is greater than the maximum temporal\n"
                            "       cluster size of %g %s%s.\n",
                            BasePrint::P_ERROR, gParameters.GetTimeAggregationLength(),
                            sPrecisionString.GetCString(), (gParameters.GetTimeAggregationLength() == 1 ? "" : "s"),
                            gParameters.GetMaximumTemporalClusterSize(), sPrecisionString.GetCString(),
                            (gParameters.GetMaximumTemporalClusterSize() == 1 ? "" : "s"));
    else if (gParameters.GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE)
      PrintDirection.Printf("Error: With the maximum temporal cluster size as %g percent of a %d %s study period,\n"
                            "       the time aggregation as %d %s%s is greater than the resulting maximum\n"
                            "       temporal cluster size of %g %s%s.\n", BasePrint::P_ERROR,
                            gParameters.GetMaximumTemporalClusterSize(), static_cast<int>(dStudyPeriodLengthInUnits),
                            sPrecisionString.GetCString(), gParameters.GetTimeAggregationLength(),
                            sPrecisionString.GetCString(), (gParameters.GetTimeAggregationLength() == 1 ? "" : "s"),
                            dMaxTemporalLengthInUnits, sPrecisionString.GetCString(),
                            (dMaxTemporalLengthInUnits == 1 ? "" : "s"));
    return false;
  }
    
  return true;
}

