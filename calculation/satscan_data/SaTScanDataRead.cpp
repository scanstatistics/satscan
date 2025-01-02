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
#include "BatchedDataSetHandler.h"
#include "Toolkit.h"

const long SaTScanDataReader::_identifier_column_index = 0;

/** class constructor */
SaTScanDataReader::SaTScanDataReader(CSaTScanData& DataHub):
    gDataHub(DataHub), gParameters(gDataHub.GetParameters()), gPrint(gDataHub.gPrint),
    _identifier_mgr(*DataHub._identifiers_manager), gCentroidsHandler(*gDataHub.gCentroidsHandler.get()) {}


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
      case BATCHED              : bReadSuccess = ReadBatchedData(); break;
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
    std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(
        getFilenameFormatTime(filename, gParameters.getTimestamp(), true), gParameters.getInputSource(ADJ_BY_RR_FILE), gPrint)
    );
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
          SaTScanDataReader::RecordStatusType eStatus = RetrieveIdentifierIndex(*Source, TractIndex);
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
        iMaxTract = (TractIndex == -1 ? gDataHub._num_identifiers : TractIndex + 1);
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
      gPrint.Printf("Warning: %s contains no data.\n", BasePrint::P_WARNING, gPrint.GetImpliedFileTypeString().c_str());
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
    bool bReturn=true;
    try {
        bool bNoPT_Coordinates = (
            gParameters.GetIsPurelyTemporalAnalysis() && 
            (!gParameters.UseAdjustmentForRelativeRisksFile() || (gParameters.UseAdjustmentForRelativeRisksFile() && gParameters.GetCoordinatesFileName().size() == 0))
        );

        if (bNoPT_Coordinates) {
            gDataHub._num_identifiers = _identifier_mgr.getIdentifiers().size();
        } else if (gParameters.UseLocationNeighborsFile())
            bReturn = ReadUserSpecifiedNeighbors();
        else if (gParameters.getUseLocationsNetworkFile()) {
            _identifier_mgr.getMetaIdentifiersManager().getMetaPool().additionsCompleted(_identifier_mgr);
            bReturn = ReadLocationNetworkFileAsDefinition();
        } else {
		    _identifier_mgr.getMetaIdentifiersManager().getMetaPool().additionsCompleted(_identifier_mgr);
		    gPrint.Printf("Reading the coordinates file\n", BasePrint::P_STDOUT);
		    gPrint.SetImpliedInputFileType(BasePrint::COORDFILE);
		    std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(
                getFilenameFormatTime(gParameters.GetCoordinatesFileName(), gParameters.getTimestamp(), true),
                gParameters.getInputSource(COORDFILE), gPrint)
            );
            switch (gParameters.GetCoordinatesType()) {
                case CARTESIAN : bReturn = ReadCoordinatesFileAsCartesian(*Source, gParameters.GetMultipleCoordinatesType() == ONEPERLOCATION); break;
                case LATLON    : bReturn = ReadCoordinatesFileAsLatitudeLongitude(*Source, gParameters.GetMultipleCoordinatesType() == ONEPERLOCATION); break;
                default : throw prg_error("Unknown coordinate type '%d'.","ReadCoordinatesFile()",gParameters.GetCoordinatesType());
            };
        }

        if (bReturn && gParameters.GetMultipleCoordinatesType() != ONEPERLOCATION) {
            /* If the user have selected for multiple coordinates per identifier, then read the multiple locations file. */
            gPrint.SetImpliedInputFileType(BasePrint::MULTIPLE_LOCATIONS);
            gPrint.Printf("Reading the multiple locations per observation file\n", BasePrint::P_STDOUT);
            std::auto_ptr<DataSource> MLSource(DataSource::GetNewDataSourceObject(
                getFilenameFormatTime(gParameters.getMultipleLocationsFile(), gParameters.getTimestamp(), true),
                gParameters.getInputSource(MULTIPLE_LOCATIONS_FILE), gPrint)
            );
            bool bEmpty = true;
            while (!gPrint.GetMaximumReadErrorsPrinted() && MLSource->ReadRecord()) {
                bEmpty = false;
                //determine number of dimensions from first record, 2 or more is valid
                int iScanCount = MLSource->GetNumValues();
                //there must be at least two dimensions
                if (iScanCount != 2) {
                    gPrint.Printf(
                        "Error: The records in the multiple coordinates file contains %s value%s, expecting 2.\n",
                        BasePrint::P_ERROR, iScanCount, iScanCount == 1 ? "" : "s"
                    );
                    bReturn = false;
                    break; //stop reading records, the first record defines remaining records format
                }
                const char * name = MLSource->GetValueAt(0);
                const char * locationname = MLSource->GetValueAt(1);
                switch (_identifier_mgr.addIdentifier(name, locationname)) {
                    case IdentifiersManager::UnknownLocation:
                        gPrint.Printf(
                            "Error: Record %ld of the multiple coordinates file references an unknown location.\n"
                            "       '%s' is not defined in the %s file.\n", BasePrint::P_READERROR,
                            MLSource->GetCurrentRecordIndex(), locationname, gParameters.getUseLocationsNetworkFile() ? "network" : "coordinates"
                        );
                        bReturn = false;
                        break;
                    case IdentifiersManager::MultipleLocations:
                    case IdentifiersManager::Accepted:
                    default: 
                        // If we're using the multiple locations file with the network file, warn user if location is not in network.
                        if (gParameters.getUseLocationsNetworkFile() && !gDataHub.getLocationNetwork().locationIndexInNetwork(_identifier_mgr.getLocationsManager().getLocation(locationname).first.get())) {
                            gPrint.Printf(
                                "Warning: Record %ld of the multiple coordinates file references location '%s', which is not defined in the network file.\n",
                                BasePrint::P_WARNING, MLSource->GetCurrentRecordIndex(), locationname
                            );
                        }
                        break;
                }
            }
            //signal insertions completed
            _identifier_mgr.additionsCompleted();
            //record number of locations read
            gDataHub._num_identifiers = _identifier_mgr.getIdentifiers().size();
            //record number of centroids read
            gDataHub.m_nGridTracts = gCentroidsHandler.getNumGridPoints();
        }
    } catch (prg_exception& x) {
        x.addTrace("ReadCoordinatesFile()","SaTScanDataReader");
        throw;
    }
    return bReturn;
}

