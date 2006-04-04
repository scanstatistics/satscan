//******************************************************************************
#ifndef __NormalDataSetHandler_H
#define __NormalDataSetHandler_H
//******************************************************************************
#include "DataSetHandler.h"
#include "NormalRandomizer.h"

class NormalDataSetHandler : public DataSetHandler {
  protected:
    virtual void                        AllocateCaseStructures(RealDataSet& DataSet) {/* no action */}
    virtual bool                        ReadCounts(RealDataSet& DataSet, DataSource& Source, const char* szDescription);
    virtual DataSetHandler::RecordStatusType RetrieveCaseRecordData(DataSource& Source, tract_t& tid, count_t& nCount, Julian& nDate, measure_t& tContinuousVariable);
    virtual void                        SetRandomizers();

  public:
    NormalDataSetHandler(CSaTScanData& DataHub, BasePrint& Print) : DataSetHandler(DataHub, Print) {}
    virtual ~NormalDataSetHandler() {}

    virtual SimulationDataContainer_t & AllocateSimulationData(SimulationDataContainer_t& Container) const;
    virtual AbstractDataSetGateway    & GetDataGateway(AbstractDataSetGateway& DataGatway) const;
    virtual AbstractDataSetGateway    & GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container) const;
    virtual bool                        ReadData();
    virtual void                        SetPurelyTemporalMeasureData(RealDataSet & DataSet);
    virtual void                        SetPurelyTemporalSimulationData(SimulationDataContainer_t& SimDataContainer);
};
//******************************************************************************
#endif

