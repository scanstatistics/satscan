//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SpaceTimeIncludePurelySpatialCentricAnalysis.h"

/** constructor */
SpaceTimeIncludePurelySpatialCentricAnalysis::SpaceTimeIncludePurelySpatialCentricAnalysis(const CParameters& Parameters,
                                                                                           const CSaTScanData& Data,
                                                                                           BasePrint& PrintDirection,
                                                                                           const AbstractDataSetGateway& RealDataGateway,
                                                                                           const DataSetGatewayContainer_t& vSimDataGateways)
                                             :SpaceTimeCentricAnalysis(Parameters, Data, PrintDirection, RealDataGateway, vSimDataGateways) {
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

/** Calculates greatest loglikelihood ratios about centroid for each defined data set gateway object,
    storing maximum ratio for each simulation in class member gCalculatedRatios.
    Caller is responsible for ensuring:
    1) 'vDataGateways' contains a number of elements equal to number of requested simulations
    2) DataSetInterface objects are assigned to appropriate structures used to accumulate cluster data
    3) RetrieveLoglikelihoodRatios() has not been called yet */
void SpaceTimeIncludePurelySpatialCentricAnalysis::CalculateRatiosAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const DataSetGatewayContainer_t& vDataGateways) {
  DataSetGatewayContainer_t::const_iterator  itrGateway, itrGatewayEnd=vDataGateways.end();
  std::vector<double>::iterator              itrLoglikelihoodRatios;
  tract_t                                    t, tNumNeighbors;

  //perform simulations about current centroid
  itrGateway=vDataGateways.begin();
  itrLoglikelihoodRatios=gCalculatedRatios->begin();
  for (; itrGateway != itrGatewayEnd; ++itrGateway, ++itrLoglikelihoodRatios) {
     gPSAbstractClusterData->InitializeData();
     gAbstractClusterData->InitializeData();
     tNumNeighbors = CentroidDef.GetNumNeighbors();
     for (t=0; t < tNumNeighbors; ++t) {
        //update cluster data
        gPSAbstractClusterData->AddNeighborData(CentroidDef.GetNeighborTractIndex(t), *(*itrGateway));
        //calculate loglikehood ratio and compare against current top cluster
        *itrLoglikelihoodRatios = std::max(*itrLoglikelihoodRatios, gPSAbstractClusterData->CalculateLoglikelihoodRatio(*gpLikelihoodCalculator));
        //update cluster data
        gAbstractClusterData->AddNeighborData(CentroidDef.GetNeighborTractIndex(t), *(*itrGateway));
        //calculate loglikehood ratio and compare against current top cluster
        *itrLoglikelihoodRatios = std::max(*itrLoglikelihoodRatios, gTimeIntervals_S->ComputeLoglikelihoodRatioClusterData(*gAbstractClusterData));
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
void SpaceTimeIncludePurelySpatialCentricAnalysis::CalculateTopClusterAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const AbstractDataSetGateway& DataGateway) {
  if (gTopPSCluster->GetCentroidIndex() != CentroidDef.GetCentroidIndex())
    //re-intialize top cluster object if evaluating data about new centroid
    gTopPSCluster->Initialize(CentroidDef.GetCentroidIndex());
  gPSClusterComparator->Initialize(CentroidDef.GetCentroidIndex());
  gPSClusterComparator->SetEllipseOffset(CentroidDef.GetEllipseIndex(), gDataHub);
  gPSClusterComparator->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, *gTopPSCluster, *gpLikelihoodCalculator);
  //if top cluster was found in this centroid/ellipse, calculate radius now - CentroidNeighbors object wont' be available later
  if (gTopPSCluster->GetEllipseOffset() == CentroidDef.GetEllipseIndex())
    gTopPSCluster->SetNonPersistantNeighborInfo(gDataHub, CentroidDef);

  if (gTopCluster->GetCentroidIndex() != CentroidDef.GetCentroidIndex())
    //re-intialize top cluster object if evaluating data about new centroid
    gTopCluster->Initialize(CentroidDef.GetCentroidIndex());
  gClusterComparator->Initialize(CentroidDef.GetCentroidIndex());
  gClusterComparator->SetEllipseOffset(CentroidDef.GetEllipseIndex(), gDataHub);
  gClusterComparator->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, *gTopCluster, *gTimeIntervals_R);
  //if top cluster was found in this centroid/ellipse, calculate radius now - CentroidNeighbors object wont' be available later
  if (gTopCluster->GetEllipseOffset() == CentroidDef.GetEllipseIndex())
    gTopCluster->SetNonPersistantNeighborInfo(gDataHub, CentroidDef);
}

