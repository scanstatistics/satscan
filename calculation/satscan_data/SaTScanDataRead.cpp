//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "SaTScanData.h"
#include "AdjustmentHandler.h"
#include "DateStringParser.h"
#include "SSException.h"
#include "DataSource.h"

/** Converts passed string specifiying a adjustment file date to a julian date.
    Precision is determined by date formats( YYYY/MM/DD, YYYY/MM, YYYY, YY/MM/DD,
    YY/MM, YY ) which is the complete set of valid formats that SaTScan currently
    supports. Since we accumulate errors/warnings when reading input files,
    indication of a bad date is returned and any messages sent to print direction. */
bool CSaTScanData::ConvertAdjustmentDateToJulian(DataSource& Source, Julian & JulianDate, bool bStartDate) {
  short                                 iDateIndex;
  DateStringParser                      DateParser;
  DateStringParser::ParserStatus        eStatus;

  if (gParameters.GetPrecisionOfTimesType() == NONE)
    JulianDate = (bStartDate ? m_nStartDate: m_nEndDate);
  else {
    iDateIndex = (bStartDate ? 2: 3);
    //read and validate date
    if (!Source.GetValueAt(iDateIndex)) {
      gPrint.Printf("Error: Record %ld in %s does not contain a %s date.\n", BasePrint::P_READERROR, Source.GetCurrentRecordIndex(),
                    gPrint.GetImpliedFileTypeString().c_str(), (bStartDate ? "start": "end"));
      return false;
    }
    eStatus = DateParser.ParseAdjustmentDateString(Source.GetValueAt(iDateIndex), m_nStartDate, m_nEndDate, JulianDate, bStartDate);
    switch (eStatus) {
      case DateStringParser::VALID_DATE       : break;
      case DateStringParser::AMBIGUOUS_YEAR   :
        gPrint.Printf("Error: Due to the study period being greater than 100 years, unable\n"
                      "       to determine century for two digit year in %s, record %ld.\n"
                      "       Please use four digit years.\n", BasePrint::P_READERROR,
                      gPrint.GetImpliedFileTypeString().c_str(), Source.GetCurrentRecordIndex());
        return false;
      case DateStringParser::INVALID_DATE     :
      case DateStringParser::LESSER_PRECISION :
      default                                 :
        gPrint.Printf("Error: Invalid %s date '%s' in %s, record %ld.\n", BasePrint::P_READERROR,
                      (bStartDate ? "start": "end"), Source.GetValueAt(iDateIndex),
                      gPrint.GetImpliedFileTypeString().c_str(), Source.GetCurrentRecordIndex());
        return false;
    };
    //validate that date is between study period start and end dates
    if (!(m_nStartDate <= JulianDate && JulianDate <= m_nEndDate)) {
      gPrint.Printf("Error: Date '%s' in record %ld of %s is not\n"
                    "       within study period beginning %s and ending %s.\n",
                    BasePrint::P_READERROR, Source.GetValueAt(iDateIndex),
                    Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str(),
                    gParameters.GetStudyPeriodStartDate().c_str(), gParameters.GetStudyPeriodEndDate().c_str());
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
  bool          bValid=true, bRestrictedLocations(!gParameters.UseCoordinatesFile()), bEmpty=true;
  tract_t       TractIndex, iMaxTract;
  double        dRelativeRisk;
  Julian        StartDate, EndDate;
  int           iNumWords;
  const short   uLocationIndex=0, uAdjustmentIndex=1;

  try {
    gPrint.SetImpliedInputFileType(BasePrint::ADJ_BY_RR_FILE);

    gPrint.Printf("Reading the adjustments file\n", BasePrint::P_STDOUT);
    std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(gParameters.GetAdjustmentsByRelativeRisksFilename(), gPrint));
    gRelativeRiskAdjustments.Empty();
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source->ReadRecord()) {
        bEmpty=false;
        //read tract identifier
        if (!stricmp(Source->GetValueAt(uLocationIndex),"all"))
          TractIndex = -1;
        else if (bRestrictedLocations) {
          GenerateResolvableException("Error: When adjusting for known relative risks using the purely temporal analysis\n"
                                      "       without defining a coordinates file, it is not possible to adjust for known\n"
                                      "       relative risks at the location level. In order to adjust for known relative\n"
                                      "       risks at the location level, you must define a coordinates file. Alternatively,\n"
                                      "       you may want to define known relative risks that apply to all locations.",
                                      "ReadAdjustmentsByRelativeRisksFile()");
        }
        else if ((TractIndex = gTractHandler.tiGetTractIndex(Source->GetValueAt(uLocationIndex))) == -1) {
          gPrint.Printf("Error: Unknown location ID in %s, record %ld.\n"
                        "       '%s' not specified in the coordinates file.\n",
                        BasePrint::P_READERROR, gPrint.GetImpliedFileTypeString().c_str(),
                        Source->GetCurrentRecordIndex(), Source->GetValueAt(uLocationIndex));
          bValid = false;
          continue;
        }
        //read population
        if (!Source->GetValueAt(uAdjustmentIndex)) {
          gPrint.Printf("Error: Record %d of %s missing relative risk.\n", BasePrint::P_READERROR,
                        Source->GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
          bValid = false;
          continue;
        }
        if (sscanf(Source->GetValueAt(uAdjustmentIndex), "%lf", &dRelativeRisk) != 1) {
          gPrint.Printf("Error: Relative risk value '%s' in record %ld, of %s, is not a number.\n",
                        BasePrint::P_READERROR, Source->GetValueAt(uAdjustmentIndex), Source->GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
          bValid = false;
          continue;
        }
        //validate that relative risk is not negative or exceeding type precision
        if (dRelativeRisk < 0) {//validate that count is not negative or exceeds type precision
          if (strstr(Source->GetValueAt(uAdjustmentIndex), "-"))
             gPrint.Printf("Error: Negative relative risk in record %ld of %s.\n",
                           BasePrint::P_READERROR, Source->GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
          else
             gPrint.Printf("Error: Relative risk '%s' exceeds the maximum allowed value of %i in record %lf of %s.\n",
                           BasePrint::P_READERROR, Source->GetValueAt(uAdjustmentIndex), std::numeric_limits<double>::max(),
                           Source->GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
           bValid = false;
           continue;
        }
        //read start and end dates
        iNumWords = Source->GetNumValues();
        if (iNumWords == 3) {
          gPrint.Printf("Error: Record %i, of %s, is missing the end date.\n",
                        BasePrint::P_READERROR, Source->GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
          bValid = false;
          continue;
        }
        if (iNumWords == 2) {
          StartDate = m_nStartDate;
          EndDate = m_nEndDate;
        }
        else {
          if (!ConvertAdjustmentDateToJulian(*Source, StartDate, true)) {
            bValid = false;
            continue;
          }
          if (!ConvertAdjustmentDateToJulian(*Source, EndDate, false)) {
            bValid = false;
            continue;
          }   
        }
        //check that the adjustment dates are relatively correct
        if (EndDate < StartDate) {
          gPrint.Printf("Error: For record %d of %s, the adjustment period is\n"
                        "       incorrect because the end date occurs before the start date.\n",
                        BasePrint::P_READERROR, Source->GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
          bValid = false;
          continue;
        }
        //perform adjustment
        iMaxTract = (TractIndex == -1 ? m_nTracts : TractIndex + 1);
        TractIndex = (TractIndex == -1 ? 0 : TractIndex);
        for (; TractIndex < iMaxTract; ++TractIndex)
           gRelativeRiskAdjustments.AddAdjustmentData(TractIndex, dRelativeRisk, StartDate, EndDate);
    }
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bValid)
      gPrint.Printf("Please see the 'Adjustments File' section in the user guide for help.\n", BasePrint::P_ERROR);
    //print indication if file contained no data
    else if (bEmpty) {
      gPrint.Printf("Error: %s contains no data.\n", BasePrint::P_ERROR, gPrint.GetImpliedFileTypeString().c_str());
      bValid = false;
    }
  }
  catch (ZdException &x) {
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
bool CSaTScanData::ReadCartesianCoordinates(DataSource& Source, std::vector<double>& vCoordinates, short& iScanCount, short iWordOffSet) {
  const char  * pCoordinate;
  int           i;

  for (i=0, iScanCount=0; i < gParameters.GetDimensionsOfData(); ++i, ++iWordOffSet)
     if ((pCoordinate = Source.GetValueAt(iWordOffSet)) != 0) {
       if (sscanf(pCoordinate, "%lf", &(vCoordinates[i])))
         iScanCount++; //track num successful scans, caller of function wants this information
       else {
         //unable to read word as double, print error to print direction and return false
         gPrint.Printf("Error: Value '%s' of record %ld in %s could not be read as ",
                       BasePrint::P_READERROR, pCoordinate, Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
         //we can be specific about which dimension we are attending to read to                                    
         if (i < 2)
           gPrint.Printf("%s-coordinate.\n", BasePrint::P_READERROR, (i == 0 ? "x" : "y"));
         else if (gParameters.GetDimensionsOfData() == 3)
           gPrint.Printf("z-coordinate.\n", BasePrint::P_READERROR);
         else
           gPrint.Printf("z%d-coordinate.\n", BasePrint::P_READERROR, i - 1);
         return false;
       }
     }
  return true;          
}

/** Read the geographic data file. Calls particular function for coordinate type. */
bool CSaTScanData::ReadCoordinatesFile() {
  bool          bReturn;

  try {
    if (!gParameters.UseCoordinatesFile()) {
      m_nTotalTractsAtStart = m_nTracts = gTractHandler.tiGetNumTracts();
      return true;
    }
    gPrint.Printf("Reading the coordinates file\n", BasePrint::P_STDOUT);
    std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(gParameters.GetCoordinatesFileName(), gPrint));
    gPrint.SetImpliedInputFileType(BasePrint::COORDFILE);

    switch (gParameters.GetCoordinatesType()) {
      case CARTESIAN : bReturn = ReadCoordinatesFileAsCartesian(*Source);
                       //now that the number of dimensions is known, validate against requested ellipses
                       if (gParameters.GetDimensionsOfData() > 2 && gParameters.GetSpatialWindowType() == ELLIPTIC &&
                           !(gParameters.GetCriteriaSecondClustersType() == NORESTRICTIONS ||
                            gParameters.GetCriteriaSecondClustersType() == NOGEOOVERLAP)) {
                         gPrint.Printf("Error: Invalid parameter setting for ellipses. SaTScan permits only two\n"
                                       "       dimensions be specified for a centroid when performing an analysis\n"
                                       "       which contain ellipses and restricts reporting of secondary clusters\n"
                                       "       to anything other than 'No Geographical Overlap'. You may want to\n"
                                       "       change the criteria for reporting secondary clusters and run the\n"
                                       "       analysis again.\n", BasePrint::P_ERROR);
                         bReturn = false;
                       }
                       break;
      case LATLON    : bReturn = ReadCoordinatesFileAsLatitudeLongitude(*Source); break;
      default : ZdException::Generate("Unknown coordinate type '%d'.","ReadCoordinatesFile()",gParameters.GetCoordinatesType());
    };
    m_nTotalTractsAtStart = m_nTracts;
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadCoordinatesFile()","CSaTScanData");
    throw;
  }
  return bReturn;
}

/** Read the geographic data file in Cartesian coordinate system.
    If invalid data is found in the file, an error message is printed,
    that record is ignored, and reading continues.
    Return value: true = success, false = errors encountered           */
bool CSaTScanData::ReadCoordinatesFileAsCartesian(DataSource& Source) {
  short                 iScanCount=0;
  const short           uLocationIndex=0;
  bool                  bValid=true, bEmpty=true;
  ZdString              TractIdentifier;
  std::vector<double>   vCoordinates;

  try {
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source.ReadRecord()) {
         //if empty and this record has data, then this is the first record w/ data
         if (bEmpty) {
           bEmpty = false;
           //determine number of dimensions from first record, 2 or more is valid
           iScanCount = Source.GetNumValues();
           //there must be at least two dimensions
           if (iScanCount < 3) {
             gPrint.Printf("Error: The first record of the coordinates file contains %s.\n",
                           BasePrint::P_READERROR, iScanCount == 2 ? "only x-coordinate" : "no coordinates");
             bValid = false;
             break; //stop reading records, the first record defines remaining records format 
           }
           //ok, first record indicates that there are iScanCount - 1 dimensions (first scan is tract identifier)
           //data still could be invalid, but this will be determined like the remaining records
           const_cast<CParameters&>(gParameters).SetDimensionsOfData(iScanCount - 1);
           gTractHandler.tiSetDimensions(gParameters.GetDimensionsOfData());
           gCentroidsHandler.giSetDimensions(gParameters.GetDimensionsOfData());
           vCoordinates.resize(gParameters.GetDimensionsOfData(), 0);
         }
         //read and validate dimensions skip to next record if error reading coordinates as double
         if (! ReadCartesianCoordinates(Source, vCoordinates, iScanCount, 1)) {
           bValid = false;
           continue;
         }
         //validate that we read the correct number of coordinates
         if (iScanCount < gParameters.GetDimensionsOfData()) {
           //Note: since the first record defined the number of dimensions, this error could not happen.
           gPrint.Printf("Error: Record %ld in the coordinates file contains %d dimension%s but the\n"
                         "       first record defined the number of dimensions as %d.\n", BasePrint::P_READERROR,
                         Source.GetCurrentRecordIndex(), iScanCount, (iScanCount == 1 ? "" : "s"), gParameters.GetDimensionsOfData());
           bValid = false;
           continue;
         }
         //add the tract identifier and coordinates to trac handler
         if (! gTractHandler.tiInsertTnode(Source.GetValueAt(uLocationIndex), vCoordinates)) {
           gPrint.Printf("Error: In record %ld of the coordinates file, coordinates for location ID '%s' are redefined.\n",
                         BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), Source.GetValueAt(uLocationIndex));
           bValid = false;
           continue;
         }
         //add tract identifier and coordinates as centroid if a special grid file is not being used
         if (! gParameters.UseSpecialGrid())
           //no need to check return, we would have already gotten error from tract handler
           //for duplicate tract identifier
          gCentroidsHandler.giInsertGnode(Source.GetValueAt(uLocationIndex), vCoordinates);
    }
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bValid)
      gPrint.Printf("Please see the 'Coordinate File' section in the user guide for help.\n", BasePrint::P_ERROR);
    //print indication if file contained no data
    else if (bEmpty) {
      gPrint.Printf("Error: The coordinates file contains no data.\n", BasePrint::P_ERROR);
      bValid = false;
    }
    //validate that we have more than one tract, only a purely temporal analysis is the exception to this rule
    else if (gTractHandler.tiGetNumTracts() == 1 && !gParameters.GetIsPurelyTemporalAnalysis()) {
      gPrint.Printf("Error: For a %s analysis, the coordinates file must contain more than one location.\n",
                    BasePrint::P_ERROR, gParameters.GetAnalysisTypeAsString());
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
bool CSaTScanData::ReadCoordinatesFileAsLatitudeLongitude(DataSource& Source) {
  bool                  bValid=true, bEmpty=true;
  ZdString              TractIdentifier;
  std::vector<double>   vCoordinates;
  const short           uLocationIndex=0;

  try {
    vCoordinates.resize(3/*for conversion*/, 0);
    const_cast<CParameters&>(gParameters).SetDimensionsOfData(3/*for conversion*/);
    gTractHandler.tiSetDimensions(gParameters.GetDimensionsOfData());
    gCentroidsHandler.giSetDimensions(gParameters.GetDimensionsOfData());
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source.ReadRecord()) {
        bEmpty=false;
        if (! ReadLatitudeLongitudeCoordinates(Source, vCoordinates, 1, "coordinates")) {
           bValid = false;
           continue;
        }
        //add the tract identifier and coordinates to trac handler
        if (! gTractHandler.tiInsertTnode(Source.GetValueAt(uLocationIndex), vCoordinates)) {
          gPrint.Printf("Error: For record %ld in the coordinates file, location ID '%s' has already been specified.\n",
                        BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), Source.GetValueAt(uLocationIndex));
          bValid = false;
          continue;
        }
        //add tract identifier and coordinates as centroid if a special grid file is not being used
        if (! gParameters.UseSpecialGrid())
          //no need to check return, we would have already gotten error from tract handler
          //for duplicate tract identifier
          gCentroidsHandler.giInsertGnode(Source.GetValueAt(uLocationIndex), vCoordinates);
    }
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bValid)
      gPrint.Printf("Please see the 'Coordinates File' section in the user guide for help.\n", BasePrint::P_ERROR);
    //print indication if file contained no data
    else if (bEmpty) {
      gPrint.Printf("Error: The coordinates file contains no data.\n", BasePrint::P_ERROR);
      bValid = false;
    }
    //validate that we have more than one tract, only a purely temporal analysis is the exception to this rule
    else if (gTractHandler.tiGetNumTracts() == 1 && !gParameters.GetIsPurelyTemporalAnalysis()) {
      gPrint.Printf("Error: For a %s analysis, the coordinates file must contain more than one record.\n",
                    BasePrint::P_ERROR, gParameters.GetAnalysisTypeAsString());
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

  try {
    gPrint.Printf("Reading the grid file\n", BasePrint::P_STDOUT);
    gPrint.SetImpliedInputFileType(BasePrint::GRIDFILE);
    std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(gParameters.GetSpecialGridFileName(), gPrint));
    switch (gParameters.GetCoordinatesType()) {
      case CARTESIAN : bReturn = ReadGridFileAsCartiesian(*Source); break;
      case LATLON    : bReturn = ReadGridFileAsLatitudeLongitude(*Source); break;
      default : ZdException::Generate("Unknown coordinate type '%d'.","ReadGrid()",gParameters.GetCoordinatesType());
    };
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadGridFile()", "CSaTScanData");
    throw;
  }
  return bReturn;
}

/** Read the special grid data file as Cartesian coordinates.
   If invalid data is found in the file, an error message is printed,
   that record is ignored, and reading continues.
   Return value: true = success, false = errors encountered          */
bool CSaTScanData::ReadGridFileAsCartiesian(DataSource& Source) {
  bool                          bValid=true, bEmpty=true;
  short                         iScanCount;
  std::vector<double>           vCoordinates;
  ZdString                      sId;

  try {
    vCoordinates.resize(gParameters.GetDimensionsOfData(), 0);
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source.ReadRecord()) {
        //there are records with data, but not necessarily valid
        bEmpty = false;
         //read and vaidate dimensions skip to next record if error reading coordinates as double
         if (! ReadCartesianCoordinates(Source, vCoordinates, iScanCount, 0)) {
           bValid = false;
           continue;
         }
        //validate that we read the correct number of coordinates as defined by coordinates system or coordinates file
        if (iScanCount < gParameters.GetDimensionsOfData()) {
          gPrint.Printf("Error: Record %ld in the grid file contains %d dimension%s but the\n"
                        "       coordinates file defined the number of dimensions as %d.\n",
                        BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), iScanCount,
                        (iScanCount == 1 ? "" : "s"), gParameters.GetDimensionsOfData());
          bValid = false;
          continue;
        }
        //add created tract identifer(record number) and read coordinates to structure that mantains list of centroids
        sId = Source.GetNumValues();
        gCentroidsHandler.giInsertGnode(sId.GetCString(), vCoordinates);
    }
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bValid)
      gPrint.Printf("Please see the 'grid file' section in the user guide for help.\n", BasePrint::P_ERROR);
    //print indication if file contained no data
    else if (bEmpty) {
      gPrint.Printf("Error: The Grid file does not contain any data.\n", BasePrint::P_ERROR);
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
bool CSaTScanData::ReadGridFileAsLatitudeLongitude(DataSource& Source) {
  bool    	                bValid=true, bEmpty=true;
  std::vector<double>           vCoordinates;
  ZdString                      sId;

  try {
    vCoordinates.resize(3/*for conversion*/, 0);
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source.ReadRecord()) {
        //there are records with data, but not necessarily valid
        bEmpty=false;
        if (! ReadLatitudeLongitudeCoordinates(Source, vCoordinates, 0, "grid")) {
           bValid = false;
           continue;
        }
        //add created tract identifer(record number) and read coordinates to structure that mantains list of centroids
        sId = Source.GetCurrentRecordIndex();
        gCentroidsHandler.giInsertGnode(sId.GetCString(), vCoordinates);
    }
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bValid)
      gPrint.Printf("Please see the 'grid file' section in the user guide for help.\n", BasePrint::P_ERROR);
    //print indication if file contained no data
    else if (bEmpty) {
      gPrint.Printf("Error: The grid file does not contain any data.\n", BasePrint::P_ERROR);
      bValid = false;
    }
    //record number of centroids
    m_nGridTracts = gCentroidsHandler.giGetNumTracts();
  }
  catch (ZdFileOpenFailedException &x) {
    gPrint.Printf("Error: The grid file '%s' could not be opened.\n",
                  BasePrint::P_ERROR, gParameters.GetSpecialGridFileName().c_str());
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
bool CSaTScanData::ReadLatitudeLongitudeCoordinates(DataSource& Source, std::vector<double> & vCoordinates,
                                                    short iWordOffSet, const char * sSourceFile) {
  const char  * pCoordinate;
  double        dLatitude, dLongitude;

  //read latitude, validating that string can be converted to double
  if ((pCoordinate = Source.GetValueAt(iWordOffSet)) != 0) {
    if (! sscanf(pCoordinate, "%lf", &dLatitude)) {
      gPrint.Printf("Error: The value '%s' of record %ld in the %s file could not be read as the latitude coordinate.\n",
                    BasePrint::P_READERROR, pCoordinate, Source.GetCurrentRecordIndex(), sSourceFile);
      return false;
    }
  }
  else {
    gPrint.Printf("Error: Record %d in the %s file is missing the latitude and longitude coordinates.\n",
                  BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), sSourceFile);
    return false;
  }
  //read longitude, validating that string can be converted to double
  if ((pCoordinate = Source.GetValueAt(++iWordOffSet)) != 0) {
    if (! sscanf(pCoordinate, "%lf", &dLongitude)) {
      gPrint.Printf("Error: The value '%s' of record %ld in the %s file could not be read as the longitude coordinate.\n",
                    BasePrint::P_READERROR, pCoordinate, Source.GetCurrentRecordIndex(), sSourceFile);
      return false;
    }
  }
  else {
    gPrint.Printf("Error: Record %ld in the %s file is missing the longitude coordinate.\n",
                  BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), sSourceFile);
    return false;
  }
  //validate that there is not extra data for record
  if ((pCoordinate = Source.GetValueAt(++iWordOffSet)) != 0) {
    gPrint.Printf("Error: Record %ld in the %s file contains extra data: '%s'.\n",
                  BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), sSourceFile, pCoordinate);
    return false;
  }
  //validate range of latitude value
  if ((fabs(dLatitude) > 90.0)) {
    gPrint.Printf("Error: Latitude coordinate %lf, for record %ld in the %s file, is out of range.\n"
                  "       Latitude must be between -90 and 90.\n",
                  BasePrint::P_READERROR, dLatitude, Source.GetCurrentRecordIndex(), sSourceFile);
    return false;
  }
  //validate range of longitude value
  if ((fabs(dLongitude) > 180.0)) {
    gPrint.Printf("Error: Longitude coordinate %lf, for record %ld in the %s file, is out of range.\n"
                  "       Longitude must be between -180 and 180.\n",
                  BasePrint::P_READERROR, dLongitude, Source.GetCurrentRecordIndex(), sSourceFile);
    return false;
  }
  //convert to 3 dimensions then can add tracts
  ConvertFromLatLong(dLatitude, dLongitude, vCoordinates);
  return true;
}

/** Read the special population that will be used to construct circles
    about grid points(centroids).                                      */
bool CSaTScanData::ReadMaxCirclePopulationFile() {
  bool          bValid=true, bEmpty=true;
  tract_t       TractIdentifierIndex;
  float         fPopulation;
  const short   uLocationIndex=0, uPopulationIndex=1;

  try {
    gPrint.SetImpliedInputFileType(BasePrint::MAXCIRCLEPOPFILE);

    gPrint.Printf("Reading the max circle size file\n", BasePrint::P_STDOUT);
    std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(gParameters.GetMaxCirclePopulationFileName(), gPrint));
    //initialize circle-measure array
    gvMaxCirclePopulation.resize(m_nTracts, 0);

    //1st pass, determine unique population dates. Notes errors with records and continues reading.
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source->ReadRecord()) {
        bEmpty=false;
        //read tract identifier
        if ((TractIdentifierIndex = gTractHandler.tiGetTractIndex(Source->GetValueAt(uLocationIndex))) == -1) {
          gPrint.Printf("Error: Unknown location ID in the %s, record %ld.\n"
                        "       '%s' not specified in the coordinates file.\n",
                        BasePrint::P_READERROR, gPrint.GetImpliedFileTypeString().c_str(), Source->GetCurrentRecordIndex(), Source->GetValueAt(uLocationIndex));
          bValid = false;
          continue;
        }
        //read population
        if (!Source->GetValueAt(uPopulationIndex)) {
          gPrint.Printf("Error: The population is missing in record %d of the %s.\n",
                        BasePrint::P_READERROR, Source->GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
          bValid = false;
          continue;
        }
        if (sscanf(Source->GetValueAt(uPopulationIndex), "%f", &fPopulation) != 1) {
          gPrint.Printf("Error: The population value '%s' in record %ld, of %s, is not a number.\n",
                        BasePrint::P_READERROR, Source->GetValueAt(uPopulationIndex), Source->GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
          bValid = false;
          continue;
        }
        //validate that population is not negative or exceeding type precision
        if (fPopulation < 0) {//validate that count is not negative or exceeds type precision
          if (strstr(Source->GetValueAt(uPopulationIndex), "-"))
             gPrint.Printf("Error: Negative population in record %ld of %s.\n",
                           BasePrint::P_READERROR, Source->GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
          else
             gPrint.Printf("Error: Population '%s' exceeds the maximum allowed value of %i in record %ld of %s.\n",
                           BasePrint::P_READERROR, Source->GetValueAt(uPopulationIndex), std::numeric_limits<float>::max(),
                           Source->GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
           bValid = false;
           continue;
        }
        gvMaxCirclePopulation[TractIdentifierIndex] += fPopulation;
        m_nTotalMaxCirclePopulation += fPopulation;
    }
    // total population can not be zero
    if (m_nTotalMaxCirclePopulation == 0) {
      bValid = false;
      gPrint.Printf("Error: The total population for %s is zero.\n",
                    BasePrint::P_ERROR, gPrint.GetImpliedFileTypeString().c_str());
    }
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bValid)
      gPrint.Printf("Please see the 'Max Circle Size File' section in the user guide for help.\n", BasePrint::P_READERROR);
    //print indication if file contained no data
    else if (bEmpty) {
      gPrint.Printf("Error: %s contains no data.\n", BasePrint::P_ERROR, gPrint.GetImpliedFileTypeString().c_str());
      bValid = false;
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadMaxCirclePopulationFile()", "CSaTScanData");
    throw;
  }
  return bValid;
}

