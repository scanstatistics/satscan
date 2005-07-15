//*****************************************************************************
#ifndef __ANALYSIS_H
#define __ANALYSIS_H
//*****************************************************************************
#include "AbstractAnalysis.h"

/** forward class declarations */
class CCluster;
class MostLikelyClustersContainer;
class AbstractDataSetGateway;
class DataSetInterface;

/** Abstract base class which defines methods for calculating top clusters and
    simulated log likelihood ratios. */
class CAnalysis : public AbstractAnalysis {
  protected:
    virtual const CCluster            & CalculateTopCluster(tract_t tCenter, const AbstractDataSetGateway & DataGateway) = 0;

  public:
    CAnalysis(const CParameters& Parameters, const CSaTScanData& Data, BasePrint& PrintDirection);
    virtual ~CAnalysis();

    virtual void                        AllocateSimulationObjects(const AbstractDataSetGateway & DataGateway) = 0;
    virtual void                        AllocateTopClustersObjects(const AbstractDataSetGateway & DataGateway) = 0;
    double                              ExecuteSimulation(const AbstractDataSetGateway& DataGateway);
    virtual void                        FindTopClusters(const AbstractDataSetGateway & DataGateway, MostLikelyClustersContainer& TopClustersContainer);
    virtual double                      FindTopRatio(const AbstractDataSetGateway & DataGateway);
    virtual double                      MonteCarlo(const DataSetInterface & Interface) = 0;

};
//*****************************************************************************
#endif

