//---------------------------------------------------------------------------
#ifndef __NormalDataStreamHandler_H
#define __NormalDataStreamHandler_H
//---------------------------------------------------------------------------
#include "DataStreamHandler.h"
#include "ContinuousVariableRandomizer.h"

class NormalDataStreamHandler : public DataStreamHandler {
  protected:
    std::vector<NormalRandomizer>       gvDataStreamRandomizers;

    virtual void                        AllocateCaseStructures(unsigned int iStream);
    virtual bool                        ParseCaseFileLine(StringParser& Parser, tract_t& tid, count_t& nCount, Julian& nDate, measure_t& tContinuosVariable);
    virtual bool                        ReadCounts(size_t tStream, FILE * fp, const char* szDescription);
    virtual void                        SetRandomizers();

  public:
    NormalDataStreamHandler(CSaTScanData & Data, BasePrint * pPrint);
    virtual ~NormalDataStreamHandler();

    void                                AllocatePTSimulationMeasures();
    virtual void                        AllocateSimulationMeasures();
    virtual void                        AllocateSimulationStructures();
    virtual AbtractDataStreamGateway  * GetNewDataGateway();
    virtual AbtractDataStreamGateway  * GetNewSimulationDataGateway();
    virtual void                        RandomizeData(unsigned int iSimulationNumber);
    virtual bool                        ReadData();
    virtual void                        SetPurelyTemporalMeasureData(DataStream & thisStream);
    virtual void                        SetPurelyTemporalSimulationData();
};
//---------------------------------------------------------------------------
#endif
