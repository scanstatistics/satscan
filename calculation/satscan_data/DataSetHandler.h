//******************************************************************************
#ifndef __DataSetHandler_H
#define __DataSetHandler_H
//******************************************************************************
#include "DataSet.h"
#include "DataSetGateway.h"
#include "Randomizer.h"

class CSaTScanData; /** forward class definition */

typedef ZdPointerVector<SimDataSet>  SimulationDataContainer_t;
typedef ZdPointerVector<RealDataSet> RealDataContainer_t;

/** Manages all data sets. */
class DataSetHandler {
  private:
    void                                Setup();
  
  protected:
    static const short                  guLocationIndex;           /** input record index for location */
    static const short                  guCountIndex;              /** input record index for count in case/control files */
    static const short                  guCountDateIndex;          /** input record index for date in case/control files */
    static const short                  guCountCategoryIndexNone;  /** input record index for category in case/control files w/ no date field */
    static const short                  guCountCategoryIndex;      /** input record index for category in case/control files */
    const CParameters                 & gParameters;               /** reference to parameters */
    BasePrint                         & gPrint;                    /** pointer to print direction */
    CSaTScanData                      & gDataHub;                  /** reference to data hub */
    RealDataContainer_t                 gvDataSets;                /** collection of data sets */
    RandomizerContainer_t               gvDataSetRandomizers;      /** collection of randomizers, one for each data set */

    virtual void                        AllocateCaseStructures(size_t iSetIndex);
    bool                                ConvertCountDateToJulian(StringParser& Parser, Julian& JulianDate);
    AbtractDataSetGateway             * GetNewDataGatewayObject() const;
    bool                                ParseCountLine(PopulationData& thePopulation, StringParser& Parser,
                                                       tract_t& tid, count_t& nCount,
                                                       Julian& nDate, int& iCategoryIndex);
    bool                                ParseCovariates(PopulationData& thePopulation, int& iCategoryIndex, int iCovariatesOffset, StringParser& Parser);
    virtual bool                        ReadCaseFile(size_t iSetIndex);
    virtual bool                        ReadCounts(size_t iSetIndex, FILE * fp, const char* szDescription);

    //pure virtual protected functions
    virtual void                        SetRandomizers() = 0;

  public:
    DataSetHandler(CSaTScanData& DataHub, BasePrint& pPrint);
    virtual ~DataSetHandler();

    //pure virtual public functions
    virtual AbtractDataSetGateway     * GetNewDataGateway() const = 0;
    virtual AbtractDataSetGateway     * GetNewSimulationDataGateway(const SimulationDataContainer_t& Container) const = 0;
    virtual const AbstractRandomizer  * GetRandomizer(size_t iSetIndex) const;
    virtual RandomizerContainer_t     & GetRandomizerContainer(RandomizerContainer_t& Container) const;
    virtual SimulationDataContainer_t & GetSimulationDataContainer(SimulationDataContainer_t& Container) const = 0;
    virtual void                        RandomizeData(RandomizerContainer_t& Container, SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber) const;
    virtual bool                        ReadData() = 0;

    size_t                              GetNumDataSets() const {return gvDataSets.size();}
    const RealDataSet                 & GetDataSet(size_t iSetIndex=0) const {return *gvDataSets[iSetIndex];}
    RealDataSet                       & GetDataSet(size_t iSetIndex=0) {return *gvDataSets[iSetIndex];}
    void                                ReportZeroPops(CSaTScanData& Data, FILE* pDisplay, BasePrint* pPrintDirection);
    virtual void                        SetPurelyTemporalMeasureData(RealDataSet& thisRealSet);
    virtual void                        SetPurelyTemporalSimulationData(SimulationDataContainer_t& SimDataContainer);
};
#endif