/** Read the geographic data file in Cartesian coordinate system.
    If invalid data is found in the file, an error message is printed, that record is ignored, and reading continues.
    Return value: true = success, false = errors encountered           */
bool SaTScanDataReader::ReadCoordinatesFileAsCartesian(DataSource& Source, bool closeAdditions) {
    short iScanCount=0;
    const long uLocationIndex=0;
    bool bValid=true, bEmpty=true;
    std::vector<double> coordinates;

    try {
        while (!gPrint.GetMaximumReadErrorsPrinted() && Source.ReadRecord()) {
            //if empty and this record has data, then this is the first record w/ data
            if (bEmpty) {
                bEmpty = false;
                //determine number of dimensions from first record, 2 or more is valid
                iScanCount = Source.GetNumValues();
                //there must be at least two dimensions
                if (iScanCount < 3) {
                    gPrint.Printf(
                        "Error: The first record of the coordinates file contains %s.\n",
                        BasePrint::P_READERROR, iScanCount == 2 ? "only x-coordinate" : "no coordinates"
                    );
                    bValid = false;
                    break; //stop reading records, the first record defines remaining records format
                }
                //now that the number of dimensions is known, validate against requested ellipses
                if (iScanCount - 1 > 2 && gParameters.GetSpatialWindowType() == ELLIPTIC) {
                    gPrint.Printf(
                        "Error: Only two dimensions can be specified when using the elliptic window shape. Record %ld has %d dimensions.\n",
                        BasePrint::P_ERROR, Source.GetCurrentRecordIndex(), iScanCount - 1
                    );
                    bValid = false;
                    break;
                }
                //ok, first record indicates that there are iScanCount - 1 dimensions (first scan is tract identifier)
                //data still could be invalid, but this will be determined like the remaining records
		        _identifier_mgr.setExpectedCoordinateDimensions(iScanCount - 1);
		        coordinates.resize(iScanCount - 1, 0);
            }
            //read and validate dimensions skip to next record if error reading coordinates as double
            if (!ReadCartesianCoordinates(Source, gPrint, coordinates, iScanCount, 1)) {
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
            if (!_identifier_mgr.aggregatingIdentifiers() && coordinates.size() != _identifier_mgr.getLocationsManager().expectedDimensions()) {
                gPrint.Printf(
                    "Error: Record %ld in the coordinates file defines coordinates with %u dimension but expecting %u.\n",
                    BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), coordinates.size(), _identifier_mgr.getLocationsManager().expectedDimensions()
                );
                bValid = false;
                continue;
            }
            if (gParameters.getUseLocationsNetworkFile()) {
                // When using the network file, we've already defined the locations of interest but w/o coordinates. So when 
                // reading the coordinates file, we are only interested in obtaining the coordinates of the location in the
                // network, any other locations in the coordinates file should not be added to our collection of locations.
                switch (_identifier_mgr.setLocationCoordinates(identifier, coordinates)) {
                    case LocationsManager::CoordinateExists:
                        gPrint.Printf(
                            "Error: Record %ld in the coordinates file is attempting to define the same coordinates to more than one location.\n",
                            BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), coordinates.size(), _identifier_mgr.getLocationsManager().expectedDimensions()
                        );
                        bValid = false;
                        break;
                    case LocationsManager::CoordinateRedefinition:
                        gPrint.Printf(
                            "Error: Record %ld in the coordinates file is attempting to redefine the coordinates for '%s'.\n",
                            BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), identifier
                        );
                        bValid = false;
                        break;
                }
                continue;
            }
            switch (_identifier_mgr.addLocation(identifier, coordinates)) {
                case LocationsManager::CoordinateExists:
                    gPrint.Printf(
                        "Error: Record %ld in the coordinates file is attempting to define the same coordinates to more than one location.\n",
                        BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), coordinates.size(), _identifier_mgr.getLocationsManager().expectedDimensions()
                    );
                    bValid = false;
                    break;
                case LocationsManager::WrongDimensions:
                    gPrint.Printf(
                        "Error: Record %ld in the coordinates file defines coordinates with %u dimension but expecting %u.\n",
                        BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), coordinates.size(), _identifier_mgr.getLocationsManager().expectedDimensions()
                    );
                    bValid = false;
                    break;
                case LocationsManager::CoordinateRedefinition:
                    gPrint.Printf(
                        "Error: Record %ld in the coordinates file is attempting to redefine the coordinates for '%s'.\n",
                        BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), identifier
                    );
                    bValid = false;
                    break;
                case LocationsManager::NameExists:
                case LocationsManager::Duplicate:
                case LocationsManager::Accepted:
                default: break;
            }
        }
        //if invalid at this point then read encountered problems with data format, inform user of section to refer to in user guide for assistance
        if (! bValid) gPrint.Printf("Please see the 'Coordinate File' section in the user guide for help.\n", BasePrint::P_ERROR);
        else if (bEmpty) { //print indication if file contained no data
            gPrint.Printf("Error: The coordinates file contains no data.\n", BasePrint::P_ERROR);
            bValid = false;
        } else if (_identifier_mgr.getIdentifiers().size() == 1 && !gParameters.GetIsPurelyTemporalAnalysis()) {
            //validate that we have more than one tract, only a purely temporal analysis is the exception to this rule
            gPrint.Printf(
                "Error: For a %s analysis, the coordinates file must contain more than one location.\n",
                BasePrint::P_ERROR, ParametersPrint(gParameters).GetAnalysisTypeAsString()
            );
            bValid = false;
        }
	    if (closeAdditions) {
		    //signal insertions completed
		    _identifier_mgr.additionsCompleted();
		    //record number of locations read
		    gDataHub._num_identifiers = _identifier_mgr.getIdentifiers().size();
		    //record number of centroids read
		    gDataHub.m_nGridTracts = gCentroidsHandler.getNumGridPoints();
	    }
    } catch (prg_exception& x) {
        x.addTrace("ReadCoordinatesFileAsCartesian()", "SaTScanDataReader");
        throw;
    }
    return bValid;
}

