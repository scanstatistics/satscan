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
    if (!ValidateMonotoneRisk(PrintDirection))
      bValid = false;
    if (!ValidateSVTTAnalysisSettings(PrintDirection))
      bValid = false;
    if ((gParameters.GetProbabilityModelType() == ORDINAL || gParameters.GetProbabilityModelType() == CATEGORICAL)
        && gParameters.GetNumDataSets() > 1 && gParameters.GetMultipleDataSetPurposeType() == ADJUSTMENT) {
      bValid = false;
      PrintDirection.Printf("Invalid Parameter Setting:\n"
                            "Adjustment purpose for multiple data sets is not permitted with %s model in this version of SaTScan.\n", 
                            BasePrint::P_PARAMERROR, ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
    }
    if (gParameters.UseLocationNeighborsFile() && !gParameters.GetIsPurelyTemporalAnalysis() &&
        !(gParameters.GetCriteriaSecondClustersType() == NORESTRICTIONS || gParameters.GetCriteriaSecondClustersType() == NOGEOOVERLAP)) {
      bValid = false;
      PrintDirection.Printf("Invalid Parameter Setting:\n"
                            "When using the non-Eucledian neighbors file, the criteria for reporting secondary clusters "
                            "can either be set to 'no restrictions' or 'no geographical overlap'.\n", BasePrint::P_PARAMERROR);
    }
    if (!ValidateExecutionTypeParameters(PrintDirection))
      bValid = false;
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

    if (!ValidateContinuousPoissonParameters(PrintDirection))
      bValid = false;

    //validate input/oupt files
    if (! ValidateFileParameters(PrintDirection))
      bValid = false;

    if (! ValidateOutputOptionParameters(PrintDirection))
      bValid = false;

    //validate model parameters
    if (gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION) {
      if (!(gParameters.GetAnalysisType() == SPACETIME || gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\n"
                              "For the %s model, the analysis type must be either Retrospective or Prospective Space-Time.\n",
                              BasePrint::P_PARAMERROR, ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
      }
    }
    //validate range parameters
    if (! ValidateRangeParameters(PrintDirection))
      bValid = false;

    //validate iterative scan parameters
    if (! ValidateIterativeScanParameters(PrintDirection))
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
  catch (prg_exception& x) {
    x.addTrace("ValidateParameters()","ParametersValidate");
    throw;
  }
  return bValid;
}

/** Validates date parameters based upon current settings. Error messages
    sent to print direction object and indication of valid settings returned. */
bool ParametersValidate::ValidateDateParameters(BasePrint& PrintDirection) const {
  bool          bValid=true, bStartDateValid=true, bEndDateValid=true, bProspectiveDateValid=true;
  Julian        StudyPeriodStartDate, StudyPeriodEndDate, ProspectiveStartDate;

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
        PrintDirection.Printf("Invalid Parameter Setting:\n"
                              "The study period start date occurs after the end date.\n", BasePrint::P_PARAMERROR);
      }
      if (bValid && gParameters.GetIsProspectiveAnalysis() && gParameters.GetAdjustForEarlierAnalyses() && bProspectiveDateValid) {
        //validate prospective start date
        ProspectiveStartDate = CharToJulian(gParameters.GetProspectiveStartDate().c_str());
        if (ProspectiveStartDate < StudyPeriodStartDate || ProspectiveStartDate > StudyPeriodEndDate) {
          bValid = false;
          PrintDirection.Printf("Invalid Parameter Setting:\n"
                                "The start date of prospective surveillance does not occur within the study period.\n", BasePrint::P_PARAMERROR);
        }
      }
    }
  }
  catch (prg_exception& x) {
    x.addTrace("ValidateDateParameters()","ParametersValidate");
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
    if (gParameters.GetIsPurelyTemporalAnalysis() || gParameters.UseLocationNeighborsFile() || gParameters.GetSpatialWindowType() != ELLIPTIC) return true;

    if (gParameters.GetNumRequestedEllipses() < 1 || gParameters.GetNumRequestedEllipses() > CParameters::MAXIMUM_ELLIPSOIDS) {
      bValid = false;
      PrintDirection.Printf("Invalid Parameter Setting:\n"
                            "The number of requested ellipses '%d' is not within allowable range of 1 - %d.\n",
                            BasePrint::P_PARAMERROR, gParameters.GetNumRequestedEllipses(), CParameters::MAXIMUM_ELLIPSOIDS);
    }
    //analyses with ellipses can not be performed with coordinates defiend in latitude/longitude system (currently)
    if (gParameters.GetCoordinatesType() == LATLON) {
      bValid = false;
      PrintDirection.Printf("Invalid Parameter Setting:\n"
                            "SaTScan does not support lat/long coordinates when ellipses are used. "
                            "Please use the Cartesian coordinate system if possible.\n", BasePrint::P_PARAMERROR);
    }
    if (gParameters.GetEllipseShapes().size() != gParameters.GetEllipseRotations().size()) {
      bValid = false;
      PrintDirection.Printf("Invalid Parameter Setting:\n"
                            "Settings indicate %i elliptic shapes but %i variables for respective angles of rotation.\n",
                            BasePrint::P_PARAMERROR, gParameters.GetEllipseShapes().size(), gParameters.GetEllipseRotations().size());
    }
    for (t=0; t < gParameters.GetEllipseShapes().size(); ++t)
       if (gParameters.GetEllipseShapes()[t] < 1) {
         bValid = false;
         PrintDirection.Printf("Invalid Parameter Setting:\n"
                               "Ellipse shape '%g' is invalid. The shape can not be less than one.\n",
                               BasePrint::P_PARAMERROR, gParameters.GetEllipseShapes()[t]);
       }
    for (t=0; t < gParameters.GetEllipseRotations().size(); t++)
       if (gParameters.GetEllipseRotations()[t] < 1) {
         bValid = false;
         PrintDirection.Printf("Invalid Parameter Setting:\n"
                               "The number of angles, '%d', is invalid. The number can not be less than one.\n",
                               BasePrint::P_PARAMERROR, gParameters.GetEllipseRotations()[t]);
       }
    if (gParameters.GetExecutionType() == CENTRICALLY && gParameters.GetNonCompactnessPenaltyType() > NOPENALTY) {
      bValid = false;
      PrintDirection.Printf("Invalid Parameter Setting:\n"
                            "The non-compactness penalty for elliptic scans can not be applied "
                            "with the centric analysis execution.\n", BasePrint::P_PARAMERROR);
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
    if (gParameters.GetExecutionType() == CENTRICALLY && gParameters.GetTerminateSimulationsEarly()) {
      bValid = false;
      PrintDirection.Printf("Invalid Parameter Setting:\n"
                            "The early termination of simulations option can not be applied "
                            "with the centric analysis execution.\n", BasePrint::P_PARAMERROR);
    }
    if (gParameters.GetExecutionType() == CENTRICALLY &&
        (gParameters.GetIsPurelyTemporalAnalysis() ||
         (gParameters.GetAnalysisType() == PURELYSPATIAL && gParameters.GetRiskType() == MONOTONERISK))) {
      bValid = false;
      PrintDirection.Printf("Invalid Parameter Setting:\n"
                            "The centric analysis execution is not available for:\n"
                            " purely temporal analyses\n purely spatial analyses with isotonic scan\n"
                            " spatial variation in temporal trends analysis\n", BasePrint::P_PARAMERROR);
    }
    if (gParameters.GetExecutionType() == CENTRICALLY && gParameters.UseLocationNeighborsFile()) {
      bValid = false;
      PrintDirection.Printf("Invalid Parameter Setting:\n"
                            "Centric analysis execution is not implemented with the non-Eucludian neighbors file.\n", BasePrint::P_PARAMERROR);
    }
    if (gParameters.GetExecutionType() == CENTRICALLY && gParameters.GetMultipleCoordinatesType() != ONEPERLOCATION) {
      bValid = false;
      PrintDirection.Printf("Invalid Parameter Setting:\n"
                            "Centric analysis execution is not implemented with the multiple coordinates per location id feature.\n", BasePrint::P_PARAMERROR);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("ValidateExecutionTypeParameters()","ParametersValidate");
    throw;
  }
  return bValid;
}

/** Validates input/output file parameters. */
bool ParametersValidate::ValidateFileParameters(BasePrint& PrintDirection) const {
  bool          bValid=true;
  size_t        t;

  try {
    if (gParameters.GetProbabilityModelType() != HOMOGENEOUSPOISSON) {
        //validate case file
        if (!gParameters.GetCaseFileNames().size()) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nNo case file was specified.\n", BasePrint::P_PARAMERROR);
        }
        for (t=0; t < gParameters.GetCaseFileNames().size(); ++t) {
            if (!ValidateFileAccess(gParameters.GetCaseFileNames()[t])) {
                bValid = false;
                PrintDirection.Printf("Invalid Parameter Setting:\n"
                               "The case file '%s' could not be opened for reading. "
                               "Please confirm that the path and/or file name are valid and that you "
                               "have permissions to read from this directory and file.\n",
                               BasePrint::P_PARAMERROR, gParameters.GetCaseFileNames()[t].c_str());
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
            PrintDirection.Printf("Invalid Parameter Setting:\n"
                                  "For the Poisson model with purely temporal analyses, the population file "
                                  "is optional but all data sets must either specify a population file or omit it.\n",
                                  BasePrint::P_PARAMERROR);
         }
         else if (!iNumDataSetsWithoutPopFile) {
           const_cast<CParameters&>(gParameters).SetPopulationFile(true);
           for (t=0; t < gParameters.GetPopulationFileNames().size(); ++t) {
              if (!ValidateFileAccess(gParameters.GetPopulationFileNames()[t])) {
                bValid = false;
                PrintDirection.Printf("Invalid Parameter Setting:\n"
                                      "The population file '%s' could not be opened for reading. "
                                      "Please confirm that the path and/or file name are valid and that you "
                                      "have permissions to read from this directory and file.\n",
                                      BasePrint::P_PARAMERROR, gParameters.GetPopulationFileNames()[t].c_str());
              }
           }
         }
      }
      else {
        const_cast<CParameters&>(gParameters).SetPopulationFile(true);
        if (!gParameters.GetPopulationFileNames().size()) {
          bValid = false;
          PrintDirection.Printf("Invalid Parameter Setting:\n"
                                "For the Poisson model, a population file must be specified unless analysis "
                                "is purely temporal. In which case the population file is optional.\n", BasePrint::P_PARAMERROR);
        }
        for (t=0; t < gParameters.GetPopulationFileNames().size(); ++t) {
          if (!ValidateFileAccess(gParameters.GetPopulationFileNames()[t])) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\n"
                                  "The population file '%s' could not be opened for reading. "
                                  "Please confirm that the path and/or file name are valid and that you "
                                  "have permissions to read from this directory and file.\n",
                                  BasePrint::P_PARAMERROR, gParameters.GetPopulationFileNames()[t].c_str());
          }
        }
      }
    }
    //validate control file for a bernoulli model.
    if (gParameters.GetProbabilityModelType() == BERNOULLI) {
      if (!gParameters.GetControlFileNames().size()) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\nFor the Bernoulli model, a Control file must be specified.\n", BasePrint::P_PARAMERROR);
      }
      for (t=0; t < gParameters.GetControlFileNames().size(); ++t) {
        if (!ValidateFileAccess(gParameters.GetControlFileNames()[t])) {
          bValid = false;
          PrintDirection.Printf("Invalid Parameter Setting:\n"
                                "The control file '%s' could not be opened for reading. "
                                "Please confirm that the path and/or file name are valid and that you "
                                "have permissions to read from this directory and file.\n",
                                BasePrint::P_PARAMERROR, gParameters.GetControlFileNames()[t].c_str());
        }
      }
    }
    //validate coordinates file
    if (gParameters.UseCoordinatesFile()) {
      if (gParameters.GetCoordinatesFileName().empty()) {
         bValid = false;
         PrintDirection.Printf("Invalid Parameter Setting:\nNo coordinates file specified.\n", BasePrint::P_PARAMERROR);
      }
      else if (!ValidateFileAccess(gParameters.GetCoordinatesFileName())) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\n"
                              "The coordinates file '%s' could not be opened for reading. "
                              "Please confirm that the path and/or file name are valid and that you "
                              "have permissions to read from this directory and file.\n",
                              BasePrint::P_PARAMERROR, gParameters.GetCoordinatesFileName().c_str());
      }
    }  
    //validate special grid file
    if (gParameters.GetIsPurelyTemporalAnalysis() || gParameters.UseLocationNeighborsFile())
      const_cast<CParameters&>(gParameters).SetUseSpecialGrid(false);
    else if (gParameters.UseSpecialGrid() && gParameters.GetSpecialGridFileName().empty()) {
      bValid = false;
      PrintDirection.Printf("Invalid Parameter Setting:\nThe settings indicate to the use a grid file, but a grid file name is not specified.\n", BasePrint::P_PARAMERROR);
    }
    else if (gParameters.UseSpecialGrid() && !ValidateFileAccess(gParameters.GetSpecialGridFileName())) {
      bValid = false;
      PrintDirection.Printf("Invalid Parameter Setting:\n"
                            "The grid file '%s' could not be opened for reading. "
                            "Please confirm that the path and/or file name are valid and that you "
                            "have permissions to read from this directory and file.\n",
                            BasePrint::P_PARAMERROR, gParameters.GetSpecialGridFileName().c_str());
    }
    //validate adjustment for known relative risks file
    if (gParameters.GetProbabilityModelType() == POISSON) {
      if (gParameters.UseAdjustmentForRelativeRisksFile() && gParameters.GetAdjustmentsByRelativeRisksFilename().empty()) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\nThe settings indicate to the use the adjustments file, but a file name not specified.\n", BasePrint::P_PARAMERROR);
      }
      else if (gParameters.UseAdjustmentForRelativeRisksFile() && !ValidateFileAccess(gParameters.GetAdjustmentsByRelativeRisksFilename())) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\n"
                              "The adjustments file '%s' could not be opened for reading. "
                              "Please confirm that the path and/or file name are valid and that you "
                              "have permissions to read from this directory and file.\n",
                              BasePrint::P_PARAMERROR, gParameters.GetAdjustmentsByRelativeRisksFilename().c_str());
      }
    }
    else
      const_cast<CParameters&>(gParameters).SetUseAdjustmentForRelativeRisksFile(false);

    //validate maximum circle population file for a prospective space-time analysis w/ maximum geographical cluster size
    //defined as a percentage of the population and adjusting for earlier analyses.
    if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME && gParameters.GetAdjustForEarlierAnalyses()) {
      if (gParameters.UseMetaLocationsFile() && !gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFPOPULATION, false)) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\n"
                              "For a prospective space-time analysis adjusting for ealier analyses and defining "
                              "non-eucledian neighbors, the maximum spatial cluster size must be defined as a "
                              "percentage of the population as defined in a max circle size file.\n", BasePrint::P_PARAMERROR);
      }
      else if (!gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFPOPULATION, false) && !gParameters.GetRestrictMaxSpatialSizeForType(MAXDISTANCE, false)) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\n"
                              "For a prospective space-time analysis adjusting for ealier analyses, the maximum spatial "
                              "cluster size must be defined as a percentage of the population as defined in a max "
                              "circle size file. Alternatively you may choose to specify the maximum as a fixed radius, "
                              "in which case a max circle size file is not required.\n", BasePrint::P_PARAMERROR);
      }
    }
    if (gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false) ||
        (gParameters.GetRestrictingMaximumReportedGeoClusterSize() && gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true))) {
      if (gParameters.GetMaxCirclePopulationFileName().empty() && gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false)) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\nThe settings indicate to the use a max circle size file, but a file name was not specified.\n", BasePrint::P_PARAMERROR);
      }
      else if (gParameters.GetMaxCirclePopulationFileName().empty() &&
               gParameters.GetRestrictingMaximumReportedGeoClusterSize() && gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true)) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\n"
                              "Maximum circle size file name was not specified. A maximum circle file is required "
                              "when restricting the maximum reported spatial cluster size by a population defined "
                              "in that maximum circle file.\n", BasePrint::P_PARAMERROR);
      }
      else if (!ValidateFileAccess(gParameters.GetMaxCirclePopulationFileName())) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\n"
                              "The max circle size file '%s' could not be opened for reading. "
                              "Please confirm that the path and/or file name are valid and that you "
                              "have permissions to read from this directory and file.\n",
                              BasePrint::P_PARAMERROR, gParameters.GetMaxCirclePopulationFileName().c_str());
      }
    }
    //validate neighbor array file
    if (gParameters.UseLocationNeighborsFile() && gParameters.GetLocationNeighborsFileName().empty()) {
      bValid = false;
      PrintDirection.Printf("Invalid Parameter Setting:\nNo locations neighbors file specified.\n", BasePrint::P_PARAMERROR);
    }
    else if (gParameters.UseLocationNeighborsFile() && !ValidateFileAccess(gParameters.GetLocationNeighborsFileName())) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\n"
                              "The location neighbors file '%s' could not be opened for reading. "
                              "Please confirm that the path and/or file name are valid and that you "
                              "have permissions to read from this directory and file.\n",
                              BasePrint::P_PARAMERROR, gParameters.GetLocationNeighborsFileName().c_str());
    }
    //validate meta locations file
    if (gParameters.UseMetaLocationsFile() && gParameters.getMetaLocationsFilename().empty()) {
      bValid = false;
      PrintDirection.Printf("Invalid Parameter Setting:\nNo meta locations file specified.\n", BasePrint::P_PARAMERROR);
    }
    else if (gParameters.UseMetaLocationsFile() && !ValidateFileAccess(gParameters.getMetaLocationsFilename())) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\n"
                              "The meta locations file '%s' could not be opened for reading. "
                              "Please confirm that the path and/or file name are valid and that you "
                              "have permissions to read from this directory and file.\n",
                              BasePrint::P_PARAMERROR, gParameters.getMetaLocationsFilename().c_str());
    }
    //validate output file
    if (gParameters.GetOutputFileName().empty()) {
      bValid = false;
      PrintDirection.Printf("Invalid Parameter Setting:\nNo results file specified.\n", BasePrint::P_PARAMERROR);
    }
    else if (!ValidateFileAccess(gParameters.GetOutputFileName(), true)) {
      bValid = false;
      PrintDirection.Printf("Invalid Parameter Setting:\n"
                            "Results file '%s' could not be opened for writing. "
                            "Please confirm that the path and/or file name are valid and that you "
                            "have permissions to write to this directory and file.\n",
                            BasePrint::P_PARAMERROR, gParameters.GetOutputFileName().c_str());
    }
  }
  catch (prg_exception& x) {
    x.addTrace("ValidateFileParameters()","ParametersValidate");
    throw;
  }
  return bValid;
}

