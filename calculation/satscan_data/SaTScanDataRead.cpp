//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "SaTScanData.h"
#include "AdjustmentHandler.h"
#include "DateStringParser.h"

/** Converts passed string specifiying a adjustment file date to a julian date.
    Precision is determined by date formats( YYYY/MM/DD, YYYY/MM, YYYY, YY/MM/DD,
    YY/MM, YY ) which is the complete set of valid formats that SaTScan currently
    supports. Since we accumulate errors/warnings when reading input files,
    indication of a bad date is returned and any messages sent to print direction. */
bool CSaTScanData::ConvertAdjustmentDateToJulian(StringParser & Parser, Julian & JulianDate, bool bStartDate) {
  int                                   iDateIndex;
  DateStringParser                      DateParser;
  DateStringParser::ParserStatus        eStatus;

  if (m_pParameters->GetPrecisionOfTimesType() == NONE)
    JulianDate = (bStartDate ? m_nStartDate: m_nEndDate);
  else {
    iDateIndex = (bStartDate ? 2: 3);
    //read and validate date
    if (!Parser.GetWord(iDateIndex)) {
      gpPrint->PrintInputWarning("Error: Record %ld in %s does not contain a %s date.\n",
                                 Parser.GetReadCount(), gpPrint->GetImpliedFileTypeString().c_str(),
                                 (bStartDate ? "start": "end"));
      return false;
    }
    eStatus = DateParser.ParseAdjustmentDateString(Parser.GetWord(iDateIndex), m_nStartDate, m_nEndDate, JulianDate, bStartDate);
    switch (eStatus) {
      case DateStringParser::VALID_DATE       : break;
      case DateStringParser::AMBIGUOUS_YEAR   :
        gpPrint->PrintInputWarning("Error: Due to the study period being greater than 100 years, unable\n"
                                   "       to determine century for two digit year in %s, record %ld.\n"
                                   "       Please use four digit years.\n",
                                   gpPrint->GetImpliedFileTypeString().c_str(), Parser.GetReadCount());
        return false;
      case DateStringParser::INVALID_DATE     :
      case DateStringParser::LESSER_PRECISION :
      default                                 :
        gpPrint->PrintInputWarning("Error: Invalid %s date '%s' in %s, record %ld.\n",
                                   (bStartDate ? "start": "end"), Parser.GetWord(iDateIndex),
                                   gpPrint->GetImpliedFileTypeString().c_str(), Parser.GetReadCount());
        return false;
    };
    //validate that date is between study period start and end dates
    if (!(m_nStartDate <= JulianDate && JulianDate <= m_nEndDate)) {
      gpPrint->PrintInputWarning("Error: Date '%s' in record %ld of %s is not\n"
                                 "       within study period beginning %s and ending %s.\n",
                                 Parser.GetWord(iDateIndex), Parser.GetReadCount(),
                                 gpPrint->GetImpliedFileTypeString().c_str(),
                                 m_pParameters->GetStudyPeriodStartDate().c_str(),
                                 m_pParameters->GetStudyPeriodEndDate().c_str());
      return false;
    }
  }
  return true;
}

/** Read the relative risks file
    -- unlike other input files of system, records read from relative risks
       file are applied directly to the measure structure, just post calculation
       of measure and prior to temporal adjustments and making cumulative. */
