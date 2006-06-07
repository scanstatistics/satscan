//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SaTScanDataRead.h"
#include "SaTScanData.h"
#include "DateStringParser.h"
#include "SSException.h"
#include "DataSource.h"
#include "BernoulliDataSetHandler.h"
#include "RankDataSetHandler.h"
#include "ExponentialDataSetHandler.h"
#include "NormalDataSetHandler.h"
#include "PoissonDataSetHandler.h"
#include "SpaceTimePermutationDataSetHandler.h"
#include "OrdinalDataSetHandler.h"

const short SaTScanDataReader::guLocationIndex = 0;

/** class constructor */
SaTScanDataReader::SaTScanDataReader(CSaTScanData& DataHub)
                  :gDataHub(DataHub), gParameters(gDataHub.GetParameters()), gPrint(gDataHub.gPrint),
                   gCentroidsHandler(*gDataHub.gCentroidsHandler.get()), gTractHandler(*gDataHub.gTractHandler.get()) {}


/** Converts passed string specifiying a adjustment file date to a julian date.
    Precision is determined by date formats( YYYY/MM/DD, YYYY/MM, YYYY, YY/MM/DD,
    YY/MM, YY ) which is the complete set of valid formats that SaTScan currently
    supports. Since we accumulate errors/warnings when reading input files,
    indication of a bad date is returned and any messages sent to print direction. */
