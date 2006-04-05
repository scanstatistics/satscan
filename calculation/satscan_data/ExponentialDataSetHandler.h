//******************************************************************************
#ifndef __ExponentialDataSetHandler_H
#define __ExponentialDataSetHandler_H
//******************************************************************************
#include "DataSetHandler.h"
#include "ExponentialRandomizer.h"

class ExponentialDataSetHandler : public DataSetHandler {
  protected:
    static const count_t                gtMinimumNotCensoredCases;

    virtual void                        AllocateCaseStructures(RealDataSet& DataSet) {/* no action */}
    virtual bool                        ReadCounts(RealDataSet& DataSet, DataSource& Source, const char* szDescription);
    virtual RecordStatusType            RetrieveCaseRecordData(DataSource& Source, tract_t& tid, count_t& tPatients, Julian& nDate, measure_t& tContinuousVariable, count_t& tCensorAttribute);
    virtual void                        SetRandomizers();

  public:
    ExponentialDataSetHandler(CSaTScanData& DataHub, BasePrint& Print) : DataSetHandler(DataHub, Print) {}
    virtual ~ExponentialDataSetHandler() {}

    virtual SimulationDataContainer_t & AllocateSimulationData(SimulationDataContainer_t& Container) const;
    virtual AbstractDataSetGateway    & GetDataGateway(AbstractDataSetGateway& DataGatway) const;
    virtual AbstractDataSetGateway    & GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container) const;
    virtual bool                        ReadData();
    virtual void                        SetPurelyTemporalSimulationData(SimulationDataContainer_t& SimDataContainer);
};
//******************************************************************************
#endif