/** Read the latitude/longitude geographic data file.
    If invalid data is found in the file, an error message is printed, that record is ignored, and reading continues.
    Return value: true = success, false = errors encountered   */
bool SaTScanDataReader::ReadCoordinatesFileAsLatitudeLongitude(DataSource& Source, bool closeAdditions) {
    bool bValid=true, bEmpty=true;
    std::vector<double> coordinates;
    const long uLocationIndex=0;

    try {
	    coordinates.resize(3/*for conversion*/, 0);
	    _identifier_mgr.setExpectedCoordinateDimensions(3/*for conversion*/);
        while (!gPrint.GetMaximumReadErrorsPrinted() && Source.ReadRecord()) {
            bEmpty=false;
            if (!ReadLatitudeLongitudeCoordinates(Source, gPrint, coordinates, 1, "coordinates")) {
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
            if (gParameters.getUseLocationsNetworkFile()) {
                // When using the network file, we've already defined the locations of interest but w/o coordinates. So when 
                // reading the coordinates file, we are only interested in obtaining the coordinates of the location in the
                // network, any other locations in the coordinates file should not be added to our collection of locations.
                switch (_identifier_mgr.setLocationCoordinates(identifier, coordinates)) {
                    case LocationsManager::CoordinateExists:
                        gPrint.Printf(
                            "Error: Record %ld in the coordinates file is attempting to define the same coordinates to more than one location.\n",
                            BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), coordinates.size(), _identifier_mgr.getLocationsManager().expectedDimensions()
                        );
                        bValid = false;
                        break;
                    case LocationsManager::CoordinateRedefinition:
                        gPrint.Printf(
                            "Error: Record %ld in the coordinates file is attempting to redefine the coordinates for '%s'.\n",
                            BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), identifier
                        );
                        bValid = false;
                        break;
                }
                continue;
            }
            switch (_identifier_mgr.addLocation(identifier, coordinates)) {
                case LocationsManager::CoordinateExists:
                    gPrint.Printf(
                        "Error: Record %ld in the coordinates file is attempting to define the same coordinates to more than one location.\n",
                        BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), coordinates.size(), _identifier_mgr.getLocationsManager().expectedDimensions()
                    );
                    bValid = false;
                    break;
                case LocationsManager::CoordinateRedefinition:
                    gPrint.Printf(
                        "Error: Record %ld in the coordinates file is attempting to redefine the coordinates for '%s'.\n",
                        BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), identifier
                    );
                    bValid = false;
                    break;
                case LocationsManager::NameExists:
                case LocationsManager::Duplicate:
                case LocationsManager::Accepted:
                default: break;
            }
        }
        //if invalid at this point then read encountered problems with data format, inform user of section to refer to in user guide for assistance
        if (! bValid) gPrint.Printf("Please see the 'Coordinates File' section in the user guide for help.\n", BasePrint::P_ERROR);
        else if (bEmpty) { //print indication if file contained no data
            gPrint.Printf("Error: The coordinates file contains no data.\n", BasePrint::P_ERROR);
            bValid = false;
        } else if (_identifier_mgr.getIdentifiers().size() == 1 && !gParameters.GetIsPurelyTemporalAnalysis()) {
            //validate that we have more than one tract, only a purely temporal analysis is the exception to this rule
            gPrint.Printf(
                "Error: For a %s analysis, the coordinates file must contain more than one record.\n",
                BasePrint::P_ERROR, ParametersPrint(gParameters).GetAnalysisTypeAsString()
            );
            bValid = false;
        }
	    if (closeAdditions) {
		    //signal insertions completed
		    _identifier_mgr.additionsCompleted();
		    //record number of locations read
		    gDataHub._num_identifiers = _identifier_mgr.getIdentifiers().size();
		    //record number of centroids read
		    gDataHub.m_nGridTracts = gCentroidsHandler.getNumGridPoints();
	    }
    } catch (prg_exception& x) {
        x.addTrace("ReadCoordinatesFileAsLatitudeLongitude()", "SaTScanDataReader");
        throw;
    }
    return bValid;
}