bool SaTScanDataReader::ConvertAdjustmentDateToJulian(DataSource& Source, Julian & JulianDate, bool bStartDate) {
  short                                 iDateIndex;
  DateStringParser                      DateParser;
  DateStringParser::ParserStatus        eStatus;

  if (gParameters.GetPrecisionOfTimesType() == NONE)
    JulianDate = (bStartDate ? gDataHub.m_nStartDate: gDataHub.m_nEndDate);
  else {
    iDateIndex = (bStartDate ? 2: 3);
    //read and validate date
    if (!Source.GetValueAt(iDateIndex)) {
      gPrint.Printf("Error: Record %ld in %s does not contain a %s date.\n", BasePrint::P_READERROR, Source.GetCurrentRecordIndex(),
                    gPrint.GetImpliedFileTypeString().c_str(), (bStartDate ? "start": "end"));
      return false;
    }
    eStatus = DateParser.ParseAdjustmentDateString(Source.GetValueAt(iDateIndex), gDataHub.m_nStartDate, gDataHub.m_nEndDate, JulianDate, bStartDate);
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
    if (!(gDataHub.m_nStartDate <= JulianDate && JulianDate <= gDataHub.m_nEndDate)) {
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

/** First calls internal methods to prepare internal structure:
    - calculate time interval start times
    - calculate a clusters maximum temporal window length in terms of time intervals
    - calculate indexes of flexible scanning window, if requested
    - calculate start interval index of start date of prospective analyses
    - read input data from files base upon probability model
    Throws ResolvableException if read fails. */
void SaTScanDataReader::Read() {
  bool  bReadSuccess;

  try {
    // First calculate time interval indexes, these values will be utilized by data read process.
    gDataHub.CalculateTimeIntervalIndexes();
    switch (gParameters.GetProbabilityModelType()) {
      case POISSON              : bReadSuccess = ReadPoissonData(); break;
      case BERNOULLI            : bReadSuccess = ReadBernoulliData(); break;
      case SPACETIMEPERMUTATION : bReadSuccess = ReadSpaceTimePermutationData(); break;
      case ORDINAL              : bReadSuccess = ReadOrdinalData(); break;
      case EXPONENTIAL          : bReadSuccess = ReadExponentialData(); break;
      case NORMAL               : bReadSuccess = ReadNormalData(); break;
      case RANK                 : bReadSuccess = ReadRankData(); break;
      default :
        ZdGenerateException("Unknown probability model type '%d'.","ReadDataFromFiles()", gParameters.GetProbabilityModelType());
    };
    if (gParameters.UseLocationNeighborsFile() && !ReadUserSpecifiedNeighbors())
      bReadSuccess = false;
    if (!bReadSuccess)
      GenerateResolvableException("\nProblem encountered when reading the data from the input files.", "ReadDataFromFiles");
    //now that all data has been read, the tract handler can combine references locations with duplicate coordinates
    gTractHandler.tiConcaticateDuplicateTractIdentifiers();
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadDataFromFiles()","SaTScanDataReader");
    throw;
  }
}


/** Read the relative risks file
    -- unlike other input files of system, records read from relative risks
       file are applied directly to the measure structure, just post calculation
       of measure and prior to temporal adjustments and making cumulative. */
bool SaTScanDataReader::ReadAdjustmentsByRelativeRisksFile() {
  bool          bValid=true, bRestrictedLocations(!gDataHub.GetParameters().UseCoordinatesFile()), bEmpty=true;
  tract_t       TractIndex, iMaxTract;
  double        dRelativeRisk;
  Julian        StartDate, EndDate;
  int           iNumWords;
  const short   uLocationIndex=0, uAdjustmentIndex=1;

  try {
    gPrint.SetImpliedInputFileType(BasePrint::ADJ_BY_RR_FILE);

    gPrint.Printf("Reading the adjustments file\n", BasePrint::P_STDOUT);
    std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(gParameters.GetAdjustmentsByRelativeRisksFilename(), gPrint));
    gDataHub.gRelativeRiskAdjustments.Empty();
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
        else {
          //Validate that tract identifer is one of those defined in the coordinates file.
          SaTScanDataReader::RecordStatusType eStatus = RetrieveLocationIndex(*Source, TractIndex);
          if (eStatus == SaTScanDataReader::Ignored)
            continue;
          if (eStatus == SaTScanDataReader::Rejected) {
            bValid = false;
            continue;
          }
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
          StartDate = gDataHub.m_nStartDate;
          EndDate = gDataHub.m_nEndDate;
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
        iMaxTract = (TractIndex == -1 ? gDataHub.m_nTracts : TractIndex + 1);
        TractIndex = (TractIndex == -1 ? 0 : TractIndex);
        for (; TractIndex < iMaxTract; ++TractIndex)
           gDataHub.gRelativeRiskAdjustments.AddAdjustmentData(TractIndex, dRelativeRisk, StartDate, EndDate);
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
    x.AddCallpath("ReadAdjustmentsByRelativeRisksFile()","SaTScanDataReader");
    throw;
  }
  return bValid;
}

/** reads data from input files for a Bernoulli probability model */
bool SaTScanDataReader::ReadBernoulliData() {
  try {
    if (!ReadCoordinatesFile())
      return false;

    gDataHub.gDataSets.reset(new BernoulliDataSetHandler(gDataHub, gPrint));
    if (!gDataHub.gDataSets->ReadData())
      return false;
    if (gParameters.UseMaxCirclePopulationFile() && !ReadMaxCirclePopulationFile())
        return false;
    if (gParameters.UseSpecialGrid() && !ReadGridFile())
      return false;
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadBernoulliData()","SaTScanDataReader");
    throw;
  }
  return true;
}

/** Reads cartesian coordinates into vector.
    Note: coordinate vector should already be sized to defined dimensions.
    Returns indication of whether words in passed string could be converted to
    coordinates. Tracks number of words successfully scanned, the caller of
    function will use this information to confirm that coordinates scanned is
    not less than defined dimensions. The reason we don't check scanned dimensions
    here is that a generic error message could not be implemented. */
bool SaTScanDataReader::ReadCartesianCoordinates(DataSource& Source, std::vector<double>& vCoordinates, short& iScanCount, short iWordOffSet) {
  const char  * pCoordinate;
  int           i;

  for (i=0, iScanCount=0; i < (int)vCoordinates.size(); ++i, ++iWordOffSet)
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
         else if (vCoordinates.size() == 3)
           gPrint.Printf("z-coordinate.\n", BasePrint::P_READERROR);
         else
           gPrint.Printf("z%d-coordinate.\n", BasePrint::P_READERROR, i - 1);
         return false;
       }
     }
  return true;          
}

/** Read the geographic data file. Calls particular function for coordinate type. */
bool SaTScanDataReader::ReadCoordinatesFile() {
  bool          bReturn;

  try {
    if (!gParameters.UseCoordinatesFile()) {
      gDataHub.m_nTotalTractsAtStart = gDataHub.m_nTracts = gTractHandler.tiGetNumTracts();
      return true;
    }
    gPrint.Printf("Reading the coordinates file\n", BasePrint::P_STDOUT);
    std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(gParameters.GetCoordinatesFileName(), gPrint));
    gPrint.SetImpliedInputFileType(BasePrint::COORDFILE);

    if (gParameters.UseLocationNeighborsFile())
      bReturn = ReadCoordinatesFileAsLocationIdFile(*Source);
    else {
      switch (gParameters.GetCoordinatesType()) {
        case CARTESIAN : bReturn = ReadCoordinatesFileAsCartesian(*Source);
                         //now that the number of dimensions is known, validate against requested ellipses
                         if (gTractHandler.tiGetDimensions() > 2 && gParameters.GetSpatialWindowType() == ELLIPTIC &&
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
    }
    gDataHub.m_nTotalTractsAtStart = gDataHub.m_nTracts;
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadCoordinatesFile()","SaTScanDataReader");
    throw;
  }
  return bReturn;
}

/** Read the geographic data file in Cartesian coordinate system.
    If invalid data is found in the file, an error message is printed,
    that record is ignored, and reading continues.
    Return value: true = success, false = errors encountered           */
bool SaTScanDataReader::ReadCoordinatesFileAsCartesian(DataSource& Source) {
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
           gTractHandler.tiSetCoordinateDimensions(iScanCount - 1);
           vCoordinates.resize(iScanCount - 1, 0);
         }
         //read and validate dimensions skip to next record if error reading coordinates as double
         if (! ReadCartesianCoordinates(Source, vCoordinates, iScanCount, 1)) {
           bValid = false;
           continue;
         }
         //validate that we read the correct number of coordinates
         if (iScanCount < gTractHandler.tiGetDimensions()) {
           //Note: since the first record defined the number of dimensions, this error could not happen.
           gPrint.Printf("Error: Record %ld in the coordinates file contains %d dimension%s but the\n"
                         "       first record defined the number of dimensions as %d.\n", BasePrint::P_READERROR,
                         Source.GetCurrentRecordIndex(), iScanCount, (iScanCount == 1 ? "" : "s"), gTractHandler.tiGetDimensions());
           bValid = false;
           continue;
         }
         //add the tract identifier and coordinates to trac handler
         gTractHandler.tiInsertTnode(Source.GetValueAt(uLocationIndex), vCoordinates);
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
    //now sort tracts by identifiers
    gTractHandler.SortTractsByIndentifiers();
    //record number of locations read
    gDataHub.m_nTracts = gTractHandler.tiGetNumTracts();
    //record number of centroids read
    gDataHub.m_nGridTracts = gCentroidsHandler.giGetNumTracts();
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadCoordinatesFileAsCartesian()", "SaTScanDataReader");
    throw;
  }
  return bValid;
}

