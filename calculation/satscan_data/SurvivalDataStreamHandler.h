//---------------------------------------------------------------------------
#ifndef __SurvivalDataStreamHandler_H
#define __SurvivalDataStreamHandler_H
//---------------------------------------------------------------------------
#include "DataStreamHandler.h"
#include "ContinuousVariableRandomizer.h"

class SurvivalDataStreamHandler : public DataStreamHandler {
  protected:
    std::vector<SurvivalRandomizer>     gvDataStreamRandomizers;

    virtual void                        AllocateCaseStructures(unsigned int iStream);
    virtual bool                        ParseCaseFileLine(StringParser& Parser, tract_t& tid,
                                                          count_t& nCount, Julian& nDate,
                                                          measure_t& tContinuosVariable,
                                                          count_t& tCensored);
    virtual bool                        ReadCounts(size_t tStream, FILE * fp, const char* szDescription);
    virtual void                        SetRandomizers();

  public:
    SurvivalDataStreamHandler(CSaTScanData & Data, BasePrint * pPrint);
    virtual ~SurvivalDataStreamHandler();

    void                                AllocatePTSimulationMeasures();
    virtual void                        AllocateSimulationMeasures();
    virtual void                        AllocateSimulationStructures();
    virtual AbtractDataStreamGateway  * GetNewDataGateway();
    virtual AbtractDataStreamGateway  * GetNewSimulationDataGateway();
    virtual void                        RandomizeData(unsigned int iSimulationNumber);
    virtual bool                        ReadData();
    virtual void                        SetPurelyTemporalSimulationData();
};
//---------------------------------------------------------------------------
#endif
