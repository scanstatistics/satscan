//---------------------------------------------------------------------------
#ifndef __RankDataStreamHandler_H
#define __RankDataStreamHandler_H
//---------------------------------------------------------------------------
#include "DataStreamHandler.h"
#include "ContinuousVariableRandomizer.h"

class RankDataStreamHandler : public DataStreamHandler {
  protected:
    ZdPointerVector<RankRandomizer>     gvDataStreamRandomizers;

    virtual void                        AllocateCaseStructures(unsigned int iStream);
    virtual bool                        ParseCaseFileLine(StringParser& Parser, tract_t& tid,
                                                          count_t& nCount, Julian& nDate,
                                                          measure_t& tContinuosVariable);
    virtual bool                        ReadCounts(size_t tStream, FILE * fp, const char* szDescription);
    virtual void                        SetRandomizers();

  public:
    RankDataStreamHandler(CSaTScanData & Data, BasePrint * pPrint);
    virtual ~RankDataStreamHandler();

    virtual AbtractDataStreamGateway  * GetNewDataGateway() const;
    virtual AbtractDataStreamGateway  * GetNewSimulationDataGateway(const SimulationDataContainer_t& Container) const;
    virtual RandomizerContainer_t     & GetRandomizerContainer(RandomizerContainer_t& Container) const;
    virtual SimulationDataContainer_t & GetSimulationDataContainer(SimulationDataContainer_t& Container) const;
    virtual bool                        ReadData();
    virtual void                        SetPurelyTemporalSimulationData(SimulationDataContainer_t& SimDataContainer);
};
//---------------------------------------------------------------------------
#endif