/** Read the latitude/longitude geographic data file.
    If invalid data is found in the file, an error message
    is printed, that record is ignored, and reading continues.
    Return value: true = success, false = errors encountered   */
bool SaTScanDataReader::ReadCoordinatesFileAsLatitudeLongitude(DataSource& Source) {
  bool                  bValid=true, bEmpty=true;
  ZdString              TractIdentifier;
  std::vector<double>   vCoordinates;
  const short           uLocationIndex=0;

  try {
    vCoordinates.resize(3/*for conversion*/, 0);
    gTractHandler.tiSetCoordinateDimensions(3/*for conversion*/);
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source.ReadRecord()) {
        bEmpty=false;
        if (! ReadLatitudeLongitudeCoordinates(Source, vCoordinates, 1, "coordinates")) {
           bValid = false;
           continue;
        }
        //add the tract identifier and coordinates to trac handler
        gTractHandler.tiInsertTnode(Source.GetValueAt(uLocationIndex), vCoordinates);
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
    //now sort tracts by identifiers
    gTractHandler.SortTractsByIndentifiers();        
    //record number of locations read
    gDataHub.m_nTracts = gTractHandler.tiGetNumTracts();
    //record number of centroids read
    gDataHub.m_nGridTracts = gCentroidsHandler.giGetNumTracts();
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadCoordinatesFileAsLatitudeLongitude()", "SaTScanDataReader");
    throw;
  }
  return bValid;
}

