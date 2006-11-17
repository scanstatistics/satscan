//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SVTTCentricAnalysis.h"
#include "SSException.h"

/** constructor */
SpatialVarTempTrendCentricAnalysis::SpatialVarTempTrendCentricAnalysis(const CParameters& Parameters,
                                                           const CSaTScanData& Data,
                                                           BasePrint& PrintDirection,
                                                           const AbstractDataSetGateway& RealDataGateway,
                                                           const DataSetGatewayContainer_t& vSimDataGateways)
                             :AbstractCentricAnalysis(Parameters, Data, PrintDirection) {
  try {
    Setup(RealDataGateway, vSimDataGateways);
  }
  catch (prg_exception& x) {
    x.addTrace("constructor()","SpatialVarTempTrendCentricAnalysis");
    throw;
  }
}

/** destructor */
SpatialVarTempTrendCentricAnalysis::~SpatialVarTempTrendCentricAnalysis() {}

/** Calculates greatest loglikelihood ratios about centroid for each defined data set gateway object,
    storing maximum ratio for each simulation in class member gCalculatedRatios.
    Caller is responsible for ensuring:
    1) 'vDataGateways' contains a number of elements equal to number of requested simulations
    2) DataSetInterface objects are assigned to appropriate structures used to accumulate cluster data
    3) RetrieveLoglikelihoodRatios() has not been called yet */
void SpatialVarTempTrendCentricAnalysis::CalculateRatiosAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const DataSetGatewayContainer_t& vDataGateways) {
  DataSetGatewayContainer_t::const_iterator  itrGateway, itrGatewayEnd=vDataGateways.end();
  std::vector<double>::iterator              itrLoglikelihoodRatios;
  tract_t                                    t, tNumNeighbors;

  //perform simulations about current centroid
  itrGateway=vDataGateways.begin();
  itrLoglikelihoodRatios=gCalculatedRatios->begin();
  for (; itrGateway != itrGatewayEnd; ++itrGateway, ++itrLoglikelihoodRatios) {
     gClusterData->InitializeSVTTData(*(*itrGateway));
     tNumNeighbors = CentroidDef.GetNumNeighbors();
     for (t=0; t < tNumNeighbors; ++t) {
        //update cluster data
        gClusterData->AddNeighborData(CentroidDef.GetNeighborTractIndex(t), *(*itrGateway));
        //calculate loglikehood ratio and compare against current top cluster
        *itrLoglikelihoodRatios = std::max(*itrLoglikelihoodRatios, gClusterData->CalculateSVTTLoglikelihoodRatio(*gpLikelihoodCalculator, *(*itrGateway)));
     }
     //NOTE: This process assumes no-compactness correction for ellipses - otherwise we would
     //      use an IntermediateClustersContainer, as used in other top cluster method to
     //      calculates most likely cluster among circle and ellipses. Not sure how to code this
     //      should we want compactnes correction later on.
  }
}

/** Calculates clustering with greastest loglikelihood ratio, storing results in gTopCluster
    class member. Note that gTopCluster is re-intialized when centroid being evaluated is
    different than that of gTopCluster; this permits repeated calls to this function, inorder
    to compare clusterings about same centroid but with different ellipse shapes.

    Caller is responsible for ensuring:
    1) DataSetInterface objects are assigned to appropriate structures used to accumulate cluster data */
void SpatialVarTempTrendCentricAnalysis::CalculateTopClusterAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const AbstractDataSetGateway& DataGateway) {
  if (gTopCluster->GetCentroidIndex() != CentroidDef.GetCentroidIndex())
    //re-intialize top cluster object if evaluating data about new centroid
    gTopCluster->InitializeSVTT(CentroidDef.GetCentroidIndex(), DataGateway);
  gClusterComparator->InitializeSVTT(CentroidDef.GetCentroidIndex(), DataGateway);
  gClusterComparator->SetEllipseOffset(CentroidDef.GetEllipseIndex(), gDataHub);
  gClusterComparator->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, *gTopCluster, *gpLikelihoodCalculator);
  //if top cluster was found in this centroid/ellipse, calculate radius now - CentroidNeighbors object wont' be available later
  if (gTopCluster->GetEllipseOffset() == CentroidDef.GetEllipseIndex())
    gTopCluster->SetNonPersistantNeighborInfo(gDataHub, CentroidDef);
}

/** Returns cluster object with greatest llr value as specified by gTopCluster.
    Note that this object will potentially be updated with next call to
    CalculateTopClusterAboutCentroidDefinition(), so calling Clone() on returned object
    is necessary if you want to keep object around in current state. */
const CCluster& SpatialVarTempTrendCentricAnalysis::GetTopCalculatedCluster() {
  gTopCluster->SetTimeTrend(gParameters.GetTimeAggregationUnitsType(), gParameters.GetTimeAggregationLength());
  return *gTopCluster;
}

/** Updates CMeasureList objects for each defined data set of gateway object.
    Caller is responsible for ensuring:
    1) 'vDataGateways' contains a number of elements equal to number of requested simulations
    2) DataSetInterface objects are assigned to appropriate structures used to accumulate cluster data
    3) RetrieveLoglikelihoodRatios() has not been called yet */
void SpatialVarTempTrendCentricAnalysis::MonteCarloAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const DataSetGatewayContainer_t& vDataGateways) {
  throw prg_error("MonteCarloAboutCentroidDefinition(const CentroidNeighbors&,const DataSetGatewayContainer_t&) not implementated.","SpatialVarTempTrendCentricAnalysis");
}

/** Internal function which allocates appropriate objects for evaulate real and simulated data. */
void SpatialVarTempTrendCentricAnalysis::Setup(const AbstractDataSetGateway& RealDataGateway, const DataSetGatewayContainer_t& vSimDataGateways) {
  try {
    //allocate objects used to evaluate real data
    gClusterComparator.reset(new CSVTTCluster(gpClusterDataFactory, RealDataGateway));
    gTopCluster.reset(new CSVTTCluster(gpClusterDataFactory, RealDataGateway));

    //allocate objects used to evaluate simulation data
    if (gParameters.GetNumReplicationsRequested()) {
      //Simulations performed using same process as real data set. Since we will be taking
      //the greatest loglikelihood ratios among all centroids, but analyzing each centroids
      //replications separately, we need to maintain a vector of llr values.
      gCalculatedRatios.reset(new std::vector<double>(gParameters.GetNumReplicationsRequested(), 0));
      gClusterData.reset(new SVTTClusterData(gDataHub.GetNumTimeIntervals()));
    }
  }
  catch (prg_exception& x) {
    gClusterComparator.reset(0); gClusterData.reset(0);
    x.addTrace("Setup()","SpatialVarTempTrendCentricAnalysis");
    throw;
  }
}

