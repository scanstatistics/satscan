//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SpaceTimeCentricAnalysis.h"
#include "PurelyTemporalCluster.h"
#include "MostLikelyClustersContainer.h"

/** constructor */
SpaceTimeCentricAnalysis::SpaceTimeCentricAnalysis(const CParameters& Parameters,
                                                   const CSaTScanData& Data,
                                                   BasePrint& PrintDirection,
                                                   const AbtractDataSetGateway& RealDataGateway,
                                                   const DataSetGatewayContainer_t& vSimDataGateways)
                         :AbstractCentricAnalysis(Parameters, Data, PrintDirection), gTopShapeClusters(Data) {
  try {
    Setup(RealDataGateway, vSimDataGateways);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","SpaceTimeCentricAnalysis");
    throw;
  }
}

/** destructor */
SpaceTimeCentricAnalysis::~SpaceTimeCentricAnalysis() {}

/** Calculates purely temporal cluster and added to top cluster container. */
void SpaceTimeCentricAnalysis::CalculatePurelyTemporalCluster(MostLikelyClustersContainer& TopClustersContainer,
                                                              const AbtractDataSetGateway& DataGateway) {
  IncludeClustersType           eIncludeClustersType;

  try {
    //calculate top purely temporal cluster
    if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)
      eIncludeClustersType = ALIVECLUSTERS;
    else
      eIncludeClustersType = gParameters.GetIncludeClustersType();
    //create top cluster
    CPurelyTemporalCluster TopCluster(gpClusterDataFactory, DataGateway, eIncludeClustersType, gDataHub);
    //create comparator cluster
    CPurelyTemporalCluster ClusterComparator(gpClusterDataFactory, DataGateway, eIncludeClustersType, gDataHub);
    gTimeIntervals_R->CompareClusters(ClusterComparator, TopCluster);
    if (TopCluster.ClusterDefined()) {
      TopClustersContainer.Add(TopCluster);
      TopClustersContainer.SortTopClusters();
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculatePurelyTemporalCluster()","SpaceTimeCentricAnalysis");
    throw;
  }
}

/** Calculates greatest loglikelihood ratios about centroid for each defined
    simulation data set. Vector of CentroidNeighbors objects must reference the
    same centroid index (this is not validated in function). */