bool CSaTScanData::ReadAdjustmentsByRelativeRisksFile() {
  bool                                  bValid=true, bEmpty=true;
  tract_t                               TractIndex, iMaxTract;
  double                                dRelativeRisk;
  Julian                                StartDate, EndDate;
  FILE                                * fp=0;
  int                                   iNumWords;

  try {
    gpPrint->SetImpliedInputFileType(BasePrint::ADJ_BY_RR_FILE);
    StringParser Parser(*gpPrint);

    gpPrint->SatScanPrintf("Reading the adjustments file\n");
    if ((fp = fopen(m_pParameters->GetAdjustmentsByRelativeRisksFilename().c_str(), "r")) == NULL) {
      gpPrint->SatScanPrintWarning("Error: Could not open adjustments file:\n'%s'.\n",
                                   m_pParameters->GetAdjustmentsByRelativeRisksFilename().c_str());
      return false;
    }

    gRelativeRiskAdjustments.Empty();
    while (Parser.ReadString(fp)) {
        //skip lines that do not contain data
        if (!Parser.HasWords())
          continue;
        bEmpty=false;
        //read tract identifier
        if (!stricmp(Parser.GetWord(0),"all"))
          TractIndex = -1;
        else if ((TractIndex = gTractHandler.tiGetTractIndex(Parser.GetWord(0))) == -1) {
          gpPrint->PrintInputWarning("Error: Unknown location identifier in %s, record %ld.\n",
                                     gpPrint->GetImpliedFileTypeString().c_str(), Parser.GetReadCount());
          gpPrint->PrintInputWarning("       '%s' not specified in the coordinates file.\n", Parser.GetWord(0));
          bValid = false;
          continue;
        }
        //read population
        if (!Parser.GetWord(1)) {
          gpPrint->PrintInputWarning("Error: Record %d of %s missing relative risk.\n",
                                     Parser.GetReadCount(), gpPrint->GetImpliedFileTypeString().c_str());
          bValid = false;
          continue;
        }
        if (sscanf(Parser.GetWord(1), "%lf", &dRelativeRisk) != 1) {
          gpPrint->PrintInputWarning("Error: Relative risk value '%s' in record %ld, of %s, is not a number.\n",
                                     Parser.GetWord(1), Parser.GetReadCount(), gpPrint->GetImpliedFileTypeString().c_str());
          bValid = false;
          continue;
        }
        //validate that relative risk is not negative or exceeding type precision
        if (dRelativeRisk < 0) {//validate that count is not negative or exceeds type precision
          if (strstr(Parser.GetWord(1), "-"))
             gpPrint->PrintInputWarning("Error: Negative relative risk in record %ld of %s.\n",
                                        Parser.GetReadCount(), gpPrint->GetImpliedFileTypeString().c_str());
          else
             gpPrint->PrintInputWarning("Error: Relative risk '%s' exceeds maximum value of %i in record %lf of %s.\n",
                                        Parser.GetWord(1), std::numeric_limits<double>::max(),
                                        Parser.GetReadCount(), gpPrint->GetImpliedFileTypeString().c_str());
           bValid = false;
           continue;
        }
        //read start and end dates
        iNumWords = Parser.GetNumberWords();
        if (iNumWords == 3) {
          gpPrint->PrintInputWarning("Error: Record %i, of %s, missing end date.\n",
                                     Parser.GetReadCount(), gpPrint->GetImpliedFileTypeString().c_str());
          bValid = false;
          continue;
        }
        if (iNumWords == 2) {
          StartDate = m_nStartDate;
          EndDate = m_nEndDate;
        }
        else {
          if (!ConvertAdjustmentDateToJulian(Parser, StartDate, true)) {
            bValid = false;
            continue;
          }
          if (!ConvertAdjustmentDateToJulian(Parser, EndDate, false)) {
            bValid = false;
            continue;
          }   
        }
        //check that the adjustment dates are relatively correct
        if (EndDate < StartDate) {
          gpPrint->PrintInputWarning("Error: For record %d of %s, the adjustment period is\n",
                                     Parser.GetReadCount(), gpPrint->GetImpliedFileTypeString().c_str());
          gpPrint->PrintInputWarning("       incorrect because the end date occurs before the start date.\n");
          bValid = false;
          continue;
        }
        //perform adjustment
        iMaxTract = (TractIndex == -1 ? m_nTracts : TractIndex + 1);
        TractIndex = (TractIndex == -1 ? 0 : TractIndex);
        for (; TractIndex < iMaxTract; ++TractIndex)
           gRelativeRiskAdjustments.AddAdjustmentData(TractIndex, dRelativeRisk, StartDate, EndDate);
    }
    //close file pointer
    fclose(fp); fp=0;
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bValid)
      gpPrint->PrintWarningLine("Please see 'Adjustments file format' in the user guide for help.\n");
    //print indication if file contained no data
    else if (bEmpty) {
      gpPrint->SatScanPrintWarning("Error: %s contains no data.\n", gpPrint->GetImpliedFileTypeString().c_str());
      bValid = false;
    }
  }
  catch (ZdException &x) {
    if (fp) fclose(fp); //close file pointer
    x.AddCallpath("ReadAdjustmentsByRelativeRisksFile()", "CSaTScanData");
    throw;
  }
  return bValid;
}

