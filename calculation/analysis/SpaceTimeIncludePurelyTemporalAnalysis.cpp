//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "SpaceTimeIncludePurelyTemporalAnalysis.h"
#include "ClusterData.h"
#include "MostLikelyClustersContainer.h"

/** Constructor */
C_ST_PT_Analysis::C_ST_PT_Analysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
                 :CSpaceTimeAnalysis(Parameters, DataHub, PrintDirection) {
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
void C_ST_PT_Analysis::AllocateSimulationObjects(const AbtractDataSetGateway & DataGateway) {
  IncludeClustersType           eIncludeClustersType;

  try {
    //allocate objects for space-time part of simulations
    CSpaceTimeAnalysis::AllocateSimulationObjects(DataGateway);
    if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)
      eIncludeClustersType = ALLCLUSTERS;
    else
      eIncludeClustersType = gParameters.GetIncludeClustersType();

    //create simulation objects based upon which process used to perform simulations
    if (geReplicationsProcessType == MeasureListEvaluation)
      gpPTClusterData = new TemporalData(DataGateway);
    else {
      //allocate purely temporal, comparator cluster and top cluster
      gpTopPurelyTemporalCluster = new CPurelyTemporalCluster(gpClusterDataFactory, DataGateway, eIncludeClustersType, gDataHub);
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
void C_ST_PT_Analysis::FindTopClusters(const AbtractDataSetGateway & DataGateway, MostLikelyClustersContainer& TopClustersContainer) {
  IncludeClustersType           eIncludeClustersType;

  try {
    //calculate top cluster over all space-time
    CSpaceTimeAnalysis::FindTopClusters(DataGateway, TopClustersContainer);
    //detect user cancellation
    if (gPrintDirection.GetIsCanceled())
      return;
    //calculate top purely temporal cluster
    if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)
      eIncludeClustersType = ALIVECLUSTERS;
    else
      eIncludeClustersType = gParameters.GetIncludeClustersType();
    //create top cluster
    CPurelyTemporalCluster TopCluster(gpClusterDataFactory, DataGateway, eIncludeClustersType, gDataHub);
    //create comparator cluster
    CPurelyTemporalCluster ClusterComparator(gpClusterDataFactory, DataGateway, eIncludeClustersType, gDataHub);
    gpTimeIntervals->CompareClusters(ClusterComparator, TopCluster);
    TopClustersContainer.Add(TopCluster);
  }
  catch (ZdException &x) {
    x.AddCallpath("FindTopClusters()","C_ST_PT_Analysis");
    throw;
  }
}

/** calculates largest loglikelihood ratio for simulation data - using same
    process as real data */
double C_ST_PT_Analysis::FindTopRatio(const AbtractDataSetGateway & DataGateway) {
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
double C_ST_PT_Analysis::MonteCarlo(const DataSetInterface & Interface) {
  tract_t               k, i;
  CentroidNeighbors     CentroidDef;

  gpMeasureList->Reset();
  //compare purely temporal cluster in same ratio correction as circle
  gpTimeIntervals->CompareMeasures(*gpPTClusterData, *gpMeasureList);
  //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
  for (k=0; k <= gParameters.GetNumTotalEllipses(); ++k) {
     for (i=0; i < gDataHub.m_nGridTracts; ++i)
        gpClusterData->AddNeighborDataAndCompare(CentroidDef.Set(k, i, gDataHub), Interface, *gpTimeIntervals, *gpMeasureList);
     gpMeasureList->SetForNextIteration(k);
  }
  return gpMeasureList->GetMaximumLogLikelihoodRatio();
}