/** Validates parameters used in optional iterative scan feature.
    Prints errors to print direction and returns whether values are vaild.*/
bool ParametersValidate::ValidateIterativeScanParameters(BasePrint & PrintDirection) const {
  bool  bValid=true;

  try {
    if (gParameters.GetIsIterativeScanning()) {
      if (gParameters.GetSimulationType() == FILESOURCE) {
        PrintDirection.Printf("Invalid Parameter Setting:\nThe iterative scan feature can not be combined with the feature to read simulation data from file.\n", BasePrint::P_PARAMERROR);
        return false;
      }
      if (gParameters.GetOutputSimulationData()) {
        PrintDirection.Printf("Invalid Parameter Setting:\nThe iterative scan feature can not be combined with the feature to write simulation data to file.\n", BasePrint::P_PARAMERROR);
        return false;
      }
      if (!(gParameters.GetAnalysisType() == PURELYSPATIAL || gParameters.GetAnalysisType() == SPATIALVARTEMPTREND || gParameters.GetIsPurelyTemporalAnalysis())) {
        PrintDirection.Printf("Invalid Parameter Setting:\nThe iterative scan option is not implemented for the %s analysis.\n",
                              BasePrint::P_PARAMERROR, ParametersPrint(gParameters).GetAnalysisTypeAsString());
        return false;
      }
      if (!(gParameters.GetProbabilityModelType() == POISSON || gParameters.GetProbabilityModelType() == BERNOULLI ||
            gParameters.GetProbabilityModelType() == ORDINAL || gParameters.GetProbabilityModelType() == CATEGORICAL ||
            gParameters.GetProbabilityModelType() == NORMAL || gParameters.GetProbabilityModelType() == EXPONENTIAL)) {
        PrintDirection.Printf("Invalid Parameter Setting:\nThe iterative scan feature is not implemented for %s model.\n", 
                              BasePrint::P_PARAMERROR, ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
        return false;
      }
      if (gParameters.GetNumIterativeScansRequested() > static_cast<unsigned int>(CParameters::MAXIMUM_ITERATIVE_ANALYSES)) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\n%d exceeds the maximum number of iterative analyses allowed (%d).\n",
                              BasePrint::P_PARAMERROR, gParameters.GetNumIterativeScansRequested(), CParameters::MAXIMUM_ITERATIVE_ANALYSES);
      }
      if (gParameters.GetIterativeCutOffPValue() < 0 || gParameters.GetIterativeCutOffPValue() > 1) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\nThe iterative scan analysis cutoff p-value of '%2g' is not a decimal value between 0 and 1.\n",
                              BasePrint::P_PARAMERROR, gParameters.GetIterativeCutOffPValue());
      }
    }
  }
  catch (prg_exception& x) {
    x.addTrace("ValidateIterativeScanParameters()","ParametersValidate");
    throw;
  }
  return bValid;
}

