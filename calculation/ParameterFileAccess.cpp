//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "ParameterFileAccess.h"
#include "ScanLineParameterFileAccess.h"
#include "IniParameterFileAccess.h"

/** constructor */
ParameterAccessCoordinator::ParameterAccessCoordinator(CParameters& Parameters)
                           :gParameters(Parameters) {}

/** destructor */
ParameterAccessCoordinator::~ParameterAccessCoordinator() {}

/** Determines format of parameter file and invokes particular parameter reader
    class to read parameters from file. */
bool ParameterAccessCoordinator::Read(const char* sFilename, BasePrint& PrintDirection) {
  bool  bSuccess=false;
  try {
    if (access(sFilename, 04) == -1)
      GenerateResolvableException("Unable to open parameter file '%s'.\n", "Read()", sFilename);

    if (ZdIniFile(sFilename, true, false).GetNumSections())
      bSuccess = IniParameterFileAccess(gParameters, PrintDirection).Read(sFilename);
    else
      bSuccess = ScanLineParameterFileAccess(gParameters, PrintDirection).Read(sFilename);
  }
  catch (ZdException &x) {
    GenerateResolvableException("Unable to read parameters from file '%s'.\n", "Read()", sFilename);
  }
  return bSuccess;
}

/** Writes parameters to ini file in most recent format. */
void ParameterAccessCoordinator::Write(const char * sFilename, BasePrint& PrintDirection /* ability to specify a version to write as ?*/) {
  try {
    IniParameterFileAccess(gParameters, PrintDirection).Write(sFilename);
  }
  catch (ZdException &x) {
    x.AddCallpath("Write()","ParameterAccessCoordinator");
    throw;
  }
}


/** constructor */
AbtractParameterFileAccess::AbtractParameterFileAccess(CParameters& Parameters, BasePrint& PrintDirection)
                           :gParameters(Parameters), gPrintDirection(PrintDirection) {}

/** destructor */
AbtractParameterFileAccess::~AbtractParameterFileAccess() {}

/** Prints message to print direction that parameter was missing when read from
    parameter file and that a default value as assigned. */
