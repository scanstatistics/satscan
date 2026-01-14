//******************************************************************************
#ifndef SpaceTimePermutationDataSetHandlerH
#define SpaceTimePermutationDataSetHandlerH
//******************************************************************************
#include "DataSetHandler.h"
#include "SpaceTimeRandomizer.h"

class SpaceTimePermutationDataSetHandler : public DataSetHandler {
  protected:
    virtual DataSetHandler::CountFileReadStatus  ReadCounts(RealDataSet& DataSet, DataSource& Source);
    virtual void                                 SetRandomizers();

  public:
    SpaceTimePermutationDataSetHandler(CSaTScanData& DataHub, BasePrint& Print);
    virtual ~SpaceTimePermutationDataSetHandler();

    virtual SimulationDataContainer_t & AllocateSimulationData(SimulationDataContainer_t& Container) const;
    virtual void                        assignMetaData(RealDataContainer_t& Container) const;
    virtual AbstractDataSetGateway    & GetDataGateway(AbstractDataSetGateway& DataGatway) const;
    virtual AbstractDataSetGateway    & GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container, const RandomizerContainer_t& rContainer) const;
    virtual void                        RandomizeData(RandomizerContainer_t& Container, SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber) const;
    virtual bool                        ReadData();
};
//******************************************************************************
#endif

