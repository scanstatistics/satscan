#include "SaTScan.h"
#pragma hdrstop
#include "SpaceTimeIncludePureAnalysis.h"

/** Constructor */
C_ST_PS_PT_Analysis::C_ST_PS_PT_Analysis(CParameters*  pParameters, CSaTScanData* pData, BasePrint *pPrintDirection)
                    :C_ST_PS_Analysis(pParameters, pData, pPrintDirection) {
  Init();                  
}

/** Desctructor */
C_ST_PS_PT_Analysis::~C_ST_PS_PT_Analysis() {
  try {
    delete gpTopPurelyTemporalCluster;
    delete gpPTClusterComparator;
    delete gpPTClusterData;
  }
  catch(...){}  
}

/** Allocates objects used during simulations, instead of repeated allocations
    for each simulation. Which objects that are allocated depends on whether
    the simluations process uses same process as real data or uses measure list. */
void C_ST_PS_PT_Analysis::AllocateSimulationObjects(const AbtractDataStreamGateway & DataGateway) {
  IncludeClustersType           eIncludeClustersType;

  try {
    //allocate objects for space-time part of simulations
    C_ST_PS_Analysis::AllocateSimulationObjects(DataGateway);
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
      gpPTClusterComparator = gpTopPurelyTemporalCluster->Clone();
    }  
  }
  catch (ZdException &x) {
    delete gpPTClusterData; gpPTClusterData=0;
    delete gpTopPurelyTemporalCluster; gpTopPurelyTemporalCluster=0;
    delete gpPTClusterComparator; gpPTClusterComparator=0;
    x.AddCallpath("AllocateSimulationObjects()","C_ST_PS_PT_Analysis");
    throw;
  }
}

/** Given data gate way, calculates and collects most likely clusters about
    each grid point. Collection of clusters are sorted by loglikelihood ratio
    and condensed based upon overlapping parameter settings.                */
bool C_ST_PS_PT_Analysis::FindTopClusters(const AbtractDataStreamGateway & DataGateway) {
  IncludeClustersType           eIncludeClustersType;
  CTimeIntervals              * pTimeIntervals=0;

  try {
    //calculate top cluster over all space-time
    if (!C_ST_PS_Analysis::FindTopClusters(DataGateway))
      return false;
    if (m_pParameters->GetAnalysisType() == PROSPECTIVESPACETIME)
      eIncludeClustersType = ALIVECLUSTERS;
    else
      eIncludeClustersType = m_pParameters->GetIncludeClustersType();
    //create new time intervals object
    pTimeIntervals = GetNewTimeIntervalsObject(eIncludeClustersType);
    //create cluster objects
    CPurelyTemporalCluster TopCluster(gpClusterDataFactory, DataGateway, eIncludeClustersType, *m_pData, *gpPrintDirection);
    CPurelyTemporalCluster ClusterComparator(gpClusterDataFactory, DataGateway, eIncludeClustersType, *m_pData, *gpPrintDirection);
    //iterate through time intervals - looking for top purely temporal cluster
    pTimeIntervals->CompareClusters(ClusterComparator, TopCluster);
    if (TopCluster.ClusterDefined()) {
      m_pTopClusters[m_nClustersRetained] = TopCluster.Clone();
      m_pTopClusters[m_nClustersRetained]->SetStartAndEndDates(m_pData->GetTimeIntervalStartTimes(), m_pData->m_nTimeIntervals);
      ++m_nClustersRetained;
      //re-sort top clusters array given addition of purely temporal cluster
      SortTopClusters();
    }
    delete pTimeIntervals; pTimeIntervals=0;
  }
  catch (ZdException &x) {
    delete pTimeIntervals;
    x.AddCallpath("FindTopClusters()","C_ST_PS_PT_Analysis");
    throw;
  }
  return true;
}

double C_ST_PS_PT_Analysis::FindTopRatio(const AbtractDataStreamGateway & DataGateway) {
  int                   i;
  double                dMaxLogLikelihoodRatio=0;

  //calculate top loglikelihood for purely spatial/space-time portion
  dMaxLogLikelihoodRatio = C_ST_PS_Analysis::FindTopRatio(DataGateway);
  //calculate top loglikelihood for purely temporal portion
  gpTopPurelyTemporalCluster->Initialize();
  gpPTClusterComparator->Initialize();
  gpTimeIntervals->CompareClusters(*gpPTClusterComparator, *gpTopPurelyTemporalCluster);
  return std::max(gpTopPurelyTemporalCluster->m_nRatio, dMaxLogLikelihoodRatio);
}  

/** internal initialization */
void C_ST_PS_PT_Analysis::Init() {
  gpTopPurelyTemporalCluster=0;
  gpPTClusterComparator=0;
  gpPTClusterData=0;
}

/** Returns loglikelihood for Monte Carlo replication. */
double C_ST_PS_PT_Analysis::MonteCarlo(const DataStreamInterface & Interface) {
  double                        dMaxLogLikelihoodRatio;
  tract_t                       k, i;

  if (m_pParameters->GetAnalysisType() == PROSPECTIVESPACETIME)
    return MonteCarloProspective(Interface);
    
  gpMeasureList->Reset();
  //Add measure values for purely space first - so that this cluster's values
  //will be calculated with circle's measure values.
  gpTimeIntervals->CompareMeasures(gpPTClusterData, gpMeasureList);
  //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
  for (k=0; k <= m_pParameters->GetNumTotalEllipses(); ++k) {
     for (i=0; i < m_pData->m_nGridTracts; ++i) {
        m_pData->SetImpliedCentroid(k, i);
        gpPSClusterData->AddMeasureList(Interface, gpMeasureList, m_pData);
        gpClusterData->AddNeighborDataAndCompare(Interface, m_pData, gpTimeIntervals, gpMeasureList);
     }
     gpMeasureList->SetForNextIteration(k);
  }
  return gpMeasureList->GetMaximumLogLikelihoodRatio();
}

/** Returns loglikelihood for Monte Carlo Prospective replication. */
double C_ST_PS_PT_Analysis::MonteCarloProspective(const DataStreamInterface & Interface) {
  double                        dMaxLogLikelihoodRatio;
  tract_t                       k, i;

  gpMeasureList->Reset();
  //Add measure values for purely space first - so that this cluster's values
  //will be calculated with circle's measure values.
  gpTimeIntervals->CompareMeasures(gpPTClusterData, gpMeasureList);
  //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
  for (k=0; k <= m_pParameters->GetNumTotalEllipses(); ++k) {
     for (i=0; i < m_pData->m_nGridTracts; ++i) {
        m_pData->SetImpliedCentroid(k, i);
        gpPSPClusterData->AddMeasureList(Interface, gpMeasureList, m_pData);
        gpClusterData->AddNeighborDataAndCompare(Interface, m_pData, gpTimeIntervals, gpMeasureList);
     }
     gpMeasureList->SetForNextIteration(k);
  }
  return gpMeasureList->GetMaximumLogLikelihoodRatio();
}

