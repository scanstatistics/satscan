//*****************************************************************************
#ifndef __BRUTEFORCEANALYSIS_H
#define __BRUTEFORCEANALYSIS_H
//*****************************************************************************
#include "Analysis.h"
#include "ptr_vector.h"
#include "Randomizer.h"

/** forward class declarations */
class CCluster;
class MostLikelyClustersContainer;
class AbstractDataSetGateway;
class DataSetInterface;

class CentroidNeighborCalculator;
class CentroidNeighbors;

/** Abstract base class which defines methods for calculating top clusters and
    simulated log likelihood ratios. */
class AbstractBruteForceAnalysis : public CAnalysis {
  protected:    
    typedef ptr_vector<CentroidNeighbors> NeighborContainer_t;

    NeighborContainer_t                       gNeighborInfo;
    std::auto_ptr<CentroidNeighborCalculator> gCentroidCalculator; //centroid neighbors calculator

    virtual double                      MonteCarlo(const DataSetInterface & Interface) = 0;
    virtual double                      MonteCarlo(tract_t tCenter, const AbstractDataSetGateway & DataGateway) = 0;

  public:
    AbstractBruteForceAnalysis(const CParameters& Parameters, const CSaTScanData& Data, BasePrint& PrintDirection);
    virtual ~AbstractBruteForceAnalysis();

    virtual void                        AllocateSimulationObjects(const AbstractDataSetGateway & DataGateway) = 0;
    virtual void                        AllocateTopClustersObjects(const AbstractDataSetGateway & DataGateway) = 0;
    virtual void                        FindTopClusters(const AbstractDataSetGateway & DataGateway, MLC_Collections_t& TopClustersContainers);
    double                              ExecuteSimulation(const AbstractDataSetGateway& DataGateway);
    virtual void                        AllocateAdditionalSimulationObjects(RandomizerContainer_t& Container) = 0;
};
//*****************************************************************************
#endif

