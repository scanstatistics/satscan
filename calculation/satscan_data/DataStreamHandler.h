//---------------------------------------------------------------------------
#ifndef DataStreamHandlerH
#define DataStreamHandlerH
//---------------------------------------------------------------------------
#include "DataStream.h"
#include "DataStreamGateway.h"
#include "Randomizer.h"

class CSaTScanData; /** forward class definition */

typedef ZdPointerVector<SimulationDataStream> SimulationDataContainer_t;
typedef ZdPointerVector<RealDataStream>       RealDataContainer_t;

/** Manages all data streams. */
class DataStreamHandler {
  private:
    void                                Setup();
  
  protected:
    const CParameters                 & gParameters;            /** reference to parameters */
    BasePrint                         & gPrint;                /** pointer to print direction */
    CSaTScanData                      & gDataHub;               /** reference to data hub */
    RealDataContainer_t                 gvDataStreams;          /** collection of data streams */
    static const short                  COUNT_DATE_OFFSET;      /** field index of date in case/control files */

    virtual void                        AllocateCaseStructures(unsigned int iStream);
    bool                                ConvertCountDateToJulian(StringParser & Parser, Julian & JulianDate);
    AbtractDataStreamGateway          * GetNewDataGatewayObject() const;
    bool                                ParseCountLine(PopulationData & thePopulation, StringParser & Parser,
                                                       tract_t& tid, count_t& nCount,
                                                       Julian& nDate, int& iCategoryIndex);
    bool                                ParseCovariates(PopulationData & thePopulation, int& iCategoryIndex, int iCovariatesOffset, StringParser & Parser);
    virtual bool                        ReadCaseFile(size_t tStream);
    virtual bool                        ReadCounts(size_t tStream, FILE * fp, const char* szDescription);
    void                                SetCaseArrays(count_t**  pCases, count_t** pCases_NC, count_t*  pCasesByTimeInt);

    //pure virtual protected functions
    virtual void                        SetRandomizers() = 0;

  public:
    DataStreamHandler(CSaTScanData& DataHub, BasePrint& pPrint);
    virtual ~DataStreamHandler();

    //pure virtual public functions
    virtual AbtractDataStreamGateway  * GetNewDataGateway() const = 0;
    virtual AbtractDataStreamGateway  * GetNewSimulationDataGateway(const SimulationDataContainer_t& Container) const = 0;
    virtual RandomizerContainer_t     & GetRandomizerContainer(RandomizerContainer_t& Container) const = 0;
    virtual SimulationDataContainer_t & GetSimulationDataContainer(SimulationDataContainer_t& Container) const = 0;
    virtual void                        RandomizeData(RandomizerContainer_t& Container, SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber) const;
    virtual bool                        ReadData() = 0;

    size_t                              GetNumStreams() const {return gvDataStreams.size();}
    const RealDataStream              & GetStream(unsigned int iStream) const {return *gvDataStreams[iStream];}
    RealDataStream                    & GetStream(unsigned int iStream) {return *gvDataStreams[iStream];}
    void                                ReportZeroPops(CSaTScanData & Data, FILE *pDisplay, BasePrint * pPrintDirection);
    virtual void                        SetPurelyTemporalMeasureData(RealDataStream & thisRealStream);
    virtual void                        SetPurelyTemporalSimulationData(SimulationDataContainer_t& SimDataContainer);
};
#endif
