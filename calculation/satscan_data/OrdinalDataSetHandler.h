//******************************************************************************
#ifndef __OrdinalDataSetHandler_H
#define __OrdinalDataSetHandler_H
//******************************************************************************
#include "DataSetHandler.h"
#include "OrdinalDataRandomizer.h"

class DataSource;     /** forward class definition */

class OrdinalDataSetHandler : public DataSetHandler {
  protected:
    static const size_t                 gtMinimumCategories;
    static const count_t                gtMinimumCases;

    virtual void                        AllocateCaseStructures(size_t tSetIndex) {/*no action*/}
    virtual bool                        ParseCaseFileLine(DataSource& Source, tract_t& tid, count_t& nCount, Julian& nDate, measure_t& tContinuousVariable);
    virtual bool                        ReadCounts(size_t tSetIndex, DataSource& Source, const char*);
    virtual void                        SetRandomizers();

  public:
    OrdinalDataSetHandler(CSaTScanData& DataHub, BasePrint& Print) : DataSetHandler(DataHub, Print) {}
    virtual ~OrdinalDataSetHandler() {}

    virtual SimulationDataContainer_t & AllocateSimulationData(SimulationDataContainer_t& Container) const;
    virtual AbstractDataSetGateway     & GetDataGateway(AbstractDataSetGateway& DataGatway) const;
    virtual AbstractDataSetGateway     & GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container) const;
    virtual double                      GetSimulationDataSetAllocationRequirements() const;
    virtual bool                        ReadData();
    virtual void                        SetPurelyTemporalSimulationData(SimulationDataContainer_t& SimDataContainer);
};
//******************************************************************************
#endif
