//******************************************************************************
#ifndef __OrdinalDataSetHandler_H
#define __OrdinalDataSetHandler_H
//******************************************************************************
#include "DataSetHandler.h"
#include "OrdinalDataRandomizer.h"

class OrdinalDataSetHandler : public DataSetHandler {
  protected:
    static const size_t                 gtMinimumCategories;
    static const count_t                gtMinimumCases;

    virtual void                        AllocateCaseStructures(size_t tSetIndex);
    virtual bool                        ParseCaseFileLine(StringParser& Parser, tract_t& tid,
                                                          count_t& nCount, Julian& nDate, measure_t& tContinuosVariable);
    virtual bool                        ReadCounts(size_t tSetIndex, FILE * fp, const char*);
    virtual void                        SetRandomizers();

  public:
    OrdinalDataSetHandler(CSaTScanData& DataHub, BasePrint& Print);
    virtual ~OrdinalDataSetHandler();

    virtual SimulationDataContainer_t & AllocateSimulationData(SimulationDataContainer_t& Container) const;
    virtual AbstractDataSetGateway     & GetDataGateway(AbstractDataSetGateway& DataGatway) const;
    virtual AbstractDataSetGateway     & GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container) const;
    virtual double                      GetSimulationDataSetAllocationRequirements() const;
    virtual bool                        ReadData();
    virtual void                        SetPurelyTemporalSimulationData(SimulationDataContainer_t& SimDataContainer);
};
//******************************************************************************
#endif
