//******************************************************************************
#ifndef __BernoulliDataSetHandler_H
#define __BernoulliDataSetHandler_H
//******************************************************************************
#include "DataSetHandler.h"
#include "BernoulliRandomizer.h"

class BernoulliDataSetHandler : public DataSetHandler {
  protected:
    CountFileReadStatus                 ReadControlFile(RealDataSet& DataSet);
    virtual void                        SetRandomizers();
	virtual void                        removeDataSet(size_t iSetIndex);

  public:
    BernoulliDataSetHandler(CSaTScanData& DataHub, BasePrint& Print) : DataSetHandler(DataHub, Print) {}
    virtual ~BernoulliDataSetHandler() {}

    virtual SimulationDataContainer_t & AllocateSimulationData(SimulationDataContainer_t& Container) const;
    virtual void                        assignMetaData(RealDataContainer_t& Container) const;
    virtual AbstractDataSetGateway    & GetDataGateway(AbstractDataSetGateway& DataGatway) const;
    virtual AbstractDataSetGateway    & GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container, const RandomizerContainer_t& rContainer) const;
    virtual void                        RandomizeData(RandomizerContainer_t& Container, SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber) const;
    virtual bool                        ReadData();
};
//******************************************************************************
#endif
