//---------------------------------------------------------------------------
#ifndef DataStreamHandlerH
#define DataStreamHandlerH
//---------------------------------------------------------------------------
#include "DataStream.h"
#include "DataStreamGateway.h"
#include "Randomizer.h"

class CSaTScanData; /** forward class definition */

/** Manages all data streams. */
class DataStreamHandler {
  private:
    void                                Setup();
  
  protected:
    const CParameters                 & gParameters;            /** reference to parameters */
    BasePrint                         * gpPrint;                /** pointer to print direction */
    CSaTScanData                      & gData;                  /** reference to data hub */
    std::vector<DataStream>             gvDataStreams;          /** collection of data streams */

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
    DataStreamHandler(CSaTScanData & Data, BasePrint * pPrint);
    virtual ~DataStreamHandler();

    //pure virtual public functions
    virtual void                        AllocateSimulationStructures() = 0;
    virtual AbtractDataStreamGateway  * GetNewDataGateway() = 0;
    virtual AbtractDataStreamGateway  * GetNewSimulationDataGateway() = 0;
    virtual void                        RandomizeData(unsigned int iSimulationNumber) = 0;
    virtual bool                        ReadData() = 0;

    void                                AllocateNCSimulationCases();
    void                                AllocatePTSimulationCases();
    void                                AllocateSimulationCases();
    void                                FreeSimulationStructures();
    size_t                              GetNumStreams() const {return gvDataStreams.size();}
    const DataStream                  & GetStream(unsigned int iStream) const {return gvDataStreams[iStream];}
    DataStream                        & GetStream(unsigned int iStream) {return gvDataStreams[iStream];}
    void                                ReportZeroPops(CSaTScanData & Data, FILE *pDisplay, BasePrint * pPrintDirection);
    virtual void                        SetPurelyTemporalMeasureData(DataStream & thisStream);
    virtual void                        SetPurelyTemporalSimulationData();
};
#endif