/** Reads the coordinates file source to parse location identifiers only. That is,
    no coordinates are expected or read. If invalid data is found in the file,
    an error message is printed, that record is ignored, and reading continues.
    Return value: true = success, false = errors encountered   */
bool SaTScanDataReader::ReadCoordinatesFileAsLocationIdFile(DataSource& Source) {
  bool                  bValid=true, bEmpty=true;
  ZdString              TractIdentifier;
  const short           uLocationIndex=0;

  try {
    gTractHandler.tiSetCoordinateDimensions(0);
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source.ReadRecord()) {
        bEmpty=false;
        //add the tract identifier and coordinates to tract handler
        gTractHandler.tiInsertTnode(Source.GetValueAt(uLocationIndex));
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
    gDataHub.m_nTracts = gTractHandler.tiGetNumTracts();
    //record number of centroids read
    gDataHub.m_nGridTracts = gCentroidsHandler.giGetNumTracts();
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadCoordinatesFileAsLocationIdFile()", "SaTScanDataReader");
    throw;
  }
  return bValid;
}

/** reads data from input files for a Exponential probability model */
bool SaTScanDataReader::ReadExponentialData() {
  try {
    if (!ReadCoordinatesFile())
      return false;
    gDataHub.gDataSets.reset(new ExponentialDataSetHandler(gDataHub, gPrint));
    if (!gDataHub.gDataSets->ReadData())
      return false;
    if (gParameters.UseMaxCirclePopulationFile() && !ReadMaxCirclePopulationFile())
      return false;
    if (gParameters.UseSpecialGrid() && !ReadGridFile())
      return false;
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadExponentialData()","SaTScanDataReader");
    throw;
  }
  return true;
}

/** Read the special grid file.  Calls particular read given coordinate type. */
bool SaTScanDataReader::ReadGridFile() {
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
    x.AddCallpath("ReadGridFile()", "SaTScanDataReader");
    throw;
  }
  return bReturn;
}

/** Read the special grid data file as Cartesian coordinates.
   If invalid data is found in the file, an error message is printed,
   that record is ignored, and reading continues.
   Return value: true = success, false = errors encountered          */
bool SaTScanDataReader::ReadGridFileAsCartiesian(DataSource& Source) {
  bool                          bValid=true, bEmpty=true;
  short                         iScanCount;
  std::vector<double>           vCoordinates;
  ZdString                      sId;
  CentroidHandler             * pGridPoints;
  
  try {
    if ((pGridPoints = dynamic_cast<CentroidHandler*>(&gCentroidsHandler)) == 0)
      ZdGenerateException("Not a CentroidHandler type.", "ReadGridFileAsCartiesian()");
    pGridPoints->giSetDimensions(gTractHandler.tiGetDimensions());
    vCoordinates.resize(gTractHandler.tiGetDimensions(), 0);
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source.ReadRecord()) {
        //there are records with data, but not necessarily valid
        bEmpty = false;
         //read and vaidate dimensions skip to next record if error reading coordinates as double
         if (! ReadCartesianCoordinates(Source, vCoordinates, iScanCount, 0)) {
           bValid = false;
           continue;
         }
        //validate that we read the correct number of coordinates as defined by coordinates system or coordinates file
        if (iScanCount < gTractHandler.tiGetDimensions()) {
          gPrint.Printf("Error: Record %ld in the grid file contains %d dimension%s but the\n"
                        "       coordinates file defined the number of dimensions as %d.\n",
                        BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), iScanCount,
                        (iScanCount == 1 ? "" : "s"), gTractHandler.tiGetDimensions());
          bValid = false;
          continue;
        }
        //add created tract identifer(record number) and read coordinates to structure that mantains list of centroids
        sId = Source.GetCurrentRecordIndex();
        pGridPoints->giInsertGnode(sId.GetCString(), vCoordinates);
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
    //now sort grid points by labels
    pGridPoints->SortGridPointsByLabel();
    //record number of centroids read
    gDataHub.m_nGridTracts = pGridPoints->giGetNumTracts();
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadGridFileAsCartiesian()","SaTScanDataReader");
    throw;
  }
  return bValid;
}