/** Reads cartesian coordinates into vector.
    Note: coordinate vector should already be sized to defined dimensions.
    Returns indication of whether words in passed string could be converted to
    coordinates. Tracks number of words successfully scanned, the caller of
    function will use this information to confirm that coordinates scanned is
    not less than defined dimensions. The reason we don't check scanned dimensions
    here is that a generic error message could not be implemented. */
bool CSaTScanData::ReadCartesianCoordinates(StringParser & Parser, std::vector<double>& vCoordinates,
                                            int & iScanCount, int iWordOffSet) {
  const char  * pCoordinate;
  int           i;

  for (i=0, iScanCount=0; i < m_pParameters->GetDimensionsOfData(); ++i, ++iWordOffSet)
     if ((pCoordinate = Parser.GetWord(iWordOffSet)) != 0) {
       if (sscanf(pCoordinate, "%lf", &(vCoordinates[i])))
         iScanCount++; //track num successful scans, caller of function wants this information
       else {
         //unable to read word as double, print error to print direction and return false
         gpPrint->PrintInputWarning("Error: Value '%s' of record %ld in %s could not be read as ",
                                    pCoordinate, Parser.GetReadCount(), gpPrint->GetImpliedFileTypeString().c_str());
         //we can be specific about which dimension we are attending to read to                                    
         if (i < 2)
           gpPrint->PrintInputWarning("%s-coordinate.\n", (i == 0 ? "x" : "y"));
         else if (m_pParameters->GetDimensionsOfData() == 3)
           gpPrint->PrintInputWarning("z-coordinate.\n");
         else
           gpPrint->PrintInputWarning("z%d-coordinate.\n", i - 1);
         return false;
       }
     }
  return true;          
}

/** Read the geographic data file. Calls particular function for coordinate type. */
bool CSaTScanData::ReadCoordinatesFile() {
  bool          bReturn;
  FILE        * fp=0; // Ptr to coordinates file

  try {
    gpPrint->SatScanPrintf("Reading the coordinates file\n");
    if ((fp = fopen(m_pParameters->GetCoordinatesFileName().c_str(), "r")) == NULL) {
      gpPrint->SatScanPrintWarning("Error: Coordinates file '%s' could not be opened.\n",
                                   m_pParameters->GetCoordinatesFileName().c_str());
      return false;
    }
    gpPrint->SetImpliedInputFileType(BasePrint::COORDFILE);

    switch (m_pParameters->GetCoordinatesType()) {
      case CARTESIAN : bReturn = ReadCoordinatesFileAsCartesian(fp);
                       //now that the number of dimensions is known, validate against requested ellipses
                       if (m_pParameters->GetDimensionsOfData() > 2 && m_pParameters->GetNumRequestedEllipses() &&
                           !(m_pParameters->GetCriteriaSecondClustersType() == NORESTRICTIONS ||
                            m_pParameters->GetCriteriaSecondClustersType() == NOGEOOVERLAP)) {
                         gpPrint->SatScanPrintWarning("Error: Invalid parameter setting for ellipses. SaTScan permits only two\n"
                                                      "       dimensions be specified for a centroid when performing an analysis\n"
                                                      "       which contain ellipses and restricts reporting of secondary clusters\n"
                                                      "       to anything other than 'No Geographical Overlap'. You may want to\n"
                                                      "       change the criteria for reporting secondary clusters and run the\n"
                                                      "       analysis again.\n");
                         bReturn = false;          
                       }
                       break;
      case LATLON    : bReturn = ReadCoordinatesFileAsLatitudeLongitude(fp); break;
      default : ZdException::Generate("Unknown coordinate type '%d'.","ReadCoordinatesFile()",m_pParameters->GetCoordinatesType());
    };
    fclose(fp); fp=0;
    m_nTotalTractsAtStart = m_nTracts;
  }
  catch (ZdException &x) {
    if (fp) fclose(fp);
    x.AddCallpath("ReadCoordinatesFile()","CSaTScanData");
    throw;
  }
  return bReturn;
}

