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

    virtual AbtractDataSetGateway  * GetNewDataGateway() const;
    virtual AbtractDataSetGateway  * GetNewSimulationDataGateway(const SimulationDataContainer_t& Container) const;
    virtual SimulationDataContainer_t & GetSimulationDataContainer(SimulationDataContainer_t& Container) const;
    virtual bool                        ReadData();
};
//******************************************************************************
#endif
