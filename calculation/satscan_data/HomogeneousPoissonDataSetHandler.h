//******************************************************************************
#ifndef __HomogeneousPoissonDataSetHandler_H
#define __HomogeneousPoissonDataSetHandler_H
//******************************************************************************
#include "DataSetHandler.h"
#include "HomogeneousPoissonRandomizer.h"

class IdentifiersManager;
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
	IdentifiersManager                & _identifier_mgr;
    GInfo                             & gCentroidsHandler;

    bool                                ReadCoordinates(RealDataSet& DataSet, DataSource& Source);
    bool                                ReadCoordinatesFile(RealDataSet& DataSet);
    bool                                ReadGridFile(DataSource& Source);
    bool                                ReadCartesianCoordinates(DataSource& Source, std::vector<double>& vCoordinates, short& iScanCount, short iWordOffSet);
    virtual void                        SetRandomizers();

  public:
    HomogeneousPoissonDataSetHandler(CSaTScanData& DataHub, IdentifiersManager& identifierMgr, GInfo& CentroidsHandler, BasePrint& Print);
    virtual ~HomogeneousPoissonDataSetHandler() {}

    virtual SimulationDataContainer_t & AllocateSimulationData(SimulationDataContainer_t& Container) const;
    virtual void                        assignMetaData(RealDataContainer_t& Container) const {}
    virtual AbstractDataSetGateway    & GetDataGateway(AbstractDataSetGateway& DataGatway) const;
    bool                                isPointInRegions(double x, double y) const;
    virtual RandomizerContainer_t     & GetRandomizerContainer(RandomizerContainer_t& Container) const;
    virtual AbstractDataSetGateway    & GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container, const RandomizerContainer_t& rContainer) const;
    double                              getTotalArea() const;
    virtual void                        RandomizeData(RandomizerContainer_t& Container, SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber) const;
    virtual bool                        ReadData();
};
//******************************************************************************
#endif

