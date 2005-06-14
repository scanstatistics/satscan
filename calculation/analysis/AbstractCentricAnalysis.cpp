//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "cluster.h"
#include "AbstractCentricAnalysis.h"
#include "MeasureList.h"
#include "MostLikelyClustersContainer.h"
#include "SaTScanData.h"

/** constructor */
AbstractCentricAnalysis::AbstractCentricAnalysis(const CParameters& Parameters, const CSaTScanData& Data, BasePrint& PrintDirection)
                        :AbstractAnalysis(Parameters, Data, PrintDirection) {}
                        
/** destructor */
AbstractCentricAnalysis::~AbstractCentricAnalysis() {}

/** Calculates most likely temporal cluster - adds to MostLikelyClustersContainer object.
    No action taken in base class. */
void AbstractCentricAnalysis::CalculatePurelyTemporalCluster(MostLikelyClustersContainer&, const AbtractDataSetGateway&) {}

/** Calculates most likely cluster and evaluates simulated data about 'tCentroidIndex'.
    Cluster object added to TopClustersContainer if significant cluster found. Simulated
    loglikelihood ratios recorded to CalculatedRatioContainer_t class member. */
void AbstractCentricAnalysis::ExecuteAboutCentroids(tract_t tCentroidIndex,
                                                    MostLikelyClustersContainer& TopClustersContainer,
                                                    CentroidNeighborCalculator& CentroidCalculator,
                                                    const AbtractDataSetGateway& RealDataGateway,
                                                    const DataSetGatewayContainer_t& vSimDataGateways) {
  try {
    //centroid neighbor information objects
    CentroidDefinitionContainer_t  CentroidDefs(gDataHub.GetParameters().GetNumTotalEllipses() + 1);

    //calculate neigbor about current centroid
    for (int j=0; j <= gParameters.GetNumTotalEllipses(); ++j)
       CentroidCalculator.CalculateNeighborsAboutCentroid(j, tCentroidIndex, CentroidDefs[j]);
    // find top cluster about current centroid
    const CCluster& TopCluster = CalculateTopClusterAboutCentroidDefinition(CentroidDefs, RealDataGateway);
    if (TopCluster.ClusterDefined()) {
      //calculates radius - we'll possibly need to know this later when restricting overlap and reporting
      const_cast<CCluster&>(TopCluster).SetCartesianRadius(gDataHub, CentroidDefs[TopCluster.GetEllipseOffset()]);
      TopClustersContainer.Add(TopCluster);
    }
    //perform simulations about current centroid
    ExecuteSimulationsAboutCentroidDefinition(CentroidDefs, vSimDataGateways);
  }
  catch (ZdException &x) {
    x.AddCallpath("ExecuteAboutCentroids()","AbstractCentricAnalysis");
    throw;
  }
}

/** Executes simulation. Calls MonteCarlo() for analyses that can utilize
    CMeasureList class or FindTopRatio() for analyses which must perform
    simulations by the same algorithm as the real data. */
void AbstractCentricAnalysis::ExecuteSimulationsAboutCentroidDefinition(CentroidDefinitionContainer_t& vCentroid, const DataSetGatewayContainer_t& vDataGateways) {
  if (geReplicationsProcessType == MeasureListEvaluation)
    MonteCarloAboutCentroidDefinition(vCentroid, vDataGateways);
  else
    CalculateRatiosAboutCentroidDefinition(vCentroid, vDataGateways);
}

/** Retrieves calculated loglikehood ratio from class CalculatedRatioContainer_t object.
    If passed CalculatedRatioContainer_t already contains data, combines data with class
    object; else calculates ratios as needed and assigns to passed object. */
void AbstractCentricAnalysis::RetrieveLoglikelihoodRatios(CalculatedRatioContainer_t& Ratios) {
  if (Ratios.get()) {
    std::vector<double>::iterator   itrRatios=Ratios->begin(), itrRatios_end=Ratios->end();
    //aleady has someones results - combine with this analysis results
    if (geReplicationsProcessType == MeasureListEvaluation) {
      MeasureListContainer_t::iterator   itr=gvMeasureLists.begin(), itr_end=gvMeasureLists.end();
      for (; itr != itr_end; ++itr, ++itrRatios)
        *itrRatios = std::max(*itrRatios, (*itr)->GetMaximumLogLikelihoodRatio());
    }
    else {
      std::vector<double>::iterator   itr=gCalculatedRatios->begin(), itr_end=gCalculatedRatios->end();
      for (; itr != itr_end; ++itr, ++itrRatios)
        *itrRatios = std::max(*itrRatios, *itr);
    }
  }
  else {
    if (geReplicationsProcessType == MeasureListEvaluation) {
      MeasureListContainer_t::iterator   itr=gvMeasureLists.begin(), itr_end=gvMeasureLists.end();
      Ratios.reset(new std::vector<double>(gParameters.GetNumReplicationsRequested(), 0));
      std::vector<double>::iterator   itrRatios=Ratios->begin(), itrRatios_end=Ratios->end();
      for (; itr != itr_end; ++itr, ++itrRatios)
        *itrRatios = (*itr)->GetMaximumLogLikelihoodRatio();
    }
    else
      Ratios = gCalculatedRatios;
  }
}

