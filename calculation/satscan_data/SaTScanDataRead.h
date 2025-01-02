//******************************************************************************
#ifndef __SATSCANDATAREAD_H
#define __SATSCANDATAREAD_H
//******************************************************************************
#include "SaTScan.h"
#include "Parameters.h"
#include "Tracts.h"
#include "GridTractCoordinates.h"

class DataSource;   /** forward class definition */
class CSaTScanData; /** forward class definition */
class RelativeRiskAdjustmentHandler;

/** Reads data from input sources into class members of CSaTScanData object.
    Note: We might want to make this class an abstraction, deriving for each
          probability model. Currently the DataSetHandler hierarchy does this but
          it might be better to simplfy those classes and move reading here. */
class SaTScanDataReader {
  public:
    typedef boost::shared_ptr<RelativeRiskAdjustmentHandler> RiskAdjustments_t;
    typedef std::vector<RiskAdjustments_t> RiskAdjustmentsContainer_t;
    enum RecordStatusType       {Rejected=0, Ignored, Accepted};

  private:
    static const long           _identifier_column_index;           /** input record index for location */
    CSaTScanData              & gDataHub;
    BasePrint                 & gPrint;
    const CParameters         & gParameters;
    GInfo                     & gCentroidsHandler;
	IdentifiersManager        & _identifier_mgr;
    std::deque<void*>           gmSourceLocationWarned;    /** indicates whether user has already been warned that records are being ignored */

    bool                        ConvertAdjustmentDateToJulian(DataSource& Source, Julian& JulianDate, bool bStartDate);
    bool                        ReadBatchedData();
    bool                        ReadBernoulliData();
    bool                        ReadCoordinatesFile();
    bool                        ReadCoordinatesFileAsCartesian(DataSource& Source, bool closeAdditions = true);
    bool                        ReadCoordinatesFileAsLatitudeLongitude(DataSource& Source, bool closeAdditions=true);
    bool                        ReadExponentialData();
    bool                        ReadGridFile();
    bool                        ReadGridFileAsCartiesian(DataSource& Source);
    bool                        ReadGridFileAsLatitudeLongitude(DataSource& Source);
    bool                        ReadHomogeneousPoissonData();
    bool                        ReadIntervalDates(DataSource& Source, GInfo::FocusInterval_t& focusInterval, short iSourceOffset, bool& warned);
	bool                        ReadLocationNetworkFileAsDefinition();
	bool                        ReadMaxCirclePopulationFile();
    bool                        ReadMetaLocationsFile();
    bool                        ReadNormalData();
    bool                        ReadOrdinalData();
    bool                        ReadPoissonData();
    bool                        ReadRankData();
    bool                        ReadUniformTimeData();
    bool                        ReadSpaceTimePermutationData();
    bool                        ReadUserSpecifiedNeighbors();
    RecordStatusType            RetrieveIdentifierIndex(DataSource& Source, tract_t& tLocationIndex);

  public:
    SaTScanDataReader(CSaTScanData& DataHub);
    ~SaTScanDataReader() {}

    void                        Read();
	bool                        ReadAdjustmentsByRelativeRisksFile(const std::string& filename, RiskAdjustmentsContainer_t& rrAdjustments, bool consolidate);
	void                        ReadBernoulliDrilldown();
	static bool                 ReadCartesianCoordinates(DataSource& Source, BasePrint & Print, std::vector<double> & vCoordinates, short & iScanCount, short iWordOffSet);
	static bool                 ReadLatitudeLongitudeCoordinates(DataSource& Source, BasePrint & Print, std::vector<double> & vCoordinates, short iWordOffSet, const char * sSourceFile);
};
//*****************************************************************************
#endif