/** Read the geographic data file in Cartesian coordinate system.
    If invalid data is found in the file, an error message is printed,
    that record is ignored, and reading continues.
    Return value: true = success, false = errors encountered           */
bool CSaTScanData::ReadCoordinatesFileAsCartesian(FILE * fp) {
  int                           i, iScanCount=0;
  bool                          bValidRecord, bValid=true, bEmpty=true;
  const char                  * pCoordinate, * pDimension;
  ZdString                      TractIdentifier;
  std::vector<double>           vCoordinates;
  StringParser                  Parser(*gpPrint);

  try {
    while (Parser.ReadString(fp)) {
         //skip records with no data
         if (!Parser.HasWords())
           continue;
         //if empty and this record has data, then this is the first record w/ data
         if (bEmpty) {
           bEmpty = false;
           //determine number of dimensions from first record, 2 or more is valid
           iScanCount = Parser.GetNumberWords();
           //there must be at least two dimensions
           if (iScanCount < 3) {
             gpPrint->PrintInputWarning("Error: First record of coordinates file contains %s.\n",
                                                 iScanCount == 2 ? "only x-coordinate" : "no coordinates");
             bValid = false;
             break; //stop reading records, the first record defines remaining records format 
           }
           //ok, first record indicates that there are iScanCount - 1 dimensions (first scan is tract identifier)
           //data still could be invalid, but this will be determined like the remaining records
           const_cast<CParameters*>(m_pParameters)->SetDimensionsOfData(iScanCount - 1);
           gTractHandler.tiSetDimensions(m_pParameters->GetDimensionsOfData());
           gCentroidsHandler.giSetDimensions(m_pParameters->GetDimensionsOfData());
           vCoordinates.resize(m_pParameters->GetDimensionsOfData(), 0);
         }
         //read and validate dimensions skip to next record if error reading coordinates as double
         if (! ReadCartesianCoordinates(Parser, vCoordinates, iScanCount, 1)) {
           bValid = false;
           continue;
         }
         //validate that we read the correct number of coordinates
         if (iScanCount < m_pParameters->GetDimensionsOfData()) {
           //Note: since the first record defined the number of dimensions, this error could not happen.
           gpPrint->PrintInputWarning("Error: Record %ld in coordinates file contains %d dimension%s but the\n",
                                               Parser.GetReadCount(), iScanCount, (iScanCount == 1 ? "" : "s"));
           gpPrint->PrintInputWarning("       first record defined the number of dimensions as %d.\n", m_pParameters->GetDimensionsOfData());
           bValid = false;
           continue;
         }
         //add the tract identifier and coordinates to trac handler
         if (! gTractHandler.tiInsertTnode(Parser.GetWord(0), vCoordinates)) {
           gpPrint->PrintInputWarning("Error: For record %ld in coordinates file, location '%s' already exists.\n", Parser.GetReadCount(), Parser.GetWord(0));
           bValid = false;
           continue;
         }
         //add tract identifier and coordinates as centroid if a special grid file is not being used
         if (! m_pParameters->UseSpecialGrid())
           //no need to check return, we would have already gotten error from tract handler
           //for duplicate tract identifier
          gCentroidsHandler.giInsertGnode(Parser.GetWord(0), vCoordinates);
    }
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bValid)
      gpPrint->PrintWarningLine("Please see 'coordinate file format' in the user guide for help.\n");
    //print indication if file contained no data
    else if (bEmpty) {
      gpPrint->PrintWarningLine("Error: Coordinates file contains no data.\n");
      bValid = false;
    }
    //validate that we have more than one tract, only a purely temporal analysis is the exception to this rule
    else if (gTractHandler.tiGetNumTracts() == 1 && !m_pParameters->GetIsPurelyTemporalAnalysis()) {
      gpPrint->SatScanPrintWarning("Error: For a %s analysis, the coordinates file must contain more than one location.\n",
                                   m_pParameters->GetAnalysisTypeAsString());
      bValid = false;
    }
    //record number of locations read
    m_nTracts = gTractHandler.tiGetNumTracts();
    //record number of centroids read
    m_nGridTracts = gCentroidsHandler.giGetNumTracts();
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadCoordinatesFileAsCartesian()", "CSaTScanData");
    throw;
  }
  return bValid;
}

