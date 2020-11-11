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
#include "ParametersPrint.h"
#include "MetaTractManager.h"
#include "HomogeneousPoissonDataSetHandler.h"
#include "UniformTimeDataSetHandler.h"

const long SaTScanDataReader::guLocationIndex = 0;

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
  long                                  iDateIndex;
  DateStringParser                      DateParser(gDataHub.GetParameters().GetPrecisionOfTimesType());
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
        gPrint.Printf("Error: Invalid %s date '%s' in %s, record %ld.\n%s", BasePrint::P_READERROR,
                      (bStartDate ? "start": "end"), Source.GetValueAt(iDateIndex),
                      gPrint.GetImpliedFileTypeString().c_str(), Source.GetCurrentRecordIndex(), DateParser.getLastParseError().c_str());
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
      case CATEGORICAL          :
      case ORDINAL              : bReadSuccess = ReadOrdinalData(); break;
      case EXPONENTIAL          : bReadSuccess = ReadExponentialData(); break;
      case NORMAL               : bReadSuccess = ReadNormalData(); break;
      case RANK                 : bReadSuccess = ReadRankData(); break;
      case UNIFORMTIME          : bReadSuccess = ReadUniformTimeData(); break;
      case HOMOGENEOUSPOISSON   : bReadSuccess = ReadHomogeneousPoissonData(); break;
      default :
        throw prg_error("Unknown probability model type '%d'.","Read()", gParameters.GetProbabilityModelType());
    };
    if (!bReadSuccess)
      throw resolvable_error("\nProblem encountered when reading the data from the input files.");
  }
  catch (prg_exception& x) {
    x.addTrace("Read()","SaTScanDataReader");
    throw;
  }
}

void SaTScanDataReader::ReadBernoulliDrilldown() {
	bool  bReadSuccess = true;

	try {
		// First calculate time interval indexes, these values will be utilized by data read process.
		gDataHub.CalculateTimeIntervalIndexes();
		// Read coordinates, max circle and grid file - skip data files.
		if (!ReadCoordinatesFile())
			bReadSuccess = false;
		gDataHub.gDataSets.reset(new BernoulliDataSetHandler(gDataHub, gPrint));
		gDataHub.gDataSets->SetRandomizers();
		if (gParameters.UseMaxCirclePopulationFile() && !ReadMaxCirclePopulationFile())
			bReadSuccess = false;
		if (gParameters.UseSpecialGrid() && !ReadGridFile())
			bReadSuccess = false;
		if (!bReadSuccess)
			throw resolvable_error("\nProblem encountered when reading the data from the input files.");
	} catch (prg_exception& x) {
		x.addTrace("ReadBernoulliDrilldown()", "SaTScanDataReader");
		throw;
	}
}

/** Read the relative risks file
    -- unlike other input files of system, records read from relative risks
       file are applied directly to the measure structure, just post calculation
       of measure and prior to temporal adjustments and making cumulative. */