void AbtractParameterFileAccess::MarkAsMissingDefaulted(ParameterType eParameterType, BasePrint& PrintDirection) {
  ZdString      sDefaultValue, sParameterLineLabel;

  try {
    switch (eParameterType) {
      case ANALYSISTYPE             : sDefaultValue = gParameters.GetAnalysisType(); break;
      case SCANAREAS                : sDefaultValue = gParameters.GetAreaScanRateType(); break;
      case CASEFILE                 : sDefaultValue = "<blank>"; break;
      case POPFILE                  : sDefaultValue = "<blank>"; break;
      case COORDFILE                : sDefaultValue = "<blank>"; break;
      case OUTPUTFILE               : sDefaultValue = "<blank>"; break;
      case PRECISION                : sDefaultValue = gParameters.GetPrecisionOfTimesType(); break;
      case DIMENSION                : /*  */ break;
      case SPECIALGRID              : sDefaultValue = (gParameters.UseSpecialGrid() ? YES : NO); break;
      case GRIDFILE                 : sDefaultValue = "<blank>"; break;
      case GEOSIZE                  : sDefaultValue = gParameters.GetMaximumGeographicClusterSize(); break;
      case STARTDATE                : sDefaultValue = gParameters.GetStudyPeriodStartDate().c_str(); break;
      case ENDDATE                  : sDefaultValue = gParameters.GetStudyPeriodEndDate().c_str(); break;
      case CLUSTERS                 : sDefaultValue = gParameters.GetIncludeClustersType(); break;
      case EXACTTIMES               : /* no longer used */ break;
      case TIME_AGGREGATION_UNITS   : sDefaultValue = gParameters.GetTimeAggregationUnitsType(); break;
      case TIME_AGGREGATION         : sDefaultValue = gParameters.GetTimeAggregationLength(); break;
      case PURESPATIAL              : sDefaultValue = (gParameters.GetIncludePurelySpatialClusters() ? YES : NO); break;
      case TIMESIZE                 : sDefaultValue = gParameters.GetMaximumTemporalClusterSize(); break;
      case REPLICAS                 : sDefaultValue << gParameters.GetNumReplicationsRequested(); break;
      case MODEL                    : sDefaultValue = gParameters.GetProbabiltyModelType(); break;
      case RISKFUNCTION             : sDefaultValue = gParameters.GetRiskType(); break;
      case POWERCALC                : sDefaultValue = (gParameters.GetIsPowerCalculated() ? YES : NO); break;
      case POWERX                   : sDefaultValue = gParameters.GetPowerCalculationX(); break;
      case POWERY                   : sDefaultValue = gParameters.GetPowerCalculationY(); break;
      case TIMETREND                : sDefaultValue = gParameters.GetTimeTrendAdjustmentType(); break;
      case TIMETRENDPERC            : sDefaultValue = gParameters.GetTimeTrendAdjustmentPercentage(); break;
      case PURETEMPORAL             : sDefaultValue = (gParameters.GetIncludePurelyTemporalClusters() ? YES : NO); break;
      case CONTROLFILE              : sDefaultValue = "<blank>"; break;
      case COORDTYPE                : sDefaultValue = gParameters.GetCoordinatesType(); break;
      case OUTPUT_SIM_LLR_ASCII     : sDefaultValue = (gParameters.GetOutputSimLoglikeliRatiosAscii() ? YES : NO); break;
      case SEQUENTIAL               : sDefaultValue = (gParameters.GetIsSequentialScanning() ? YES : NO); break;
      case SEQNUM                   : sDefaultValue << gParameters.GetNumSequentialScansRequested(); break;
      case SEQPVAL                  : sDefaultValue = gParameters.GetSequentialCutOffPValue(); break;
      case VALIDATE                 : sDefaultValue = (gParameters.GetValidatingParameters() ? YES : NO); break;
      case OUTPUT_RR_ASCII          : sDefaultValue = (gParameters.GetOutputRelativeRisksAscii() ? YES : NO); break;
      case ELLIPSES                 : sDefaultValue = gParameters.GetNumRequestedEllipses(); break;
      case ESHAPES                  : sDefaultValue = "<blank>"; break;
      case ENUMBERS                 : sDefaultValue = "<blank>"; break;
      case START_PROSP_SURV         : sDefaultValue = gParameters.GetProspectiveStartDate().c_str(); break;
      case OUTPUT_AREAS_ASCII       : sDefaultValue = (gParameters.GetOutputAreaSpecificAscii() ? YES : NO); break;
      case OUTPUT_MLC_ASCII         : sDefaultValue = (gParameters.GetOutputClusterLevelAscii() ? YES : NO); break;
      case CRITERIA_SECOND_CLUSTERS : sDefaultValue = gParameters.GetCriteriaSecondClustersType(); break;
      case MAX_TEMPORAL_TYPE        : sDefaultValue = gParameters.GetMaximumTemporalClusterSizeType(); break;
      case MAX_SPATIAL_TYPE         : sDefaultValue = gParameters.GetMaxGeographicClusterSizeType(); break;
      case RUN_HISTORY_FILENAME     : /* no longer read in from parameter file */ break;
      case OUTPUT_MLC_DBASE         : sDefaultValue = (gParameters.GetOutputClusterLevelDBase() ? YES : NO); break;
      case OUTPUT_AREAS_DBASE       : sDefaultValue = (gParameters.GetOutputAreaSpecificDBase() ? YES : NO); break;
      case OUTPUT_RR_DBASE          : sDefaultValue = (gParameters.GetOutputRelativeRisksDBase() ? YES : NO); break;
      case OUTPUT_SIM_LLR_DBASE     : sDefaultValue = (gParameters.GetOutputSimLoglikeliRatiosDBase() ? YES : NO); break;
      case NON_COMPACTNESS_PENALTY  : sDefaultValue = (gParameters.GetNonCompactnessPenalty() ? YES : NO); break;
      case INTERVAL_STARTRANGE      : sDefaultValue.printf("%s,%s", gParameters.GetStartRangeStartDate().c_str(), gParameters.GetStartRangeEndDate().c_str());
                                      break;
      case INTERVAL_ENDRANGE        : sDefaultValue.printf("%s,%s", gParameters.GetEndRangeStartDate().c_str(), gParameters.GetEndRangeEndDate().c_str());
                                      break;
      case TIMETRENDCONVRG	    : sDefaultValue = gParameters.GetTimeTrendConvergence(); break;
      case MAXCIRCLEPOPFILE         : sDefaultValue = "<blank>"; break;
      case EARLY_SIM_TERMINATION    : sDefaultValue = (gParameters.GetTerminateSimulationsEarly() ? YES : NO); break;
      case REPORTED_GEOSIZE         : sDefaultValue = gParameters.GetMaximumReportedGeoClusterSize(); break;
      case USE_REPORTED_GEOSIZE     : sDefaultValue = (gParameters.GetRestrictingMaximumReportedGeoClusterSize() ? YES : NO); break;
      case SIMULATION_TYPE          : sDefaultValue = gParameters.GetSimulationType(); break;
      case SIMULATION_SOURCEFILE    : sDefaultValue = "<blank>"; break;
      case ADJ_BY_RR_FILE           : sDefaultValue = "<blank>"; break;
      case OUTPUT_SIMULATION_DATA   : sDefaultValue = (gParameters.GetOutputSimulationData() ? YES : NO); break;
      case SIMULATION_DATA_OUTFILE  : sDefaultValue = "<blank>"; break;
      case ADJ_FOR_EALIER_ANALYSES  : sDefaultValue = (gParameters.GetAdjustForEarlierAnalyses() ? YES : NO); break;
      case USE_ADJ_BY_RR_FILE       : sDefaultValue = (gParameters.UseAdjustmentForRelativeRisksFile() ? YES : NO); break;
      case SPATIAL_ADJ_TYPE         : sDefaultValue = gParameters.GetSpatialAdjustmentType(); break;
      case MULTI_DATASET_PURPOSE_TYPE : sDefaultValue = gParameters.GetMultipleDataStreamPurposeType(); break;
      case CREATION_VERSION         : sDefaultValue.printf("%u.%u.%u", gParameters.GetCreationVersion().iMajor,
                                                           gParameters.GetCreationVersion().iMinor, gParameters.GetCreationVersion().iRelease); break;
      case RANDOMIZATION_SEED       : break; //this parameter is not advertised
      case REPORT_CRITICAL_VALUES   : sDefaultValue = (gParameters.GetReportCriticalValues() ? YES : NO); break;
      default : InvalidParameterException::Generate("Unknown parameter enumeration %d.","MarkAsMissingDefaulted()", eParameterType);
    };

    if (sDefaultValue.GetLength()) {
      gvParametersMissingDefaulted.push_back(static_cast<int>(eParameterType)); //and default retained.
      PrintDirection.SatScanPrintWarning("Warning: The parameter '%s' is missing from the parameter file,\n"
                                         "         defaulted value '%s' assigned.\n",
                                         GetParameterLabel(eParameterType), sDefaultValue.GetCString());
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("MarkAsMissingDefaulted()","AbtractParameterFileAccess");
    throw;
 }
}

/** Attempts to interpret passed string as a boolean value. Throws InvalidParameterException. */
bool AbtractParameterFileAccess::ReadBoolean(const ZdString& sValue, ParameterType eParameterType) const {
  bool          bReadResult;

  try {
    if (sValue.GetIsEmpty()) {
      InvalidParameterException::Generate("Error: Parameter '%s' is not set.\n", "ReadBoolean()", GetParameterLabel(eParameterType));
    }
    else if (!(!stricmp(sValue.GetCString(),"y")   || !stricmp(sValue.GetCString(),"n") ||
               !strcmp(sValue.GetCString(),"1")    || !strcmp(sValue.GetCString(),"0")   ||
               !stricmp(sValue.GetCString(),"yes")  || !stricmp(sValue.GetCString(),"no"))) {
      InvalidParameterException::Generate("Error: For parameter '%s', setting '%s' is invalid. Valid values are 'y' or 'n'.\n",
                                          "ReadBoolean()", GetParameterLabel(eParameterType), sValue.GetCString());
    }
    else
      bReadResult = (!stricmp(sValue.GetCString(),"y") || !stricmp(sValue.GetCString(),"yes") || !strcmp(sValue.GetCString(),"1"));
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadBoolean()","AbtractParameterFileAccess");
    throw;
  }
  return bReadResult;
}


/** Set date parameter with passed string using appropriate set function. */
void AbtractParameterFileAccess::ReadDate(const ZdString& sValue, ParameterType eParameterType) const {
 try {
   switch (eParameterType) {
     case START_PROSP_SURV      : //As a legacy of the old parameters code,
                                  //we need to check that the length of the
                                  //string is not one. The prospective start
                                  //date took the line position of an extra
                                  //parameter as seen in old file
                                  //"0                     // Extra Parameter #4".
                                  //We don't want to produce an error for
                                  //an invalid parameter that the user didn't
                                  //miss set. So, treat a value of "0" as blank.
                                  if (sValue == "0")
                                    gParameters.SetProspectiveStartDate("");
                                  else
                                    gParameters.SetProspectiveStartDate(sValue);
                                  break;
     case STARTDATE             : gParameters.SetStudyPeriodStartDate(sValue); break;
     case ENDDATE               : gParameters.SetStudyPeriodEndDate(sValue); break;
     default : ZdException::Generate("Parameter enumeration '%d' is not listed for date read.\n","ReadDate()", eParameterType);
   };
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadDate()","AbtractParameterFileAccess");
    throw;
  }
}

/** Attempts to interpret passed string as comma separated string of dates. Throws InvalidParameterException. */
void AbtractParameterFileAccess::ReadDateRange(const ZdString& sValue, ParameterType eParameterType, DateRange_t& Range) const {
  int                   i, iNumTokens;

  try {
    if (sValue.GetLength()) {
      ZdStringTokenizer     Tokenizer(sValue, ",");
      iNumTokens = Tokenizer.GetNumTokens();
      if (iNumTokens != 2)
        InvalidParameterException::Generate("Error: For parameter '%s', %d values specified but should have 2.\n",
                                            "ReadDateRange()", GetParameterLabel(eParameterType), iNumTokens);
      Range.first = Tokenizer.GetNextToken().GetCString();
      Range.second = Tokenizer.GetNextToken().GetCString();
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadDateRange()","AbtractParameterFileAccess");
    throw;
  }
}

/** Attempts to interpret passed string as a double value. Throws InvalidParameterException. */
double AbtractParameterFileAccess::ReadDouble(const ZdString & sValue, ParameterType eParameterType) const {
  double        dReadResult;

  try {
    if (sValue.GetIsEmpty()) {
      InvalidParameterException::Generate("Error: Parameter '%s' is not set.\n", "ReadDouble()",
                                          GetParameterLabel(eParameterType));
    }
    else if (sscanf(sValue.GetCString(), "%lf", &dReadResult) != 1) {
      InvalidParameterException::Generate("Error: For parameter '%s', setting '%s' is not a valid real number.\n", "ReadDouble()",
                                          GetParameterLabel(eParameterType), sValue.GetCString());
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadDouble()","AbtractParameterFileAccess");
    throw;
  }
  return dReadResult;
}

/** Attempts to interpret passed string as a space/comma delimited string of integers that represent
    the number of rotations ellipse will make. No attempt to convert is made if no
    ellipses defined.  Throws InvalidParameterException. */
void AbtractParameterFileAccess::ReadEllipseRotations(const ZdString& sParameter) const {
  int                   i, iNumTokens, iReadNumberRotations;
  ZdString              sLabel;

  try {
    if (sParameter.GetLength() && gParameters.GetNumRequestedEllipses()) {
      ZdStringTokenizer     Tokenizer(sParameter, (sParameter.Find(',') == -1 ? " " : "," ));
      iNumTokens = Tokenizer.GetNumTokens();
      for (i=0; i < iNumTokens; i++) {
         if (sscanf(Tokenizer.GetToken(i).GetCString(), "%i", &iReadNumberRotations))
           gParameters.SetNumberEllipsoidRotations(iReadNumberRotations);
         else
           InvalidParameterException::Generate("Error: For parameter '%s', setting '%s' is not an integer.\n", "ReadEllipseRotations()",
                                               GetParameterLabel(ENUMBERS), Tokenizer.GetToken(i).GetCString());
      }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadEllipseRotations()","AbtractParameterFileAccess");
    throw;
  }
}

/** Attempts to interpret passed string as a space delimited string of integers that represent
    the shape of each ellipsoid. No attempt to convert is made if there are no
    ellipses defined.  Throws InvalidParameterException. */
void AbtractParameterFileAccess::ReadEllipseShapes(const ZdString& sParameter) const {
  int                   i, iNumTokens;
  double                dReadShape;

  try {
    if (sParameter.GetLength() && gParameters.GetNumRequestedEllipses()) {
      ZdStringTokenizer     Tokenizer(sParameter, (sParameter.Find(',') == -1 ? " " : "," ));
      iNumTokens = Tokenizer.GetNumTokens();
      for (i=0; i < iNumTokens; i++) {
         if (sscanf(Tokenizer.GetToken(i).GetCString(), "%lf", &dReadShape))
           gParameters.SetEllipsoidShape(dReadShape);
         else
           InvalidParameterException::Generate("Error: For parameter '%s', setting '%s' is not an decimal number.\n",
                                               "ReadEllipseShapes()", GetParameterLabel(ESHAPES), Tokenizer.GetToken(i).GetCString());
      }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadEllipseShapes()","AbtractParameterFileAccess");
    throw;
  }
}

/** Attempts to validate integer as enumeration within specified range. Throws InvalidParameterException. */
int AbtractParameterFileAccess::ReadEnumeration(int iValue, ParameterType eParameterType, int iLow, int iHigh) const {
  try {
    if (iValue < iLow || iValue > iHigh)
      InvalidParameterException::Generate("Error: For parameter '%s', setting '%d' is out of range(%d - %d).\n", "SetCoordinatesType()",
                                          GetParameterLabel(eParameterType), iValue, iLow, iHigh);
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadEnumeration()","AbtractParameterFileAccess");
    throw;
  }
  return iValue;
}

/** Attempts to interpret passed string as a float value. Throws InvalidParameterException. */
float AbtractParameterFileAccess::ReadFloat(const ZdString& sValue, ParameterType eParameterType) const {
  float         fReadResult;

  try {
    if (sValue.GetIsEmpty()) {
      InvalidParameterException::Generate("Error: Parameter '%s' is not set.\n",
                                          "ReadFloat()", GetParameterLabel(eParameterType));
    }
    else if (sscanf(sValue.GetCString(), "%f", &fReadResult) != 1) {
      InvalidParameterException::Generate("Error: For parameter '%s', setting '%s' is not a valid real number.\n",
                                          "ReadFloat()", GetParameterLabel(eParameterType), sValue.GetCString());
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadFloat()","AbtractParameterFileAccess");
    throw;
  }
  return fReadResult;
}


/** Attempts to interpret passed string as an integer value. Throws InvalidParameterException. */
int AbtractParameterFileAccess::ReadInt(const ZdString& sValue, ParameterType eParameterType) const {
  int           iReadResult;

  try {
    if (sValue.GetIsEmpty()) {
      InvalidParameterException::Generate("Error: Parameter '%s' is not set.\n",
                                          "ReadInt()", GetParameterLabel(eParameterType));
    }
    else if (sscanf(sValue.GetCString(), "%i", &iReadResult) != 1) {
      InvalidParameterException::Generate("Error: For parameter '%s', setting '%s' is not a valid integer.\n",
                                         "ReadInt()", GetParameterLabel(eParameterType), sValue.GetCString());
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadInt()","AbtractParameterFileAccess");
    throw;
  }
  return iReadResult;
}

/** Attempts to interpret passed string as an integer value. Throws InvalidParameterException. */
int AbtractParameterFileAccess::ReadUnsignedInt(const ZdString& sValue, ParameterType eParameterType) const {
  int           iReadResult;

  try {
   if (sValue.GetIsEmpty()) {
     InvalidParameterException::Generate("Error: Parameter '%s' is not set.\n",
                                         "ReadUnsignedInt()", GetParameterLabel(eParameterType));
   }
   else if (sscanf(sValue.GetCString(), "%u", &iReadResult) != 1) {
     InvalidParameterException::Generate("Error: For parameter '%s', setting '%s' is not a valid integer.\n",
                                         "ReadUnsignedInt()", GetParameterLabel(eParameterType), sValue.GetCString());
   }
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadUnsignedInt()","AbtractParameterFileAccess");
    throw;
  }
  return iReadResult;
}

/** Attempts to interpret passed string as version number of format '#.#.#'. Throws InvalidParameterException. */
void AbtractParameterFileAccess::ReadVersion(const ZdString& sValue) const {
  CParameters::CreationVersion       vVersion;

   if (sValue.GetIsEmpty())
     InvalidParameterException::Generate("Error: Parameter '%s' is not set.\n", "ReadVersion()", GetParameterLabel(CREATION_VERSION));
   else if (sscanf(sValue.GetCString(), "%u.%u.%u", &vVersion.iMajor, &vVersion.iMinor, &vVersion.iRelease) < 3)
    InvalidParameterException::Generate("Error: Parameter '%s' is not set.\n", "ReadVersion()", GetParameterLabel(CREATION_VERSION));
  gParameters.SetVersion(vVersion);
}

/** Calls appropriate read and set function for parameter type. */
void AbtractParameterFileAccess::SetParameter(ParameterType eParameterType, const ZdString& sParameter, BasePrint& PrintDirection) {
  int           iValue;
  DateRange_t   Range;

  try {
    switch (eParameterType) {
      case ANALYSISTYPE              : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, PURELYSPATIAL, PROSPECTIVEPURELYTEMPORAL);
                                       gParameters.SetAnalysisType((AnalysisType)iValue); break;
      case SCANAREAS                 : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, HIGH, HIGHANDLOW);
                                       gParameters.SetAreaRateType((AreaRateType)iValue); break;
      case CASEFILE                  : gParameters.SetCaseFileName(sParameter.GetCString(), true); break;
      case POPFILE                   : gParameters.SetPopulationFileName(sParameter.GetCString(), true); break;
      case COORDFILE                 : gParameters.SetCoordinatesFileName(sParameter.GetCString(), true); break;
      case OUTPUTFILE                : gParameters.SetOutputFileName(sParameter.GetCString(), true); break;
      case PRECISION                 : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, NONE, DAY);
                                       gParameters.SetPrecisionOfTimesType((DatePrecisionType)iValue); break;
      case DIMENSION                 : //Dimensions no longer read from file.
                                       break;
      case SPECIALGRID               : gParameters.SetUseSpecialGrid(ReadBoolean(sParameter, eParameterType)); break;
      case GRIDFILE                  : gParameters.SetSpecialGridFileName(sParameter.GetCString(), true); break;
      case GEOSIZE                   : gParameters.SetMaximumGeographicClusterSize(ReadFloat(sParameter, eParameterType)); break;
      case STARTDATE                 : ReadDate(sParameter, eParameterType); break;
      case ENDDATE                   : ReadDate(sParameter, eParameterType); break;
      case CLUSTERS                  : gParameters.SetIncludeClustersType((IncludeClustersType)ReadInt(sParameter, eParameterType)); break;
      case EXACTTIMES                : //No longer used. No documentation as to previous usage.
                                       break;
      case TIME_AGGREGATION_UNITS    : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, NONE, DAY);
                                       gParameters.SetTimeAggregationUnitsType((DatePrecisionType)iValue); break;
      case TIME_AGGREGATION          : gParameters.SetTimeAggregationLength((long)ReadInt(sParameter, eParameterType)); break;
      case PURESPATIAL               : gParameters.SetIncludePurelySpatialClusters(ReadBoolean(sParameter, eParameterType)); break;
      case TIMESIZE                  : gParameters.SetMaximumTemporalClusterSize(ReadFloat(sParameter, eParameterType)); break;
      case REPLICAS                  : gParameters.SetNumberMonteCarloReplications(ReadUnsignedInt(sParameter, eParameterType)); break;
      case MODEL                     : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, POISSON, RANK);
                                       gParameters.SetProbabilityModelType((ProbabiltyModelType)iValue); break;
      case RISKFUNCTION              : gParameters.SetRiskType((RiskType)ReadInt(sParameter, eParameterType)); break;
      case POWERCALC                 : gParameters.SetPowerCalculation(ReadBoolean(sParameter, eParameterType)); break;
      case POWERX                    : gParameters.SetPowerCalculationX(ReadDouble(sParameter, eParameterType)); break;
      case POWERY                    : gParameters.SetPowerCalculationY(ReadDouble(sParameter, eParameterType)); break;
      case TIMETREND                 : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, NOTADJUSTED, STRATIFIED_RANDOMIZATION);
                                       gParameters.SetTimeTrendAdjustmentType((TimeTrendAdjustmentType)ReadInt(sParameter, eParameterType)); break;
      case TIMETRENDPERC             : gParameters.SetTimeTrendAdjustmentPercentage(ReadDouble(sParameter, eParameterType)); break;
      case PURETEMPORAL              : gParameters.SetIncludePurelyTemporalClusters(ReadBoolean(sParameter, eParameterType)); break;
      case CONTROLFILE               : gParameters.SetControlFileName(sParameter.GetCString(), true); break;
      case COORDTYPE                 : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, CARTESIAN, LATLON);
                                       gParameters.SetCoordinatesType((CoordinatesType)ReadInt(sParameter, eParameterType)); break;
      case OUTPUT_SIM_LLR_ASCII      : gParameters.SetOutputSimLogLikeliRatiosAscii(ReadBoolean(sParameter, eParameterType)); break;
      case SEQUENTIAL                : gParameters.SetSequentialScanning(ReadBoolean(sParameter, eParameterType)); break;
      case SEQNUM                    : gParameters.SetNumSequentialScans(ReadUnsignedInt(sParameter, eParameterType)); break;
      case SEQPVAL                   : gParameters.SetSequentialCutOffPValue(ReadDouble(sParameter, eParameterType)); break;
      case VALIDATE                  : gParameters.SetValidatePriorToCalculation(ReadBoolean(sParameter, eParameterType)); break;
      case OUTPUT_RR_ASCII           : gParameters.SetOutputRelativeRisksAscii(ReadBoolean(sParameter, eParameterType)); break;
      case ELLIPSES                  : gParameters.SetNumberEllipses(ReadInt(sParameter, eParameterType)); break;
      case ESHAPES                   : ReadEllipseShapes(sParameter); break;
      case ENUMBERS                  : ReadEllipseRotations(sParameter); break;
      case START_PROSP_SURV          : ReadDate(sParameter, eParameterType); break;
      case OUTPUT_AREAS_ASCII        : gParameters.SetOutputAreaSpecificAscii(ReadBoolean(sParameter, eParameterType)); break;
      case OUTPUT_MLC_ASCII          : gParameters.SetOutputClusterLevelAscii(ReadBoolean(sParameter, eParameterType)); break;
      case CRITERIA_SECOND_CLUSTERS  : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, NOGEOOVERLAP, NORESTRICTIONS);
                                       gParameters.SetCriteriaForReportingSecondaryClusters((CriteriaSecondaryClustersType)ReadInt(sParameter, eParameterType)); break;
      case MAX_TEMPORAL_TYPE         : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, PERCENTAGETYPE, TIMETYPE);
                                       gParameters.SetMaximumTemporalClusterSizeType((TemporalSizeType)ReadInt(sParameter, eParameterType)); break;
      case MAX_SPATIAL_TYPE          : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, PERCENTOFPOPULATIONTYPE, PERCENTOFPOPULATIONFILETYPE);
                                       gParameters.SetMaximumSpacialClusterSizeType((SpatialSizeType)ReadInt(sParameter, eParameterType)); break;
      case RUN_HISTORY_FILENAME      : //Run History no longer scanned from parameters file. Set through setters/getters and copy() only.
                                       break;
      case OUTPUT_MLC_DBASE          : gParameters.SetOutputClusterLevelDBase(ReadBoolean(sParameter, eParameterType)); break;
      case OUTPUT_AREAS_DBASE        : gParameters.SetOutputAreaSpecificDBase(ReadBoolean(sParameter, eParameterType)); break;
      case OUTPUT_RR_DBASE           : gParameters.SetOutputRelativeRisksDBase(ReadBoolean(sParameter, eParameterType)); break;
      case OUTPUT_SIM_LLR_DBASE      : gParameters.SetOutputSimLogLikeliRatiosDBase(ReadBoolean(sParameter, eParameterType)); break;
      case NON_COMPACTNESS_PENALTY   : gParameters.SetNonCompactnessPenalty(ReadBoolean(sParameter, eParameterType)); break;
      case INTERVAL_STARTRANGE       : ReadDateRange(sParameter, eParameterType, Range);
                                       gParameters.SetStartRangeStartDate(Range.first.c_str());
                                       gParameters.SetStartRangeEndDate(Range.second.c_str()); break;
      case INTERVAL_ENDRANGE         : ReadDateRange(sParameter, eParameterType, Range);
                                       gParameters.SetEndRangeStartDate(Range.first.c_str());
                                       gParameters.SetEndRangeEndDate(Range.second.c_str()); break;
      case TIMETRENDCONVRG           : /*gParameters.SetTimeTrendConvergence(ReadDouble(sParameter, eParameterType));*/ break;
      case MAXCIRCLEPOPFILE          : gParameters.SetMaxCirclePopulationFileName(sParameter.GetCString(), true); break;
      case EARLY_SIM_TERMINATION     : gParameters.SetTerminateSimulationsEarly(ReadBoolean(sParameter, eParameterType)); break;
      case REPORTED_GEOSIZE          : gParameters.SetMaximumReportedGeographicalClusterSize(ReadFloat(sParameter, eParameterType)); break;
      case USE_REPORTED_GEOSIZE      : gParameters.SetRestrictReportedClusters(ReadBoolean(sParameter, eParameterType)); break;
      case SIMULATION_TYPE           : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, STANDARD, FILESOURCE);
                                       gParameters.SetSimulationType((SimulationType)ReadInt(sParameter, eParameterType)); break;
      case SIMULATION_SOURCEFILE     : gParameters.SetSimulationDataSourceFileName(sParameter.GetCString(), true); break;
      case ADJ_BY_RR_FILE            : gParameters.SetAdjustmentsByRelativeRisksFilename(sParameter.GetCString(), true); break;
      case OUTPUT_SIMULATION_DATA    : gParameters.SetOutputSimulationData(ReadBoolean(sParameter, eParameterType)); break;
      case SIMULATION_DATA_OUTFILE   : gParameters.SetSimulationDataOutputFileName(sParameter.GetCString(), true); break;
      case ADJ_FOR_EALIER_ANALYSES   : gParameters.SetAdjustForEarlierAnalyses(ReadBoolean(sParameter, eParameterType)); break;
      case USE_ADJ_BY_RR_FILE        : gParameters.SetUseAdjustmentForRelativeRisksFile(ReadBoolean(sParameter, eParameterType)); break;
      case SPATIAL_ADJ_TYPE          : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, NO_SPATIAL_ADJUSTMENT, SPATIALLY_STRATIFIED_RANDOMIZATION);
                                       gParameters.SetSpatialAdjustmentType((SpatialAdjustmentType)ReadInt(sParameter, eParameterType)); break;
      case MULTI_DATASET_PURPOSE_TYPE: iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, MULTIVARIATE, ADJUSTMENT);
                                       gParameters.SetMultipleDataStreamPurposeType((MultipleStreamPurposeType)ReadInt(sParameter, eParameterType)); break;
      case CREATION_VERSION          : ReadVersion(sParameter); break;
      case RANDOMIZATION_SEED        : gParameters.SetRandomizationSeed(ReadInt(sParameter, eParameterType)); break;
      case REPORT_CRITICAL_VALUES    : gParameters.SetReportCriticalValues(ReadBoolean(sParameter, eParameterType)); break;
      default : InvalidParameterException::Generate("Unknown parameter enumeration %d.","SetParameter()", eParameterType);
    };
  }
  catch (InvalidParameterException &x) {
    gbReadStatusError = true;
    PrintDirection.SatScanPrintWarning(x.GetErrorMessage());
  }
  catch (ZdException &x) {
    x.AddCallpath("SetParameter()","AbtractParameterFileAccess");
    throw;
  }
}

//var_arg constructor
InvalidParameterException::InvalidParameterException(va_list varArgs, const char *sMessage, const char *sSourceModule, ZdException::Level iLevel)
                          :ResolvableException(varArgs, sMessage, sSourceModule, iLevel){}

//static generation function:
void InvalidParameterException::Generate(const char *sMessage, const char *sSourceModule, ...) {
   va_list varArgs;
   va_start(varArgs, sSourceModule);

   InvalidParameterException theException(varArgs, sMessage, sSourceModule, Normal);
   va_end(varArgs);
   throw theException;
}

