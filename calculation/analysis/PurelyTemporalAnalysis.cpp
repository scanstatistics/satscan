//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "PurelyTemporalAnalysis.h"

/** Constructor */
CPurelyTemporalAnalysis::CPurelyTemporalAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
                        :CAnalysis(Parameters, DataHub, PrintDirection) {
  Init();
}

/** Destructor */
CPurelyTemporalAnalysis::~CPurelyTemporalAnalysis() {
  try {
    delete gpTopCluster;
    delete gpClusterData;
    delete gpMeasureList;
    delete gpTimeIntervals;
    delete gpClusterComparator;
  }
  catch (...){}
}

/** Allocates objects used during simulations, instead of repeated allocations
    for each simulation. Which objects that are allocated depends on whether
    the simluations process uses same process as real data or uses measure list. */
void CPurelyTemporalAnalysis::AllocateSimulationObjects(const AbtractDataStreamGateway& DataGateway) {
  IncludeClustersType           eIncludeClustersType;

  try {
    //create new time intervals object - delete existing object used during real data process
    delete gpTimeIntervals; gpTimeIntervals=0;
    if (gParameters.GetAnalysisType() == PROSPECTIVEPURELYTEMPORAL)
      //for prospective analyses, allocate time object with ALLCLUSTERS for simulations
      eIncludeClustersType = ALLCLUSTERS;
    else
      eIncludeClustersType = gParameters.GetIncludeClustersType();
    gpTimeIntervals = GetNewTemporalDataEvaluatorObject(eIncludeClustersType);

    //create simulation objects based upon which process used to perform simulations
    if (gbMeasureListReplications) {
      //create new cluster data object
      gpClusterData = new TemporalData(DataGateway);
      //create new measure list object
      gpMeasureList = GetNewMeasureListObject();
    }
    else { //simulations performed using same process as real data set
      gpTopCluster = new CPurelyTemporalCluster(gpClusterDataFactory, DataGateway, eIncludeClustersType, gDataHub);
      gpClusterComparator = new CPurelyTemporalCluster(gpClusterDataFactory, DataGateway, eIncludeClustersType, gDataHub);
    }
  }
  catch (ZdException &x) {
    delete gpClusterData; gpClusterData=0;
    delete gpMeasureList; gpMeasureList=0;
    delete gpTimeIntervals; gpTimeIntervals=0;
    delete gpClusterComparator; gpClusterComparator=0;
    delete gpTopCluster;gpTopCluster=0;
    x.AddCallpath("AllocateSimulationObjects()","CPurelyTemporalAnalysis");
    throw;
  }
}

/** Allocates objects used during calculation of most likely clusters, instead
    of repeated allocations for each simulation.
    NOTE: No action taken in this function for this class. Objects are allocated
          directly in CPurelyTemporalAnalysis::FindTopClusters(). */
void CPurelyTemporalAnalysis::AllocateTopClustersObjects(const AbtractDataStreamGateway& DataGateway) {}

const CCluster & CPurelyTemporalAnalysis::CalculateTopCluster(tract_t tCenter, const AbtractDataStreamGateway & DataGateway) {
  ZdGenerateException("CalculateTopCluster() can not be called for CPurelyTemporalAnalysis.","CPurelyTemporalAnalysis");
  return *gpTopCluster;
}

/** Calculate most likely, purely temporal, cluster and adds clone of top cluster
    to top cluster array. */
void CPurelyTemporalAnalysis::FindTopClusters(const AbtractDataStreamGateway& DataGateway, MostLikelyClustersContainer& TopClustersContainer) {
  IncludeClustersType           eIncludeClustersType;
  CTimeIntervals              * pTimeIntervals=0;  

  try {
    //determine the type of clusters to compare
    if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)
      eIncludeClustersType = ALIVECLUSTERS;
    else
      eIncludeClustersType = gParameters.GetIncludeClustersType();
    //create cluster objects
    CPurelyTemporalCluster TopCluster(gpClusterDataFactory, DataGateway, eIncludeClustersType, gDataHub);
    CPurelyTemporalCluster ClusterComparator(gpClusterDataFactory, DataGateway, eIncludeClustersType, gDataHub);
    //get new time intervals objects
    pTimeIntervals = GetNewTemporalDataEvaluatorObject(eIncludeClustersType);
    //iterate through time intervals, finding top cluster
    pTimeIntervals->CompareClusters(ClusterComparator, TopCluster);
    //if any interesting clusters found, add to top cluster array
    if (TopCluster.ClusterDefined())
      TopClustersContainer.Add(TopCluster);
    delete pTimeIntervals; pTimeIntervals=0;
  }
  catch (ZdException &x) {
    delete pTimeIntervals;
    x.AddCallpath("FindTopClusters()","CPurelyTemporalAnalysis");
    throw;
  }
}

/** calculates greatest loglikelihood ratio for a temporal cluster */
double CPurelyTemporalAnalysis::FindTopRatio(const AbtractDataStreamGateway&) {
  //re-initialize comparator cluster and top cluster
  gpClusterComparator->Initialize();
  gpTopCluster->Initialize();
  //iterate through time intervals, finding top cluster
  gpTimeIntervals->CompareClusters(*gpClusterComparator, *gpTopCluster);
  //return ratio of top cluster
  return gpTopCluster->m_nRatio;
}

/** internal initialization */
void CPurelyTemporalAnalysis::Init() {
  gpTopCluster=0;
  gpClusterData=0;
  gpMeasureList=0;
  gpTimeIntervals=0;
  gpClusterComparator=0;
}

/** Returns log likelihood ratio for Monte Carlo replication. */
double CPurelyTemporalAnalysis::MonteCarlo(const DataStreamInterface&) {
  gpMeasureList->Reset();
  gpTimeIntervals->CompareMeasures(*gpClusterData, *gpMeasureList);
  return gpMeasureList->GetMaximumLogLikelihoodRatio();
}

