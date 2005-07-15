//******************************************************************************
#ifndef __BernoulliDataSetHandler_H
#define __BernoulliDataSetHandler_H
//******************************************************************************
#include "DataSetHandler.h"
#include "BernoulliRandomizer.h"

class BernoulliDataSetHandler : public DataSetHandler {
  protected:
    void                                AllocateControlStructures(size_t tSetIndex);
    bool                                ReadControlFile(size_t tSetIndex);
    virtual void                        SetRandomizers();

  public:
    BernoulliDataSetHandler(CSaTScanData& DataHub, BasePrint& Print);
    virtual ~BernoulliDataSetHandler();

    virtual SimulationDataContainer_t & AllocateSimulationData(SimulationDataContainer_t& Container) const;
    virtual AbstractDataSetGateway     & GetDataGateway(AbstractDataSetGateway& DataGatway) const;
    virtual AbstractDataSetGateway     & GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container) const;
    virtual double                      GetSimulationDataSetAllocationRequirements() const;
    virtual bool                        ReadData();
};
//******************************************************************************
#endif