/** Read the latitude/longitude geographic data file.
    If invalid data is found in the file, an error message
    is printed, that record is ignored, and reading continues.
    Return value: true = success, false = errors encountered   */
bool CSaTScanData::ReadCoordinatesFileAsLatitudeLongitude(FILE * fp) {
  int                           iScanCount;
  const char                  * pCoordinate;
  bool                          bValid=true, bEmpty=true;
  ZdString                      TractIdentifier;
  std::vector<double>           vCoordinates;
  StringParser                  Parser(*gpPrint);

  try {
    vCoordinates.resize(3/*for conversion*/, 0);
    const_cast<CParameters*>(m_pParameters)->SetDimensionsOfData(3/*for conversion*/);
    gTractHandler.tiSetDimensions(m_pParameters->GetDimensionsOfData());
    gCentroidsHandler.giSetDimensions(m_pParameters->GetDimensionsOfData());
    while (Parser.ReadString(fp)) {
        //skip records with no data 
        if (! Parser.HasWords())
          continue;
        bEmpty=false;
        if (! ReadLatitudeLongitudeCoordinates(Parser, vCoordinates, 1, "coordinates")) {
           bValid = false;
           continue;
        }
        //add the tract identifier and coordinates to trac handler
        if (! gTractHandler.tiInsertTnode(Parser.GetWord(0), vCoordinates)) {
          gpPrint->PrintInputWarning("Error: For record %ld in coordinates file, location '%s' already exists.\n", Parser.GetReadCount(), Parser.GetWord(0));
          bValid = false;
          continue;
        }
        //add tract identifier and coordinates as centroid if a special grid file is not being used
        if (! m_pParameters->UseSpecialGrid())
          //no need to check return, we would have already gotten error from tract handler
          //for duplicate tract identifier
          gCentroidsHandler.giInsertGnode(Parser.GetWord(0), vCoordinates);
    }
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bValid)
      gpPrint->PrintWarningLine("Please see 'coordinates file format' in the user guide for help.\n");
    //print indication if file contained no data
    else if (bEmpty) {
      gpPrint->PrintWarningLine("Error: Coordinates file contains no data.\n");
      bValid = false;
    }
    //validate that we have more than one tract, only a purely temporal analysis is the exception to this rule
    else if (gTractHandler.tiGetNumTracts() == 1 && !m_pParameters->GetIsPurelyTemporalAnalysis()) {
      gpPrint->PrintInputWarning("Error: For a %s analysis, the coordinates file must contain more than one record.\n",
                                          m_pParameters->GetAnalysisTypeAsString());
      bValid = false;
    }
    //record number of locations read
    m_nTracts = gTractHandler.tiGetNumTracts();
    //record number of centroids read
    m_nGridTracts = gCentroidsHandler.giGetNumTracts();
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadCoordinatesFileAsLatitudeLongitude()", "CSaTScanData");
    throw;
  }
  return bValid;
}