/** Validates the maximum temporal cluster size parameters. */
bool ParametersValidate::ValidateMaximumTemporalClusterSize(BasePrint& PrintDirection) const {
  std::string   sPrecisionString;
  double        dStudyPeriodLengthInUnits, dMaxTemporalLengthInUnits;

  try {
    //Maximum temporal cluster size parameters not used for these analyses.
    if (gParameters.GetAnalysisType() == PURELYSPATIAL || gParameters.GetAnalysisType() == SPATIALVARTEMPTREND)
      return true;

    if (gParameters.GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE) {
      //validate for maximum specified as percentage of study period
      if (gParameters.GetMaximumTemporalClusterSize() <= 0) {
        PrintDirection.Printf("Invalid Parameter Setting:\n"
                              "The maximum temporal cluster size of '%g' is invalid. "
                              "Specifying the maximum as a percentage of the study period "
                              "requires the value to be a decimal number that is greater than zero.\n",
                              BasePrint::P_PARAMERROR, gParameters.GetMaximumTemporalClusterSize());
        return false;
      }
      //check maximum temporal cluster size(as percentage of population) is less than maximum for given probability model
      if (gParameters.GetMaximumTemporalClusterSize() > 100.0/*(gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION ? 50 : 90)*/) {
        PrintDirection.Printf("Invalid Parameter Setting:\n"
                              "For the %s model, the maximum temporal cluster size as a percent "
                              "of the study period is %d percent.\n", BasePrint::P_PARAMERROR,
                              ParametersPrint(gParameters).GetProbabilityModelTypeAsString(),
                              100/*(gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION ? 50 : 90)*/);
        return false;
      }
      //validate that the time aggregation length agrees with the study period and maximum temporal cluster size
      dStudyPeriodLengthInUnits = ceil(CalculateNumberOfTimeIntervals(CharToJulian(gParameters.GetStudyPeriodStartDate().c_str()),
                                                                      CharToJulian(gParameters.GetStudyPeriodEndDate().c_str()),
                                                                      gParameters.GetTimeAggregationUnitsType(), 1));
      dMaxTemporalLengthInUnits = floor(dStudyPeriodLengthInUnits * gParameters.GetMaximumTemporalClusterSize()/100.0);
      if (dMaxTemporalLengthInUnits < 1) {
        GetDatePrecisionAsString(gParameters.GetTimeAggregationUnitsType(), sPrecisionString, false, false);
        PrintDirection.Printf("Invalid Parameter Setting:\n"
                              "A maximum temporal cluster size as %g percent of a %d %s study period results in a maximum "
                              "temporal cluster size that is less than one time aggregation %s.\n", BasePrint::P_PARAMERROR,
                              gParameters.GetMaximumTemporalClusterSize(), static_cast<int>(dStudyPeriodLengthInUnits),
                              sPrecisionString.c_str(), sPrecisionString.c_str());
        return false;
      }
    }
    else if (gParameters.GetMaximumTemporalClusterSizeType() == TIMETYPE) {
      //validate for maximum specified as time aggregation unit 
      if (gParameters.GetMaximumTemporalClusterSize() < 1) {
        PrintDirection.Printf("Invalid Parameter Setting:\n"
                              "The maximum temporal cluster size of '%2g' is invalid. "
                              "Specifying the maximum in time aggregation units requires "
                              "the value to be a whole number that is greater than zero.\n",
                              BasePrint::P_PARAMERROR, gParameters.GetMaximumTemporalClusterSize());
        return false;
      }
      GetDatePrecisionAsString(gParameters.GetTimeAggregationUnitsType(), sPrecisionString, false, false);
      dStudyPeriodLengthInUnits = ceil(CalculateNumberOfTimeIntervals(CharToJulian(gParameters.GetStudyPeriodStartDate().c_str()),
                                                                      CharToJulian(gParameters.GetStudyPeriodEndDate().c_str()),
                                                                      gParameters.GetTimeAggregationUnitsType(), 1));
      dMaxTemporalLengthInUnits = floor(dStudyPeriodLengthInUnits * (gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION ? 50 : 90)/100.0);
      if (gParameters.GetMaximumTemporalClusterSize() > dMaxTemporalLengthInUnits) {
        PrintDirection.Printf("Invalid Parameter Setting:\n"
                              "A maximum temporal cluster size of %d %s%s exceeds %d percent of a %d %s study period. "
                              "Note that current settings limit the maximum to %d %s%s.\n",
                              BasePrint::P_PARAMERROR, static_cast<int>(gParameters.GetMaximumTemporalClusterSize()),
                              sPrecisionString.c_str(), (gParameters.GetMaximumTemporalClusterSize() == 1 ? "" : "s"),
                              (gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION ? 50 : 90),
                              static_cast<int>(dStudyPeriodLengthInUnits), sPrecisionString.c_str(),
                              static_cast<int>(dMaxTemporalLengthInUnits), sPrecisionString.c_str(),
                              (dMaxTemporalLengthInUnits == 1 ? "" : "s"));
        return false;
      }
    }
    else
      throw prg_error("Unknown temporal percentage type: %d.",
                      "ValidateMaximumTemporalClusterSize()", gParameters.GetMaximumTemporalClusterSizeType());
  }
  catch (prg_exception& x) {
    x.addTrace("ValidateMaximumTemporalClusterSize()","ParametersValidate");
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
        PrintDirection.Printf("Invalid Parameter Setting:\nThe isotonic scan is implemented only for the purely spatial analysis.\n", BasePrint::P_PARAMERROR);
      }
      if (gParameters.GetNumDataSets() > 1) {
        bReturn = false;
        PrintDirection.Printf("Invalid Parameter Setting:\nMultiple data sets are not permitted with isotonic scan in this version of SaTScan.\n", BasePrint::P_PARAMERROR);
      }
      if (!(gParameters.GetProbabilityModelType() == POISSON || gParameters.GetProbabilityModelType() == BERNOULLI)) {
        bReturn = false;
        PrintDirection.Printf("Invalid Parameter Setting:\nThe isotonic scan is implemented for only the Poisson and Bernoulli models.\n", BasePrint::P_PARAMERROR);
      }
      if (!gParameters.UseMetaLocationsFile() && gParameters.GetSpatialWindowType() == ELLIPTIC) {
        bReturn = false;
        PrintDirection.Printf("Invalid Parameter Setting:\nThe isotonic scan is not implemented for elliptic shaped windows.\n", BasePrint::P_PARAMERROR);
      }
    }
  }
  catch (prg_exception& x) {
    x.addTrace("ValidateMonotoneRisk()","ParametersValidate");
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
        PrintDirection.Printf("Invalid Parameter Setting:\nThe maximum spatial cluster size can not be specified interms of as percentage\n"
                              "of population defined in maximum circle file. This feature is not implemented for %s model.\n", 
                              BasePrint::P_PARAMERROR, ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
    }
    if (gParameters.GetExecutionType() == CENTRICALLY) {
      bReturn = false;
      PrintDirection.Printf("Invalid Parameter Setting:\nCentric analysis execution is not a valid settings for %s model.\n", 
                            BasePrint::P_PARAMERROR, ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
    }
    if (gParameters.GetSpatialWindowType() == ELLIPTIC) {
      bReturn = false;
      PrintDirection.Printf("Invalid Parameter Setting:\nElliptical scans are not implemented for %s model.\n", 
                            BasePrint::P_PARAMERROR, ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
    }
    if (gParameters.GetNumDataSets() > 1) {
       bReturn = false;
       PrintDirection.Printf("Invalid Parameter Setting:\nMultiple data sets are not permitted with %s model.\n", 
                             BasePrint::P_PARAMERROR, ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
    }
    if (gParameters.GetCoordinatesType() == LATLON) {
      bReturn = false;
      PrintDirection.Printf("Invalid Parameter Setting:\nLatitude/Longitude coordinates are not implemented for %s model.\n", 
                            BasePrint::P_PARAMERROR, ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
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
    for (size_t i=0; i < polygons.size() - 1; ++i) {
        for (size_t j=i+1; j < polygons.size(); ++j) {
            if (polygons[i].intersectsRegion(polygons[j]))
                throw region_exception("Inequalities define regions that overlap.\n"
                                       "Please check inequalities and/or redefine to not have overlap.");
        }
    }
  }
  catch (region_exception& x) {
    PrintDirection.Printf("Invalid Parameter Setting:\n%s\n", BasePrint::P_PARAMERROR, x.what());
    bReturn=false;
  } 
  catch (prg_exception& x) {
    x.addTrace("ValidateContinuousPoissonParameters()","ParametersValidate");
    throw;
  }
  return bReturn;
}

/** Validates output options. */
bool ParametersValidate::ValidateOutputOptionParameters(BasePrint & PrintDirection) const {
  try {
    // Sometime in a previous version, it was decided to just suppress this setting
    // for situations that don't allow it. 
    if (gParameters.GetOutputRelativeRisksFiles() &&
        (gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION || gParameters.GetProbabilityModelType() == HOMOGENEOUSPOISSON ||
         gParameters.GetProbabilityModelType() == ORDINAL || gParameters.GetProbabilityModelType() == CATEGORICAL ||
         gParameters.GetAnalysisType() == SPATIALVARTEMPTREND)) {
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

  } 
  catch (prg_exception& x) {
    x.addTrace("ValidateOutputOptionParameters()","ParametersValidate");
    throw;
  }
  return true;
}

/** Validates power calculation parameters.
   Prints errors to print direction and returns validity. */
bool ParametersValidate::ValidatePowerCalculationParameters(BasePrint& PrintDirection) const {
  bool  bValid=true;

  try {
    if (gParameters.GetIsPowerCalculated()) {
      if (gParameters.GetNumReplicationsRequested() == 0) {
        PrintDirection.Printf("Invalid Parameter Setting:\nThe power calculation feature requires that Monte Carlo simulations be performed.\n", BasePrint::P_PARAMERROR);
        return false;
      }
      if (0.0 > gParameters.GetPowerCalculationX() || gParameters.GetPowerCalculationX() > std::numeric_limits<double>::max()) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\n"
                              "The power calculation value X. Please use a value between 0 and %12.4f\n",
                              BasePrint::P_PARAMERROR, std::numeric_limits<double>::max());
      }
      if (0.0 > gParameters.GetPowerCalculationY() || gParameters.GetPowerCalculationY() > std::numeric_limits<double>::max()) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\n"
                              "The power calculation value Y. Please use a value between 0 and %12.4f\n",
                              BasePrint::P_PARAMERROR, std::numeric_limits<double>::max());
      }
    }
  }
  catch (prg_exception& x) {
    x.addTrace("ValidatePowerCalculationParameters()","ParametersValidate");
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

  try {
    //validate study period end date based upon precision of times parameter setting
    //parse date in parts
    if (CharToMDY(&uiMonth, &uiDay, &uiYear, gParameters.GetProspectiveStartDate().c_str()) != 3) {
      PrintDirection.Printf("Invalid Parameter Setting:\n"
                            "The specified prospective surveillance start date, '%s', is not valid. "
                            "Please specify as YYYY/MM/DD.\n", BasePrint::P_PARAMERROR, gParameters.GetProspectiveStartDate().c_str());
      return false;
    }
    //validate date
    if (!IsDateValid(uiMonth, uiDay, uiYear)) {
      PrintDirection.Printf("Invalid Parameter Setting:\nThe specified prospective surveillance start date, %s, is not a valid date.\n",
                            BasePrint::P_PARAMERROR, gParameters.GetProspectiveStartDate().c_str());
      bReturnValue = false;
    }
  }
  catch (prg_exception& x) {
    x.addTrace("ValidateProspectiveDate()","ParametersValidate");
    throw;
  }
  return bReturnValue;
}

/** Validates parameters parameters related to randomization seed.
    Prints errors to print direction and returns whether values are vaild. */
bool ParametersValidate::ValidateRandomizationSeed(BasePrint& PrintDirection) const {
  double        dMaxRandomizationSeed, dMaxReplications, dMaxSeed;

  if (gParameters.GetNumReplicationsRequested()) {
    if (gParameters.GetIsRandomlyGeneratingSeed()) {
      dMaxSeed = (double)RandomNumberGenerator::glM - (double)gParameters.GetNumReplicationsRequested() - (double)(gParameters.GetNumDataSets() -1) * AbstractRandomizer::glDataSetSeedOffSet - 1;
      boost::minstd_rand generator(static_cast<int>(time(0)));
      const_cast<CParameters&>(gParameters).SetRandomizationSeed(boost::uniform_int<>(1,static_cast<int>(dMaxSeed))(generator));
      return true;
    }
    //validate hidden parameter which specifies randomization seed
    if (!(0 < gParameters.GetRandomizationSeed() && gParameters.GetRandomizationSeed() < RandomNumberGenerator::glM)) {
      PrintDirection.Printf("Invalid Parameter Setting:\nRandomization seed out of range [1 - %ld].\n",
                            BasePrint::P_PARAMERROR, RandomNumberGenerator::glM - 1);
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
        PrintDirection.Printf("Invalid Parameter Setting:\nRequested number of replications causes randomization seed to exceed defined limit. "
                              "Maximum number of replications is %.0lf.\n", BasePrint::P_PARAMERROR, dMaxReplications);
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
        PrintDirection.Printf("Invalid Parameter Setting:\nRandomization seed will exceed defined limit. "
                              "The specified initial seed, in conjunction with the number of replications, "
                              "contend for numerical range in defined limits. Please modify the specified "
                              "initial seed and/or lessen the number of replications and try again.\n", BasePrint::P_PARAMERROR);
      }
      //check that randomization seed is not so large that we can't run any replications
      else if (dMaxReplications < 9) {
        PrintDirection.Printf("Invalid Parameter Setting:\nRandomization seed will exceed defined limit. "
                              "The intial seed specified prevents any replications from being performed. "
                              "With %ld replications, the initial seed can be [0 - %.0lf].\n",
                              BasePrint::P_PARAMERROR, gParameters.GetNumReplicationsRequested(), dMaxSeed);
      }
      //check that number of replications isn't too large
      else if (dMaxSeed <= 0 || dMaxSeed > RandomNumberGenerator::glM) {
        PrintDirection.Printf("Invalid Parameter Setting:\nRequested number of replications causes randomization seed to exceed defined limit. "
                              "With initial seed of %i, maximum number of replications is %.0lf.\n",
                              BasePrint::P_PARAMERROR, gParameters.GetRandomizationSeed(), dMaxReplications);
      }
      else {
        PrintDirection.Printf("Invalid Parameter Setting:\nRandomization seed will exceed defined limit. "
                              "Either limit the number of replications to %.0lf or "
                              "define the initial seed to a value less than %.0lf.\n",
                              BasePrint::P_PARAMERROR, dMaxReplications, dMaxSeed);
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
        PrintDirection.Printf("Invalid Parameter Setting:\nThe start date of start range in flexible temporal window definition, "
                              "'%s', is not valid. Please specify as YYYY/MM/DD.\n",
                              BasePrint::P_PARAMERROR, gParameters.GetStartRangeStartDate().c_str());
        bValid = false;
      }
      if (!IsDateValid(uiMonth, uiDay, uiYear)) {
        PrintDirection.Printf("Invalid Parameter Setting:\nThe start date of start range in flexible temporal window definition, "
                              "%s, is not a valid date.\n", BasePrint::P_PARAMERROR, gParameters.GetStartRangeStartDate().c_str());
        bValid = false;
      }
      //validate start range end date
      if (CharToMDY(&uiMonth, &uiDay, &uiYear, gParameters.GetStartRangeEndDate().c_str()) != 3) {
        PrintDirection.Printf("Invalid Parameter Setting:\nThe end date of start range in flexible temporal window definition, "
                              "'%s', is not valid. Please specify as YYYY/MM/DD.\n",
                              BasePrint::P_PARAMERROR, gParameters.GetStartRangeEndDate().c_str());
        bValid = false;
      }
      if (!IsDateValid(uiMonth, uiDay, uiYear)) {
        PrintDirection.Printf("Invalid Parameter Setting:\nThe end date of start range in flexible temporal window definition, "
                              "%s, is not a valid date.\n", BasePrint::P_PARAMERROR, gParameters.GetStartRangeEndDate().c_str());
        bValid = false;
      }
      //validate end range start date
      if (CharToMDY(&uiMonth, &uiDay, &uiYear, gParameters.GetEndRangeStartDate().c_str()) != 3) {
        PrintDirection.Printf("Invalid Parameter Setting:\nThe start date of end range in flexible temporal window definition, "
                              "'%s', is not valid. Please specify as YYYY/MM/DD.\n",
                              BasePrint::P_PARAMERROR, gParameters.GetEndRangeStartDate().c_str());
        bValid = false;
      }
      if (!IsDateValid(uiMonth, uiDay, uiYear)) {
        PrintDirection.Printf("Invalid Parameter Setting:\nThe start date of end range in flexible temporal window definition, "
                              "%s, is not a valid date.\n", BasePrint::P_PARAMERROR, gParameters.GetEndRangeStartDate().c_str());
        bValid = false;
      }
      //validate end range end date
      if (CharToMDY(&uiMonth, &uiDay, &uiYear, gParameters.GetEndRangeStartDate().c_str()) != 3) {
        PrintDirection.Printf("Invalid Parameter Setting:\nThe end date of end range in flexible temporal window definition, "
                              "'%s', is not valid. Please specify as YYYY/MM/DD.\n",
                              BasePrint::P_PARAMERROR, gParameters.GetEndRangeStartDate().c_str());
        bValid = false;
      }
      if (!IsDateValid(uiMonth, uiDay, uiYear)) {
        PrintDirection.Printf("Invalid Parameter Setting:\nThe end date of end range in flexible temporal window definition, "
                              "%s, is not a valid date.\n", BasePrint::P_PARAMERROR, gParameters.GetEndRangeStartDate().c_str());
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
          PrintDirection.Printf("Invalid Parameter Setting:\nInvalid scanning window end range. Range date '%s' occurs after date '%s'.\n",
                                BasePrint::P_PARAMERROR, gParameters.GetEndRangeStartDate().c_str(), gParameters.GetEndRangeEndDate().c_str());
        }
        else {
          if (EndRangeStartDate < StudyPeriodStartDate || EndRangeStartDate > StudyPeriodEndDate) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nThe scanning window end range date '%s' is not within the study period (%s - %s).\n",
                                  BasePrint::P_PARAMERROR, gParameters.GetEndRangeStartDate().c_str(),
                                  gParameters.GetStudyPeriodStartDate().c_str(), gParameters.GetStudyPeriodEndDate().c_str());
          }
          if (EndRangeEndDate < StudyPeriodStartDate || EndRangeEndDate > StudyPeriodEndDate) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nThe scanning window end range date '%s' is not within the study period (%s - %s) \n",
                                  BasePrint::P_PARAMERROR, gParameters.GetEndRangeEndDate().c_str(),
                                  gParameters.GetStudyPeriodStartDate().c_str(), gParameters.GetStudyPeriodEndDate().c_str());
          }
        }

        StartRangeStartDate = CharToJulian(gParameters.GetStartRangeStartDate().c_str());
        StartRangeEndDate = CharToJulian(gParameters.GetStartRangeEndDate().c_str());
        if (StartRangeStartDate > StartRangeEndDate) {
          bValid = false;
          PrintDirection.Printf("Invalid Parameter Setting:\nInvalid scanning window start range. The range date '%s' occurs after date '%s'.\n",
                                BasePrint::P_PARAMERROR, gParameters.GetStartRangeStartDate().c_str(),
                                gParameters.GetStartRangeEndDate().c_str());
        }
        else {
          if (StartRangeStartDate < StudyPeriodStartDate || StartRangeStartDate > StudyPeriodEndDate) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nThe scanning window start range date '%s' is not within the study period (%s - %s).\n",
                                  BasePrint::P_PARAMERROR, gParameters.GetStartRangeStartDate().c_str(),
                                  gParameters.GetStudyPeriodStartDate().c_str(), gParameters.GetStudyPeriodEndDate().c_str());
          }
          if (StartRangeEndDate < StudyPeriodStartDate || StartRangeEndDate > StudyPeriodEndDate) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nThe scanning window start range date '%s' is not within the study period (%s - %s) \n",
                                  BasePrint::P_PARAMERROR, gParameters.GetStartRangeEndDate().c_str(),
                                  gParameters.GetStudyPeriodStartDate().c_str(), gParameters.GetStudyPeriodEndDate().c_str());
          }
        }
        if (StartRangeStartDate >= EndRangeEndDate) {
          bValid = false;
          PrintDirection.Printf("Invalid Parameter Setting:\nThe scanning window start range does not occur before the end range.\n", BasePrint::P_PARAMERROR);
        }
      }
    }
  }
  catch (prg_exception& x) {
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
      PrintDirection.Printf("Invalid Parameter Setting:\nSimulation data output file not specified.\n", BasePrint::P_PARAMERROR);
    }

    switch (gParameters.GetSimulationType()) {
      case STANDARD         : break;
      case HA_RANDOMIZATION :
        if (gParameters.GetProbabilityModelType() == POISSON) {
          if (gParameters.GetAdjustmentsByRelativeRisksFilename().empty()) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nNo adjustments file specified.\n", BasePrint::P_PARAMERROR);
          }
          else if (access(gParameters.GetAdjustmentsByRelativeRisksFilename().c_str(), 00)) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nThe adjustments file '%s' does not exist. Please ensure the path is correct.\n",
                                  BasePrint::P_PARAMERROR, gParameters.GetAdjustmentsByRelativeRisksFilename().c_str());
          }
        }
        else {
          bValid = false;
          PrintDirection.Printf("Invalid Parameter Setting:\nThe alternative hypothesis method of creating simulated data"
                                "is only implemented for the Poisson model.\n", BasePrint::P_PARAMERROR);
        }
        break;
      case FILESOURCE       :
        if (gParameters.GetProbabilityModelType() == EXPONENTIAL || gParameters.GetProbabilityModelType() == NORMAL ||
            gParameters.GetProbabilityModelType() == RANK) {
          bValid = false;
          PrintDirection.Printf("Invalid Parameter Setting:\nThe feature to read simulated data from a file is not implemented for "
                                "the %s probability model.\n", BasePrint::P_PARAMERROR,
                                ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
        }
        if (gParameters.GetNumDataSets() > 1){
          bValid = false;
          PrintDirection.Printf("Invalid Parameter Setting:\nThe feature to read simulated data from a file is not implemented for analyses "
                                "that read data from multiple data sets.\n", BasePrint::P_PARAMERROR);
        }
        if (gParameters.GetSimulationDataSourceFilename().empty()) {
          bValid = false;
          PrintDirection.Printf("Invalid Parameter Setting:\nThe simulated data input file was not specified.\n", BasePrint::P_PARAMERROR);
        }
        else if (access(gParameters.GetSimulationDataSourceFilename().c_str(), 00)) {
          bValid = false;
          PrintDirection.Printf("Invalid Parameter Setting:\nThe simulated data input file '%s' does not exist. Please ensure the path is correct.\n",
                                BasePrint::P_PARAMERROR, gParameters.GetSimulationDataSourceFilename().c_str());
        }
        if (gParameters.GetOutputSimulationData() && gParameters.GetSimulationDataSourceFilename() == gParameters.GetSimulationDataOutputFilename()) {
          bValid = false;
          PrintDirection.Printf("Invalid Parameter Setting:\nThe file '%s' is specified as both the input and the output file for simulated data.\n",
                                BasePrint::P_PARAMERROR, gParameters.GetSimulationDataSourceFilename().c_str());
        }
        break;
      default : throw prg_error("Unknown simulation type '%d'.","ValidateSimulationDataParameters()", gParameters.GetSimulationType());
    };
  }
  catch (prg_exception& x) {
    x.addTrace("ValidateSimulationDataParameters()","ParametersValidate");
    throw;
  }
  return bValid;
}

