//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "SpaceTimeAnalysis.h"

/** Constructor */
CSpaceTimeAnalysis::CSpaceTimeAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
                   :CAnalysis(Parameters, DataHub, PrintDirection) {
  try {
    Init();
    Setup();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CSpaceTimeAnalysis");
    throw;
  }
}

/** Destructor */
CSpaceTimeAnalysis::~CSpaceTimeAnalysis() {
  try {
    delete gpTopShapeClusters;
    delete gpClusterComparator;
    delete gpClusterData;
    delete gpMeasureList;
    delete gpTimeIntervals;
  }
  catch(...){}
}

/** Allocates objects used during simulations, instead of repeated allocations
    for each simulation. Which objects that are allocated depends on whether
    the simluations process uses same process as real data or uses measure list. */
void CSpaceTimeAnalysis::AllocateSimulationObjects(const AbtractDataStreamGateway & DataGateway) {
  IncludeClustersType           eIncludeClustersType;

  try {
    //delete object used during real data
    delete gpClusterComparator; gpClusterComparator=0;
    //create new time intervals object - delete existing object used during real data process
    delete gpTimeIntervals; gpTimeIntervals=0;
    if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)
      eIncludeClustersType = ALLCLUSTERS;
    else
      eIncludeClustersType = gParameters.GetIncludeClustersType();
    gpTimeIntervals = GetNewTimeIntervalsObject(eIncludeClustersType);

    //create simulation objects based upon which process used to perform simulations
    if (gbMeasureListReplications) {
      //create new cluster data object
      gpClusterData = new SpaceTimeData(DataGateway);
      //create new measure list object
      gpMeasureList = GetNewMeasureListObject();
    }
    else { //simulations performed using same process as real data set
      //create cluster object used as comparator when iterating over centroids and time intervals
      gpClusterComparator = new CSpaceTimeCluster(gpClusterDataFactory, DataGateway);
      //initialize list of top circle/ellipse clusters
      gpTopShapeClusters->SetTopClusters(*gpClusterComparator);
    }
  }
  catch (ZdException &x) {
    delete gpClusterData; gpClusterData=0;
    delete gpMeasureList; gpMeasureList=0;
    delete gpTimeIntervals; gpTimeIntervals=0;
    delete gpClusterComparator; gpClusterComparator=0;
    x.AddCallpath("AllocateSimulationObjects()","CSpaceTimeAnalysis");
    throw;
  }
}

/** Allocates objects used during calculation of most likely clusters, instead
    of repeated allocations for each grid point.                             */
void CSpaceTimeAnalysis::AllocateTopClustersObjects(const AbtractDataStreamGateway & DataGateway) {
  IncludeClustersType           eIncludeClustersType;

  try {
    //create new time intervals object - delete existing object used during real data process
    delete gpTimeIntervals; gpTimeIntervals=0;
    if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)
      eIncludeClustersType = ALIVECLUSTERS;
    else
      eIncludeClustersType = gParameters.GetIncludeClustersType();
    gpTimeIntervals = GetNewTimeIntervalsObject(eIncludeClustersType);
    //create cluster object used as comparator when iterating over centroids and time intervals
    gpClusterComparator = new CSpaceTimeCluster(gpClusterDataFactory, DataGateway);
    //initialize list of top circle/ellipse clusters
    gpTopShapeClusters->SetTopClusters(*gpClusterComparator);
  }
  catch (ZdException &x) {
    delete gpClusterComparator; gpClusterComparator=0;
    delete gpTimeIntervals; gpTimeIntervals=0;
    x.AddCallpath("AllocateTopClustersObjects()","CSpaceTimeAnalysis");
    throw;
  }
}

/** Returns cluster centered at grid point nCenter, with the greatest loglikelihood.
    Caller is responsible for deleting returned cluster. */
const CCluster & CSpaceTimeAnalysis::CalculateTopCluster(tract_t tCenter, const AbtractDataStreamGateway & DataGateway) {
  tract_t       k;

  gpTopShapeClusters->Reset(tCenter);
  //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
  for (k=0; k <= gParameters.GetNumTotalEllipses(); ++k) {
     gpClusterComparator->Initialize(tCenter);
     gpClusterComparator->SetRate(gParameters.GetAreaScanRateType());
     gpClusterComparator->SetEllipseOffset(k);
     gpClusterComparator->SetNonCompactnessPenalty((k == 0 || !gParameters.GetNonCompactnessPenalty() ? 1 : gDataHub.GetShapesArray()[k - 1]));
     CSpaceTimeCluster & TopCluster = (CSpaceTimeCluster&)(gpTopShapeClusters->GetTopCluster(k));
     gpClusterComparator->AddNeighborDataAndCompare(k, tCenter, DataGateway, &gDataHub, TopCluster, gpTimeIntervals);
  }
  return gpTopShapeClusters->GetTopCluster();
}

/** internal initialization */
void CSpaceTimeAnalysis::Init() {
  gpTopShapeClusters=0;
  gpClusterComparator=0;
  gpClusterData=0;
  gpMeasureList=0;
  gpTimeIntervals=0;
}

/** Returns loglikelihood for Monte Carlo replication. */
double CSpaceTimeAnalysis::MonteCarlo(const DataStreamInterface & Interface) {
  double                        dMaxLogLikelihoodRatio;
  tract_t                       k, i, j, iNumNeighbors;

  gpMeasureList->Reset();
  //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
  for (k=0; k <= gParameters.GetNumTotalEllipses(); k++) {
     for (i=0; i < gDataHub.m_nGridTracts; i++) {
        gpClusterData->AddNeighborDataAndCompare(k, i, Interface, &gDataHub, gpTimeIntervals, gpMeasureList);
     }
     gpMeasureList->SetForNextIteration(k);
  }
  return gpMeasureList->GetMaximumLogLikelihoodRatio();
}

/** internal setup function */
void CSpaceTimeAnalysis::Setup() {
  try {
    gpTopShapeClusters = new TopClustersContainer(gDataHub);
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()", "CSpaceTimeAnalysis");
    delete gpTopShapeClusters;
    throw;
  }
}

