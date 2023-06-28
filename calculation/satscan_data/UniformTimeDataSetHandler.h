//******************************************************************************
#ifndef __UniformTimeDataSetHandler_H
#define __UniformTimeDataSetHandler_H
//******************************************************************************
#include "DataSetHandler.h"

/** Data set handler for the Poisson probablity model, expanding on
    functionality of base class DataSetHandler.
    Defines processes for:
     - reading population data from file(s) into datasets' structures
     - allocation of dataset's randomizers
     - allocation of dataset objects used during simulations 
     - allocation of gateway objects used by analysis object to access data
       set structures for real data and simulation data */
class UniformTimeDataSetHandler : public DataSetHandler {
  protected:
    bool                                CreatePopulationData(RealDataSet& DataSet);
    virtual void                        SetRandomizers();

  public:
    UniformTimeDataSetHandler(CSaTScanData& DataHub, BasePrint& Print) : DataSetHandler(DataHub, Print) {}
    virtual ~UniformTimeDataSetHandler() {}

    virtual SimulationDataContainer_t & AllocateSimulationData(SimulationDataContainer_t& Container) const;
    virtual void                        assignMetaData(RealDataContainer_t& Container) const;
    virtual AbstractDataSetGateway    & GetDataGateway(AbstractDataSetGateway& DataGatway) const;
    virtual AbstractDataSetGateway    & GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container, const RandomizerContainer_t& rContainer) const;
    virtual void                        RandomizeData(RandomizerContainer_t& Container, SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber) const;
    virtual bool                        ReadData();
};
//******************************************************************************
#endif

