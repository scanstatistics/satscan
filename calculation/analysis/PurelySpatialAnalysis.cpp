#include "SaTScan.h"
#pragma hdrstop
#include "PurelySpatialAnalysis.h"

/** Constructor */
CPurelySpatialAnalysis::CPurelySpatialAnalysis(CParameters*  pParameters, CSaTScanData* pData, BasePrint *pPrintDirection)
                       :CAnalysis(pParameters, pData, pPrintDirection) {
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
      gpClusterData = new SpatialData(DataGateway, m_pParameters->GetAreaScanRateType());
      gpMeasureList = GetNewMeasureListObject();
    }
    else { //simulations performed using same process as real data set
      gpClusterComparator = new CPurelySpatialCluster(gpClusterDataFactory, DataGateway, m_pParameters->GetAreaScanRateType(), gpPrintDirection);
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
    gpClusterComparator = new CPurelySpatialCluster(gpClusterDataFactory, DataGateway, m_pParameters->GetAreaScanRateType(), gpPrintDirection);
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
  CModel                      & Model(m_pData->GetProbabilityModel());

  gpTopShapeClusters->Reset(tCenter);
  for (j=0; j <= m_pParameters->GetNumTotalEllipses(); ++j) {
     m_pData->SetImpliedCentroid(j, tCenter);
     gpClusterComparator->Initialize(tCenter);
     gpClusterComparator->SetEllipseOffset(j);                       // store the ellipse link in the cluster obj
     gpClusterComparator->SetDuczmalCorrection((j == 0 || !m_pParameters->GetDuczmalCorrectEllipses() ? 1 : m_pData->GetShapesArray()[j - 1]));
     CPurelySpatialCluster & TopCluster = (CPurelySpatialCluster&)(gpTopShapeClusters->GetTopCluster(j));
     gpClusterComparator->AddNeighborDataAndCompare(DataGateway, m_pData, TopCluster, *gpLikelihoodCalculator);
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
  double                          dMaxLogLikelihoodRatio;
  tract_t                         k, i;

  gpMeasureList->Reset();
  for (k=0; k <= m_pParameters->GetNumTotalEllipses(); ++k) { //circle is 0 offset... (always there)
     for (i=0; i < m_pData->m_nGridTracts; ++i) {
        m_pData->SetImpliedCentroid(k, i);
        gpClusterData->AddMeasureList(Interface, gpMeasureList, m_pData);
     }
     gpMeasureList->SetForNextIteration(k);
  }
  return gpMeasureList->GetMaximumLogLikelihoodRatio();
}

void CPurelySpatialAnalysis::Setup() {
  try {
    gpTopShapeClusters = new TopClustersContainer(*m_pData);
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()", "CPurelySpatialAnalysis");
    delete gpTopShapeClusters;
    throw;
  }
}