void SpaceTimeCentricAnalysis::CalculateRatiosAboutCentroidDefinition(CentroidDefinitionContainer_t& vCentroid, const DataSetGatewayContainer_t& vDataGateways) {
  DataSetGatewayContainer_t::const_iterator  itrGateway, itrGatewayEnd=vDataGateways.end();
  CentroidDefinitionContainer_t::iterator    itrCentroidDef=vCentroid.begin(), itrCentroidDefEnd=vCentroid.end();
  std::vector<double>::iterator              itrLoglikelihoodRatios;
  tract_t                                    t, tNumNeighbors;

  //perform simulations about current centroid
  for (; itrCentroidDef != itrCentroidDefEnd; ++itrCentroidDef) {
     itrGateway=vDataGateways.begin();
     itrLoglikelihoodRatios=gCalculatedRatios->begin();
     for (; itrGateway != itrGatewayEnd; ++itrGateway, ++itrLoglikelihoodRatios) {
        gAbstractClusterData->InitializeData();
        tNumNeighbors = itrCentroidDef->GetNumNeighbors();
        for (t=0; t < tNumNeighbors; ++t) {
           //update cluster data
           gAbstractClusterData->AddNeighborData(itrCentroidDef->GetNeighborTractIndex(t), *(*itrGateway));
           //calculate loglikehood ratio and compare against current top cluster
           *itrLoglikelihoodRatios = std::max(*itrLoglikelihoodRatios, gTimeIntervals_S->ComputeLoglikelihoodRatioClusterData(*gAbstractClusterData));
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
const CCluster& SpaceTimeCentricAnalysis::CalculateTopClusterAboutCentroidDefinition(CentroidDefinitionContainer_t& vCentroid, const AbtractDataSetGateway& DataGateway) {
  CentroidDefinitionContainer_t::iterator      itrCentroid=vCentroid.begin(), itrCentroidEnd=vCentroid.end();

  gTopShapeClusters.Reset(vCentroid.back().GetCentroidIndex());
  for (; itrCentroid != itrCentroidEnd; ++itrCentroid) {
     gClusterComparator->Initialize(itrCentroid->GetCentroidIndex());
     gClusterComparator->SetEllipseOffset(itrCentroid->GetEllipseIndex(), gDataHub);
     gClusterComparator->CalculateTopClusterAboutCentroidDefinition(DataGateway, *itrCentroid,
                                                                          gTopShapeClusters.GetTopCluster(itrCentroid->GetEllipseIndex()),
                                                                          *gTimeIntervals_R);
  }
  return gTopShapeClusters.GetTopCluster();
}

/** Returns loglikelihood ratio for Monte Carlo replication. */
void SpaceTimeCentricAnalysis::MonteCarloAboutCentroidDefinition(CentroidDefinitionContainer_t& vCentroid, const DataSetGatewayContainer_t& vDataGateways) {
  DataSetGatewayContainer_t::const_iterator  itrGateway, itrGatewayEnd=vDataGateways.end();
  MeasureListContainer_t::iterator           itrMeasureList;
  CentroidDefinitionContainer_t::iterator    itrCentroid=vCentroid.begin(), itrCentroidEnd=vCentroid.end();

  //perform simulations about current centroid
  for (; itrCentroid != itrCentroidEnd; ++itrCentroid) {
    itrCentroid->SetMaximumClusterSize_SimulatedData();
    itrGateway = vDataGateways.begin();
    itrMeasureList=gvMeasureLists.begin();
    for (; itrGateway != itrGatewayEnd; ++itrGateway, ++itrMeasureList)
       gClusterData->AddNeighborDataAndCompare(*itrCentroid, (*itrGateway)->GetDataSetInterface(), *gTimeIntervals_S, *(*itrMeasureList));
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
void SpaceTimeCentricAnalysis::Setup(const AbtractDataSetGateway& RealDataGateway, const DataSetGatewayContainer_t& vSimDataGateways) {
  IncludeClustersType           eIncludeClustersType;

  try {
    //allocate objects used to evaluate real data
    gClusterComparator.reset(new CSpaceTimeCluster(gpClusterDataFactory, RealDataGateway));
    gTopShapeClusters.SetTopClusters(*gClusterComparator);
    //allocate time interval evaluation of real data
    if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)
      eIncludeClustersType = ALIVECLUSTERS;
    else
      eIncludeClustersType = gParameters.GetIncludeClustersType();
    gTimeIntervals_R.reset(GetNewTemporalDataEvaluatorObject(eIncludeClustersType));

    //allocate objects used to evaluate simulation data
    if (gParameters.GetNumReplicationsRequested()) {
      DataSetGatewayContainer_t::const_iterator  itr=vSimDataGateways.begin(), itr_end=vSimDataGateways.end();

      gvMeasureLists.DeleteAllElements();
      //create simulation objects based upon which process used to perform simulations
      if (geReplicationsProcessType == MeasureListEvaluation) {
         //create a measure list object for each requested replication - we do this inorder
         //to prevent excess calls to loglikelihood calculation method; unfortunately this
         //also consumes more memory...
         gClusterData.reset(new SpaceTimeData(*(*itr)));
         for (; itr != itr_end; ++itr)
           gvMeasureLists.push_back(GetNewMeasureListObject());
      }
      else {
        //Simulations performed using same process as real data set. Since we will be taking
        //the greatest loglikelihood ratios among all centroids, but analyzing each centroids
        //replications separately, we need to maintain a vector of llr values.
        gCalculatedRatios.reset(new std::vector<double>(gParameters.GetNumReplicationsRequested(), 0));
        gAbstractClusterData.reset(gpClusterDataFactory->GetNewSpaceTimeClusterData(*(*itr)));
      }
      //allocate time interval evaluator for simulation process
      if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)
        eIncludeClustersType = ALLCLUSTERS;
      else
        eIncludeClustersType = gParameters.GetIncludeClustersType();
      gTimeIntervals_S.reset(GetNewTemporalDataEvaluatorObject(eIncludeClustersType));
    }  
  }
  catch (ZdException &x) {
    gClusterComparator.reset(0); gClusterData.reset(0); gAbstractClusterData.reset(0);
    gvMeasureLists.DeleteAllElements(); gCalculatedRatios.reset();
    x.AddCallpath("Setup()","SpaceTimeCentricAnalysis");
    throw;
  }
}

