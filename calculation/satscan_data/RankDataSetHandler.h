//******************************************************************************
#ifndef __RankDataSetHandler_H
#define __RankDataSetHandler_H
//******************************************************************************
#include "DataSetHandler.h"
#include "RankRandomizer.h"

class RankDataSetHandler : public DataSetHandler {
  protected:
    virtual void                         AllocateCaseStructures(size_t iSetIndex) {/* no action */}
    virtual bool                         ParseCaseFileLine(DataSource& Source, tract_t& tid, count_t& nCount, Julian& nDate, measure_t& tContinuousVariable);
    virtual bool                         ReadCounts(size_t iSetIndex, DataSource& Source, const char* szDescription);
    virtual void                         SetRandomizers();

  public:
    RankDataSetHandler(CSaTScanData& DataHub, BasePrint& Print) : DataSetHandler(DataHub, Print) {}
    virtual ~RankDataSetHandler() {}

    virtual SimulationDataContainer_t  & AllocateSimulationData(SimulationDataContainer_t& Container) const;
    virtual AbstractDataSetGateway     & GetDataGateway(AbstractDataSetGateway& DataGatway) const;
    virtual AbstractDataSetGateway     & GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container) const;
    virtual double                       GetSimulationDataSetAllocationRequirements() const;
    virtual bool                         ReadData();
    virtual void                         SetPurelyTemporalSimulationData(SimulationDataContainer_t& SimDataContainer);
};
//******************************************************************************
#endif

