//---------------------------------------------------------------------------
#ifndef DataStreamHandlerH
#define DataStreamHandlerH
//---------------------------------------------------------------------------
#include "DataStream.h"
#include "DataStreamGateway.h"

class CSaTScanData; /** forward class definition */

/** Manages all data streams. */
class DataStreamHandler {
  private:
    const CParameters                 & gParameters;            /** reference to parameters */
    BasePrint                         * gpPrint;                /** pointer to print direction */
    CSaTScanData                      & gData;                  /** reference to data hub */
    std::deque<DataStream>              gvDataStreams;          /** collection of data streams */

    void                                AllocateCaseStructures(unsigned int iStream);
    void                                AllocateControlStructures(unsigned int iStream);
    bool                                ConvertCountDateToJulian(StringParser & Parser, const char * szDescription, Julian & JulianDate);    
    bool                                ConvertPopulationDateToJulian(const char * sDateString, int iRecordNumber, Julian & JulianDate);
    bool                                ParseCountLine(PopulationData & thePopulation, const char*  szDescription, StringParser & Parser,
                                                       tract_t& tid, count_t& nCount,
                                                       Julian& nDate, int& iCategoryIndex);
    bool                                ParseCovariates(PopulationData & thePopulation, int& iCategoryIndex, int iCovariatesOffset, const char*  szDescription, StringParser & Parser);
    bool                                ReadCounts(size_t tStream, FILE * fp, const char* szDescription);
    void                                SetCaseArrays(count_t**  pCases, count_t** pCases_NC, count_t*  pCasesByTimeInt);
    void                                Setup();

  public:
    DataStreamHandler(CSaTScanData & Data, BasePrint * pPrint);
    ~DataStreamHandler();

    void                                AllocateNCSimCases();
    void                                AllocatePTSimCases();
    void                                AllocateSimulationCases();
    void                                AllocateSimulationMeasure();
    void                                AllocateSimulationPTMeasure();
    void                                FreeSimulationStructures();
    DataStreamGateway                 * GetNewDataGateway();
    DataStreamGateway                 * GetNewSimulationDataGateway();
    size_t                              GetNumStreams() const {return gvDataStreams.size();}
    const DataStream                  & GetStream(unsigned int iStream) const {return gvDataStreams[iStream];}
    DataStream                        & GetStream(unsigned int iStream) {return gvDataStreams[iStream];}
    const std::deque<DataStream>      & GetStreams() const {return gvDataStreams;}
    bool                                ReadCaseFile(size_t tStream);
    bool                                ReadControlFile(size_t tStream);
    bool                                ReadPopulationFile(size_t tStream);
    void                                ReportZeroPops(CSaTScanData & Data, FILE *pDisplay, BasePrint * pPrintDirection);
    void                                SetPurelyTemporalSimCases();
};
#endif