/** Read the special grid data file as latitude/longitude coordinates.
   If invalid data is found in the file, an error message is printed,
   that record is ignored, and reading continues.
   Return value: true = success, false = errors encountered           */
bool SaTScanDataReader::ReadGridFileAsLatitudeLongitude(DataSource& Source) {
  bool    	                bValid=true, bEmpty=true;
  std::vector<double>           vCoordinates;
  ZdString                      sId;
  CentroidHandler             * pGridPoints;
  
  try {
    if ((pGridPoints = dynamic_cast<CentroidHandler*>(&gCentroidsHandler)) == 0)
      ZdGenerateException("Not a CentroidHandler type.", "ReadGridFileAsCartiesian()");
    pGridPoints->giSetDimensions(gTractHandler.tiGetDimensions());
    vCoordinates.resize(gTractHandler.tiGetDimensions(), 0);
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source.ReadRecord()) {
        //there are records with data, but not necessarily valid
        bEmpty=false;
        if (! ReadLatitudeLongitudeCoordinates(Source, vCoordinates, 0, "grid")) {
           bValid = false;
           continue;
        }
        //add created tract identifer(record number) and read coordinates to structure that mantains list of centroids
        sId = Source.GetCurrentRecordIndex();
        pGridPoints->giInsertGnode(sId.GetCString(), vCoordinates);
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
    //now sort grid points by labels
    pGridPoints->SortGridPointsByLabel();
    //record number of centroids read
    gDataHub.m_nGridTracts = pGridPoints->giGetNumTracts();
  }
  catch (ZdFileOpenFailedException &x) {
    gPrint.Printf("Error: The grid file '%s' could not be opened.\n",
                  BasePrint::P_ERROR, gParameters.GetSpecialGridFileName().c_str());
    return false;
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadGridFileAsLatitudeLongitude()","SaTScanDataReader");
    throw;
  }
  return bValid;
}

/** Reads latitude/longitude coordinates into vector.
    Note: coordinate vector should already be sized to 3 dimensions.
    Returns indication of whether words in passed string could be converted to
    coordinates. Checks that coordinates are in range and converts to cartesian
    coordinates. */
