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
                                                           const AbstractDataSetGateway& RealDataGateway,
                                                           const DataSetGatewayContainer_t& vSimDataGateways)
                             :AbstractCentricAnalysis(Parameters, Data, PrintDirection) {
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

/** Calculates greatest loglikelihood ratios about centroid for each defined data set gateway object,
    storing maximum ratio for each simulation in class member gCalculatedRatios.
    Caller is responsible for ensuring:
    1) 'vDataGateways' contains a number of elements equal to number of requested simulations
    2) DataSetInterface objects are assigned to appropriate structures used to accumulate cluster data
    3) RetrieveLoglikelihoodRatios() has not been called yet */
void PurelySpatialCentricAnalysis::CalculateRatiosAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const DataSetGatewayContainer_t& vDataGateways) {
  DataSetGatewayContainer_t::const_iterator  itrGateway, itrGatewayEnd=vDataGateways.end();
  std::vector<double>::iterator              itrLoglikelihoodRatios;
  tract_t                                    t, tNumNeighbors;

  //perform simulations about current centroid
  itrGateway=vDataGateways.begin();
  itrLoglikelihoodRatios=gCalculatedRatios->begin();
  for (; itrGateway != itrGatewayEnd; ++itrGateway, ++itrLoglikelihoodRatios) {
     gAbstractClusterData->InitializeData();
     tNumNeighbors = CentroidDef.GetNumNeighbors();
     for (t=0; t < tNumNeighbors; ++t) {
        //update cluster data
        gAbstractClusterData->AddNeighborData(CentroidDef.GetNeighborTractIndex(t), *(*itrGateway));
        //calculate loglikehood ratio and compare against current top cluster
        *itrLoglikelihoodRatios = std::max(*itrLoglikelihoodRatios, gAbstractClusterData->CalculateLoglikelihoodRatio(*gpLikelihoodCalculator));
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
void PurelySpatialCentricAnalysis::CalculateTopClusterAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const AbstractDataSetGateway& DataGateway) {
  if (gTopCluster->GetCentroidIndex() != CentroidDef.GetCentroidIndex())
    //re-intialize top cluster object if evaluating data about new centroid
    gTopCluster->Initialize(CentroidDef.GetCentroidIndex());
  gClusterComparator->Initialize(CentroidDef.GetCentroidIndex());
  gClusterComparator->SetEllipseOffset(CentroidDef.GetEllipseIndex(), gDataHub);
  gClusterComparator->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, *gTopCluster, *gpLikelihoodCalculator);
  //if top cluster was found in this centroid/ellipse, calculate radius now - CentroidNeighbors object wont' be available later
  if (gTopCluster->GetEllipseOffset() == CentroidDef.GetEllipseIndex())
    gTopCluster->SetCartesianRadius(gDataHub, CentroidDef);
}

/** Returns cluster object with greatest llr value as specified by gTopCluster.
    Note that this object will potentially be updated with next call to
    CalculateTopClusterAboutCentroidDefinition(), so calling Clone() on returned object
    is necessary if you want to keep object around in current state. */
const CCluster& PurelySpatialCentricAnalysis::GetTopCalculatedCluster() {
  return *gTopCluster;
}

/** Updates CMeasureList objects for each defined data set of gateway object.
    Caller is responsible for ensuring:
    1) 'vDataGateways' contains a number of elements equal to number of requested simulations
    2) DataSetInterface objects are assigned to appropriate structures used to accumulate cluster data
    3) RetrieveLoglikelihoodRatios() has not been called yet */
void PurelySpatialCentricAnalysis::MonteCarloAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const DataSetGatewayContainer_t& vDataGateways) {
  DataSetGatewayContainer_t::const_iterator  itrGateway, itrGatewayEnd=vDataGateways.end();
  MeasureListContainer_t::const_iterator     itrMeasureList;

  //perform simulations about current centroid
  itrGateway = vDataGateways.begin();
  itrMeasureList=gvMeasureLists.begin();
  for (; itrGateway != itrGatewayEnd; ++itrGateway, ++itrMeasureList)
     gClusterData->AddMeasureList(CentroidDef, (*itrGateway)->GetDataSetInterface(), (*itrMeasureList));
     //NOTE: This process assumes no-compactness correction for ellipses - otherwise
     //      we would call: gpMeasureList->SetForNextIteration(itrCentroidEnd->GetCentroidIndex());
     //      to calculate the loglikelihood with accumulated measure list for current circle/ellipse.
     //      Note that if we did in fact call that method, the number of times the loglikelihood is calculated
     //      would increase dramtically compared to other Monte Carlo method. We would need an additional
     //      measurelist object for each ellipse/centroid pair to prevent to many loglikelihood calculations, but
     //      that would increase the amount of memory needed.
}

/** Internal function which allocates appropriate objects for evaulate real and simulated data. */
void PurelySpatialCentricAnalysis::Setup(const AbstractDataSetGateway& RealDataGateway, const DataSetGatewayContainer_t& vSimDataGateways) {
  try {
    //allocate objects used to evaluate real data
    gClusterComparator.reset(new CPurelySpatialCluster(gpClusterDataFactory, RealDataGateway, gParameters.GetAreaScanRateType()));
    gTopCluster.reset(new CPurelySpatialCluster(gpClusterDataFactory, RealDataGateway, gParameters.GetAreaScanRateType()));
    
    //allocate objects used to evaluate simulation data
    if (gParameters.GetNumReplicationsRequested()) {
      ZdPointerVector<AbstractDataSetGateway>::const_iterator  itr=vSimDataGateways.begin(), itr_end=vSimDataGateways.end();

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

