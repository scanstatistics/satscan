//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "SpaceTimeIncludePureAnalysis.h"

/** Constructor */
C_ST_PS_PT_Analysis::C_ST_PS_PT_Analysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
                    :C_ST_PS_Analysis(Parameters, DataHub, PrintDirection) {
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
    if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)
      eIncludeClustersType = ALLCLUSTERS;
    else
      eIncludeClustersType = gParameters.GetIncludeClustersType();

    //create simulation objects based upon which process used to perform simulations
    if (gbMeasureListReplications)
      gpPTClusterData = new TemporalData(DataGateway);
    else {
      //allocate purely temporal, comparator cluster and top cluster
      gpTopPurelyTemporalCluster = new CPurelyTemporalCluster(gpClusterDataFactory, DataGateway, eIncludeClustersType, gDataHub);
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
void C_ST_PS_PT_Analysis::FindTopClusters(const AbtractDataStreamGateway & DataGateway, MostLikelyClustersContainer& TopClustersContainer) {
  IncludeClustersType           eIncludeClustersType;

  //calculate top cluster over all space-time
  C_ST_PS_Analysis::FindTopClusters(DataGateway, TopClustersContainer);
  //detect user cancellation
  if (gPrintDirection.GetIsCanceled())
    return;
  //calculate top purely temporal cluster  
  if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)
    eIncludeClustersType = ALIVECLUSTERS;
  else
    eIncludeClustersType = gParameters.GetIncludeClustersType();
  //create cluster objects
  CPurelyTemporalCluster TopCluster(gpClusterDataFactory, DataGateway, eIncludeClustersType, gDataHub);
  CPurelyTemporalCluster ClusterComparator(gpClusterDataFactory, DataGateway, eIncludeClustersType, gDataHub);
  //iterate through time intervals - looking for top purely temporal cluster
  gpTimeIntervals->CompareClusters(ClusterComparator, TopCluster);
  if (TopCluster.ClusterDefined()) {
    TopClustersContainer.Add(TopCluster);
    TopClustersContainer.SortTopClusters();
  }
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
  tract_t                       k, i, * pNeighborCounts, ** ppSorted_Tract_T;
  unsigned short             ** ppSorted_UShort_T;

  if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)
    return MonteCarloProspective(Interface);
    
  gpMeasureList->Reset();
  //Add measure values for purely space first - so that this cluster's values
  //will be calculated with circle's measure values.
  gpTimeIntervals->CompareMeasures(*gpPTClusterData, *gpMeasureList);
  //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
  for (k=0; k <= gParameters.GetNumTotalEllipses(); ++k) {
     ppSorted_Tract_T = gDataHub.GetSortedArrayAsTract_T(k);
     ppSorted_UShort_T = gDataHub.GetSortedArrayAsUShort_T(k);
     pNeighborCounts = gDataHub.GetNeighborCountArray()[k];
     for (i=0; i < gDataHub.m_nGridTracts; ++i) {
        gpPSClusterData->AddMeasureList(i, Interface, gpMeasureList, pNeighborCounts[i],
                                        ppSorted_UShort_T, ppSorted_Tract_T);
        gpClusterData->AddNeighborDataAndCompare(i, Interface, pNeighborCounts[i],
                                                 ppSorted_UShort_T, ppSorted_Tract_T,
                                                 *gpTimeIntervals, *gpMeasureList);
     }
     gpMeasureList->SetForNextIteration(k);
  }
  return gpMeasureList->GetMaximumLogLikelihoodRatio();
}

/** Returns loglikelihood for Monte Carlo Prospective replication. */
double C_ST_PS_PT_Analysis::MonteCarloProspective(const DataStreamInterface & Interface) {
  tract_t                 k, i, * pNeighborCounts, ** ppSorted_Tract_T;
  unsigned short       ** ppSorted_UShort_T;

  gpMeasureList->Reset();
  //Add measure values for purely space first - so that this cluster's values
  //will be calculated with circle's measure values.
  gpTimeIntervals->CompareMeasures(*gpPTClusterData, *gpMeasureList);
  //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
  for (k=0; k <= gParameters.GetNumTotalEllipses(); ++k) {
     ppSorted_Tract_T = gDataHub.GetSortedArrayAsTract_T(k);
     ppSorted_UShort_T = gDataHub.GetSortedArrayAsUShort_T(k);
     pNeighborCounts = gDataHub.GetNeighborCountArray()[k];
     for (i=0; i < gDataHub.m_nGridTracts; ++i) {
        gpPSPClusterData->AddMeasureList(i, Interface, gpMeasureList, pNeighborCounts[i],
                                         ppSorted_UShort_T, ppSorted_Tract_T);
        gpClusterData->AddNeighborDataAndCompare(i, Interface, pNeighborCounts[i],
                                                 ppSorted_UShort_T, ppSorted_Tract_T,
                                                 *gpTimeIntervals, *gpMeasureList);
     }
     gpMeasureList->SetForNextIteration(k);
  }
  return gpMeasureList->GetMaximumLogLikelihoodRatio();
}