bool SaTScanDataReader::ReadLatitudeLongitudeCoordinates(DataSource& Source, std::vector<double> & vCoordinates,
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
bool SaTScanDataReader::ReadMaxCirclePopulationFile() {
  bool          bValid=true, bEmpty=true;
  tract_t       TractIdentifierIndex;
  float         fPopulation;
  const short   uLocationIndex=0, uPopulationIndex=1;

  try {
    gPrint.SetImpliedInputFileType(BasePrint::MAXCIRCLEPOPFILE);

    gPrint.Printf("Reading the max circle size file\n", BasePrint::P_STDOUT);
    std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(gParameters.GetMaxCirclePopulationFileName(), gPrint));
    //initialize circle-measure array
    gDataHub.gvMaxCirclePopulation.resize(gDataHub.m_nTracts, 0);

    //1st pass, determine unique population dates. Notes errors with records and continues reading.
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source->ReadRecord()) {
        bEmpty=false;
        //Validate that tract identifer is one of those defined in the coordinates file.
        SaTScanDataReader::RecordStatusType eStatus = RetrieveLocationIndex(*Source, TractIdentifierIndex);
        if (eStatus == SaTScanDataReader::Ignored)
          continue;
        if (eStatus == SaTScanDataReader::Rejected) {
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
        gDataHub.gvMaxCirclePopulation[TractIdentifierIndex] += fPopulation;
        gDataHub.m_nTotalMaxCirclePopulation += fPopulation;
    }
    // total population can not be zero
    if (gDataHub.m_nTotalMaxCirclePopulation == 0) {
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
    x.AddCallpath("ReadMaxCirclePopulationFile()","SaTScanDataReader");
    throw;
  }
  return bValid;
}

/** reads data from input files for a normal probability model */
bool SaTScanDataReader::ReadNormalData() {
  try {
    if (!ReadCoordinatesFile())
      return false;
    gDataHub.gDataSets.reset(new NormalDataSetHandler(gDataHub, gPrint));
    if (!gDataHub.gDataSets->ReadData())
      return false;
    if (gParameters.UseMaxCirclePopulationFile() && !ReadMaxCirclePopulationFile())
        return false;
    if (gParameters.UseSpecialGrid() && !ReadGridFile())
      return false;
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadNormalData()","SaTScanDataReader");
    throw;
  }
  return true;
}

/** reads data from input files for a Ordinal probability model */
bool SaTScanDataReader::ReadOrdinalData() {
  try {
    if (!ReadCoordinatesFile())
      return false;
    gDataHub.gDataSets.reset(new OrdinalDataSetHandler(gDataHub, gPrint));
    if (!gDataHub.gDataSets->ReadData())
      return false;
    if (gParameters.UseMaxCirclePopulationFile() && !ReadMaxCirclePopulationFile())
        return false;
    if (gParameters.UseSpecialGrid() && !ReadGridFile())
      return false;
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadOrdinalData()","SaTScanDataReader");
    throw;
  }
  return true;
}


/** reads data from input files for a Poisson probability model */
bool SaTScanDataReader::ReadPoissonData() {
  try {
    if (!ReadCoordinatesFile())
      return false;

    gDataHub.gDataSets.reset(new PoissonDataSetHandler(gDataHub, gPrint));
    if (!gDataHub.gDataSets->ReadData())
      return false;
    if (gParameters.UseAdjustmentForRelativeRisksFile() && !ReadAdjustmentsByRelativeRisksFile())
      return false;
    if (gParameters.UseMaxCirclePopulationFile() && !ReadMaxCirclePopulationFile())
        return false;
    if (gParameters.UseSpecialGrid() && !ReadGridFile())
      return false;
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadPoissonData()","SaTScanDataReader");
    throw;
  }
  return true;
}

/** reads data from input files for a Rank probability model */
bool SaTScanDataReader::ReadRankData() {
  try {
    if (!ReadCoordinatesFile())
      return false;
    gDataHub.gDataSets.reset(new RankDataSetHandler(gDataHub, gPrint));
    if (!gDataHub.gDataSets->ReadData())
      return false;
    if (gParameters.UseMaxCirclePopulationFile() && !ReadMaxCirclePopulationFile())
        return false;
    if (gParameters.UseSpecialGrid() && !ReadGridFile())
      return false;
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadRankData()","SaTScanDataReader");
    throw;
  }
  return true;
}

/** reads data from input files for a space-time permutation probability model */
bool SaTScanDataReader::ReadSpaceTimePermutationData() {
  try {
    if (!ReadCoordinatesFile())
      return false;
    if (gParameters.UseMaxCirclePopulationFile() && !ReadMaxCirclePopulationFile())
       return false;
    gDataHub.gDataSets.reset(new SpaceTimePermutationDataSetHandler(gDataHub, gPrint));
    if (!gDataHub.gDataSets->ReadData())
      return false;
    if (gParameters.UseSpecialGrid() && !ReadGridFile())
      return false;
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadSpaceTimePermutationData()","SaTScanDataReader");
    throw;
  }
  return true;
}

/** Opens coordinates file data source and parses a list of location identifiers from each
    record. These lists represent the geographical extension of a network of neighboring locations,
    centralized at the first location identifier in list. This process is alternative to the process
    of calculating neighbors about centroids through expanding circles. The lists read from file are
    stored in the same sorted array structure as expanding circle process. Errors encountered during
    the read process are printed to BasePrint object and reading continues. Returns true if read completes
    without detecting errors, else returns false. */
bool SaTScanDataReader::ReadUserSpecifiedNeighbors() {
  bool                  bValid=true, bEmpty=true;
  short                 uLocation0ffset;
  tract_t               tLocationIndex;
  std::vector<tract_t>  vRecordNeighborList;

  try {
    gPrint.Printf("Reading the location neighbors file\n", BasePrint::P_STDOUT);
    std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(gParameters.GetLocationNeighborsFileName().c_str(), gPrint));
    gDataHub.AllocateSortedArray();
    gPrint.SetImpliedInputFileType(BasePrint::LOCATION_NEIGHBORS_FILE);
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source->ReadRecord()) {
      uLocation0ffset=0;
      vRecordNeighborList.clear();
      while (Source->GetValueAt(uLocation0ffset)) {
        //Validate that tract identifer is one of those defined in the coordinates file.
        if ((tLocationIndex = gDataHub.GetTInfo()->tiGetTractIndex(Source->GetValueAt(uLocation0ffset))) == -1) {
           bValid = false;
           gPrint.Printf("Error: Unknown location ID in %s, record %ld.\n"
                         "       '%s' not specified in the coordinates file.\n", BasePrint::P_READERROR,
                         gPrint.GetImpliedFileTypeString().c_str(), Source->GetCurrentRecordIndex(), Source->GetValueAt(uLocation0ffset));
           break;
        }
        if (std::find(vRecordNeighborList.begin(), vRecordNeighborList.end(), tLocationIndex) != vRecordNeighborList.end()) {
           bValid = false;
           gPrint.Printf("Error: Location ID '%s' occurs multiple times in record %ld of %s.\n", BasePrint::P_READERROR,
                          Source->GetValueAt(uLocation0ffset), Source->GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
           break;               
        }
          vRecordNeighborList.push_back(tLocationIndex);
        ++uLocation0ffset;
        bEmpty = false;
      }
      if (bValid)
        gDataHub.AllocateSortedArrayNeighbors(vRecordNeighborList);
    }
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bValid)
      gPrint.Printf("Please see the 'Location Neighbors File' section in the user guide for help.\n", BasePrint::P_READERROR);
    //print indication if file contained no data
    else if (bEmpty) {
      gPrint.Printf("Error: %s contains no data.\n", BasePrint::P_ERROR, gPrint.GetImpliedFileTypeString().c_str());
      bValid = false;
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadUserSpecifiedNeighbors()","SaTScanDataReader");
    throw;
  }
  return bValid;
}

