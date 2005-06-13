//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SpaceTimeIncludePurelySpatialCentricAnalysis.h"

/** constructor */
SpaceTimeIncludePurelySpatialCentricAnalysis::SpaceTimeIncludePurelySpatialCentricAnalysis(const CParameters& Parameters,
                                                                                           const CSaTScanData& Data,
                                                                                           BasePrint& PrintDirection,
                                                                                           const AbtractDataSetGateway& RealDataGateway,
                                                                                           const DataSetGatewayContainer_t& vSimDataGateways)
                                             :SpaceTimeCentricAnalysis(Parameters, Data, PrintDirection, RealDataGateway, vSimDataGateways),
                                              gPSTopShapeClusters(Data) {
  try {
    Setup(RealDataGateway, vSimDataGateways);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","SpaceTimeIncludePurelySpatialCentricAnalysis");
    throw;
  }
}

/** destructor */
SpaceTimeIncludePurelySpatialCentricAnalysis::~SpaceTimeIncludePurelySpatialCentricAnalysis() {}

/** Calculates greatest loglikelihood ratios about centroid for each defined
    simulation data set. Vector of CentroidNeighbors objects must reference the
    same centroid index (this is not validated in function). */
void SpaceTimeIncludePurelySpatialCentricAnalysis::CalculateRatiosAboutCentroidDefinition(CentroidDefinitionContainer_t& vCentroid, const DataSetGatewayContainer_t& vDataGateways) {
  DataSetGatewayContainer_t::const_iterator  itrGateway, itrGatewayEnd=vDataGateways.end();
  CentroidDefinitionContainer_t::iterator    itrCentroidDef=vCentroid.begin(), itrCentroidDefEnd=vCentroid.end();
  std::vector<double>::iterator              itrLoglikelihoodRatios;
  tract_t                                    t, tNumNeighbors;

  //perform simulations about current centroid
  for (; itrCentroidDef != itrCentroidDefEnd; ++itrCentroidDef) {
     itrGateway=vDataGateways.begin();
     itrLoglikelihoodRatios=gCalculatedRatios->begin();
     for (; itrGateway != itrGatewayEnd; ++itrGateway, ++itrLoglikelihoodRatios) {
        gPSAbstractClusterData->InitializeData();
        gAbstractClusterData->InitializeData();
        tNumNeighbors = itrCentroidDef->GetNumNeighbors();
        for (t=0; t < tNumNeighbors; ++t) {
           //update cluster data
           gPSAbstractClusterData->AddNeighborData(itrCentroidDef->GetNeighborTractIndex(t), *(*itrGateway));
           //calculate loglikehood ratio and compare against current top cluster
           *itrLoglikelihoodRatios = std::max(*itrLoglikelihoodRatios, gPSAbstractClusterData->CalculateLoglikelihoodRatio(*gpLikelihoodCalculator));
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
const CCluster& SpaceTimeIncludePurelySpatialCentricAnalysis::CalculateTopClusterAboutCentroidDefinition(CentroidDefinitionContainer_t& vCentroid, const AbtractDataSetGateway& DataGateway) {
  CentroidDefinitionContainer_t::iterator      itrCentroid=vCentroid.begin(), itrCentroidEnd=vCentroid.end();

  gPSTopShapeClusters.Reset(vCentroid.back().GetCentroidIndex());
  gTopShapeClusters.Reset(vCentroid.back().GetCentroidIndex());
  for (; itrCentroid != itrCentroidEnd; ++itrCentroid) {
     gPSClusterComparator->Initialize(itrCentroid->GetCentroidIndex());
     gPSClusterComparator->SetEllipseOffset(itrCentroid->GetEllipseIndex(), gDataHub);
     gPSClusterComparator->CalculateTopClusterAboutCentroidDefinition(DataGateway,
                                                                            *itrCentroid,
                                                                            gPSTopShapeClusters.GetTopCluster(itrCentroid->GetEllipseIndex()),
                                                                            *gpLikelihoodCalculator);
     gClusterComparator->Initialize(itrCentroid->GetCentroidIndex());
     gClusterComparator->SetEllipseOffset(itrCentroid->GetEllipseIndex(), gDataHub);
     gClusterComparator->CalculateTopClusterAboutCentroidDefinition(DataGateway,
                                                                          *itrCentroid,
                                                                          gTopShapeClusters.GetTopCluster(itrCentroid->GetEllipseIndex()),
                                                                          *gTimeIntervals_R);
  }

  CPurelySpatialCluster& PSCluster = gPSTopShapeClusters.GetTopCluster();
  CSpaceTimeCluster& STCluster = gTopShapeClusters.GetTopCluster();
  if (!PSCluster.ClusterDefined())
    return STCluster;
  else if (PSCluster.m_nRatio > STCluster.m_nRatio)
    return PSCluster;
  else
    return STCluster;
}

void SpaceTimeIncludePurelySpatialCentricAnalysis::MonteCarloAboutCentroidDefinition(CentroidDefinitionContainer_t& vCentroid, const DataSetGatewayContainer_t& vDataGateways) {
  if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)
    return MonteCarloProspectiveAboutCentroidDefinition(vCentroid, vDataGateways);

  DataSetGatewayContainer_t::const_iterator  itrGateway, itrGatewayEnd=vDataGateways.end();
  MeasureListContainer_t::iterator           itrMeasureList;
  CentroidDefinitionContainer_t::iterator    itrCentroid=vCentroid.begin(), itrCentroidEnd=vCentroid.end();

  //perform simulations about current centroid
  for (; itrCentroid != itrCentroidEnd; ++itrCentroid) {
    itrCentroid->SetMaximumClusterSize_SimulatedData();
    itrGateway = vDataGateways.begin();
    itrMeasureList=gvMeasureLists.begin();
    for (; itrGateway != itrGatewayEnd; ++itrGateway, ++itrMeasureList) {
       gPSClusterData->AddMeasureList(*itrCentroid, (*itrGateway)->GetDataSetInterface(), (*itrMeasureList));
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
}

void SpaceTimeIncludePurelySpatialCentricAnalysis::MonteCarloProspectiveAboutCentroidDefinition(CentroidDefinitionContainer_t& vCentroid, const DataSetGatewayContainer_t& vDataGateways) {
  DataSetGatewayContainer_t::const_iterator  itrGateway, itrGatewayEnd=vDataGateways.end();
  MeasureListContainer_t::iterator           itrMeasureList;
  CentroidDefinitionContainer_t::iterator    itrCentroid=vCentroid.begin(), itrCentroidEnd=vCentroid.end();

  //perform simulations about current centroid
  for (; itrCentroid != itrCentroidEnd; ++itrCentroid) {
    itrCentroid->SetMaximumClusterSize_SimulatedData();
    itrGateway = vDataGateways.begin();
    itrMeasureList=gvMeasureLists.begin();
    for (; itrGateway != itrGatewayEnd; ++itrGateway, ++itrMeasureList) {
       gPSPClusterData->AddMeasureList(*itrCentroid, (*itrGateway)->GetDataSetInterface(), (*itrMeasureList));
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
}

/** Internal function which allocates appropriate objects for evaulate real and simulated data. */
void SpaceTimeIncludePurelySpatialCentricAnalysis::Setup(const AbtractDataSetGateway& RealDataGateway, const DataSetGatewayContainer_t& vSimDataGateways) {
  IncludeClustersType           eIncludeClustersType;

  try {
    //allocate objects used to evaluate real data
    gPSClusterComparator.reset(new CPurelySpatialCluster(gpClusterDataFactory, RealDataGateway, gParameters.GetAreaScanRateType()));
    gPSTopShapeClusters.SetTopClusters(*gPSClusterComparator);

    //allocate objects used to evaluate simulation data
    if (gParameters.GetNumReplicationsRequested()) {
      if (geReplicationsProcessType == MeasureListEvaluation) {
        if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)
          gPSPClusterData.reset(new ProspectiveSpatialData(gDataHub, *(*vSimDataGateways.begin())));
        else
          gPSClusterData.reset(new SpatialData(*(*vSimDataGateways.begin()), gParameters.GetAreaScanRateType()));
      }
      else {
        if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)
          gPSAbstractClusterData.reset(gpClusterDataFactory->GetNewProspectiveSpatialClusterData(gDataHub, *(*vSimDataGateways.begin())));
        else
          gPSAbstractClusterData.reset(gpClusterDataFactory->GetNewSpatialClusterData(*(*vSimDataGateways.begin()), gParameters.GetAreaScanRateType()));
      }
    }
  }
  catch (ZdException &x) {
    gPSClusterComparator.reset(0); gPSPClusterData.reset(0); gPSClusterData.reset(0); gPSAbstractClusterData.reset(0);
    x.AddCallpath("Setup()","SpaceTimeIncludePurelySpatialCentricAnalysis");
    throw;
  }
}