/** Read the special grid file.  Calls particular read given coordinate type. */
bool CSaTScanData::ReadGridFile() {
  bool          bReturn;
  FILE        * fp=0;

  try {
    gpPrint->SatScanPrintf("Reading the grid file\n");
    if ((fp = fopen(m_pParameters->GetSpecialGridFileName().c_str(), "r")) == NULL) {
      gpPrint->SatScanPrintWarning("Error: Could not open grid file:\n'%s'.\n",
                                   m_pParameters->GetSpecialGridFileName().c_str());
      return false;
    }
    gpPrint->SetImpliedInputFileType(BasePrint::GRIDFILE);
    switch (m_pParameters->GetCoordinatesType()) {
      case CARTESIAN : bReturn = ReadGridFileAsCartiesian(fp); break;
      case LATLON    : bReturn = ReadGridFileAsLatitudeLongitude(fp); break;
      default : ZdException::Generate("Unknown coordinate type '%d'.","ReadGrid()",m_pParameters->GetCoordinatesType());
    };
    fclose(fp);fp=0;
  }
  catch (ZdException &x) {
    if (fp) fclose(fp);
    x.AddCallpath("ReadGridFile()", "CSaTScanData");
    throw;
  }
  return bReturn;
}

/** Read the special grid data file as Cartesian coordinates.
   If invalid data is found in the file, an error message is printed,
   that record is ignored, and reading continues.
   Return value: true = success, false = errors encountered          */
bool CSaTScanData::ReadGridFileAsCartiesian(FILE * fp) {
  bool                          bValidRecord, bValid=true, bEmpty=true;
  int                           i, iScanCount;
  const char                  * pCoordinate;
  std::vector<double>           vCoordinates;
  StringParser                  Parser(*gpPrint);
  ZdString                      sId;

  try {
    vCoordinates.resize(m_pParameters->GetDimensionsOfData(), 0);
    while (Parser.ReadString(fp)) {
        //skip blank lines
        if (!Parser.HasWords())
          continue;
        //there are records with data, but not necessarily valid
        bEmpty = false;
         //read and vaidate dimensions skip to next record if error reading coordinates as double
         if (! ReadCartesianCoordinates(Parser, vCoordinates, iScanCount, 0)) {
           bValid = false;
           continue;
         }
        //validate that we read the correct number of coordinates as defined by coordinates system or coordinates file
        if (iScanCount < m_pParameters->GetDimensionsOfData()) {
          gpPrint->PrintInputWarning("Error: Record %ld in grid file contains %d dimension%s but the\n",
                                     Parser.GetReadCount(), iScanCount, (iScanCount == 1 ? "" : "s"));
          gpPrint->PrintInputWarning("       coordinates file defined the number of dimensions as %d.\n",
                                     m_pParameters->GetDimensionsOfData());
          bValid = false;
          continue;
        }
        //add created tract identifer(record number) and read coordinates to structure that mantains list of centroids
        sId = Parser.GetReadCount();
        gCentroidsHandler.giInsertGnode(sId.GetCString(), vCoordinates);
    }
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bValid)
      gpPrint->PrintWarningLine("Please see 'grid file format' in the user guide for help.\n");
    //print indication if file contained no data
    else if (bEmpty) {
      gpPrint->PrintWarningLine("Error: Grid file does not contain data.\n");
      bValid = false;
    }
    //record number of centroids read
    m_nGridTracts = gCentroidsHandler.giGetNumTracts();
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadGridFileAsCartiesian()","CSaTScanData");
    throw;
  }
  return bValid;
}

/** Read the special grid data file as latitude/longitude coordinates.
   If invalid data is found in the file, an error message is printed,
   that record is ignored, and reading continues.
   Return value: true = success, false = errors encountered           */
