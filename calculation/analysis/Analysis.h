//*****************************************************************************
#ifndef __ANALYSIS_H
#define __ANALYSIS_H
//*****************************************************************************
#include "AbstractAnalysis.h"
#include "SSException.h"
#include "MostLikelyClustersContainer.h"
#include "IntermediateClustersContainer.h"

/** forward class declarations */
class CCluster;
class MostLikelyClustersContainer;
class AbstractDataSetGateway;
class DataSetInterface;

/** Abstract base class which defines methods for calculating top clusters and
    simulated log likelihood ratios. */
class CAnalysis : public AbstractAnalysis {
  protected:
    virtual const SharedClusterVector_t CalculateTopClusters(tract_t tCenter, const AbstractDataSetGateway & DataGateway) = 0;
    virtual double                      MonteCarlo(const DataSetInterface & Interface) = 0;
    virtual double                      MonteCarlo(tract_t tCenter, const AbstractDataSetGateway & DataGateway) = 0;

  public:
    CAnalysis(const CParameters& Parameters, const CSaTScanData& Data, BasePrint& PrintDirection);
    virtual ~CAnalysis() {}

    virtual void                        AllocateSimulationObjects(const AbstractDataSetGateway & DataGateway) = 0;
    virtual void                        AllocateTopClustersObjects(const AbstractDataSetGateway & DataGateway) = 0;
    virtual void                        FindTopClusters(const AbstractDataSetGateway & DataGateway, MLC_Collections_t& TopClustersContainers);
    double                              ExecuteSimulation(const AbstractDataSetGateway& DataGateway);
};
//*****************************************************************************
#endif