/** Retrieves location id index from data source. If location id not found:
    - if coordinates data checking is strict, reports error to BasePrint object
      and returns SaTScanDataReader::Rejected
    - if coordinates data checking is relaxed, reports warning to BasePrint object
      and returns SaTScanDataReader::Ignored; reports only first occurance
    - else returns SaTScanDataReader::Accepted */
SaTScanDataReader::RecordStatusType SaTScanDataReader::RetrieveLocationIndex(DataSource& Source, tract_t& tLocationIndex) {
   //Validate that tract identifer is one of those defined in the coordinates file.
   if ((tLocationIndex = gDataHub.GetTInfo()->tiGetTractIndex(Source.GetValueAt(guLocationIndex))) == -1) {
     if (gParameters.GetCoordinatesDataCheckingType() == STRICTCOORDINATES) {
       gPrint.Printf("Error: Unknown location ID in %s, record %ld.\n"
                     "       '%s' not specified in the coordinates file.\n", BasePrint::P_READERROR,
                     gPrint.GetImpliedFileTypeString().c_str(), Source.GetCurrentRecordIndex(), Source.GetValueAt(guLocationIndex));
       return SaTScanDataReader::Rejected;
     }
     if (std::find(gmSourceLocationWarned.begin(), gmSourceLocationWarned.end(), reinterpret_cast<void*>(&Source)) == gmSourceLocationWarned.end()) {
       gPrint.Printf("Warning: Some records in %s reference a location ID that was not specified in the coordinates file.\n"
                     "         These are ignored in the analysis.\n", BasePrint::P_WARNING, gPrint.GetImpliedFileTypeString().c_str());
       gmSourceLocationWarned.push_back(reinterpret_cast<void*>(&Source));
     }
     return SaTScanDataReader::Ignored;
  }
  return SaTScanDataReader::Accepted;
}

