//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "PurelySpatialAnalysis.h"
#include "ClusterData.h"

/** Constructor */
CPurelySpatialAnalysis::CPurelySpatialAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
                       :CAnalysis(Parameters, DataHub, PrintDirection), gTopShapeClusters(DataHub),
                        gpClusterComparator(0), gpClusterData(0), gpMeasureList(0) {}

/** Desctructor */
CPurelySpatialAnalysis::~CPurelySpatialAnalysis(){
  try {
    delete gpClusterComparator;
    delete gpClusterData;
    delete gpMeasureList;
  }
  catch(...){}
}

/** Allocates objects used during Monte Carlo simulations instead of repeated
    allocations for each simulation. This method must be called prior to MonteCarlo(). */
void CPurelySpatialAnalysis::AllocateSimulationObjects(const AbstractDataSetGateway& DataGateway) {
  try {
    delete gpMeasureList; gpMeasureList=0;
    delete gpClusterData; gpClusterData=0;
    delete gpClusterComparator; gpClusterComparator=0;
    //create simulation objects based upon which process used to perform simulations
    if (geReplicationsProcessType == MeasureListEvaluation) {
      gpClusterData = new SpatialData(DataGateway, gParameters.GetExecuteScanRateType());
      gpMeasureList = GetNewMeasureListObject();
    }
    else { //simulations performed using same process as real data set
      gpClusterComparator = new CPurelySpatialCluster(gpClusterDataFactory, DataGateway, gParameters.GetExecuteScanRateType());
      gTopShapeClusters.SetTopClusters(*gpClusterComparator);
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
    of repeated allocations. This method must be called prior to CalculateTopCluster(). */
void CPurelySpatialAnalysis::AllocateTopClustersObjects(const AbstractDataSetGateway& DataGateway) {
  try {
    delete gpClusterComparator; gpClusterComparator=0;
    gpClusterComparator = new CPurelySpatialCluster(gpClusterDataFactory, DataGateway, gParameters.GetExecuteScanRateType());
    gTopShapeClusters.SetTopClusters(*gpClusterComparator);
  }
  catch (ZdException &x) {
    delete gpClusterComparator; gpClusterComparator=0;
    x.AddCallpath("AllocateTopClustersObjects()","CPurelySpatialAnalysis");
    throw;
  }
}

/** Returns cluster centered at grid point nCenter, with the greatest loglikelihood ratio.
    Caller should not assume that returned reference is persistent, but should either call
    Clone() method or overloaded assignment operator. */
const CCluster& CPurelySpatialAnalysis::CalculateTopCluster(tract_t tCenter, const AbstractDataSetGateway& DataGateway) {
  int                   j;

  gTopShapeClusters.Reset(tCenter);
  for (j=0; j <= gParameters.GetNumTotalEllipses(); ++j) {
     CentroidNeighbors CentroidDef(j, gDataHub);
     CentroidDef.Set(tCenter);
     gpClusterComparator->Initialize(tCenter);
     gpClusterComparator->SetEllipseOffset(j, gDataHub);
     gpClusterComparator->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef,
                                                                     gTopShapeClusters.GetTopCluster(j),
                                                                     *gpLikelihoodCalculator);
  }
  return gTopShapeClusters.GetTopCluster();
}

/** Returns loglikelihood ratio for Monte Carlo replication. */
double CPurelySpatialAnalysis::MonteCarlo(const DataSetInterface& Interface) {
  tract_t               k, i;

  gpMeasureList->Reset();
  for (k=0; k <= gParameters.GetNumTotalEllipses(); ++k) {
     CentroidNeighbors CentroidDef(k, gDataHub);
     for (i=0; i < gDataHub.m_nGridTracts; ++i) {
        CentroidDef.Set(i);
        gpClusterData->AddMeasureList(CentroidDef, Interface, gpMeasureList);
     }
     gpMeasureList->SetForNextIteration(k);
  }
  return gpMeasureList->GetMaximumLogLikelihoodRatio();
}

