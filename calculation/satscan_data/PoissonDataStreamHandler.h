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

    virtual AbtractDataStreamGateway  * GetNewDataGateway() const;
    virtual AbtractDataStreamGateway  * GetNewSimulationDataGateway(const SimulationDataContainer_t& Container) const;
    virtual RandomizerContainer_t     & GetRandomizerContainer(RandomizerContainer_t& Container) const;
    virtual SimulationDataContainer_t & GetSimulationDataContainer(SimulationDataContainer_t& Container) const;
    virtual bool                        ReadData();
};
#endif
