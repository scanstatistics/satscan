//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "PurelySpatialAnalysis.h"

/** Constructor */
CPurelySpatialAnalysis::CPurelySpatialAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
                       :CAnalysis(Parameters, DataHub, PrintDirection) {
  try {
    Init();
    Setup();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CPurelySpatialAnalysis");
    throw;
  }
}

/** Desctructor */
CPurelySpatialAnalysis::~CPurelySpatialAnalysis(){
  try {
    delete gpTopShapeClusters;
    delete gpClusterComparator;
    delete gpClusterData;
    delete gpMeasureList;
  }
  catch(...){}
}

void CPurelySpatialAnalysis::AllocateSimulationObjects(const AbtractDataStreamGateway & DataGateway) {
  try {
    delete gpClusterComparator; gpClusterComparator=0;
    //create simulation objects based upon which process used to perform simulations
    if (gbMeasureListReplications) {
      gpClusterData = new SpatialData(DataGateway, gParameters.GetAreaScanRateType());
      gpMeasureList = GetNewMeasureListObject();
    }
    else { //simulations performed using same process as real data set
      gpClusterComparator = new CPurelySpatialCluster(gpClusterDataFactory, DataGateway, gParameters.GetAreaScanRateType());
      gpTopShapeClusters->SetTopClusters(*gpClusterComparator);
    }
  }
  catch (ZdException &x) {
    delete gpClusterComparator; gpClusterComparator=0;
    delete gpClusterData; gpClusterData=0;
    delete gpMeasureList; gpMeasureList=0;
    x.AddCallpath("AllocateSimulationObjects()","CPurelySpatialAnalysis");
    throw;
  }
}

/** Allocates objects used during calculation of most likely clusters, instead
    of repeated allocations for each simulation.                                */
void CPurelySpatialAnalysis::AllocateTopClustersObjects(const AbtractDataStreamGateway & DataGateway) {
  try {
    delete gpClusterComparator; gpClusterComparator=0;
    gpClusterComparator = new CPurelySpatialCluster(gpClusterDataFactory, DataGateway, gParameters.GetAreaScanRateType());
    gpTopShapeClusters->SetTopClusters(*gpClusterComparator);
  }
  catch (ZdException &x) {
    delete gpClusterComparator; gpClusterComparator=0;
    x.AddCallpath("AllocateTopClustersObjects()","CPurelySpatialAnalysis");
    throw;
  }
}

/** Returns cluster centered at grid point nCenter, with the greatest loglikelihood.
    Caller is responsible for deleting returned cluster. */
const CCluster& CPurelySpatialAnalysis::CalculateTopCluster(tract_t tCenter, const AbtractDataStreamGateway & DataGateway) {
  int                           i, j;

  gpTopShapeClusters->Reset(tCenter);
  for (j=0; j <= gParameters.GetNumTotalEllipses(); ++j) {
     gpClusterComparator->Initialize(tCenter);
     gpClusterComparator->SetEllipseOffset(j);                       // store the ellipse link in the cluster obj
     gpClusterComparator->SetNonCompactnessPenalty((j == 0 || !gParameters.GetNonCompactnessPenalty() ? 1 : gDataHub.GetShapesArray()[j - 1]));
     CPurelySpatialCluster & TopCluster = (CPurelySpatialCluster&)(gpTopShapeClusters->GetTopCluster(j));
     gpClusterComparator->AddNeighborDataAndCompare(j, tCenter, DataGateway, &gDataHub, TopCluster, *gpLikelihoodCalculator);
  }
  return gpTopShapeClusters->GetTopCluster();
}

/** internal initialization */
void CPurelySpatialAnalysis::Init() {
  gpTopShapeClusters=0;
  gpClusterComparator=0;
  gpClusterData=0;
  gpMeasureList=0;
}

/** Returns loglikelihood for Monte Carlo replication. */
double CPurelySpatialAnalysis::MonteCarlo(const DataStreamInterface & Interface) {
  tract_t               k, i, * pNeighborCounts, ** ppSorted_Tract_T;
  unsigned short     ** ppSorted_UShort_T;

  gpMeasureList->Reset();
  for (k=0; k <= gParameters.GetNumTotalEllipses(); ++k) { //circle is 0 offset... (always there)
     ppSorted_Tract_T = gDataHub.GetSortedArrayAsTract_T(k);
     ppSorted_UShort_T = gDataHub.GetSortedArrayAsUShort_T(k);
     pNeighborCounts = gDataHub.GetNeighborCountArray()[k];
     for (i=0; i < gDataHub.m_nGridTracts; ++i) {
        gpClusterData->AddMeasureList(i, Interface, gpMeasureList, pNeighborCounts[i],
                                      ppSorted_UShort_T, ppSorted_Tract_T);
     }
     gpMeasureList->SetForNextIteration(k);
  }
  return gpMeasureList->GetMaximumLogLikelihoodRatio();
}

void CPurelySpatialAnalysis::Setup() {
  try {
    gpTopShapeClusters = new TopClustersContainer(gDataHub);
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()", "CPurelySpatialAnalysis");
    delete gpTopShapeClusters;
    throw;
  }
}

