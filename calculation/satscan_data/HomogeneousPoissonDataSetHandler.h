//******************************************************************************
#ifndef __HomogeneousPoissonDataSetHandler_H
#define __HomogeneousPoissonDataSetHandler_H
//******************************************************************************
#include "DataSetHandler.h"
#include "HomogeneousPoissonRandomizer.h"

class TractHandler;
class GInfo;

/** Data set handler for the Homogeneous Poisson probablity model, expanding on
    functionality of base class DataSetHandler.
    Defines processes for:
     - reading population data from file(s) into datasets' structures
     - allocation of dataset's randomizers
     - allocation of dataset objects used during simulations 
     - allocation of gateway objects used by analysis object to access data
       set structures for real data and simulation data */
class HomogeneousPoissonDataSetHandler : public DataSetHandler {
  protected:
    ObserverableRegionContainer_t       gPolygons;
    TractHandler                      & gTractHandler;
    GInfo                             & gCentroidsHandler;

    bool                                ReadGridFile(DataSource& Source);
    bool                                ReadCartesianCoordinates(DataSource& Source, std::vector<double>& vCoordinates, short& iScanCount, short iWordOffSet);
    virtual bool                        ReadCounts(RealDataSet& DataSet, DataSource& Source);
    virtual void                        SetRandomizers();

  public:
    HomogeneousPoissonDataSetHandler(CSaTScanData& DataHub, TractHandler& TractHandler, GInfo& CentroidsHandler, BasePrint& Print);
    virtual ~HomogeneousPoissonDataSetHandler() {}

    virtual SimulationDataContainer_t & AllocateSimulationData(SimulationDataContainer_t& Container) const;
    virtual void                        assignMetaLocationData(RealDataContainer_t& Container) const {}
    virtual AbstractDataSetGateway    & GetDataGateway(AbstractDataSetGateway& DataGatway) const;
    bool                                isPointInRegions(double x, double y) const;
    virtual RandomizerContainer_t     & GetRandomizerContainer(RandomizerContainer_t& Container) const;
    virtual AbstractDataSetGateway    & GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container) const;
    double                              getTotalArea() const;
    virtual void                        RandomizeData(RandomizerContainer_t& Container, SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber) const;
    virtual bool                        ReadData();
};
//******************************************************************************
#endif

