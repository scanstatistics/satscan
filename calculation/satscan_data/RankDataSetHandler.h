//******************************************************************************
#ifndef __RankDataSetHandler_H
#define __RankDataSetHandler_H
//******************************************************************************
#include "DataSetHandler.h"
#include "RankRandomizer.h"

class RankDataSetHandler : public DataSetHandler {
  protected:
    virtual CountFileReadStatus          ReadCounts(RealDataSet& DataSet, DataSource& Source);
    virtual RecordStatusType             RetrieveCaseRecordData(DataSource& Source, tract_t& tid, count_t& nCount, Julian& nDate, measure_t& tContinuousVariable);
    virtual void                         SetRandomizers();

  public:
    RankDataSetHandler(CSaTScanData& DataHub, BasePrint& Print) : DataSetHandler(DataHub, Print) {}
    virtual ~RankDataSetHandler() {}

    virtual SimulationDataContainer_t  & AllocateSimulationData(SimulationDataContainer_t& Container) const;
    virtual void                         assignMetaLocationData(RealDataContainer_t& Container) const;
    virtual AbstractDataSetGateway     & GetDataGateway(AbstractDataSetGateway& DataGatway) const;
    virtual AbstractDataSetGateway     & GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container, const RandomizerContainer_t& rContainer) const;
    virtual void                         RandomizeData(RandomizerContainer_t& Container, SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber) const;
    virtual bool                         ReadData();
    virtual void                         SetPurelyTemporalSimulationData(SimulationDataContainer_t& SimDataContainer);
};
//******************************************************************************
#endif

