//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "PurelySpatialCentricAnalysis.h"
#include "ClusterDataFactory.h"
#include "NormalClusterDataFactory.h"
#include "CategoricalClusterDataFactory.h"

/** constructor */
PurelySpatialCentricAnalysis::PurelySpatialCentricAnalysis(const CParameters& Parameters,
                                                           const CSaTScanData& Data,
                                                           BasePrint& PrintDirection,
                                                           const AbtractDataSetGateway& RealDataGateway,
                                                           const DataSetGatewayContainer_t& vSimDataGateways)
                             :AbstractCentricAnalysis(Parameters, Data, PrintDirection), gTopShapeClusters(Data) {
  try {
    Setup(RealDataGateway, vSimDataGateways);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","PurelySpatialCentricAnalysis");
    throw;
  }
}

/** destructor */
PurelySpatialCentricAnalysis::~PurelySpatialCentricAnalysis() {}

/** Calculates greatest loglikelihood ratios about centroid for each defined
    simulation data set. Vector of CentroidNeighbors objects must reference the
    same centroid index (this is not validated in function). */
void PurelySpatialCentricAnalysis::CalculateRatiosAboutCentroidDefinition(CentroidDefinitionContainer_t& vCentroid, const DataSetGatewayContainer_t& vDataGateways) {
  DataSetGatewayContainer_t::const_iterator  itrGateway, itrGatewayEnd=vDataGateways.end();
  CentroidDefinitionContainer_t::iterator    itrCentroidDef=vCentroid.begin(), itrCentroidDefEnd=vCentroid.end();
  std::vector<double>::iterator              itrLoglikelihoodRatios;
  tract_t                                    t, tNumNeighbors;

  //perform simulations about current centroid
  for (; itrCentroidDef != itrCentroidDefEnd; ++itrCentroidDef) {
     itrCentroidDef->SetMaximumClusterSize_SimulatedData();
     itrGateway=vDataGateways.begin();
     itrLoglikelihoodRatios=gCalculatedRatios->begin();
     for (; itrGateway != itrGatewayEnd; ++itrGateway, ++itrLoglikelihoodRatios) {
        gAbstractClusterData->InitializeData();
        tNumNeighbors = itrCentroidDef->GetNumNeighbors();
        for (t=0; t < tNumNeighbors; ++t) {
           //update cluster data
           gAbstractClusterData->AddNeighborData(itrCentroidDef->GetNeighborTractIndex(t), *(*itrGateway));
           //calculate loglikehood ratio and compare against current top cluster
           *itrLoglikelihoodRatios = std::max(*itrLoglikelihoodRatios, gAbstractClusterData->CalculateLoglikelihoodRatio(*gpLikelihoodCalculator));
        }
        //NOTE: This process assumes no-compactness correction for ellipses - otherwise we would
        //      use an IntermediateClustersContainer, as used in other top cluster method to
        //      calculates most likely cluster among circle and ellipses. Not sure how to code this
        //      should we want compactnes correction later on.
     }
  }
}

/** Returns cluster centered at grid point nCenter, with the greatest loglikelihood ratio.
    Caller should not assume that returned reference is persistent, but should either call
    Clone() method or overloaded assignment operator. */
const CCluster& PurelySpatialCentricAnalysis::CalculateTopClusterAboutCentroidDefinition(CentroidDefinitionContainer_t& vCentroid, const AbtractDataSetGateway& DataGateway) {
  CentroidDefinitionContainer_t::iterator      itrCentroid=vCentroid.begin(), itrCentroidEnd=vCentroid.end();

  gTopShapeClusters.Reset(vCentroid.back().GetCentroidIndex());
  for (; itrCentroid != itrCentroidEnd; ++itrCentroid) {
     gClusterComparator->Initialize(itrCentroid->GetCentroidIndex());
     gClusterComparator->SetEllipseOffset(itrCentroid->GetEllipseIndex(), gDataHub);
     gClusterComparator->CalculateTopClusterAboutCentroidDefinition(DataGateway,
                                                                    *itrCentroid,
                                                                    gTopShapeClusters.GetTopCluster(itrCentroid->GetEllipseIndex()),
                                                                    *gpLikelihoodCalculator);
  }
  return gTopShapeClusters.GetTopCluster();
}

