//******************************************************************************
#ifndef __RankDataSetHandler_H
#define __RankDataSetHandler_H
//******************************************************************************
#include "DataSetHandler.h"
#include "ContinuousVariableRandomizer.h"

class RankDataSetHandler : public DataSetHandler {
  protected:
    virtual void                        AllocateCaseStructures(size_t iSetIndex);
    virtual bool                        ParseCaseFileLine(StringParser& Parser, tract_t& tid,
                                                          count_t& nCount, Julian& nDate,
                                                          measure_t& tContinuosVariable);
    virtual bool                        ReadCounts(size_t iSetIndex, FILE* fp, const char* szDescription);
    virtual void                        SetRandomizers();

  public:
    RankDataSetHandler(CSaTScanData& DataHub, BasePrint& Print);
    virtual ~RankDataSetHandler();

    virtual AbtractDataSetGateway  * GetNewDataGateway() const;
    virtual AbtractDataSetGateway  * GetNewSimulationDataGateway(const SimulationDataContainer_t& Container) const;
    virtual SimulationDataContainer_t & GetSimulationDataContainer(SimulationDataContainer_t& Container) const;
    virtual bool                        ReadData();
    virtual void                        SetPurelyTemporalSimulationData(SimulationDataContainer_t& SimDataContainer);
};
//******************************************************************************
#endif

