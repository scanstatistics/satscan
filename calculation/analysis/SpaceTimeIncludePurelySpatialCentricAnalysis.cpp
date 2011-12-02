//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SpaceTimeIncludePurelySpatialCentricAnalysis.h"
#include "SSException.h"

/** constructor */
SpaceTimeIncludePurelySpatialCentricAnalysis::SpaceTimeIncludePurelySpatialCentricAnalysis(const CParameters& Parameters,
                                                                                           const CSaTScanData& Data,
                                                                                           BasePrint& PrintDirection,
                                                                                           const AbstractDataSetGateway& RealDataGateway,
                                                                                           const DataSetGatewayContainer_t& vSimDataGateways)
                                             :SpaceTimeCentricAnalysis(Parameters, Data, PrintDirection, RealDataGateway, vSimDataGateways), 
                                              _top_PS_Clusters(Data) {
  try {
    Setup(RealDataGateway, vSimDataGateways);
  }
  catch (prg_exception& x) {
    x.addTrace("constructor()","SpaceTimeIncludePurelySpatialCentricAnalysis");
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

  gTimeIntervals_S->setIntervalRange(CentroidDef.GetCentroidIndex());
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
        *itrLoglikelihoodRatios = std::max(*itrLoglikelihoodRatios, gTimeIntervals_S->ComputeMaximizingValue(*gAbstractClusterData));
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
  if (_top_PS_Clusters.getClusterSet(0).get(0).getCluster().GetCentroidIndex() != CentroidDef.GetCentroidIndex())
    //re-intialize top cluster objects if evaluating data about new centroid
    _top_PS_Clusters.reset(CentroidDef.GetCentroidIndex());
  gTimeIntervals_R->setIntervalRange(CentroidDef.GetCentroidIndex());
  gPSClusterComparator->Initialize(CentroidDef.GetCentroidIndex());
  _top_PS_Clusters.resetNeighborCounts(CentroidDef);
  gPSClusterComparator->SetEllipseOffset(CentroidDef.GetEllipseIndex(), gDataHub);
  gPSClusterComparator->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, _top_PS_Clusters.getClusterSet(0), *gpLikelihoodCalculator);
  //if top cluster was found in this centroid/ellipse, calculate radius now - CentroidNeighbors object wont' be available later
  _top_PS_Clusters.setClusterNonPersistantNeighborInfo(CentroidDef);

  if (_topClusters.getClusterSet(0).get(0).getCluster().GetCentroidIndex() != CentroidDef.GetCentroidIndex())
    //re-intialize top cluster objects if evaluating data about new centroid
    _topClusters.reset(CentroidDef.GetCentroidIndex());
  gClusterComparator->Initialize(CentroidDef.GetCentroidIndex());
  gClusterComparator->SetEllipseOffset(CentroidDef.GetEllipseIndex(), gDataHub);
  gClusterComparator->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, _topClusters.getClusterSet(0), *gTimeIntervals_R);
  //if top cluster was found in this centroid/ellipse, calculate radius now - CentroidNeighbors object wont' be available later
  _topClusters.setClusterNonPersistantNeighborInfo(CentroidDef);
}

/** Returns cluster object with greatest llr value among gTopPSCluster and gTopCluster.
    Note that these objects will potentially be updated with next call to
    CalculateTopClusterAboutCentroidDefinition(), so calling Clone() on returned object
    is necessary if you want to keep object around in current state. */
const SharedClusterVector_t SpaceTimeIncludePurelySpatialCentricAnalysis::GetTopCalculatedClusters() {
  SharedClusterVector_t topClusters, psClusters; 
  _topClusters.getTopClusters(topClusters);
  _top_PS_Clusters.getTopClusters(psClusters);

  for (size_t t=0; t < topClusters.size(); ++t) {
      if (psClusters[t]->ClusterDefined() && std::fabs(psClusters[t]->m_nRatio - topClusters[t]->m_nRatio) > DBL_CMP_TOLERANCE && psClusters[t]->m_nRatio > topClusters[t]->m_nRatio) {
        topClusters[t] = psClusters[t];
      }
  }
  return _topClusters.getTopClusters(topClusters);
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

  gTimeIntervals_S->setIntervalRange(CentroidDef.GetCentroidIndex());
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

  gTimeIntervals_S->setIntervalRange(CentroidDef.GetCentroidIndex());
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
  try {
    //allocate objects used to evaluate real data
    gPSClusterComparator.reset(new CPurelySpatialCluster(gpClusterDataFactory, RealDataGateway));
    _topClusters.setTopClusters(*gPSClusterComparator.get());

    //allocate objects used to evaluate simulation data
    if (gParameters.GetNumReplicationsRequested()) {
      if (geReplicationsProcessType == MeasureListEvaluation) {
        if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)
          gPSPClusterData.reset(new ProspectiveSpatialData(gDataHub, *(*vSimDataGateways.begin())));
        else
          gPSClusterData.reset(new SpatialData(*(*vSimDataGateways.begin())));
      }
      else {
        if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)
          gPSAbstractClusterData.reset(gpClusterDataFactory->GetNewProspectiveSpatialClusterData(gDataHub, *(*vSimDataGateways.begin())));
        else
          gPSAbstractClusterData.reset(gpClusterDataFactory->GetNewSpatialClusterData(*(*vSimDataGateways.begin())));
      }
    }
  }
  catch (prg_exception& x) {
    gPSClusterComparator.reset(0); gPSPClusterData.reset(0); gPSClusterData.reset(0); gPSAbstractClusterData.reset(0);
    x.addTrace("Setup()","SpaceTimeIncludePurelySpatialCentricAnalysis");
    throw;
  }
}

