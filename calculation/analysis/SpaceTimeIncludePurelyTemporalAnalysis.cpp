#include "SaTScan.h"
#pragma hdrstop
#include "SpaceTimeIncludePurelyTemporalAnalysis.h"

/** Constructor */
C_ST_PT_Analysis::C_ST_PT_Analysis(CParameters*  pParameters, CSaTScanData* pData, BasePrint *pPrintDirection)
                 :CSpaceTimeAnalysis(pParameters, pData, pPrintDirection) {
  Init();
}

/** Destructor */
C_ST_PT_Analysis::~C_ST_PT_Analysis() {
  try {
    delete gpTopPurelyTemporalCluster;
    delete gpPurelyTemporalClusterComparator;
    delete gpPTClusterData;
  }
  catch (...){}
}

/** Allocates objects used during simulations, instead of repeated allocations
    for each simulation. Which objects that are allocated depends on whether
    the simluations process uses same process as real data or uses measure list. */
void C_ST_PT_Analysis::AllocateSimulationObjects(const AbtractDataStreamGateway & DataGateway) {
  IncludeClustersType           eIncludeClustersType;

  try {
    //allocate objects for space-time part of simulations
    CSpaceTimeAnalysis::AllocateSimulationObjects(DataGateway);
    if (m_pParameters->GetAnalysisType() == PROSPECTIVESPACETIME)
      eIncludeClustersType = ALLCLUSTERS;
    else
      eIncludeClustersType = m_pParameters->GetIncludeClustersType();

    //create simulation objects based upon which process used to perform simulations
    if (gbMeasureListReplications)
      gpPTClusterData = new TemporalData(DataGateway);
    else {
      //allocate purely temporal, comparator cluster and top cluster
      gpTopPurelyTemporalCluster = new CPurelyTemporalCluster(gpClusterDataFactory, DataGateway, eIncludeClustersType, *m_pData, *gpPrintDirection);
      gpPurelyTemporalClusterComparator = gpTopPurelyTemporalCluster->Clone();
    }  
  }
  catch (ZdException &x) {
    delete gpPTClusterData; gpPTClusterData=0;
    delete gpTopPurelyTemporalCluster; gpTopPurelyTemporalCluster=0;
    delete gpPurelyTemporalClusterComparator; gpPurelyTemporalClusterComparator=0;
    x.AddCallpath("AllocateSimulationObjects()","C_ST_PT_Analysis");
    throw;
  }
}

/** calculates top cluster about each centroid/grid point - iterating through
    all possible time intervals - populates top cluster array with most likely
    cluster about each grid point plus , possible, one more for purely temporal
    cluster. */
bool C_ST_PT_Analysis::FindTopClusters(const AbtractDataStreamGateway & DataGateway) {
  IncludeClustersType           eIncludeClustersType;

  try {
    //calculate top cluster over all space-time
    if (!CSpaceTimeAnalysis::FindTopClusters(DataGateway))
      return false;

    //calculate top purely temporal cluster
    if (m_pParameters->GetAnalysisType() == PROSPECTIVESPACETIME)
      eIncludeClustersType = ALIVECLUSTERS;
    else
      eIncludeClustersType = m_pParameters->GetIncludeClustersType();
    //create top cluster
    CPurelyTemporalCluster TopCluster(gpClusterDataFactory, DataGateway, eIncludeClustersType, *m_pData, *gpPrintDirection);
    //create comparator cluster
    CPurelyTemporalCluster ClusterComparator(gpClusterDataFactory, DataGateway, eIncludeClustersType, *m_pData, *gpPrintDirection);
    gpTimeIntervals->CompareClusters(ClusterComparator, TopCluster);
    if (TopCluster.ClusterDefined()) {
      m_pTopClusters[m_nClustersRetained] = TopCluster.Clone();
      m_pTopClusters[m_nClustersRetained]->SetStartAndEndDates(m_pData->GetTimeIntervalStartTimes(), m_pData->m_nTimeIntervals);
      m_nClustersRetained++;
      SortTopClusters();
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("FindTopClusters()","C_ST_PT_Analysis");
    throw;
  }
  return true;
}

/** calculates largest loglikelihood ratio for simulation data - using same
    process as real data */
double C_ST_PT_Analysis::FindTopRatio(const AbtractDataStreamGateway & DataGateway) {
  double  dMaxLogLikelihoodRatio=0;

  dMaxLogLikelihoodRatio = CSpaceTimeAnalysis::FindTopRatio(DataGateway);
  gpTopPurelyTemporalCluster->Initialize();
  gpPurelyTemporalClusterComparator->Initialize();
  gpTimeIntervals->CompareClusters(*gpPurelyTemporalClusterComparator, *gpTopPurelyTemporalCluster);
  return std::max(gpTopPurelyTemporalCluster->m_nRatio, dMaxLogLikelihoodRatio);
}

/** internal initialization */
void C_ST_PT_Analysis::Init() {
  gpTopPurelyTemporalCluster=0;
  gpPurelyTemporalClusterComparator=0;
  gpPTClusterData=0;
}

/** Returns loglikelihood for Monte Carlo replication. */
double C_ST_PT_Analysis::MonteCarlo(const DataStreamInterface & Interface) {
  double                        dMaxLogLikelihoodRatio;
  tract_t                       k, i, j, iNumNeighbors;

  gpMeasureList->Reset();
  //compare purely temporal cluster in same ratio correction as circle
  gpTimeIntervals->CompareMeasures(gpPTClusterData, gpMeasureList);
  //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
  for (k=0; k <= m_pParameters->GetNumTotalEllipses(); ++k) {
     for (i=0; i < m_pData->m_nGridTracts; ++i) {
        m_pData->SetImpliedCentroid(k, i);
        gpClusterData->AddNeighborDataAndCompare(Interface, m_pData, gpTimeIntervals, gpMeasureList);
     }
     gpMeasureList->SetForNextIteration(k);
  }
  return gpMeasureList->GetMaximumLogLikelihoodRatio();
}

/** Sets maximum number of clusters in top cluster array */
void C_ST_PT_Analysis::SetMaxNumClusters() {
  m_nMaxClusters = m_pData->m_nGridTracts+1;
}