/** reads data from input files for a batched probability model */
bool SaTScanDataReader::ReadBatchedData() {
    try {
        if (!ReadCoordinatesFile())
            return false;
        gDataHub.gDataSets.reset(new BatchedDataSetHandler(gDataHub, gPrint));
        if (!gDataHub.gDataSets->ReadData())
            return false;
        if (gParameters.UseMaxCirclePopulationFile() && !ReadMaxCirclePopulationFile())
            return false;
        if (gParameters.UseSpecialGrid() && !ReadGridFile())
            return false;
    } catch (prg_exception& x) {
        x.addTrace("ReadBatchedData()", "SaTScanDataReader");
        throw;
    }
    return true;
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
    std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(
        getFilenameFormatTime(gParameters.GetSpecialGridFileName(), gParameters.getTimestamp(), true),
        gParameters.getInputSource(GRIDFILE), gPrint)
    );
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
    pGridPoints->setDimensions(_identifier_mgr.getLocationsManager().expectedDimensions());
    vCoordinates.resize(_identifier_mgr.getLocationsManager().expectedDimensions(), 0);
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source.ReadRecord()) {
        //there are records with data, but not necessarily valid
        bEmpty = false;
         //read and vaidate dimensions skip to next record if error reading coordinates as double
         if (! ReadCartesianCoordinates(Source, gPrint, vCoordinates, iScanCount, 0)) {
           bValid = false;
           continue;
         }
        //validate that we read the correct number of coordinates as defined by coordinates system or coordinates file
        if (iScanCount < _identifier_mgr.getLocationsManager().expectedDimensions()) {
          gPrint.Printf("Error: Record %ld in the grid file contains %d dimension%s but the\n"
                        "       coordinates file defined the number of dimensions as %d.\n",
                        BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), iScanCount,
                        (iScanCount == 1 ? "" : "s"), _identifier_mgr.getLocationsManager().expectedDimensions());
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
    pGridPoints->setDimensions(_identifier_mgr.getLocationsManager().expectedDimensions());
    vCoordinates.resize(_identifier_mgr.getLocationsManager().expectedDimensions(), 0);
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
    gDataHub.gDataSets.reset(new HomogeneousPoissonDataSetHandler(gDataHub, _identifier_mgr, gCentroidsHandler, gPrint));
	_identifier_mgr.getMetaIdentifiersManager().getMetaPool().additionsCompleted(_identifier_mgr);
    if (!gDataHub.gDataSets->ReadData())
      return false;
    //record number of locations read
    gDataHub._num_identifiers = _identifier_mgr.getIdentifiers().size();
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
    double        distanceBetween;
    Network     & locationNetwork(gDataHub.getLocationNetwork());

    try {
        gPrint.SetImpliedInputFileType(BasePrint::NETWORK_FILE);
        gPrint.Printf("Reading the locations network file\n", BasePrint::P_STDOUT);
        std::auto_ptr<DataSource> networkSource(DataSource::GetNewDataSourceObject(
            getFilenameFormatTime(gParameters.getLocationsNetworkFilename(), gParameters.getTimestamp(), true),
            gParameters.getInputSource(NETWORK_FILE), gPrint)
        );
        // First pass - determine whether we need to read the coordinates file.
		while (!gPrint.GetMaximumReadErrorsPrinted() && networkSource->ReadRecord()) {
			bEmpty = false;
            if (!(networkSource->GetNumValues() >= 1 && networkSource->GetNumValues() <= 3)) {
                gPrint.Printf("Error: Record %ld of the %s contains %ld values but expecting either 1 , 2 or 3 values (<location id>, <location id>, <distance>).\n",
                    BasePrint::P_READERROR, networkSource->GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str(), networkSource->GetNumValues());
                bValid = false;
                continue;
            }
			_identifier_mgr.addLocation(networkSource->GetValueAt(0));
            if (networkSource->GetNumValues() > 1 && networkSource->GetValueAt(1))
				_identifier_mgr.addLocation(networkSource->GetValueAt(1));
            // If any record in the network file excludes distance value, then we need to read coordinates.
            readCoordiantesFile |= networkSource->GetNumValues() == 2;
        }
		if (bEmpty) gPrint.Printf("Error: The network file contains no data.\n", BasePrint::P_ERROR);
        if (!bValid || bEmpty) return false;
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
			//_identifier_mgr.getMetaIdentifiersManager().getMetaPool().additionsCompleted(_identifier_mgr);
            gPrint.Printf("Reading the coordinates file to obtain network distances and coordinates.\n", BasePrint::P_STDOUT);
			gPrint.SetImpliedInputFileType(BasePrint::COORDFILE);
			std::auto_ptr<DataSource> coordinatesSource(DataSource::GetNewDataSourceObject(
                getFilenameFormatTime(gParameters.GetCoordinatesFileName(), gParameters.getTimestamp(), true),
                gParameters.getInputSource(COORDFILE), gPrint)
            );
            switch (gParameters.GetCoordinatesType()) {
                case CARTESIAN:	bValid = ReadCoordinatesFileAsCartesian(*coordinatesSource, false); break;
                case LATLON: bValid = ReadCoordinatesFileAsLatitudeLongitude(*coordinatesSource, false); break;
                default: throw prg_error("Unknown coordinate type '%d'.", "ReadLocationNetworkFileAsDefinition()", gParameters.GetCoordinatesType());
            }
            // Since we've read the coordinates file, we can potentially report location coordinates but we'll know for certain once we re-read the network file.
            gDataHub._network_can_report_coordinates = true;
        } else {
            // Didn't read coordinates file, definitely can't report location coordinates.
            gDataHub._network_can_report_coordinates = false;
            gDataHub._num_identifiers = _identifier_mgr.getIdentifiers().size();
        }
		// Now re-read the network file.
        if (bValid) {
			gPrint.SetImpliedInputFileType(BasePrint::NETWORK_FILE);
			LocationsManager::LocationIdx_t firstlocation, secondlocation(boost::none, 0);
			const LocationsManager & locations(_identifier_mgr.getLocationsManager());
			std::vector<double> coordinatesA, coordinatesB;
			networkSource->GotoFirstRecord();
            while (!gPrint.GetMaximumReadErrorsPrinted() && networkSource->ReadRecord()) {
				if (!(networkSource->GetNumValues() >= 1 && networkSource->GetNumValues() <= 3)) {
					gPrint.Printf(
                        "Error: Record %ld of the %s contains %ld values but expecting either 1 , 2 or 3 values (<location id>, <location id>, <distance>).\n",
						BasePrint::P_READERROR, networkSource->GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str(), networkSource->GetNumValues()
                    );
					bValid = false;
					continue;
				}
				firstlocation = locations.getLocation(networkSource->GetValueAt(0));
				if (!firstlocation.first) {
                    // Network location is not already known through coordinates file, add location but we don't know it's coordinates.
					_identifier_mgr.addLocation(networkSource->GetValueAt(0));
					firstlocation = locations.getLocation(networkSource->GetValueAt(0));
				}
                const char * secondvalue = networkSource->GetValueAt(1);
                if (!secondvalue) {
                    locationNetwork.addNode(*firstlocation.first, *firstlocation.second);
					secondlocation = LocationsManager::LocationIdx_t(boost::none, 0);
                } else {
					secondlocation = locations.getLocation(secondvalue);
					if (!secondlocation.first) {
                        // Network location is not already known through coordinates file, add location but we don't know it's coordinates.
						_identifier_mgr.addLocation(networkSource->GetValueAt(1));
						secondlocation = locations.getLocation(networkSource->GetValueAt(1));
					}
                    if (*firstlocation.first == *secondlocation.first) {
                        gPrint.Printf("Error: Record %ld of %s defines location '%s' as a connection to itself.\n",
                            BasePrint::P_READERROR, networkSource->GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str(), networkSource->GetValueAt(0));
                        bValid = false;
                        continue;
                    } else if (networkSource->GetNumValues() == 3) {
                        if (!string_to_type<double>(networkSource->GetValueAt(2), distanceBetween) || distanceBetween <= 0) {
                            gPrint.Printf("Error: The distance between value '%s' in record %ld, of %s, is not a positive decimal number greater than zero.\n",
                                BasePrint::P_READERROR, networkSource->GetValueAt(2), networkSource->GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
                            bValid = false;
                            continue;
                        }
                        if (gParameters.requestsGeogrphaicalOutput()) {
                            // Although user specified the distance between locations, we still need to verifiy that the coordinates file defined coordinates for both here.
                            if (!locations.locations()[*firstlocation.first]->hascoordinates() || !locations.locations()[*secondlocation.first]->hascoordinates()) {
                                gPrint.Printf(
                                    "Warning: The network file identified a location in record %ld which has no coordinates defined in the coordinates file.\n"
                                    "         Coordinates are required to place locations geograghically. Geographical output files will not be created in this analysis.\n",
                                    BasePrint::P_WARNING, networkSource->GetCurrentRecordIndex()
                                );
                                const_cast<CParameters&>(gParameters).toggleGeogrphaicalOutput(false);
                            }
                        }
                    } else {
                        // Calculate distance between locations and use that value in connection.
                        if (!locations.locations()[*firstlocation.first]->hascoordinates() || !locations.locations()[*secondlocation.first]->hascoordinates()) {
                            gPrint.Printf(
                                "Error: A distance could not be calculated between locations '%s' and '%s' in the network file, record %ld.\n"
                                "       One or both of these locations does not have coordinates defined in the coordinates file.\n",
                                BasePrint::P_READERROR, networkSource->GetValueAt(0), networkSource->GetValueAt(1), networkSource->GetCurrentRecordIndex()
                            );
                            bValid = false;
                            continue;
                        }
                        distanceBetween = Coordinates::distanceBetween(
							locations.locations()[*firstlocation.first]->coordinates()->retrieve(coordinatesA),
							locations.locations()[*secondlocation.first]->coordinates()->retrieve(coordinatesB)
						);
                    }
                    NetworkNode::AddStatusType addStatus = locationNetwork.addConnection(
						*firstlocation.first, *firstlocation.second, *secondlocation.first, *secondlocation.second, distanceBetween, true
					);
                    if (addStatus >= NetworkNode::AddStatusType::SelfReference) {
                        gPrint.Printf(
                            "Error: An issue was found when adding connection between location '%s' and location '%s' in record %ld of %s: %s.\n",
                            BasePrint::P_READERROR, networkSource->GetValueAt(0), networkSource->GetValueAt(1), networkSource->GetCurrentRecordIndex(), 
                            gPrint.GetImpliedFileTypeString().c_str(), NetworkNode::getStatusMessage(addStatus)
                        );
                        bValid = false;
                        continue;
                    }
                    addStatus = locationNetwork.addConnection(*secondlocation.first, *secondlocation.second, *firstlocation.first, *firstlocation.second, distanceBetween, false);
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
                gDataHub._network_can_report_coordinates &= (
					locations.locations()[*firstlocation.first]->hascoordinates() &&
                    (!secondlocation.first || locations.locations()[*secondlocation.first]->hascoordinates())
				);
                if (!gDataHub._network_can_report_coordinates && gParameters.requestsGeogrphaicalOutput()) {
                    if (!locations.locations()[*firstlocation.first]->hascoordinates() || (secondlocation.first && !locations.locations()[*secondlocation.first]->hascoordinates())) {
                        gPrint.Printf(
                            "Warning: The network file identified a location in record %ld which has no coordinates defined in the coordinates file.\n"
                            "         Coordinates are required to place locations geograghically. Geographical output files will not be created in this analysis.\n",
                            BasePrint::P_WARNING, networkSource->GetCurrentRecordIndex()
                        );
                        const_cast<CParameters&>(gParameters).toggleGeogrphaicalOutput(false);
                    }
                }
            }

            if (gParameters.GetMultipleCoordinatesType() == ONEPERLOCATION) {
                //signal insertions completed
                _identifier_mgr.additionsCompleted();
                //record number of locations read
                gDataHub._num_identifiers = _identifier_mgr.getIdentifiers().size();
                //record number of centroids read
                gDataHub.m_nGridTracts = gCentroidsHandler.getNumGridPoints();
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("ReadLocationNetworkFileAsDefinition()", "SaTScanDataReader");
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
    std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(
        getFilenameFormatTime(gParameters.GetMaxCirclePopulationFileName(), gParameters.getTimestamp(), true),
        gParameters.getInputSource(MAXCIRCLEPOPFILE), gPrint)
    );
    //initialize circle-measure array
    gDataHub.gvMaxCirclePopulation.resize(gDataHub._num_identifiers + gDataHub.getIdentifierInfo().getMetaIdentifiersManager().getNumReferenced(), 0);

    //1st pass, determine unique population dates. Notes errors with records and continues reading.
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source->ReadRecord()) {
        bEmpty=false;
        //Validate that tract identifer is one of those defined in the coordinates file.
        SaTScanDataReader::RecordStatusType eStatus = RetrieveIdentifierIndex(*Source, TractIdentifierIndex);
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
    for (size_t t=0; t < gDataHub.getIdentifierInfo().getMetaIdentifiersManager().getNumReferenced(); ++t) {
       gDataHub.getIdentifierInfo().getMetaIdentifiersManager().getAtomicIndexes(t, atomicIndexes);
       for (size_t a=0; a < atomicIndexes.size(); ++a)
         gDataHub.gvMaxCirclePopulation[(size_t)gDataHub._num_identifiers + t] += gDataHub.gvMaxCirclePopulation[atomicIndexes[a]];
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
  std::string           sIdentifier, neighbors, metaLocationsFilename(getFilenameFormatTime(gParameters.getMetaLocationsFilename(), gParameters.getTimestamp(), true));
  long                  uLocation0ffset;

  try {
    if (!gParameters.UseMetaLocationsFile()) {
		_identifier_mgr.getMetaIdentifiersManager().getMetaPool().additionsCompleted(_identifier_mgr);
      return true;
    }
    gPrint.Printf("Reading the meta locations file\n", BasePrint::P_STDOUT);
    gPrint.SetImpliedInputFileType(BasePrint::META_LOCATIONS_FILE);
    //determine format of file, either:
    // meta1=loc1,loc2, loc3
    // meta1 loc1 loc2 loc3
    std::ifstream SourceFile(metaLocationsFilename.c_str());
    getlinePortable(SourceFile, sIdentifier);
    bStructuredMetaData = (sIdentifier.find("=") == sIdentifier.npos ? false : true);
    SourceFile.close();
    AsciiFileDataSource Source(metaLocationsFilename.c_str(), gPrint, (bStructuredMetaData ? '=' : ' '));

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
          neighbors = Source.GetValueAt(1);
      else {
        uLocation0ffset=1;
        neighbors.clear();
        while (Source.GetValueAt(uLocation0ffset)) {
           if (uLocation0ffset > 1) neighbors += ",";
           neighbors += Source.GetValueAt(uLocation0ffset);
           ++uLocation0ffset;
        }
      }
      if(!_identifier_mgr.getMetaIdentifiersManager().getMetaPool().addMetaIdentifier(sIdentifier, neighbors)) {
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
	_identifier_mgr.getMetaIdentifiersManager().getMetaPool().additionsCompleted(_identifier_mgr);
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
  bool bValid=true, bEmpty=true;
  long uLocation0ffset;
  std::vector<tract_t> vRecordNeighborList, AtomicIndexes;

  try {
    if (!ReadMetaLocationsFile()) return false;
    gPrint.Printf("Reading the neighbors file\n", BasePrint::P_STDOUT);
    gPrint.SetImpliedInputFileType(BasePrint::LOCATION_NEIGHBORS_FILE);
	_identifier_mgr.setExpectedCoordinateDimensions(0);
    std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(
        getFilenameFormatTime(gParameters.GetLocationNeighborsFileName(), gParameters.getTimestamp(), true),
        gParameters.getInputSource(LOCATION_NEIGHBORS_FILE), gPrint)
    );
    //first pass on neighbors file to determine all location identifiers referenced
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source->ReadRecord()) {
      uLocation0ffset=0;
      gDataHub.m_nGridTracts++;
      while (Source->GetValueAt(uLocation0ffset)) {
        //insert location identifier into tract handler class
	    _identifier_mgr.addLocation(Source->GetValueAt(uLocation0ffset));
        ++uLocation0ffset;
        bEmpty = false;
      }
    }
    //second pass - allocate respective sorted array
    if (!bEmpty) {
      //record number of locations read
	  _identifier_mgr.additionsCompleted(gParameters.GetOutputRelativeRisksFiles());
	  gDataHub._num_identifiers = _identifier_mgr.getIdentifiers().size();
      boost::dynamic_bitset<> NeighborsSet(gDataHub._num_identifiers);
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
		  auto locationIdx = _identifier_mgr.getIdentifierIndex(identifier);
          if (locationIdx) {
            if (NeighborsSet.test(*locationIdx)) {
              bValid = false;
              gPrint.Printf("Error: Location ID '%s' occurs multiple times in record %ld of %s.\n", BasePrint::P_READERROR,
                            Source->GetValueAt(uLocation0ffset), Source->GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
            } else {
              NeighborsSet.set(*locationIdx);
              vRecordNeighborList.push_back(*locationIdx);
            }
          } else {
			locationIdx = boost::make_optional(_identifier_mgr.getMetaIdentifiersManager().getMetaIndex(identifier));
			_identifier_mgr.getMetaIdentifiersManager().getAtomicIndexes(*locationIdx, AtomicIndexes);
            for (size_t t=0; t < AtomicIndexes.size(); ++t) {
              if (NeighborsSet.test(AtomicIndexes[t])) {
                bValid = false;
                gPrint.Printf("Error: Location ID '%s' occurs multiple times in record %ld of %s.\n", BasePrint::P_READERROR,
					_identifier_mgr.getIdentifiers()[AtomicIndexes[t]]->name().c_str(), Source->GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
              }
              NeighborsSet.set(AtomicIndexes[t]);
            }
            vRecordNeighborList.push_back(*locationIdx + gDataHub._num_identifiers);
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
SaTScanDataReader::RecordStatusType SaTScanDataReader::RetrieveIdentifierIndex(DataSource& Source, tract_t& tLocationIndex) {
   //Validate that tract identifer is one of those defined in the coordinates file.
    const char * identifier = Source.GetValueAt(_identifier_column_index);
    if (!identifier) {
       gPrint.Printf("Error: Location ID is missing in record %ld of %s.\n", BasePrint::P_READERROR,
                     Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
       return SaTScanDataReader::Rejected;
    }
	auto identifierIdx = gDataHub.getIdentifierInfo().getIdentifierIndex(identifier);
    if (!identifierIdx) {
        if (gParameters.GetCoordinatesDataCheckingType() == STRICTCOORDINATES) {
            gPrint.Printf("Error: Unknown location ID in %s, record %ld. '%s' not specified in the %s file.\n", BasePrint::P_READERROR,
                          gPrint.GetImpliedFileTypeString().c_str(), Source.GetCurrentRecordIndex(), Source.GetValueAt(_identifier_column_index),
                          (gParameters.UseLocationNeighborsFile() ? "neighbors" : "coordinates"));
            return SaTScanDataReader::Rejected;
        }
        // Report to user if the data checking option is ignoring locations - because the user requested relaxed checking, that is unless
        // this is a drilldown where we set this option programmatically.
        if (!gDataHub.isDrilldown() && std::find(gmSourceLocationWarned.begin(), gmSourceLocationWarned.end(), reinterpret_cast<void*>(&Source)) == gmSourceLocationWarned.end()) {
            gPrint.Printf("Warning: Some records in %s reference a location ID that was not specified in the %s file.\n"
                          "         These will be ignored in the analysis.\n", BasePrint::P_WARNING, gPrint.GetImpliedFileTypeString().c_str(),
                          (gParameters.UseLocationNeighborsFile() ? "neighbors" : "coordinates"));
            gmSourceLocationWarned.push_back(reinterpret_cast<void*>(&Source));
        }
        return SaTScanDataReader::Ignored;
	}
	else
		tLocationIndex = identifierIdx.get();
    return SaTScanDataReader::Accepted;
}