bool CSaTScanData::ReadGridFileAsLatitudeLongitude(FILE * fp) {
  bool    	                bValid=true, bEmpty=true;
  const char                  * pCoordinate;
  std::vector<double>           vCoordinates;
  StringParser                  Parser(*gpPrint);
  ZdString                      sId;

  try {
    vCoordinates.resize(3/*for conversion*/, 0);
    while (Parser.ReadString(fp)) {
        //skip lines with no data
        if (!Parser.HasWords())
          continue;
        //there are records with data, but not necessarily valid
        bEmpty=false;
        if (! ReadLatitudeLongitudeCoordinates(Parser, vCoordinates, 0, "grid")) {
           bValid = false;
           continue;
        }
        //add created tract identifer(record number) and read coordinates to structure that mantains list of centroids
        sId = Parser.GetReadCount();
        gCentroidsHandler.giInsertGnode(sId.GetCString(), vCoordinates);
    }
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bValid)
      gpPrint->PrintWarningLine("Please see 'grid file format' in the user guide for help.\n");
    //print indication if file contained no data
    else if (bEmpty) {
      gpPrint->PrintWarningLine("Error: Grid file is contains no data.\n");
      bValid = false;
    }
    //record number of centroids
    m_nGridTracts = gCentroidsHandler.giGetNumTracts();
  }
  catch (ZdFileOpenFailedException &x) {
    gpPrint->SatScanPrintWarning("Error: Special Grid file '%s' could not be opened.\n",
                                          m_pParameters->GetSpecialGridFileName().c_str());
    return false;
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadGridFileAsLatitudeLongitude()","CSaTScanData");
    throw;
  }
  return bValid;
}

/** Reads latitude/longitude coordinates into vector.
    Note: coordinate vector should already be sized to 3 dimensions.
    Returns indication of whether words in passed string could be converted to
    coordinates. Checks that coordinates are in range and converts to cartesian
    coordinates. */
bool CSaTScanData::ReadLatitudeLongitudeCoordinates(StringParser & Parser, std::vector<double> & vCoordinates,
                                                    int iWordOffSet, const char * sSourceFile) {
  const char  * pCoordinate;
  double        dLatitude, dLongitude;

  //read latitude, validating that string can be converted to double
  if ((pCoordinate = Parser.GetWord(iWordOffSet)) != 0) {
    if (! sscanf(pCoordinate, "%lf", &dLatitude)) {
      gpPrint->PrintInputWarning("Error: Value '%s' of record %ld in %s file could not be read as latitude.\n", pCoordinate, Parser.GetReadCount(), sSourceFile);
      return false;
    }
  }
  else {
    gpPrint->PrintInputWarning("Error: Record %d in %s file missing latitude and longitude coordinates.\n", Parser.GetReadCount(), sSourceFile);
    return false;
  }
  //read longitude, validating that string can be converted to double
  if ((pCoordinate = Parser.GetWord(++iWordOffSet)) != 0) {
    if (! sscanf(pCoordinate, "%lf", &dLongitude)) {
      gpPrint->PrintInputWarning("Error: Value '%s' of record %ld in %s file could not be read as longitude.\n", pCoordinate, Parser.GetReadCount(), sSourceFile);
      return false;
    }
  }
  else {
    gpPrint->PrintInputWarning("Error: Record %ld in %s file missing longitude coordinate.\n", Parser.GetReadCount(), sSourceFile);
    return false;
  }
  //validate that there is not extra data for record
  if ((pCoordinate = Parser.GetWord(++iWordOffSet)) != 0) {
    gpPrint->PrintInputWarning("Error: Record %ld in %s file contains extra data: '%s'.\n", Parser.GetReadCount(), sSourceFile, pCoordinate);
    return false;
  }
  //validate range of latitude value
  if ((fabs(dLatitude) > 90.0)) {
    gpPrint->PrintInputWarning("Error: Latitude %lf, for record %ld in %s file, is out of range.\n",  dLatitude, Parser.GetReadCount(), sSourceFile);
    gpPrint->PrintInputWarning("       Latitude must be between -90 and 90.\n");
    return false;
  }
  //validate range of longitude value
  if ((fabs(dLongitude) > 180.0)) {
    gpPrint->PrintInputWarning("Error: Longitude %lf, for record %ld in %s file, is out of range.\n", dLongitude, Parser.GetReadCount(), sSourceFile);
    gpPrint->PrintInputWarning("       Longitude must be between -180 and 180.\n");
    return false;
  }
  //convert to 3 dimensions then can add tracts
  ConvertFromLatLong(dLatitude, dLongitude, vCoordinates);
  return true;
}

/** Read the special population that will be used to construct circles
    about grid points(centroids).                                      */