/** Validates optional parameters particular to spatial analyses
    (i.e. purely spatial, retrospective space-time and prospective space-time).
    Prints errors to print direction and returns whether values are vaild. */
bool ParametersValidate::ValidateSpatialParameters(BasePrint & PrintDirection) const {
  bool          bValid=true;
  double        dValue;

  try {
    //validate spatial options
    if (!gParameters.GetIsPurelyTemporalAnalysis()) {
      //validate maximum is specified as a pecentage of population at risk when using neighbors file
      if (gParameters.UseLocationNeighborsFile() && gParameters.GetRestrictMaxSpatialSizeForType(MAXDISTANCE, false)) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\nWhen specifying geographical neighbors through the non-Eucledian neighbors file, "
                              "the maximum spatial cluster size can not be defined as a fixed distance.", BasePrint::P_PARAMERROR);
      }
      //validate reported maximum is specified as a pecentage of population at risk when using neighbors file
      if (gParameters.UseLocationNeighborsFile() && gParameters.GetRestrictMaxSpatialSizeForType(MAXDISTANCE, true)) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\nWhen specifying geographical neighbors through the non-Eucledian neighbors file, "
                              "the maximum reported spatial cluster size can not be defined as a fixed distance.", BasePrint::P_PARAMERROR);
      }
      //validate maximum as pecentage of population at risk
      dValue = gParameters.GetMaxSpatialSizeForType(PERCENTOFPOPULATION, false);
      if (!(gParameters.GetAnalysisType() == PROSPECTIVESPACETIME && gParameters.GetAdjustForEarlierAnalyses()) && dValue <= 0.0 || dValue > 100.0/*50.0*/) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\nThe maximum spatial cluster size, defined as percentage of population at risk, is invalid. "
                              "The specified value is %2g. Must be greater than zero and <= %d.\n", BasePrint::P_PARAMERROR, dValue, 100/*50*/);
      }
      //validate maximum as pecentage of population defined in max circle file
      dValue = gParameters.GetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false);
      if (gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false) && dValue <= 0.0 || dValue > 100.0/*50.0*/) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\nThe maximum spatial cluster size, defined as percentage of population in max circle file, is invalid. "
                              "The specified value is %2g. Must be greater than zero and <= %d.\n", BasePrint::P_PARAMERROR, dValue, 100/*50*/);
      }
      //validate maximum as pecentage of population defined in max circle file
      dValue = gParameters.GetMaxSpatialSizeForType(MAXDISTANCE, false);
      if (gParameters.GetRestrictMaxSpatialSizeForType(MAXDISTANCE, false) && dValue <= 0.0) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\nThe maximum spatial cluster size of %2g units is invalid. "
                              "The specified value is %2g. Must be greater than zero.\n", BasePrint::P_PARAMERROR, dValue);
      }
      //validate maximum as pecentage of population at risk -- reported
      dValue = gParameters.GetMaxSpatialSizeForType(PERCENTOFPOPULATION, true);
      if (!(gParameters.GetAnalysisType() == PROSPECTIVESPACETIME && gParameters.GetAdjustForEarlierAnalyses()) && dValue <= 0.0 || dValue > 100.0/*50.0*/) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\nThe maximum reported spatial cluster size, defined as percentage of population at risk, is invalid. "
                              "The specified value is %2g. Must be greater than zero and <= %d.\n", BasePrint::P_PARAMERROR, dValue, 100/*50*/);
      }
      //validate maximum as pecentage of population defined in max circle file -- reported
      dValue = gParameters.GetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true);
      if (gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true) && dValue <= 0.0 || dValue > 100.0/*50.0*/) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\nThe maximum reported spatial cluster size, defined as percentage of population in max circle file, is invalid. "
                              "The specified value is %2g. Must be greater than zero and <= %d.\n", BasePrint::P_PARAMERROR, dValue, 100/*50*/);
      }
      //validate maximum as pecentage of population defined in max circle file  -- reported
      dValue = gParameters.GetMaxSpatialSizeForType(MAXDISTANCE, true);
      if (gParameters.GetRestrictMaxSpatialSizeForType(MAXDISTANCE, true) && dValue <= 0.0) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\nThe maximum reported spatial cluster size of %2g units is invalid. "
                              "The specified value is %2g. Must be greater than zero.\n", BasePrint::P_PARAMERROR, dValue);
      }
    }

    if (gParameters.GetIncludePurelySpatialClusters()) {
      if (!gParameters.GetPermitsPurelySpatialCluster(gParameters.GetProbabilityModelType())) {
          bValid = false;
          PrintDirection.Printf("Invalid Parameter Setting:\nA purely spatial cluster cannot be included for a %s model.\n",
                                BasePrint::P_PARAMERROR, ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
      }
      else if (!gParameters.GetPermitsPurelySpatialCluster()) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\nA purely spatial cluster can not be included in %s analyses.\n",
                              BasePrint::P_PARAMERROR, ParametersPrint(gParameters).GetAnalysisTypeAsString());
      }
    }
    if (gParameters.GetSpatialAdjustmentType() == SPATIALLY_STRATIFIED_RANDOMIZATION) {
      if (!(gParameters.GetAnalysisType() == SPACETIME || gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\nSpatial adjustment by stratified randomization is valid for "
                              "either retrospective or prospective space-time analyses only.\n", BasePrint::P_PARAMERROR);
      }
      if (gParameters.GetTimeTrendAdjustmentType() == STRATIFIED_RANDOMIZATION) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\nSpatial adjustment by stratified randomization can not be performed "
                              "in conjunction with the temporal adjustment by stratified randomization.\n", BasePrint::P_PARAMERROR);
      }
      if (gParameters.GetIncludePurelySpatialClusters()) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\nSpatial adjustment by stratified randomization does not permit "
                              "the inclusion of a purely spatial cluster.\n", BasePrint::P_PARAMERROR);
      }
    }
  }
  catch (prg_exception& x) {
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
    //parse date in parts
    if (CharToMDY(&nMonth, &nDay, &nYear, gParameters.GetStudyPeriodEndDate().c_str()) != 3) {
      PrintDirection.Printf("Invalid Parameter Setting:\nThe study period end date '%s' is not valid. Please specify as YYYY/MM/DD.\n",
                            BasePrint::P_PARAMERROR, gParameters.GetStudyPeriodEndDate().c_str());
      return false;
    }
    //validate date
    if (!IsDateValid(nMonth, nDay, nYear)) {
      PrintDirection.Printf("Invalid Parameter Setting:\nThe study period end date '%s' is not a valid date.\n",
                            BasePrint::P_PARAMERROR, gParameters.GetStudyPeriodEndDate().c_str());
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
          PrintDirection.Printf("Invalid Parameter Setting:\nThe study period end date '%s' is not valid. "
                                "With the setting for %s as years, the date  must be the last day of respective year.\n",
                                BasePrint::P_PARAMERROR, gParameters.GetStudyPeriodEndDate().c_str(),
                                (gParameters.GetCreationVersion().iMajor == 4 ? "time aggregation" : "time precision"));
          return false;
        }
        break;
      case MONTH :
        if (nDay != DaysThisMonth(nYear, nMonth)) {
          PrintDirection.Printf("Invalid Parameter Setting:\nThe study period end date '%s' is not valid. "
                                "With the setting for %s as months, the date must be the last day of respective month.\n",
                                BasePrint::P_PARAMERROR, gParameters.GetStudyPeriodEndDate().c_str(),
                                (gParameters.GetCreationVersion().iMajor == 4 ? "time aggregation" : "time precision"));
          return false;
        }
      case DAY   :
      case NONE  : break;
    };
  }
  catch (prg_exception& x) {
    x.addTrace("ValidateEndDate()","ParametersValidate");
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
      PrintDirection.Printf("Invalid Parameter Setting:\nThe study period start date, '%s', is not valid. Please specify as YYYY/MM/DD.\n",
                            BasePrint::P_PARAMERROR, gParameters.GetStudyPeriodStartDate().c_str());
      return false;
    }
    //validate date
    if (!IsDateValid(nMonth, nDay, nYear)) {
      PrintDirection.Printf("Invalid Parameter Setting:\nThe study period start date, '%s', is not valid date.\n",
                            BasePrint::P_PARAMERROR, gParameters.GetStudyPeriodStartDate().c_str());
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
          PrintDirection.Printf("Invalid Parameter Setting:\nThe study period start date, '%s', is not valid. "
                                "With the setting for %s as years, the date must be the first day of respective year.\n",
                                BasePrint::P_PARAMERROR, gParameters.GetStudyPeriodStartDate().c_str(),
                                (gParameters.GetCreationVersion().iMajor == 4 ? "time aggregation" : "time precision"));
          return false;
        }
        break;
      case MONTH :
        if (nDay != 1) {
          PrintDirection.Printf("Invalid Parameter Setting:\nThe study period start date, '%s', is not valid. "
                                "With the setting for %s as months, the date must be the first day of respective month.\n",
                                BasePrint::P_PARAMERROR, gParameters.GetStudyPeriodStartDate().c_str(),
                                (gParameters.GetCreationVersion().iMajor == 4 ? "time aggregation" : "time precision"));
          return false;
        }
      case DAY   :
      case NONE  : break;
    };
  }
  catch (prg_exception& x) {
    x.addTrace("ValidateStartDate()","ParametersValidate");
    throw;
  }
  return true;
}

