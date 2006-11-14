//******************************************************************************
#ifndef __AbstractCentricAnalysis_H
#define __AbstractCentricAnalysis_H
//******************************************************************************
#include "AbstractAnalysis.h"
#include "boost/shared_ptr.hpp"
#include "ptr_vector.h"

/** forward class declarations */
class AbstractDataSetGateway;
class CentroidNeighbors;
class CentroidNeighborCalculator;
class CCluster;
class MostLikelyClustersContainer;

/** Defines analysis class which evaluates real and simulated data for each
    centroid separate than other centroids. */
class AbstractCentricAnalysis : public AbstractAnalysis {
  public:
    typedef ptr_vector<CMeasureList>                    MeasureListContainer_t;
    typedef boost::shared_ptr<std::vector<double> >     CalculatedRatioContainer_t;
    typedef ptr_vector<AbstractDataSetGateway>          DataSetGatewayContainer_t;
    typedef ptr_vector<CCluster>                        ClustersContainer_t;

  protected:
    MeasureListContainer_t      gvMeasureLists;    /** collection of CMeasurelist objects, one for each replication */
    CalculatedRatioContainer_t  gCalculatedRatios; /** collection of replication log likelihood ratios */
    ClustersContainer_t         gRetainedClusters; /** collection of retained most likely clusters */

    // pure virtual functions
    virtual void             CalculateRatiosAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const DataSetGatewayContainer_t& vDataGateways) = 0;
    virtual void             CalculateTopClusterAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const AbstractDataSetGateway& DataGateway) = 0;
    virtual const CCluster & GetTopCalculatedCluster() = 0;
    virtual void             MonteCarloAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const DataSetGatewayContainer_t& vDataGateways) = 0;

    void                     ExecuteSimulationsAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const DataSetGatewayContainer_t& vDataGateways);

  public:
    AbstractCentricAnalysis(const CParameters& Parameters, const CSaTScanData& Data, BasePrint& PrintDirection);
    virtual ~AbstractCentricAnalysis();

    virtual void             ExecuteAboutCentroid(tract_t tCentroidIndex,
                                                  CentroidNeighborCalculator& CentroidCalculator,
                                                  const AbstractDataSetGateway& RealDataGateway,
                                                  const DataSetGatewayContainer_t& vSimDataGateways);
    virtual void             ExecuteAboutPurelyTemporalCluster(const AbstractDataSetGateway& DataGateway,
                                                               const DataSetGatewayContainer_t& vSimDataGateways);
    void                     RetrieveClusters(MostLikelyClustersContainer& TopClustersContainer);
    void                     RetrieveLoglikelihoodRatios(CalculatedRatioContainer_t& RatioContainer);

};
//******************************************************************************
#endif
