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

    virtual SimulationDataContainer_t & AllocateSimulationData(SimulationDataContainer_t& Container) const;
    virtual AbtractDataSetGateway     & GetDataGateway(AbtractDataSetGateway& DataGatway) const;
    virtual AbtractDataSetGateway     & GetSimulationDataGateway(AbtractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container) const;
    virtual double                      GetSimulationDataSetAllocationRequirements() const;
    virtual bool                        ReadData();
    virtual void                        SetPurelyTemporalSimulationData(SimulationDataContainer_t& SimDataContainer);
};
//******************************************************************************
#endif

