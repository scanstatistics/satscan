//******************************************************************************
#ifndef __AbstractCentricAnalysis_H
#define __AbstractCentricAnalysis_H
//******************************************************************************
#include "AbstractAnalysis.h"
#include "boost/shared_ptr.hpp"

/** forward class declarations */
class AbtractDataSetGateway;
class CentroidNeighbors;
class CentroidNeighborCalculator;
class CCluster;
class MostLikelyClustersContainer;

/** Defines analysis class which evaluates real and simulated data for each
    centroid separate than other centroids. */
class AbstractCentricAnalysis : public AbstractAnalysis {
  public:
    typedef ZdPointerVector<CMeasureList>               MeasureListContainer_t;
    typedef boost::shared_ptr<std::vector<double> >     CalculatedRatioContainer_t;
    typedef std::vector<CentroidNeighbors>              CentroidDefinitionContainer_t;
    typedef ZdPointerVector<AbtractDataSetGateway>      DataSetGatewayContainer_t;
    typedef ZdPointerVector<CCluster>                   ClustersContainer_t;

  protected:
    MeasureListContainer_t      gvMeasureLists;    /** collection of CMeasurelist objects, one for each replication */
    CalculatedRatioContainer_t  gCalculatedRatios; /** collection of replication log likelihood ratios */
    ClustersContainer_t         gRetainedClusters; /** collection of retained most likely clusters */

    // pure virtual functions
    virtual void             CalculateRatiosAboutCentroidDefinition(CentroidDefinitionContainer_t& vCentroid, const DataSetGatewayContainer_t& vDataGateways) = 0;
    virtual const CCluster & CalculateTopClusterAboutCentroidDefinition(CentroidDefinitionContainer_t& vCentroid, const AbtractDataSetGateway& DataGateway) = 0;
    virtual void             MonteCarloAboutCentroidDefinition(CentroidDefinitionContainer_t& vCentroid, const DataSetGatewayContainer_t& vDataGateways) = 0;

    void                     ExecuteSimulationsAboutCentroidDefinition(CentroidDefinitionContainer_t& vCentroid, const DataSetGatewayContainer_t& vDataGateways);

  public:
    AbstractCentricAnalysis(const CParameters& Parameters, const CSaTScanData& Data, BasePrint& PrintDirection);
    virtual ~AbstractCentricAnalysis();

    virtual void             CalculatePurelyTemporalCluster(MostLikelyClustersContainer& TopClustersContainer, const AbtractDataSetGateway& DataGateway);
    virtual void             ExecuteAboutCentroids(tract_t tCentroidIndex,
                                                   CentroidNeighborCalculator& CentroidCalculator,
                                                   const AbtractDataSetGateway& RealDataGateway,
                                                   const DataSetGatewayContainer_t& vSimDataGateways);
    void                     RetrieveClusters(MostLikelyClustersContainer& TopClustersContainer);
    void                     RetrieveLoglikelihoodRatios(CalculatedRatioContainer_t& RatioContainer);

};
//******************************************************************************
#endif