/** Validates spatial variation in temporal trends analysis settings. */
bool ParametersValidate::ValidateSVTTAnalysisSettings(BasePrint& PrintDirection) const {
  bool          bValid=true;

  if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
    if (gParameters.GetNumDataSets() > 1) {
      bValid = false;
      PrintDirection.Printf("Invalid Parameter Setting:\n"
                            "Multiple data sets is not implemented with spatial variation in temporal trends.\n", BasePrint::P_PARAMERROR);
    }
    if (gParameters.GetProbabilityModelType() != POISSON) {
      bValid = false;
      PrintDirection.Printf("Invalid Parameter Setting:\n"
                            "Spatial variation in temporal trends is implemented only for the Poisson model.\n", BasePrint::P_PARAMERROR);
    }
    if (gParameters.GetTimeTrendConvergence() < 0.0) {
      bValid = false;
      PrintDirection.Printf("Invalid Parameter Setting:\nTime trend convergence value of '%2g' is less than zero.\n",
                            BasePrint::P_PARAMERROR, gParameters.GetTimeTrendConvergence());
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
          PrintDirection.Printf("Notice:\nFor the Bernoulli model, adjusting for temporal trends is not permitted. "
                                "Temporal trends adjustment settings will be ignored.\n", BasePrint::P_NOTICE);
          const_cast<CParameters&>(gParameters).SetTimeTrendAdjustmentType(NOTADJUSTED);
          const_cast<CParameters&>(gParameters).SetTimeTrendAdjustmentPercentage(0);
        }
        break;
      case CATEGORICAL          :
      case ORDINAL              :
      case EXPONENTIAL          :
      case NORMAL               :
      case RANK                 :
        if (gParameters.GetTimeTrendAdjustmentType() != NOTADJUSTED) {
          PrintDirection.Printf("Notice:\nFor the %s model, adjusting for temporal trends is not permitted."
                                "Temporal trends adjustment settings will be ignored.\n",
                                BasePrint::P_NOTICE, ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
          const_cast<CParameters&>(gParameters).SetTimeTrendAdjustmentType(NOTADJUSTED);
          const_cast<CParameters&>(gParameters).SetTimeTrendAdjustmentPercentage(0);
        }
        break;
      case SPACETIMEPERMUTATION :
        if (gParameters.GetTimeTrendAdjustmentType() != NOTADJUSTED) {
          PrintDirection.Printf("Notice:\nFor the space-time permutation model, adjusting for temporal trends "
                                "is not permitted nor needed, as this model automatically adjusts for "
                                "any temporal variation. Temporal trends adjustment settings will be ignored.\n",
                                BasePrint::P_NOTICE);
          const_cast<CParameters&>(gParameters).SetTimeTrendAdjustmentType(NOTADJUSTED);
          const_cast<CParameters&>(gParameters).SetTimeTrendAdjustmentPercentage(0);
        }
        break;
      case POISSON             :
        if (gParameters.GetTimeTrendAdjustmentType() != NOTADJUSTED && gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
          bValid = false;
          PrintDirection.Printf("Invalid Parameter Setting:\nTemporal adjustments can not be performed for a spatial variation in "
                                "temporal trends analysis.\n", BasePrint::P_PARAMERROR);
        }
        if (gParameters.GetTimeTrendAdjustmentType() != NOTADJUSTED && (gParameters.GetAnalysisType() == PURELYTEMPORAL || gParameters.GetAnalysisType() == PROSPECTIVEPURELYTEMPORAL)
            && gParameters.GetPopulationFileName().empty()) {
          bValid = false;
          PrintDirection.Printf("Invalid Parameter Setting:\nTemporal adjustments can not be performed for a purely temporal analysis "
                                "using the Poisson model, when no population file has been specfied.\n", BasePrint::P_PARAMERROR);
        }
        if (gParameters.GetTimeTrendAdjustmentType() == NONPARAMETRIC && (gParameters.GetAnalysisType() == PURELYTEMPORAL ||gParameters.GetAnalysisType() == PROSPECTIVEPURELYTEMPORAL)) {
          bValid = false;
          PrintDirection.Printf("Invalid Parameter Setting:\nInvalid parameter setting for time trend adjustment. "
                                "You may not use non-parametric time in a purely temporal analysis.\n", BasePrint::P_PARAMERROR);
        }
        if (gParameters.GetTimeTrendAdjustmentType() == STRATIFIED_RANDOMIZATION && (gParameters.GetAnalysisType() == PURELYTEMPORAL || gParameters.GetAnalysisType() == PROSPECTIVEPURELYTEMPORAL)) {
          bValid = false;
          PrintDirection.Printf("Invalid Parameter Setting:\nTemporal adjustment by stratified randomization is not valid "
                                 "for purely temporal analyses.\n", BasePrint::P_PARAMERROR);
        }
        if (gParameters.GetTimeTrendAdjustmentType() == LOGLINEAR_PERC && -100.0 >= gParameters.GetTimeTrendAdjustmentPercentage()) {
          bValid = false;
          PrintDirection.Printf("Invalid Parameter Setting:\nThe time adjustment percentage is '%2g', but must greater than -100.\n",
                                BasePrint::P_PARAMERROR, gParameters.GetTimeTrendAdjustmentPercentage());
        }
        if (gParameters.GetTimeTrendAdjustmentType() == NOTADJUSTED) {
          const_cast<CParameters&>(gParameters).SetTimeTrendAdjustmentPercentage(0);
          if (gParameters.GetAnalysisType() != SPATIALVARTEMPTREND) const_cast<CParameters&>(gParameters).SetTimeTrendConvergence(0);
        }
        if (gParameters.GetTimeTrendAdjustmentType() == CALCULATED_LOGLINEAR_PERC && gParameters.GetTimeTrendConvergence() < 0.0) {
           bValid = false;
           PrintDirection.Printf("Invalid Parameter Setting:\nTime trend convergence value of '%2g' is less than zero.\n",
                                 BasePrint::P_PARAMERROR, gParameters.GetTimeTrendConvergence());
        }
        break;
      default : throw prg_error("Unknown model type '%d'.", "ValidateTemporalParameters()", gParameters.GetProbabilityModelType());
    }
    //validate including purely temporal clusters
    if (gParameters.GetIncludePurelyTemporalClusters()) {
      if (!gParameters.GetPermitsPurelyTemporalCluster(gParameters.GetProbabilityModelType())) {
          bValid = false;
          PrintDirection.Printf("Invalid Parameter Setting:\nScanning for purely temporal clusters can not be included when the %s model is used.\n",
                                BasePrint::P_PARAMERROR, ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
      }
      else if (!gParameters.GetPermitsPurelyTemporalCluster()) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\nA purely temporal cluster can not be included with %s analyses.\n",
                              BasePrint::P_PARAMERROR, ParametersPrint(gParameters).GetAnalysisTypeAsString());
      }
    }

    //the locations neighbor file is irrelevant when analysis type is purely temporal
    if (gParameters.GetIsPurelyTemporalAnalysis()) const_cast<CParameters&>(gParameters).UseLocationNeighborsFile(false);
    //since there is not location data, we can no report relative risk estimates per location
  }
  catch (prg_exception& x) {
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
    PrintDirection.Printf("Invalid Parameter Setting:\nTime aggregation units can not be 'none' for a temporal analysis.\n", BasePrint::P_PARAMERROR);
    return false;
  }
  if (gParameters.GetTimeAggregationUnitsType() > gParameters.GetPrecisionOfTimesType()) {
    GetDatePrecisionAsString(gParameters.GetTimeAggregationUnitsType(), sPrecisionString, true, false);
    GetDatePrecisionAsString(gParameters.GetPrecisionOfTimesType(), sBuffer, true, false);
    PrintDirection.Printf("Invalid Parameter Setting:\nA time aggregation unit in %s exceeds precision of input case data (unit is %s).\n",
                          BasePrint::P_PARAMERROR, sPrecisionString.c_str(), sBuffer.c_str());
    return false;
  }
  if (gParameters.GetTimeAggregationLength() <= 0) {
    PrintDirection.Printf("Invalid Parameter Setting:\nThe time aggregation length of '%d' is invalid. Length must be greater than zero.\n",
                          BasePrint::P_PARAMERROR, gParameters.GetTimeAggregationLength());
    return false;
  }
  //validate that the time aggregation length agrees with the study period and maximum temporal cluster size
  dStudyPeriodLengthInUnits = ceil(CalculateNumberOfTimeIntervals(CharToJulian(gParameters.GetStudyPeriodStartDate().c_str()),
                                                                  CharToJulian(gParameters.GetStudyPeriodEndDate().c_str()),
                                                                  gParameters.GetTimeAggregationUnitsType(), 1));
  if (dStudyPeriodLengthInUnits < static_cast<double>(gParameters.GetTimeAggregationLength()))  {
    PrintDirection.Printf("Invalid Parameter Setting:\nA time aggregation of %d %s%s is greater than the %d %s study period.\n",
                          BasePrint::P_PARAMERROR, gParameters.GetTimeAggregationLength(), sPrecisionString.c_str(),
                          (gParameters.GetTimeAggregationLength() == 1 ? "" : "s"),
                          static_cast<int>(dStudyPeriodLengthInUnits), sPrecisionString.c_str());
    return false;
  }
  if (ceil(dStudyPeriodLengthInUnits/static_cast<double>(gParameters.GetTimeAggregationLength())) <= 1) {
    PrintDirection.Printf("Invalid Parameter Setting:\nA time aggregation of %d %s%s with a %d %s study period results in only "
                          "one time period to analyze. Temporal and space-time analyses can not be performed "
                          "on less than two time periods.\n", BasePrint::P_PARAMERROR,
                          gParameters.GetTimeAggregationLength(), sPrecisionString.c_str(), (gParameters.GetTimeAggregationLength() == 1 ? "" : "s"),
                          static_cast<int>(dStudyPeriodLengthInUnits), sPrecisionString.c_str());
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
      PrintDirection.Printf("Invalid Parameter Setting:\nThe time aggregation of %d %s%s is greater than the maximum temporal "
                            "cluster size of %g %s%s.\n",
                            BasePrint::P_PARAMERROR, gParameters.GetTimeAggregationLength(),
                            sPrecisionString.c_str(), (gParameters.GetTimeAggregationLength() == 1 ? "" : "s"),
                            gParameters.GetMaximumTemporalClusterSize(), sPrecisionString.c_str(),
                            (gParameters.GetMaximumTemporalClusterSize() == 1 ? "" : "s"));
    else if (gParameters.GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE)
      PrintDirection.Printf("Invalid Parameter Setting:\nWith the maximum temporal cluster size as %g percent of a %d %s study period, "
                            "the time aggregation as %d %s%s is greater than the resulting maximum "
                            "temporal cluster size of %g %s%s.\n", BasePrint::P_PARAMERROR,
                            gParameters.GetMaximumTemporalClusterSize(), static_cast<int>(dStudyPeriodLengthInUnits),
                            sPrecisionString.c_str(), gParameters.GetTimeAggregationLength(),
                            sPrecisionString.c_str(), (gParameters.GetTimeAggregationLength() == 1 ? "" : "s"),
                            dMaxTemporalLengthInUnits, sPrecisionString.c_str(),
                            (dMaxTemporalLengthInUnits == 1 ? "" : "s"));
    return false;
  }
    
  return true;
}

