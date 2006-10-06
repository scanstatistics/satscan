//******************************************************************************
#ifndef __BernoulliDataSetHandler_H
#define __BernoulliDataSetHandler_H
//******************************************************************************
#include "DataSetHandler.h"
#include "BernoulliRandomizer.h"

class BernoulliDataSetHandler : public DataSetHandler {
  protected:
    bool                                ReadControlFile(RealDataSet& DataSet);
    virtual void                        SetRandomizers();

  public:
    BernoulliDataSetHandler(CSaTScanData& DataHub, BasePrint& Print) : DataSetHandler(DataHub, Print) {}
    virtual ~BernoulliDataSetHandler() {}

    virtual SimulationDataContainer_t & AllocateSimulationData(SimulationDataContainer_t& Container) const;
    virtual void                        assignMetaLocationData(RealDataContainer_t& Container) const;
    virtual AbstractDataSetGateway    & GetDataGateway(AbstractDataSetGateway& DataGatway) const;
    virtual AbstractDataSetGateway    & GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container) const;
    virtual void                        RandomizeData(RandomizerContainer_t& Container, SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber) const;
    virtual bool                        ReadData();
};
//******************************************************************************
#endif
