//---------------------------------------------------------------------------
#ifndef __NormalDataStreamHandler_H
#define __NormalDataStreamHandler_H
//---------------------------------------------------------------------------
#include "DataStreamHandler.h"
#include "ContinuousVariableRandomizer.h"

class NormalDataStreamHandler : public DataStreamHandler {
  protected:
    virtual void                        AllocateCaseStructures(unsigned int iStream);
    virtual bool                        ParseCaseFileLine(StringParser& Parser, tract_t& tid, count_t& nCount, Julian& nDate, measure_t& tContinuosVariable);
    virtual bool                        ReadCounts(size_t tStream, FILE * fp, const char* szDescription);
    virtual void                        SetRandomizers();

  public:
    NormalDataStreamHandler(CSaTScanData& Data, BasePrint& pPrint);
    virtual ~NormalDataStreamHandler();

    virtual AbtractDataStreamGateway  * GetNewDataGateway() const;
    virtual AbtractDataStreamGateway  * GetNewSimulationDataGateway(const SimulationDataContainer_t& Container) const;
    virtual SimulationDataContainer_t & GetSimulationDataContainer(SimulationDataContainer_t& Container) const;
    virtual bool                        ReadData();
    virtual void                        SetPurelyTemporalMeasureData(RealDataStream & thisRealStream);
    virtual void                        SetPurelyTemporalSimulationData(SimulationDataContainer_t& SimDataContainer);
};
//---------------------------------------------------------------------------
#endif
