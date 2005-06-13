//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SpaceTimeIncludePurelyTemporalCentricAnalysis.h"

/** constructor */
SpaceTimeIncludePurelyTemporalCentricAnalysis::SpaceTimeIncludePurelyTemporalCentricAnalysis(const CParameters& Parameters,
                                                   const CSaTScanData& Data,
                                                   BasePrint& PrintDirection,
                                                   const AbtractDataSetGateway& RealDataGateway,
                                                   const DataSetGatewayContainer_t& vSimDataGateways)
                         :SpaceTimeCentricAnalysis(Parameters, Data, PrintDirection, RealDataGateway, vSimDataGateways) {
  try {
    Setup(vSimDataGateways);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","SpaceTimeIncludePurelyTemporalCentricAnalysis");
    throw;
  }
}

/** destructor */
SpaceTimeIncludePurelyTemporalCentricAnalysis::~SpaceTimeIncludePurelyTemporalCentricAnalysis() {}

void SpaceTimeIncludePurelyTemporalCentricAnalysis::CalculateRatiosAboutCentroidDefinition(CentroidDefinitionContainer_t& vCentroid, const DataSetGatewayContainer_t& vDataGateways) {
  DataSetGatewayContainer_t::const_iterator  itrGateway, itrGatewayEnd=vDataGateways.end();
  CentroidDefinitionContainer_t::iterator    itrCentroidDef=vCentroid.begin(), itrCentroidDefEnd=vCentroid.end();
  std::vector<double>::iterator              itrLoglikelihoodRatios;
  tract_t                                    t, tNumNeighbors;

  //perform simulation about purely temporal data
  itrGateway=vDataGateways.begin();
  itrLoglikelihoodRatios=gCalculatedRatios->begin();
  for (; itrGateway != itrGatewayEnd; ++itrGateway, ++itrLoglikelihoodRatios) {
     gPTClusterData->Reassociate(*(*itrGateway));
     *itrLoglikelihoodRatios = std::max(*itrLoglikelihoodRatios, gTimeIntervals_S->ComputeLoglikelihoodRatioClusterData(*gPTClusterData));
  }
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

/** Returns loglikelihood ratio for Monte Carlo replication. */
void SpaceTimeIncludePurelyTemporalCentricAnalysis::MonteCarloAboutCentroidDefinition(CentroidDefinitionContainer_t& vCentroid, const DataSetGatewayContainer_t& vDataGateways) {
  DataSetGatewayContainer_t::const_iterator  itrGateway, itrGatewayEnd=vDataGateways.end();
  MeasureListContainer_t::iterator           itrMeasureList;
  CentroidDefinitionContainer_t::iterator    itrCentroid=vCentroid.begin(), itrCentroidEnd=vCentroid.end();

  //perform simulation about purely temporal data
  itrGateway=vDataGateways.begin();
  itrMeasureList=gvMeasureLists.begin();
  for (; itrGateway != itrGatewayEnd; ++itrGateway, ++itrMeasureList) {
     gPTClusterData->Reassociate(*(*itrGateway));
     gTimeIntervals_S->CompareMeasures(*gPTClusterData, *(*itrMeasureList));
  }
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
void SpaceTimeIncludePurelyTemporalCentricAnalysis::Setup(const DataSetGatewayContainer_t& vSimDataGateways) {
  IncludeClustersType           eIncludeClustersType;

  try {
    //allocate objects used to evaluate simulation data
    if (gParameters.GetNumReplicationsRequested()) {
      if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)
        eIncludeClustersType = ALLCLUSTERS;
      else
        eIncludeClustersType = gParameters.GetIncludeClustersType();
      //create simulation objects based upon which process used to perform simulations
      if (geReplicationsProcessType == MeasureListEvaluation)
        gPTClusterData.reset(new TemporalData(*(*vSimDataGateways.begin())));
      else
        gPTClusterData.reset(gpClusterDataFactory->GetNewTemporalClusterData(*(*vSimDataGateways.begin())));
    }
  }
  catch (ZdException &x) {
    gClusterComparator.reset(0); gClusterData.reset(0); gAbstractClusterData.reset(0); gvMeasureLists.DeleteAllElements();
    x.AddCallpath("Setup()","SpaceTimeCentricAnalysis");
    throw;
  }
}