bool CSaTScanData::ReadMaxCirclePopulationFile() {
  int                           iRecNum=0;
  bool                          bValid=true, bEmpty=true;
  tract_t                       TractIdentifierIndex;
  float                         fPopulation;
  FILE                        * fp=0; // Ptr to population file

  try {
    gpPrint->SetImpliedInputFileType(BasePrint::MAXCIRCLEPOPFILE);
    StringParser Parser(*gpPrint);

    gpPrint->SatScanPrintf("Reading the max circle size file\n");
    if ((fp = fopen(m_pParameters->GetMaxCirclePopulationFileName().c_str(), "r")) == NULL) {
      gpPrint->SatScanPrintWarning("Error: Could not open max circle size file:\n'%s'.\n",
                                   m_pParameters->GetMaxCirclePopulationFileName().c_str());
      return false;
    }

    //initialize circle-measure array
    gvCircleMeasure.resize(m_nTracts, 0);

    //1st pass, determine unique population dates. Notes errors with records and continues reading.
    while (Parser.ReadString(fp)) {
        ++iRecNum;
        //skip lines that do not contain data
        if (!Parser.HasWords())
          continue;
        bEmpty=false;
        //read tract identifier
        if ((TractIdentifierIndex = gTractHandler.tiGetTractIndex(Parser.GetWord(0))) == -1) {
          gpPrint->PrintInputWarning("Error: Unknown location identifier in %s, record %ld.\n",
                                     gpPrint->GetImpliedFileTypeString().c_str(), iRecNum);
          gpPrint->PrintInputWarning("       '%s' not specified in the coordinates file.\n", Parser.GetWord(0));
          bValid = false;
          continue;
        }
        //read population
        if (!Parser.GetWord(1)) {
          gpPrint->PrintInputWarning("Error: Record %d of %s missing population.\n",
                                     iRecNum, gpPrint->GetImpliedFileTypeString().c_str());
          bValid = false;
          continue;
        }
        if (sscanf(Parser.GetWord(1), "%f", &fPopulation) != 1) {
          gpPrint->PrintInputWarning("Error: Population value '%s' in record %ld, of %s, is not a number.\n",
                                     Parser.GetWord(1), iRecNum, gpPrint->GetImpliedFileTypeString().c_str());
          bValid = false;
          continue;
        }
        //validate that population is not negative or exceeding type precision
        if (fPopulation < 0) {//validate that count is not negative or exceeds type precision
          if (strstr(Parser.GetWord(1), "-"))
             gpPrint->PrintInputWarning("Error: Negative population in record %ld of %s.\n",
                                        iRecNum, gpPrint->GetImpliedFileTypeString().c_str());
          else
             gpPrint->PrintInputWarning("Error: Population '%s' exceeds maximum value of %i in record %ld of %s.\n",
                                        Parser.GetWord(1), std::numeric_limits<float>::max(),
                                        iRecNum, gpPrint->GetImpliedFileTypeString().c_str());
           bValid = false;
           continue;
        }
        gvCircleMeasure[TractIdentifierIndex] += fPopulation;
        m_nTotalMaxCirclePopulation += fPopulation;
    }
    //close file pointer
    fclose(fp); fp=0;
    // total population can not be zero
    if (m_nTotalMaxCirclePopulation == 0) {
      bValid = false;
      gpPrint->SatScanPrintWarning("Error: Total population for %s is zero.\n",
                                   gpPrint->GetImpliedFileTypeString().c_str());
    }
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bValid)
      gpPrint->PrintWarningLine("Please see 'Special Max Circle Size File' in the user guide for help.\n");
    //print indication if file contained no data
    else if (bEmpty) {
      gpPrint->SatScanPrintWarning("Error: %s contains no data.\n", gpPrint->GetImpliedFileTypeString().c_str());
      bValid = false;
    }
  }
  catch (ZdException &x) {
    //close file pointer
    if (fp) fclose(fp);
    x.AddCallpath("ReadMaxCirclePopulationFile()", "CSaTScanData");
    throw;
  }
  return bValid;
}