/** Returns loglikelihood ratio for Monte Carlo replication. */
void PurelySpatialCentricAnalysis::MonteCarloAboutCentroidDefinition(CentroidDefinitionContainer_t& vCentroid, const DataSetGatewayContainer_t& vDataGateways) {
  DataSetGatewayContainer_t::const_iterator  itrGateway, itrGatewayEnd=vDataGateways.end();
  MeasureListContainer_t::const_iterator     itrMeasureList;
  CentroidDefinitionContainer_t::iterator    itrCentroid=vCentroid.begin(), itrCentroidEnd=vCentroid.end();

  //perform simulations about current centroid
  for (; itrCentroid != itrCentroidEnd; ++itrCentroid) {
    itrCentroid->SetMaximumClusterSize_SimulatedData();
    itrGateway = vDataGateways.begin();
    itrMeasureList=gvMeasureLists.begin();
    for (; itrGateway != itrGatewayEnd; ++itrGateway, ++itrMeasureList)
       gClusterData->AddMeasureList(*itrCentroid, (*itrGateway)->GetDataSetInterface(), (*itrMeasureList));
       //NOTE: This process assumes no-compactness correction for ellipses - otherwise
       //      we would call: gpMeasureList->SetForNextIteration(itrCentroidEnd->GetCentroidIndex());
       //      to calculate the loglikelihood with accumulated measure list for current circle/ellipse.
       //      Note that if we did in fact call that method, the number of times the loglikelihood is calculated
       //      would increase dramtically compared to other Monte Carlo method. We would need an additional
       //      measurelist object for each ellipse/centroid pair to prevent to many loglikelihood calculations, but
       //      that would increase the amount of memory needed.
  }
}

/** Internal function which allocates appropriate objects for evaulate real and simulated data. */
void PurelySpatialCentricAnalysis::Setup(const AbtractDataSetGateway& RealDataGateway, const DataSetGatewayContainer_t& vSimDataGateways) {
  try {
    //allocate objects used to evaluate real data
    gClusterComparator.reset(new CPurelySpatialCluster(gpClusterDataFactory, RealDataGateway, gParameters.GetAreaScanRateType()));
    gTopShapeClusters.SetTopClusters(*gClusterComparator);
    
    //allocate objects used to evaluate simulation data
    if (gParameters.GetNumReplicationsRequested()) {
      ZdPointerVector<AbtractDataSetGateway>::const_iterator  itr=vSimDataGateways.begin(), itr_end=vSimDataGateways.end();

      gvMeasureLists.DeleteAllElements();
      //create simulation objects based upon which process used to perform simulations
      if (geReplicationsProcessType == MeasureListEvaluation) {
         //create a measure list object for each requested replication - we do this inorder
         //to prevent excess calls to loglikelihood calculation method; unfortunately this
         //also consumes more memory...
         gClusterData.reset(new SpatialData(*(*itr), gParameters.GetAreaScanRateType()));
         for (; itr != itr_end; ++itr)
           gvMeasureLists.push_back(GetNewMeasureListObject());
      }
      else {
        //Simulations performed using same process as real data set. Since we will be taking
        //the greatest loglikelihood ratios among all centroids, but analyzing each centroids
        //replications separately, we need to maintain a vector of llr values.
        gCalculatedRatios.reset(new std::vector<double>(gParameters.GetNumReplicationsRequested(), 0));
        gAbstractClusterData.reset(gpClusterDataFactory->GetNewSpatialClusterData(*(*itr), gParameters.GetAreaScanRateType()));
      }
    }  
  }
  catch (ZdException &x) {
    gClusterComparator.reset(0); gClusterData.reset(0); gAbstractClusterData.reset(0);
    gvMeasureLists.DeleteAllElements(); gCalculatedRatios.reset();
    x.AddCallpath("Setup()","PurelySpatialCentricAnalysis");
    throw;
  }
}

