//---------------------------------------------------------------------------
#ifndef PoissonDataStreamHandlerH
#define PoissonDataStreamHandlerH
//---------------------------------------------------------------------------
#include "DataStreamHandler.h"
#include "PoissonRandomizer.h"

class PoissonDataStreamHandler : public DataStreamHandler {
  protected:
    ZdPointerVector<AbstractRandomizer>  gvDataStreamRandomizers;

    bool                                ConvertPopulationDateToJulian(const char * sDateString, int iRecordNumber, Julian & JulianDate);
    bool                                ReadPopulationFile(size_t tStream);
    virtual void                        SetRandomizers();

  public:
    PoissonDataStreamHandler(CSaTScanData & Data, BasePrint * pPrint);
    virtual ~PoissonDataStreamHandler();

    virtual void                        AllocateSimulationStructures();
    virtual AbtractDataStreamGateway  * GetNewDataGateway();
    virtual AbtractDataStreamGateway  * GetNewSimulationDataGateway();
    virtual void                        RandomizeData(unsigned int iSimulationNumber);
    virtual bool                        ReadData();
};
#endif
 