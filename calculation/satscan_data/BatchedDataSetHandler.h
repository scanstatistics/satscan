//******************************************************************************
#ifndef __BatchedDataSetHandler_H
#define __BatchedDataSetHandler_H
//******************************************************************************
#include "DataSetHandler.h"
#include "BatchedRandomizer.h"

class BatchedDataSetHandler : public DataSetHandler {
  protected:
    virtual CountFileReadStatus         ReadCounts(RealDataSet& DataSet, DataSource& Source);
    virtual RecordStatusType            RetrieveCaseRecordData(DataSource& Source, tract_t& tid, count_t& trapCount, Julian& nDate, count_t& batchSize, count_t& positive);
    virtual void                        SetRandomizers();

  public:
    BatchedDataSetHandler(CSaTScanData& DataHub, BasePrint& Print) : DataSetHandler(DataHub, Print) {}
    virtual ~BatchedDataSetHandler() {}

    virtual SimulationDataContainer_t & AllocateSimulationData(SimulationDataContainer_t& Container) const;
    virtual void                        assignMetaData(RealDataContainer_t& Container) const;
    virtual AbstractDataSetGateway    & GetDataGateway(AbstractDataSetGateway& DataGatway) const;
    virtual AbstractDataSetGateway    & GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container, const RandomizerContainer_t& rContainer) const;
    virtual void                        RandomizeData(RandomizerContainer_t& Container, SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber) const;
    virtual bool                        ReadData();
    virtual void                        SetPurelyTemporalSimulationData(SimulationDataContainer_t& SimDataContainer);
    virtual void                        SetPurelyTemporalMeasureData(RealDataSet& thisRealSet);
};
//******************************************************************************
#endif
