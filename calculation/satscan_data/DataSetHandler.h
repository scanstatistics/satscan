//******************************************************************************
#ifndef __DataSetHandler_H
#define __DataSetHandler_H
//******************************************************************************
#include "DataSet.h"
#include "DataSetGateway.h"
#include "Randomizer.h"

class CSaTScanData; /** forward class definition */
class DataSource;   /** forward class definition */

typedef ptr_vector<DataSet>     SimulationDataContainer_t;
typedef ptr_vector<RealDataSet> RealDataContainer_t;

/** Manages all data sets. */
class DataSetHandler {
    friend class SaTScanDataReader;
    friend class BernoulliAnalysisDrilldown;
    friend class AbstractAnalysisDrilldown;
    friend class DataDemographicsProcessor;

  public:
     enum CountFileReadStatus { ReadSuccess=0, ReadError, NoCounts, NotMinimum };
     enum RecordStatusType { Rejected = 0, Ignored, Accepted };

  private:
    void                                Setup();
  
  protected:
    static const short                  _identifier_column_index;           /** input record index for location */
    static const short                  guCountIndex;              /** input record index for count in case/control files */
    static const short                  guCountDateIndex;          /** input record index for date in case/control files */
    static const short                  guCountCategoryIndexNone;  /** input record index for category in case/control files w/ no date field */
    static const short                  guCountCategoryIndex;      /** input record index for category in case/control files */
    const CParameters                 & gParameters;               /** reference to parameters */
    BasePrint                         & gPrint;                    /** pointer to print direction */
    CSaTScanData                      & gDataHub;                  /** reference to data hub */
    RealDataContainer_t                 gvDataSets;                /** collection of data sets */
    RealDataContainer_t                 _oliveira_data_sets;       /** collection of data sets for Oliveira feature */
    RandomizerContainer_t               gvDataSetRandomizers;      /** collection of randomizers, one for each data set */
    mutable std::deque<void*>           gmSourceDateWarned;        /** indicates whether user has already been warned that records are being ignored */
    mutable std::deque<void*>           gmSourceLocationWarned;    /** indicates whether user has already been warned that records are being ignored */
    std::vector<unsigned int>           _removed_data_sets;        /* Index of data set that was removed -- in original data collection. */
    unsigned int                        _approximate_case_records; /* approximate number of case records read */

    virtual void                        printFileReadMessage(BasePrint::eInputFileType impliedFile, size_t iSetIndex, bool oneDataSet);
    virtual void                        printReadStatusMessage(CountFileReadStatus status, bool isControls, size_t iSetIndex, bool oneDataSet) const;
    virtual CountFileReadStatus         ReadCaseFile(RealDataSet& DataSet);
    virtual CountFileReadStatus         ReadCounts(RealDataSet& DataSet, DataSource& Source);
    virtual void                        removeDataSet(size_t iSetIndex) { throw prg_error("removeDataSet().", "DataSetHandler()"); }
    RecordStatusType                    RetrieveCaseRecordData(PopulationData& thePopulation, DataSource& Source, tract_t& tid, count_t& nCount, Julian& nDate, int& iCategoryIndex);
    bool                                RetrieveCovariatesIndex(PopulationData& thePopulation, int& iCategoryIndex, short iCovariatesOffset, DataSource& Source);

    //pure virtual protected functions
    virtual void                        SetRandomizers() = 0;

  public:
    DataSetHandler(CSaTScanData& DataHub, BasePrint& pPrint);
    virtual ~DataSetHandler() {}

    //pure virtual public functions
    virtual SimulationDataContainer_t & AllocateSimulationData(SimulationDataContainer_t& Container) const = 0;
    virtual void                        assignMetaData(RealDataContainer_t& Container) const = 0;
    virtual const RealDataContainer_t & buildOliveiraDataSets();
    virtual AbstractDataSetGateway    & GetDataGateway(AbstractDataSetGateway& DataGatway) const = 0;
    virtual AbstractDataSetGateway    & GetOliveraDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container) const;
    virtual AbstractDataSetGateway    & GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container, const RandomizerContainer_t& rContainer) const = 0;
    virtual bool                        ReadData() = 0;

    const std::vector<unsigned int>   & getRemovedDataSetIndexes() const { return _removed_data_sets; }
    AbstractDataSetGateway            * GetNewDataGatewayObject() const;
    size_t                              GetNumDataSets() const {return gvDataSets.size();}
    const RealDataContainer_t         & getDataSets() const { return gvDataSets; }
    RealDataContainer_t               & getDataSets() {return gvDataSets;}
    const RealDataSet                 & GetDataSet(size_t iSetIndex=0) const {return *gvDataSets.at(iSetIndex);}
    RealDataSet                       & GetDataSet(size_t iSetIndex=0) {return *gvDataSets.at(iSetIndex);}
    virtual AbstractRandomizer        * GetRandomizer(size_t iSetIndex);
    virtual const AbstractRandomizer  * GetRandomizer(size_t iSetIndex) const;
    virtual RandomizerContainer_t     & GetRandomizerContainer(RandomizerContainer_t& Container) const;
    virtual SimulationDataContainer_t & GetSimulationDataContainer(SimulationDataContainer_t& Container) const;
    virtual void                        RandomizeData(RandomizerContainer_t& Container, SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber) const;
    void                                ReportZeroPops(CSaTScanData& Data, FILE* pDisplay, BasePrint* pPrintDirection);
	RecordStatusType                    RetrieveCaseCounts(DataSource& Source, count_t& nCount) const;
	RecordStatusType                    RetrieveCountDate(DataSource& Source, Julian& JulianDate) const;
	RecordStatusType                    RetrieveIdentifierIndex(DataSource& Source, tract_t& tLocationIndex) const;
	void                                removeDataSetsWithNoData();
    size_t                              getDataSetRelativeIndex(size_t iSet) const;
    virtual void                        SetPurelyTemporalMeasureData(RealDataSet& thisRealSet);
    virtual void                        SetPurelyTemporalSimulationData(SimulationDataContainer_t& SimDataContainer);
};
//******************************************************************************
#endif