bool SaTScanDataReader::ReadAdjustmentsByRelativeRisksFile(const std::string& filename, RiskAdjustmentsContainer_t& rrAdjustments, bool consolidate) {
  bool          bValid=true, bEmpty=true,
                bRestrictedLocations(gParameters.GetIsPurelyTemporalAnalysis() &&
                                     gParameters.GetCoordinatesFileName().size() == 0);
  tract_t       TractIndex, iMaxTract;
  double        dRelativeRisk;
  Julian        StartDate, EndDate;
  int           iNumWords;
  const long    uLocationIndex=0, uAdjustmentIndex=1;

  try {
    gPrint.SetImpliedInputFileType(BasePrint::ADJ_BY_RR_FILE);

    gPrint.Printf("Reading the adjustments file\n", BasePrint::P_STDOUT);
    std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(filename, gParameters.getInputSource(ADJ_BY_RR_FILE), gPrint));
    // start with one collection of adjustments
    rrAdjustments.clear();
    rrAdjustments.push_back(RiskAdjustments_t(new RelativeRiskAdjustmentHandler(gDataHub)));

    RiskAdjustments_t adjustments = rrAdjustments.front();
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source->ReadRecord()) {
        // if not consolidating adjustments, then we'll use blank record as trigger to create new adjustment collection
        if (!consolidate && Source->detectBlankRecordFlag()) {
            if (adjustments->get().size()) { // create new collection of adjustments
                rrAdjustments.push_back(RiskAdjustments_t(new RelativeRiskAdjustmentHandler(gDataHub)));
                adjustments = rrAdjustments.back();
            }
            Source->clearBlankRecordFlag();
        }

        //read tract identifier
        if (!stricmp(Source->GetValueAt(uLocationIndex),"all"))
          TractIndex = -1;
        else if (bRestrictedLocations) {
          throw resolvable_error("Error: When adjusting for known relative risks using the purely temporal analysis\n"
                                 "       without defining a coordinates file, it is not possible to adjust for known\n"
                                 "       relative risks at the location level. In order to adjust for known relative\n"
                                 "       risks at the location level, you must define a coordinates file. Alternatively,\n"
                                 "       you may want to define known relative risks that apply to all locations.");
        } else {
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
        if (!string_to_type<double>(Source->GetValueAt(uAdjustmentIndex), dRelativeRisk)) {
          gPrint.Printf("Error: Relative risk value '%s' in record %ld, of %s, is not a decimal number.\n",
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
		bEmpty = false;
        //perform adjustment
        iMaxTract = (TractIndex == -1 ? gDataHub.m_nTracts : TractIndex + 1);
        TractIndex = (TractIndex == -1 ? 0 : TractIndex);
        for (; TractIndex < iMaxTract; ++TractIndex)
           adjustments->add(TractIndex, dRelativeRisk, StartDate, EndDate);
    }
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bValid)
      gPrint.Printf("Please see the 'Adjustments File' section in the user guide for help.\n", BasePrint::P_ERROR);
    //print indication if file contained no data
    else if (bEmpty) {
      gPrint.Printf("Error: %s contains no data.\n", BasePrint::P_ERROR, gPrint.GetImpliedFileTypeString().c_str());
      bValid = false;
    } else {
        // safety check - remove any collections that have no adjustments
        for (RiskAdjustmentsContainer_t::iterator itr=rrAdjustments.begin(); itr != rrAdjustments.end(); ++itr) {
            if (itr->get()->get().size() == 0) itr = rrAdjustments.erase(itr);
        }
    }
  } catch (prg_exception& x) {
    x.addTrace("ReadAdjustmentsByRelativeRisksFile()","SaTScanDataReader");
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
  catch (prg_exception& x) {
    x.addTrace("ReadBernoulliData()","SaTScanDataReader");
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
bool SaTScanDataReader::ReadCartesianCoordinates(DataSource& Source, BasePrint& Print, std::vector<double>& vCoordinates, short& iScanCount, short iWordOffSet) {
  const char  * pCoordinate;
  int           i;

  for (i=0, iScanCount=0; i < (int)vCoordinates.size(); ++i, ++iWordOffSet)
     if ((pCoordinate = Source.GetValueAt(iWordOffSet)) != 0) {
       if (!string_to_type<double>(pCoordinate, vCoordinates[i])) {
         //unable to read word as double, print error to print direction and return false
		 Print.Printf("Error: Value '%s' of record %ld in %s could not be read as ",
                      BasePrint::P_READERROR, pCoordinate, Source.GetCurrentRecordIndex(), Print.GetImpliedFileTypeString().c_str());
         //we can be specific about which dimension we are attending to read to
         if (i < 2)
			Print.Printf("%s-coordinate.\n", BasePrint::P_READERROR, (i == 0 ? "x" : "y"));
         else if (vCoordinates.size() == 3)
			Print.Printf("z-coordinate.\n", BasePrint::P_READERROR);
         else
            Print.Printf("z%d-coordinate.\n", BasePrint::P_READERROR, i - 1);
         return false;
       } else {
         iScanCount++; //track num successful scans, caller of function wants this information
       }
     }
  return true;
}

/** Read the geographic data file. Calls particular function for coordinate type. */
bool SaTScanDataReader::ReadCoordinatesFile() {
  bool          bReturn;

  try {
      bool bNoPT_Coordinates = gParameters.GetIsPurelyTemporalAnalysis() &&
                               (!gParameters.UseAdjustmentForRelativeRisksFile() ||
                               (gParameters.UseAdjustmentForRelativeRisksFile() && gParameters.GetCoordinatesFileName().size() == 0));

    if (bNoPT_Coordinates) {
		gDataHub.m_nTracts = gTractHandler.getLocations().size();
		bReturn = true;
    }
    else if (gParameters.UseLocationNeighborsFile())
		bReturn = ReadUserSpecifiedNeighbors();
	else if (gParameters.getUseLocationsNetworkFile() && gParameters.getNetworkFilePurpose() == NETWORK_DEFINITION) {
		bReturn = ReadLocationNetworkFileAsDefinition();
	} else {
		gDataHub.gTractHandler->getMetaLocations().getMetaLocationPool().additionsCompleted(gTractHandler);
		gPrint.Printf("Reading the coordinates file\n", BasePrint::P_STDOUT);
		std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(gParameters.GetCoordinatesFileName(), gParameters.getInputSource(COORDFILE), gPrint));
		gPrint.SetImpliedInputFileType(BasePrint::COORDFILE);
		switch (gParameters.GetCoordinatesType()) {
			case CARTESIAN : bReturn = ReadCoordinatesFileAsCartesian(*Source); break;
			case LATLON    : bReturn = ReadCoordinatesFileAsLatitudeLongitude(*Source); break;
			default : throw prg_error("Unknown coordinate type '%d'.","ReadCoordinatesFile()",gParameters.GetCoordinatesType());
		};
		if (gParameters.getUseLocationsNetworkFile() && gParameters.getNetworkFilePurpose() == COORDINATES_OVERRIDE)
			bReturn = ReadLocationNetworkFileAsOverride();
    }
  } catch (prg_exception& x) {
    x.addTrace("ReadCoordinatesFile()","SaTScanDataReader");
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
  const long            uLocationIndex=0;
  bool                  bValid=true, bEmpty=true;
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
           //now that the number of dimensions is known, validate against requested ellipses
           if (iScanCount - 1 > 2 && gParameters.GetSpatialWindowType() == ELLIPTIC) {
             gPrint.Printf("Error: Only two dimensions can be specified when using the elliptic window shape. Record %ld has %d dimensions.\n", BasePrint::P_ERROR, Source.GetCurrentRecordIndex(), iScanCount - 1);
             bValid = false;
             break;
           }
           //ok, first record indicates that there are iScanCount - 1 dimensions (first scan is tract identifier)
           //data still could be invalid, but this will be determined like the remaining records
           gTractHandler.setCoordinateDimensions(iScanCount - 1);
           vCoordinates.resize(iScanCount - 1, 0);
         }
         //read and validate dimensions skip to next record if error reading coordinates as double
         if (! ReadCartesianCoordinates(Source, gPrint, vCoordinates, iScanCount, 1)) {
           bValid = false;
           continue;
         }
         //validate that we read the correct number of coordinates
         if (iScanCount < gTractHandler.getCoordinateDimensions()) {
           //Note: since the first record defined the number of dimensions, this error could not happen.
           gPrint.Printf("Error: Record %ld in the coordinates file contains %d dimension%s but the\n"
                         "       first record defined the number of dimensions as %d.\n", BasePrint::P_READERROR,
                         Source.GetCurrentRecordIndex(), iScanCount, (iScanCount == 1 ? "" : "s"), gTractHandler.getCoordinateDimensions());
           bValid = false;
           continue;
         }
         //add the tract identifier and coordinates to trac handler
         const char * identifier = Source.GetValueAt(uLocationIndex);
         if (!identifier) {
            gPrint.Printf("Error: Missing location id in record %ld of coordinates file.\n", BasePrint::P_ERROR, Source.GetCurrentRecordIndex());
            bValid = false;
            continue;
         }
         gTractHandler.addLocation(identifier, vCoordinates, gParameters.getUseLocationsNetworkFile() && gParameters.getNetworkFilePurpose() == NETWORK_DEFINITION);
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
    else if (gTractHandler.getLocations().size() == 1 && !gParameters.GetIsPurelyTemporalAnalysis()) {
      gPrint.Printf("Error: For a %s analysis, the coordinates file must contain more than one location.\n",
                    BasePrint::P_ERROR, ParametersPrint(gParameters).GetAnalysisTypeAsString());
      bValid = false;
    }
    //signal insertions completed
    gTractHandler.additionsCompleted();
    //record number of locations read
    gDataHub.m_nTracts = gTractHandler.getLocations().size();
    //record number of centroids read
    gDataHub.m_nGridTracts = gCentroidsHandler.getNumGridPoints();
  }
  catch (prg_exception& x) {
    x.addTrace("ReadCoordinatesFileAsCartesian()", "SaTScanDataReader");
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
  std::vector<double>   vCoordinates;
  const long            uLocationIndex=0;

  try {
    vCoordinates.resize(3/*for conversion*/, 0);
    gTractHandler.setCoordinateDimensions(3/*for conversion*/);
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source.ReadRecord()) {
        bEmpty=false;
        if (! ReadLatitudeLongitudeCoordinates(Source, gPrint, vCoordinates, 1, "coordinates")) {
           bValid = false;
           continue;
        }
        //add the tract identifier and coordinates to trac handler
        const char * identifier = Source.GetValueAt(uLocationIndex);
        if (!identifier) {
            gPrint.Printf("Error: Missing location id in record %ld of coordinates file.\n", BasePrint::P_ERROR, Source.GetCurrentRecordIndex());
            bValid = false;
            continue;
        }
        gTractHandler.addLocation(identifier, vCoordinates, gParameters.getUseLocationsNetworkFile() && gParameters.getNetworkFilePurpose() == NETWORK_DEFINITION);
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
    else if (gTractHandler.getLocations().size() == 1 && !gParameters.GetIsPurelyTemporalAnalysis()) {
      gPrint.Printf("Error: For a %s analysis, the coordinates file must contain more than one record.\n",
                    BasePrint::P_ERROR, ParametersPrint(gParameters).GetAnalysisTypeAsString());
      bValid = false;
    }

    //signal insertions completed
    gTractHandler.additionsCompleted();
    //record number of locations read
    gDataHub.m_nTracts = gTractHandler.getLocations().size();
    //record number of centroids read
    gDataHub.m_nGridTracts = gCentroidsHandler.getNumGridPoints();
  }
  catch (prg_exception& x) {
    x.addTrace("ReadCoordinatesFileAsLatitudeLongitude()", "SaTScanDataReader");
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
  catch (prg_exception& x) {
    x.addTrace("ReadExponentialData()","SaTScanDataReader");
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
    std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(gParameters.GetSpecialGridFileName(), gParameters.getInputSource(GRIDFILE), gPrint));
    switch (gParameters.GetCoordinatesType()) {
      case CARTESIAN : bReturn = ReadGridFileAsCartiesian(*Source); break;
      case LATLON    : bReturn = ReadGridFileAsLatitudeLongitude(*Source); break;
      default : throw prg_error("Unknown coordinate type '%d'.","ReadGrid()",gParameters.GetCoordinatesType());
    };
  }
  catch (prg_exception& x) {
    x.addTrace("ReadGridFile()", "SaTScanDataReader");
    throw;
  }
  return bReturn;
}

/** Read the special grid data file as Cartesian coordinates.
   If invalid data is found in the file, an error message is printed,
   that record is ignored, and reading continues.
   Return value: true = success, false = errors encountered          */
bool SaTScanDataReader::ReadGridFileAsCartiesian(DataSource& Source) {
  bool                          bValid=true, bEmpty=true, bwarned = false;
  short                         iScanCount;
  std::vector<double>           vCoordinates;
  CentroidHandler             * pGridPoints;

  try {
    if ((pGridPoints = dynamic_cast<CentroidHandler*>(&gCentroidsHandler)) == 0)
      throw prg_error("Not a CentroidHandler type.", "ReadGridFileAsCartiesian()");
    pGridPoints->setDimensions(gTractHandler.getCoordinateDimensions());
    vCoordinates.resize(gTractHandler.getCoordinateDimensions(), 0);
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source.ReadRecord()) {
        //there are records with data, but not necessarily valid
        bEmpty = false;
         //read and vaidate dimensions skip to next record if error reading coordinates as double
         if (! ReadCartesianCoordinates(Source, gPrint, vCoordinates, iScanCount, 0)) {
           bValid = false;
           continue;
         }
        //validate that we read the correct number of coordinates as defined by coordinates system or coordinates file
        if (iScanCount < gTractHandler.getCoordinateDimensions()) {
          gPrint.Printf("Error: Record %ld in the grid file contains %d dimension%s but the\n"
                        "       coordinates file defined the number of dimensions as %d.\n",
                        BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), iScanCount,
                        (iScanCount == 1 ? "" : "s"), gTractHandler.getCoordinateDimensions());
          bValid = false;
          continue;
        }
        GInfo::FocusInterval_t focusInterval;
        //check for focus interval dates - but only for analysis types that can implement this feature
        if (! ReadIntervalDates(Source, focusInterval, pGridPoints->getGridPointDimensions(), bwarned)) {
            bValid = false;
            continue;
        }

        pGridPoints->addGridPoint(vCoordinates, focusInterval);
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
    pGridPoints->additionsCompleted();
    //record number of centroids read
    gDataHub.m_nGridTracts = pGridPoints->getNumGridPoints();
  }
  catch (prg_exception& x) {
    x.addTrace("ReadGridFileAsCartiesian()","SaTScanDataReader");
    throw;
  }
  return bValid;
}

/** Reads interval ranges from data source starting at 'iSourceOffset'*/
bool SaTScanDataReader::ReadIntervalDates(DataSource& Source, GInfo::FocusInterval_t& focusInterval, short iSourceOffset, bool& warned) {
    bool                            bValid=true, hasValuesAtOffset=false;
    DateStringParser                DateParser(gParameters.GetPrecisionOfTimesType());
    DateStringParser::ParserStatus  eStatus;
    DatePrecisionType               ePrecision = gParameters.GetPrecisionOfTimesType();
    Julian                          JulianDate;

    try {
        // First detect if additional columns are present.
        focusInterval.first = Source.GetValueAt(iSourceOffset) != 0;
        if (!focusInterval.first)
            return bValid;

        // Focused grid points is only implemented for space-time analyses, ignore any focus dates otherwise.
        focusInterval.first = gParameters.GetAnalysisType() == SPACETIME;
        if (!focusInterval.first) {
            if (!warned)
                gPrint.Printf("Notice: In record %ld of %s, extra columns are present that are assumed to be dates.\n"
                              "        The focused grid point intervals are only implemented restrospective space-time analyses.\n"
                              "        The extra columns will be ignored for this analyse.\n",
                    BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
            warned = true;
            return bValid;
        }

        if (focusInterval.first) {
          //if there is one more value, then we expect 4 more: <startrange start>  <startrange end>  <endrange start>  <endrange end>
          for (int i=0; i < 4 /*expected number of dates*/; ++i) {
              if (!Source.GetValueAt(iSourceOffset)) {
                  gPrint.Printf("Error: Expecting date value in column %d, record %ld of %s.\n", BasePrint::P_READERROR, iSourceOffset + 1, Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
                  bValid = false;
                  continue;
              }
              //Attempt to convert string into Julian equivalence.
              eStatus = DateParser.ParseCountDateString(Source.GetValueAt(iSourceOffset), ePrecision, gDataHub.GetStudyPeriodStartDate(), gDataHub.GetStudyPeriodStartDate(), JulianDate);
              switch (eStatus) {
                  case DateStringParser::VALID_DATE       :
                      //validate that date is between study period start and end dates
                      if (!(gDataHub.GetStudyPeriodStartDate() <= JulianDate && JulianDate <= gDataHub.GetStudyPeriodEndDate())) {
                          gPrint.Printf("Error: The date '%s' in record %ld of the %s is not\n       within the study period beginning %s and ending %s.\n",
                                        BasePrint::P_READERROR, Source.GetValueAt(iSourceOffset), Source.GetCurrentRecordIndex(),
                                        gPrint.GetImpliedFileTypeString().c_str(), gParameters.GetStudyPeriodStartDate().c_str(), gParameters.GetStudyPeriodEndDate().c_str());
                          bValid = false;
                      } else {
                          if (i == 0) focusInterval.second.get<0>() = gDataHub.GetTimeIntervalOfDate(JulianDate);
                          else if (i == 1) focusInterval.second.get<1>() = gDataHub.GetTimeIntervalOfDate(JulianDate);
                          else if (i == 2) focusInterval.second.get<2>() = gDataHub.GetTimeIntervalOfEndDate(JulianDate);
                          else if (i == 3) focusInterval.second.get<3>() = gDataHub.GetTimeIntervalOfEndDate(JulianDate);
                      }
                      break;
                  case DateStringParser::LESSER_PRECISION : {
                      std::string sBuffer; //Dates in the case/control files must be at least as precise as ePrecision units.
                      gPrint.Printf("Error: The date '%s' of record %ld in the %s must be precise to %s,\n       as specified by time precision units.\n",
                                    BasePrint::P_READERROR, Source.GetValueAt(iSourceOffset), Source.GetCurrentRecordIndex(),
                                    gPrint.GetImpliedFileTypeString().c_str(), GetDatePrecisionAsString(ePrecision, sBuffer, false, false));
                      bValid = false;
                  }
                  case DateStringParser::AMBIGUOUS_YEAR   :
                  case DateStringParser::INVALID_DATE     :
                  default                                 :
                      gPrint.Printf("Error: Invalid date '%s' in the %s, record %ld.\n%s", BasePrint::P_READERROR,
                                      Source.GetValueAt(iSourceOffset), gPrint.GetImpliedFileTypeString().c_str(), Source.GetCurrentRecordIndex(), DateParser.getLastParseError().c_str());
                      bValid = false;
              };
              ++iSourceOffset;
          }
          if (bValid) {// now validate the dates do not conflict each other
              if (focusInterval.second.get<0>() > focusInterval.second.get<1>() ||
                  focusInterval.second.get<2>() > focusInterval.second.get<3>() ||
                  focusInterval.second.get<0>() > focusInterval.second.get<3>()) {
                  gPrint.Printf("Error: Centroid focus interval dates are conflicting in record %ld of %s.\n", BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
                  bValid = false;
              }
          }
          if (bValid) {// now validate the window evaluate clusters given maximum temporal cluster size.
              if (Source.GetValueAt(iSourceOffset)) {
                  gPrint.Printf("Error: Extra data found in column %d, record %ld of %s.\n", BasePrint::P_READERROR, iSourceOffset + 1, Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
                  bValid = false;
              }
          }
        }
    } catch (prg_exception& x) {
        x.addTrace("ReadIntervalDates()","SaTScanDataReader");
        throw;
    }
    return bValid;
}

/** Read the special grid data file as latitude/longitude coordinates.
   If invalid data is found in the file, an error message is printed,
   that record is ignored, and reading continues.
   Return value: true = success, false = errors encountered           */
bool SaTScanDataReader::ReadGridFileAsLatitudeLongitude(DataSource& Source) {
  bool    	                bValid=true, bEmpty=true, bwarned=false;
  std::vector<double>       vCoordinates;
  CentroidHandler         * pGridPoints;

  try {
    if ((pGridPoints = dynamic_cast<CentroidHandler*>(&gCentroidsHandler)) == 0)
      throw prg_error("Not a CentroidHandler type.", "ReadGridFileAsCartiesian()");
    pGridPoints->setDimensions(gTractHandler.getCoordinateDimensions());
    vCoordinates.resize(gTractHandler.getCoordinateDimensions(), 0);
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source.ReadRecord()) {
        //there are records with data, but not necessarily valid
        bEmpty=false;
        if (! ReadLatitudeLongitudeCoordinates(Source, gPrint, vCoordinates, 0, "grid")) {
           bValid = false;
           continue;
        }
        GInfo::FocusInterval_t focusInterval;
        //check for focus interval dates - but only for analysis types that can implement this feature
        if (! ReadIntervalDates(Source, focusInterval, pGridPoints->getGridPointDimensions() - 1, bwarned)) {
            bValid = false;
            continue;
        }
        pGridPoints->addGridPoint(vCoordinates, focusInterval);
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
    pGridPoints->additionsCompleted();
    //record number of centroids read
    gDataHub.m_nGridTracts = pGridPoints->getNumGridPoints();
  }
  catch (prg_exception& x) {
    x.addTrace("ReadGridFileAsLatitudeLongitude()","SaTScanDataReader");
    throw;
  }
  return bValid;
}

/** reads data from input files for a Homogeneous Poisson probability model */
bool SaTScanDataReader::ReadHomogeneousPoissonData() {
  try {
    gDataHub.gDataSets.reset(new HomogeneousPoissonDataSetHandler(gDataHub, gTractHandler, gCentroidsHandler, gPrint));
    gTractHandler.getMetaLocations().getMetaLocationPool().additionsCompleted(gTractHandler);
    if (!gDataHub.gDataSets->ReadData())
      return false;
    //record number of locations read
    gDataHub.m_nTracts = gTractHandler.getLocations().size();
    //record number of centroids read
    gDataHub.m_nGridTracts = gCentroidsHandler.getNumGridPoints();
  }
  catch (prg_exception& x) {
    x.addTrace("ReadPoissonData()","SaTScanDataReader");
    throw;
  }
  return true;
}

/** Reads latitude/longitude coordinates into vector.
    Note: coordinate vector should already be sized to 3 dimensions.
    Returns indication of whether words in passed string could be converted to
    coordinates. Checks that coordinates are in range and converts to cartesian
    coordinates. */
bool SaTScanDataReader::ReadLatitudeLongitudeCoordinates(DataSource& Source, BasePrint & Print, std::vector<double> & vCoordinates, short iWordOffSet, const char * sSourceFile) {
  const char  * pCoordinate;
  double        dLatitude, dLongitude;

  //read latitude, validating that string can be converted to double
  if ((pCoordinate = Source.GetValueAt(iWordOffSet)) != 0) {
    if (!string_to_type<double>(pCoordinate, dLatitude)) {
      Print.Printf("Error: The value '%s' of record %ld in the %s file could not be read as the latitude coordinate.\n",
                    BasePrint::P_READERROR, pCoordinate, Source.GetCurrentRecordIndex(), sSourceFile);
      return false;
    }
  }
  else {
    Print.Printf("Error: Record %d in the %s file is missing the latitude and longitude coordinates.\n",
                  BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), sSourceFile);
    return false;
  }
  //read longitude, validating that string can be converted to double
  if ((pCoordinate = Source.GetValueAt(++iWordOffSet)) != 0) {
    if (!string_to_type<double>(pCoordinate, dLongitude)) {
      Print.Printf("Error: The value '%s' of record %ld in the %s file could not be read as the longitude coordinate.\n",
                    BasePrint::P_READERROR, pCoordinate, Source.GetCurrentRecordIndex(), sSourceFile);
      return false;
    }
  }
  else {
    Print.Printf("Error: Record %ld in the %s file is missing the longitude coordinate.\n",
                  BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), sSourceFile);
    return false;
  }
  //validate range of latitude value
  if ((fabs(dLatitude) > 90.0)) {
    Print.Printf("Error: Latitude coordinate %lf, for record %ld in the %s file, is out of range.\n"
                  "       Latitude must be between -90 and 90.\n",
                  BasePrint::P_READERROR, dLatitude, Source.GetCurrentRecordIndex(), sSourceFile);
    return false;
  }
  //validate range of longitude value
  if ((fabs(dLongitude) > 180.0)) {
    Print.Printf("Error: Longitude coordinate %lf, for record %ld in the %s file, is out of range.\n"
                  "       Longitude must be between -180 and 180.\n",
                  BasePrint::P_READERROR, dLongitude, Source.GetCurrentRecordIndex(), sSourceFile);
    return false;
  }
  //convert to 3 dimensions then can add tracts
  ConvertFromLatLong(dLatitude, dLongitude, vCoordinates);
  return true;
}

bool SaTScanDataReader::ReadLocationNetworkFileAsDefinition() {
	bool          bValid = true, bEmpty = true, readCoordiantesFile = false;
	tract_t       TractIdentifierIndex, firstLocation, secondLocation;
	double        distanceBetween;
	Network     & locationNetwork(gDataHub.getLocationNetwork());

	try {

		gPrint.SetImpliedInputFileType(BasePrint::NETWORK_FILE);
		gPrint.Printf("Reading the locations network file\n", BasePrint::P_STDOUT);
		std::auto_ptr<DataSource> networkSource(DataSource::GetNewDataSourceObject(gParameters.getLocationsNetworkFilename(), gParameters.getInputSource(NETWORK_FILE), gPrint));
		// First pass - determine all location identifiers referenced in network file and define as tract in handler.
		while (!gPrint.GetMaximumReadErrorsPrinted() && networkSource->ReadRecord()) {
			if (!(networkSource->GetNumValues() >= 1 && networkSource->GetNumValues() <= 3)) {
				gPrint.Printf("Error: Record %ld of the %s contains %ld values but expecting either 1 , 2 or 3 values (<location id>, <location id>, <distance>).\n",
					BasePrint::P_READERROR, networkSource->GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str(), networkSource->GetNumValues());
				bValid = false;
				continue;
			}
			gTractHandler.addLocation(networkSource->GetValueAt(0));
			if (networkSource->GetNumValues() > 1)
				gTractHandler.addLocation(networkSource->GetValueAt(1));
			// If any record in the network file excludes distance value, then we need to read coordinates.
			readCoordiantesFile |= networkSource->GetNumValues() == 2;
		}
		// The network file does not specify the distances between all locations, so the coordinates file must be provided.
		if (readCoordiantesFile && gParameters.GetCoordinatesFileName().size() == 0) {
			gPrint.Printf(
				"Error: The locations network file references at least one connection without providing a distance.\n"
				"       All distances must be specified in network file or a coordinates file must be specified to calculate the distance between locations.\n",
				BasePrint::P_READERROR
			);
			return false;
		}
		if (readCoordiantesFile || gParameters.requestsGeogrphaicalOutput()) {
			// Now read the coordinates file - so we can have the location coordinates on hand when we need to calculate euclidean distances or write to geographical output files.
			gTractHandler.getMetaLocations().getMetaLocationPool().additionsCompleted(gTractHandler);
			gPrint.Printf("Reading the coordinates file to obtain location distances\n", BasePrint::P_STDOUT);
			std::auto_ptr<DataSource> coordinatesSource(DataSource::GetNewDataSourceObject(gParameters.GetCoordinatesFileName(), gParameters.getInputSource(COORDFILE), gPrint));
			gPrint.SetImpliedInputFileType(BasePrint::COORDFILE);
			switch (gParameters.GetCoordinatesType()) {
				case CARTESIAN:	bValid = ReadCoordinatesFileAsCartesian(*coordinatesSource); break;
				case LATLON: bValid = ReadCoordinatesFileAsLatitudeLongitude(*coordinatesSource); break;
				default: throw prg_error("Unknown coordinate type '%d'.", "ReadLocationNetworkFileAsDefinition()", gParameters.GetCoordinatesType());
			}
			// Since we're read the coordinates file, we can potnetially report location coordinates but we'll know for certain once we re-read the network file.
			gDataHub._network_can_report_coordinates = true;
		} else {
			// Didn't read coordinates file, definitely can't report location coordinates.
			gDataHub._network_can_report_coordinates = false;
            gDataHub.m_nTracts = gTractHandler.getLocations().size();
		}

		gPrint.SetImpliedInputFileType(BasePrint::NETWORK_FILE);
		if (bValid) {
			std::vector<double> coordinatesA, coordinatesB;
			networkSource->GotoFirstRecord();
			while (!gPrint.GetMaximumReadErrorsPrinted() && networkSource->ReadRecord()) {
				bEmpty = false;
				firstLocation = gTractHandler.getLocationIndex(networkSource->GetValueAt(0));
				if (networkSource->GetNumValues() == 1) {
					locationNetwork.addNode(firstLocation);
					secondLocation = -1;
				}
				else {
					secondLocation = gTractHandler.getLocationIndex(networkSource->GetValueAt(1));
					if (firstLocation == secondLocation) {
						gPrint.Printf("Error: Record %ld of %s defines location '%s' as a connection to itself.\n",
							BasePrint::P_READERROR, networkSource->GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str(), networkSource->GetValueAt(0));
						bValid = false;
						continue;
					}
					else if (networkSource->GetNumValues() == 3) {
						if (!string_to_type<double>(networkSource->GetValueAt(2), distanceBetween) || distanceBetween <= 0) {
							gPrint.Printf("Error: The distance between value '%s' in record %ld, of %s, is not a positive decimal number greater than zero.\n",
								BasePrint::P_READERROR, networkSource->GetValueAt(2), networkSource->GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
							bValid = false;
							continue;
						}
						if (gParameters.requestsGeogrphaicalOutput()) {
							// Although user specified the distance between locations, we still need to verifiy that the coordinates file define coordinates for both here.
							if (gTractHandler.getLocations()[firstLocation]->getCoordinates().size() == 0 || gTractHandler.getLocations()[secondLocation]->getCoordinates().size() == 0) {
								gPrint.Printf(
									"Warning: The network file identified a location in record %ld which has no coordinates defined in the coordinates file.\n"
									"         Coordinates are required to place locations geograghically. Geographical output files will not be created in this analysis.\n",
									BasePrint::P_WARNING, networkSource->GetCurrentRecordIndex()
								);
								const_cast<CParameters&>(gParameters).toggleGeogrphaicalOutput(false);
							}
						}
					} else {
						// Calculate distance between locations and use that value in connection - taking into account multiple coordinates.
						if (gTractHandler.getLocations()[firstLocation]->getCoordinates().size() == 0 || gTractHandler.getLocations()[secondLocation]->getCoordinates().size() == 0) {
							gPrint.Printf(
								"Error: A distance could not be calculated between locations '%s' and '%s' in the network file, record %ld.\n"
								"       One or both of these locations does not have coordinates defined in the coordinates file.\n",
								BasePrint::P_READERROR, networkSource->GetValueAt(0), networkSource->GetValueAt(1), networkSource->GetCurrentRecordIndex()
							);
							bValid = false;
							continue;
						}
						gTractHandler.getLocations()[firstLocation]->getCoordinates()[0]->retrieve(coordinatesA);
                        gTractHandler.getLocations()[secondLocation]->getCoordinates()[0]->retrieve(coordinatesB);
					    distanceBetween = std::sqrt(gTractHandler.getDistanceSquared(coordinatesA, coordinatesB));
					}
                    NetworkNode::AddStatusType addStatus = locationNetwork.addConnection(firstLocation, secondLocation, distanceBetween, true);
					if (addStatus >= NetworkNode::AddStatusType::SelfReference) {
						gPrint.Printf(
                            "Error: An issue was found when adding connection between location '%s' and location '%s' in record %ld of %s: %s.\n",
							BasePrint::P_READERROR, networkSource->GetValueAt(0), networkSource->GetValueAt(1), networkSource->GetCurrentRecordIndex(), 
                            gPrint.GetImpliedFileTypeString().c_str(), NetworkNode::getStatusMessage(addStatus)
                        );
						bValid = false;
						continue;
					}
                    addStatus = locationNetwork.addConnection(secondLocation, firstLocation, distanceBetween, false);
                    if (addStatus >= NetworkNode::AddStatusType::SelfReference) {
                        gPrint.Printf(
                            "Error: An issue was found when adding connection between location '%s' and location '%s' in record %ld of %s: %s.\n",
                            BasePrint::P_READERROR, networkSource->GetValueAt(1), networkSource->GetValueAt(0), networkSource->GetCurrentRecordIndex(),
                            gPrint.GetImpliedFileTypeString().c_str(), NetworkNode::getStatusMessage(addStatus)
                        );
                        bValid = false;
                        continue;
                    }
                }
				// Determine whether this record indicates if we can report coordinates in main output and geographical output files.
				gDataHub._network_can_report_coordinates &= gTractHandler.getLocations()[firstLocation]->getCoordinates().size() > 0 && 
					(secondLocation == -1 || gTractHandler.getLocations()[secondLocation]->getCoordinates().size() > 0);
				if (!gDataHub._network_can_report_coordinates && gParameters.requestsGeogrphaicalOutput()) {
					if (gTractHandler.getLocations()[firstLocation]->getCoordinates().size() == 0 || (secondLocation >= 0 && gTractHandler.getLocations()[secondLocation]->getCoordinates().size() == 0)) {
						gPrint.Printf(
							"Warning: The network file identified a location in record %ld which has no coordinates defined in the coordinates file.\n"
							"         Coordinates are required to place locations geograghically. Geographical output files will not be created in this analysis.\n",
							BasePrint::P_WARNING, networkSource->GetCurrentRecordIndex()
						);
						const_cast<CParameters&>(gParameters).toggleGeogrphaicalOutput(false);
					}
				}
			}
			gDataHub.m_nGridTracts = gDataHub.m_nTracts;
		}
		
	} catch (prg_exception& x) {
		x.addTrace("ReadLocationNetworkFileAsDefinition()", "SaTScanDataReader");
		throw;
	}
	return bValid;
}


bool SaTScanDataReader::ReadLocationNetworkFileAsOverride() {
	bool          bValid = true, bEmpty = true;
	tract_t       TractIdentifierIndex, firstLocation, secondLocation;
	double        distanceBetween;
	Network     & locationNetwork(gDataHub.getLocationNetwork());

	try {

		gPrint.SetImpliedInputFileType(BasePrint::NETWORK_FILE);
		gPrint.Printf("Reading the locations network file\n", BasePrint::P_STDOUT);
		std::auto_ptr<DataSource> networkSource(DataSource::GetNewDataSourceObject(gParameters.getLocationsNetworkFilename(), gParameters.getInputSource(NETWORK_FILE), gPrint));

		while (!gPrint.GetMaximumReadErrorsPrinted() && networkSource->ReadRecord()) {
			bEmpty = false;
			if (networkSource->GetNumValues() != 3) {
				gPrint.Printf("Error: Record %ld of the %s contains %ld values but expecting 3 values (<location id>, <location id>, <distance>).\n",
					BasePrint::P_READERROR, networkSource->GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str(), networkSource->GetNumValues());
				bValid = false;
				continue;
			}
			firstLocation = gTractHandler.getLocationIndex(networkSource->GetValueAt(0));
			if (firstLocation == -1) {
				gPrint.Printf("Error: Record %ld, of %s, references an unknown identifier '%s' that is not defined in the coordinates file..\n",
					BasePrint::P_READERROR, networkSource->GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str(), networkSource->GetValueAt(0));
				bValid = false;
				continue;
			}
			secondLocation = gTractHandler.getLocationIndex(networkSource->GetValueAt(1));
			if (secondLocation == -1) {
				gPrint.Printf("Error: Record %ld, of %s, references an unknown identifier '%s' that is not defined in the coordinates file..\n",
					BasePrint::P_READERROR, networkSource->GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str(), networkSource->GetValueAt(1));
				bValid = false;
				continue;
			}
			if (!string_to_type<double>(networkSource->GetValueAt(2), distanceBetween) || distanceBetween < 0) {
				gPrint.Printf("Error: The distance between value '%s' in record %ld, of %s, is not a positive decimal number.\n",
					BasePrint::P_READERROR, networkSource->GetValueAt(2), networkSource->GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
				bValid = false;
				continue;
			}
			if (!gTractHandler.addLocationsDistanceOverride(firstLocation, secondLocation, distanceBetween)) {
				gPrint.Printf("Error: The distance %s between node '%s' and '%s', in record %ld of %s, conflicts with a previous record.\n",
					BasePrint::P_READERROR, networkSource->GetValueAt(2), networkSource->GetValueAt(0), networkSource->GetValueAt(1), networkSource->GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
				bValid = false;
				continue;
			}
		}
	} catch (prg_exception& x) {
		x.addTrace("ReadLocationNetworkFileAsOverride()", "SaTScanDataReader");
		throw;
	}
	return bValid;
}

/** Read the special population that will be used to construct circles
    about grid points(centroids).                                      */
bool SaTScanDataReader::ReadMaxCirclePopulationFile() {
  bool          bValid=true, bEmpty=true;
  tract_t       TractIdentifierIndex;
  float         fPopulation;
  const long    uPopulationIndex=1;

  try {
    gPrint.SetImpliedInputFileType(BasePrint::MAXCIRCLEPOPFILE);

    gPrint.Printf("Reading the max circle size file\n", BasePrint::P_STDOUT);
    std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(gParameters.GetMaxCirclePopulationFileName(), gParameters.getInputSource(MAXCIRCLEPOPFILE), gPrint));
    //initialize circle-measure array
    gDataHub.gvMaxCirclePopulation.resize(gDataHub.m_nTracts + gDataHub.GetTInfo()->getMetaLocations().getNumReferencedLocations(), 0);

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
        if (!string_to_type<float>(Source->GetValueAt(uPopulationIndex), fPopulation) || fPopulation < 0) {
          gPrint.Printf("Error: The population value '%s' in record %ld, of %s, is not a positive decimal number.\n",
                        BasePrint::P_READERROR, Source->GetValueAt(uPopulationIndex), Source->GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
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
    //set meta locations
    std::vector<tract_t> atomicIndexes;
    for (size_t t=0; t < gDataHub.GetTInfo()->getMetaLocations().getNumReferencedLocations(); ++t) {
       gDataHub.GetTInfo()->getMetaLocations().getAtomicIndexes(t, atomicIndexes);
       for (size_t a=0; a < atomicIndexes.size(); ++a)
         gDataHub.gvMaxCirclePopulation[(size_t)gDataHub.m_nTracts + t] += gDataHub.gvMaxCirclePopulation[atomicIndexes[a]];
    }
  }
  catch (prg_exception& x) {
    x.addTrace("ReadMaxCirclePopulationFile()","SaTScanDataReader");
    throw;
  }
  return bValid;
}

/** Opens meta locations file data source and parses meta location definitions. */
bool SaTScanDataReader::ReadMetaLocationsFile() {
  bool                  bValid=true, bEmpty=true, bStructuredMetaData;
  std::string           sIdentifier, sLocations;
  long                  uLocation0ffset;

  try {
    if (!gParameters.UseMetaLocationsFile()) {
      gTractHandler.getMetaLocations().getMetaLocationPool().additionsCompleted(gTractHandler);
      return true;
    }
    gPrint.Printf("Reading the meta locations file\n", BasePrint::P_STDOUT);
    gPrint.SetImpliedInputFileType(BasePrint::META_LOCATIONS_FILE);
    //determine format of file, either:
    // meta1=loc1,loc2, loc3
    // meta1 loc1 loc2 loc3
    std::ifstream SourceFile(gParameters.getMetaLocationsFilename().c_str());
    getlinePortable(SourceFile, sIdentifier);
    bStructuredMetaData = (sIdentifier.find("=") == sIdentifier.npos ? false : true);
    SourceFile.close();
    AsciiFileDataSource Source(gParameters.getMetaLocationsFilename().c_str(), gPrint, (bStructuredMetaData ? '=' : ' '));

    //first pass on neighbors file to determine all location identifiers referenced
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source.ReadRecord()) {
      bEmpty = false;
      sIdentifier = Source.GetValueAt(0);
      trimString(sIdentifier);
      if (!Source.GetValueAt(1)) {
        gPrint.Printf("Error: Incorrectly defined meta location at record %d of the %s.\n",
                      BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
        bValid = false;
        continue;
      }
      if (bStructuredMetaData)
        sLocations = Source.GetValueAt(1);
      else {
        uLocation0ffset=1;
        sLocations.clear();
        while (Source.GetValueAt(uLocation0ffset)) {
           if (uLocation0ffset > 1) sLocations += ",";
           sLocations += Source.GetValueAt(uLocation0ffset);
           ++uLocation0ffset;
        }
      }
      if(!gTractHandler.getMetaLocations().getMetaLocationPool().addMetaLocation(sIdentifier, sLocations)) {
        gPrint.Printf("Error: Incorrectly defined meta location at record %d of the %s.\n",
                      BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
        bValid = false;
        continue;
      }
    }
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bValid)
      gPrint.Printf("Please see the 'Meta Locations File' section in the user guide for help.\n", BasePrint::P_READERROR);
    //print indication if file contained no data
    else if (bEmpty) {
      gPrint.Printf("Error: %s contains no data.\n", BasePrint::P_ERROR, gPrint.GetImpliedFileTypeString().c_str());
      bValid = false;
    }
    gTractHandler.getMetaLocations().getMetaLocationPool().additionsCompleted(gTractHandler);
  }
  catch (prg_exception& x) {
    x.addTrace("ReadMetaLocationsFile()","SaTScanDataReader");
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
  catch (prg_exception& x) {
    x.addTrace("ReadNormalData()","SaTScanDataReader");
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
  catch (prg_exception& x) {
    x.addTrace("ReadOrdinalData()","SaTScanDataReader");
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
        // Read the adjustments for known relative risks if requested and we're not performing power evaluation without reading the casefile.
        // This adjustment requires that the case data structures are defined.
        if (gParameters.UseAdjustmentForRelativeRisksFile() && !(gParameters.getPerformPowerEvaluation() && gParameters.getPowerEvaluationMethod() == PE_ONLY_SPECIFIED_CASES)) {
            RiskAdjustmentsContainer_t riskAdjustments;
            if (!ReadAdjustmentsByRelativeRisksFile(gParameters.GetAdjustmentsByRelativeRisksFilename(), riskAdjustments, true))
                return false;
            gDataHub.gRelativeRiskAdjustments = riskAdjustments.front();
        }
        if (gParameters.UseMaxCirclePopulationFile() && !ReadMaxCirclePopulationFile())
            return false;
        if (gParameters.UseSpecialGrid() && !ReadGridFile())
        return false;
    } catch (prg_exception& x) {
        x.addTrace("ReadPoissonData()","SaTScanDataReader");
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
  catch (prg_exception& x) {
    x.addTrace("ReadRankData()","SaTScanDataReader");
    throw;
  }
  return true;
}

/** reads data from input files for a Uniform Time probability model */
bool SaTScanDataReader::ReadUniformTimeData() {
    try {
        if (!ReadCoordinatesFile())
            return false;
        gDataHub.gDataSets.reset(new UniformTimeDataSetHandler(gDataHub, gPrint));
        if (!gDataHub.gDataSets->ReadData())
            return false;
        if (gParameters.UseMaxCirclePopulationFile() && !ReadMaxCirclePopulationFile())
            return false;
        if (gParameters.UseSpecialGrid() && !ReadGridFile())
            return false;
    }
    catch (prg_exception& x) {
        x.addTrace("ReadUniformTimeData()", "SaTScanDataReader");
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
  catch (prg_exception& x) {
    x.addTrace("ReadSpaceTimePermutationData()","SaTScanDataReader");
    throw;
  }
  return true;
}

/** Opens neighbors file data source and parses a list of location identifiers from each
    record. These lists represent the geographical extension of a network of neighboring locations,
    centralized at the first location identifier in list. This process is alternative to the process
    of calculating neighbors about centroids through expanding circles. The lists read from file are
    stored in the same sorted array structure as expanding circle process. Errors encountered during
    the read process are printed to BasePrint object and reading continues. Returns true if read completes
    without detecting errors, else returns false. */
bool SaTScanDataReader::ReadUserSpecifiedNeighbors() {
  bool                  bValid=true, bEmpty=true;
  long                  uLocation0ffset;
  tract_t               tLocationIndex;
  std::vector<tract_t> vRecordNeighborList, AtomicIndexes;

  try {
    if (!ReadMetaLocationsFile()) return false;
    gPrint.Printf("Reading the neighbors file\n", BasePrint::P_STDOUT);
    gPrint.SetImpliedInputFileType(BasePrint::LOCATION_NEIGHBORS_FILE);
    gTractHandler.setCoordinateDimensions(0);
    std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(gParameters.GetLocationNeighborsFileName().c_str(), gParameters.getInputSource(LOCATION_NEIGHBORS_FILE), gPrint));
    //first pass on neighbors file to determine all location identifiers referenced
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source->ReadRecord()) {
      uLocation0ffset=0;
      gDataHub.m_nGridTracts++;
      while (Source->GetValueAt(uLocation0ffset)) {
        //insert location identifier into tract handler class
        gTractHandler.addLocation(Source->GetValueAt(uLocation0ffset));
        ++uLocation0ffset;
        bEmpty = false;
      }
    }
    //second pass - allocate respective sorted array
    if (!bEmpty) {
      //record number of locations read
      gTractHandler.additionsCompleted(gParameters.GetOutputRelativeRisksFiles());
      gDataHub.m_nTracts = gTractHandler.getLocations().size();
      boost::dynamic_bitset<> NeighborsSet(gDataHub.m_nTracts);
      gDataHub.AllocateSortedArray();
      Source->GotoFirstRecord();
      while (!gPrint.GetMaximumReadErrorsPrinted() && Source->ReadRecord()) {
        uLocation0ffset=0;
        vRecordNeighborList.clear();
        NeighborsSet.reset();
        while (Source->GetValueAt(uLocation0ffset)) {
          //find location identifer internal index
          const char * identifier = Source->GetValueAt(uLocation0ffset);
          if (!identifier) {
              bValid = false;
              gPrint.Printf("Error: Location ID is missing in record %ld of %s.\n", BasePrint::P_READERROR,
                            Source->GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
              continue;
          }
          if ((tLocationIndex = gTractHandler.getLocationIndex(identifier)) > -1) {
            if (NeighborsSet.test(tLocationIndex)) {
              bValid = false;
              gPrint.Printf("Error: Location ID '%s' occurs multiple times in record %ld of %s.\n", BasePrint::P_READERROR,
                            Source->GetValueAt(uLocation0ffset), Source->GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
            }
            else {
              NeighborsSet.set(tLocationIndex);
              vRecordNeighborList.push_back(tLocationIndex);
            }
          }
          else {
            tLocationIndex = gTractHandler.getMetaLocations().getMetaLocationIndex(identifier);
            gTractHandler.getMetaLocations().getAtomicIndexes(tLocationIndex, AtomicIndexes);
            for (size_t t=0; t < AtomicIndexes.size(); ++t) {
              if (NeighborsSet.test(AtomicIndexes[t])) {
                bValid = false;
                gPrint.Printf("Error: Location ID '%s' occurs multiple times in record %ld of %s.\n", BasePrint::P_READERROR,
                              gTractHandler.getLocations()[AtomicIndexes[t]]->getIndentifier(), Source->GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
              }
              NeighborsSet.set(AtomicIndexes[t]);
            }
            vRecordNeighborList.push_back(tLocationIndex + gDataHub.m_nTracts);
          }
          ++uLocation0ffset;
          bEmpty = false;
        }
        if (bValid)
          //add created tract identifer(record number) and read coordinates to structure that mantains list of centroids
          gDataHub.AllocateSortedArrayNeighbors(Source->getNonBlankRecordsRead() - 1, vRecordNeighborList);
      }
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
  catch (prg_exception& x) {
    x.addTrace("ReadUserSpecifiedNeighbors()","SaTScanDataReader");
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
    const char * identifier = Source.GetValueAt(guLocationIndex);
    if (!identifier) {
       gPrint.Printf("Error: Location ID is missing in record %ld of %s.\n", BasePrint::P_READERROR,
                     Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
       return SaTScanDataReader::Rejected;
    }
    if ((tLocationIndex = gDataHub.GetTInfo()->getLocationIndex(identifier)) == -1) {
        if (gParameters.GetCoordinatesDataCheckingType() == STRICTCOORDINATES) {
            gPrint.Printf("Error: Unknown location ID in %s, record %ld. '%s' not specified in the %s file.\n", BasePrint::P_READERROR,
                          gPrint.GetImpliedFileTypeString().c_str(), Source.GetCurrentRecordIndex(), Source.GetValueAt(guLocationIndex),
                          (gParameters.UseLocationNeighborsFile() ? "neighbors" : "coordinates"));
            return SaTScanDataReader::Rejected;
        }
        // Report to user if the data checking option is ignoring locations - because the user requested relaxed checking, that is unless
        // this is a drilldown where we set this option programmatically.
        if (!gDataHub.isDrilldown() && std::find(gmSourceLocationWarned.begin(), gmSourceLocationWarned.end(), reinterpret_cast<void*>(&Source)) == gmSourceLocationWarned.end()) {
            gPrint.Printf("Warning: Some records in %s reference a location ID that was not specified in the %s file. "
                          "These are ignored in the analysis.\n", BasePrint::P_WARNING, gPrint.GetImpliedFileTypeString().c_str(),
                          (gParameters.UseLocationNeighborsFile() ? "neighbors" : "coordinates"));
            gmSourceLocationWarned.push_back(reinterpret_cast<void*>(&Source));
        }
        return SaTScanDataReader::Ignored;
    }
    return SaTScanDataReader::Accepted;
}