/** Returns cluster object with greatest llr value among gTopPSCluster and gTopCluster.
    Note that these objects will potentially be updated with next call to
    CalculateTopClusterAboutCentroidDefinition(), so calling Clone() on returned object
    is necessary if you want to keep object around in current state. */
const CCluster& SpaceTimeIncludePurelySpatialCentricAnalysis::GetTopCalculatedCluster() {
  if (!gTopPSCluster->ClusterDefined())
    return *gTopCluster;
  else if (gTopPSCluster->m_nRatio > gTopCluster->m_nRatio)
    return *gTopPSCluster;
  else
    return *gTopCluster;
}

/** Updates CMeasureList objects for each defined data set of gateway object.
    Caller is responsible for ensuring:
    1) 'vDataGateways' contains a number of elements equal to number of requested simulations
    2) DataSetInterface objects are assigned to appropriate structures used to accumulate cluster data
    3) RetrieveLoglikelihoodRatios() has not been called yet */
void SpaceTimeIncludePurelySpatialCentricAnalysis::MonteCarloAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const DataSetGatewayContainer_t& vDataGateways) {
  if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)
    return MonteCarloProspectiveAboutCentroidDefinition(CentroidDef, vDataGateways);

  DataSetGatewayContainer_t::const_iterator  itrGateway, itrGatewayEnd=vDataGateways.end();
  MeasureListContainer_t::iterator           itrMeasureList;

  //perform simulations about current centroid
  itrGateway = vDataGateways.begin();
  itrMeasureList=gvMeasureLists.begin();
  for (; itrGateway != itrGatewayEnd; ++itrGateway, ++itrMeasureList) {
     gPSClusterData->AddMeasureList(CentroidDef, (*itrGateway)->GetDataSetInterface(), (*itrMeasureList));
     gClusterData->AddNeighborDataAndCompare(CentroidDef, (*itrGateway)->GetDataSetInterface(), *gTimeIntervals_S, *(*itrMeasureList));
     //NOTE: This process assumes no-compactness correction for ellipses - otherwise
     //      we would call: gpMeasureList->SetForNextIteration(itrCentroidEnd->GetCentroidIndex());
     //      to calculate the loglikelihood with accumulated measure list for current circle/ellipse.
     //      Note that if we did in fact call that method, the number of times the loglikelihood is calculated
     //      would increase dramtically compared to other Monte Carlo method. We would need an additional
     //      measurelist object for each ellipse/centroid pair to prevent to many loglikelihood calculations, but
     //      that would increase the amount of memory needed.
  }
}

/** Updates CMeasureList objects for each defined data set of gateway object.
    Caller is responsible for ensuring:
    1) 'vDataGateways' contains a number of elements equal to number of requested simulations
    2) DataSetInterface objects are assigned to appropriate structures used to accumulate cluster data
    3) RetrieveLoglikelihoodRatios() has not been called yet */
void SpaceTimeIncludePurelySpatialCentricAnalysis::MonteCarloProspectiveAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const DataSetGatewayContainer_t& vDataGateways) {
  DataSetGatewayContainer_t::const_iterator  itrGateway, itrGatewayEnd=vDataGateways.end();
  MeasureListContainer_t::iterator           itrMeasureList;

  //perform simulations about current centroid
  itrGateway = vDataGateways.begin();
  itrMeasureList=gvMeasureLists.begin();
  for (; itrGateway != itrGatewayEnd; ++itrGateway, ++itrMeasureList) {
     gPSPClusterData->AddMeasureList(CentroidDef, (*itrGateway)->GetDataSetInterface(), (*itrMeasureList));
     gClusterData->AddNeighborDataAndCompare(CentroidDef, (*itrGateway)->GetDataSetInterface(), *gTimeIntervals_S, *(*itrMeasureList));
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
void SpaceTimeIncludePurelySpatialCentricAnalysis::Setup(const AbstractDataSetGateway& RealDataGateway, const DataSetGatewayContainer_t& vSimDataGateways) {
  IncludeClustersType           eIncludeClustersType;

  try {
    //allocate objects used to evaluate real data
    gPSClusterComparator.reset(new CPurelySpatialCluster(gpClusterDataFactory, RealDataGateway, gParameters.GetAreaScanRateType()));
    gTopPSCluster.reset(new CPurelySpatialCluster(gpClusterDataFactory, RealDataGateway, gParameters.GetAreaScanRateType()));

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

