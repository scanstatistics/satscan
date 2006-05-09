//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "PurelyTemporalAnalysis.h"
#include "MostLikelyClustersContainer.h"

/** Constructor */
CPurelyTemporalAnalysis::CPurelyTemporalAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
                        :CAnalysis(Parameters, DataHub, PrintDirection) {}

/** Destructor */
CPurelyTemporalAnalysis::~CPurelyTemporalAnalysis() {}

/** Allocates objects used during simulations, instead of repeated allocations
    for each simulation. Which objects that are allocated depends on whether
    the simluations process uses same process as real data or uses measure list. */
void CPurelyTemporalAnalysis::AllocateSimulationObjects(const AbstractDataSetGateway& DataGateway) {
  IncludeClustersType           eIncludeClustersType;

  try {
    eIncludeClustersType = (gParameters.GetIsProspectiveAnalysis() ? ALLCLUSTERS : gParameters.GetIncludeClustersType());
    gTimeIntervals.reset(GetNewTemporalDataEvaluatorObject(eIncludeClustersType, SUCCESSIVELY));
    if (geReplicationsProcessType == MeasureListEvaluation)
      gMeasureList.reset(GetNewMeasureListObject());
    gClusterData.reset(gpClusterDataFactory->GetNewTemporalClusterData(DataGateway));
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateSimulationObjects()","CPurelyTemporalAnalysis");
    throw;
  }
}

const CCluster & CPurelyTemporalAnalysis::CalculateTopCluster(tract_t, const AbstractDataSetGateway&) {
  ZdGenerateException("CalculateTopCluster() can not be called for CPurelyTemporalAnalysis.","CPurelyTemporalAnalysis");
  return dynamic_cast<const CCluster&>(*this);
}

/** Calculate most likely, purely temporal, cluster and adds clone of top cluster
    to top cluster array. */
void CPurelyTemporalAnalysis::FindTopClusters(const AbstractDataSetGateway& DataGateway, MostLikelyClustersContainer& TopClustersContainer) {
  IncludeClustersType           eIncludeClustersType;
  std::auto_ptr<CTimeIntervals> TimeIntervals;

  try {
    //determine the type of clusters to compare
    eIncludeClustersType = (gParameters.GetIsProspectiveAnalysis() ? ALIVECLUSTERS : gParameters.GetIncludeClustersType());
    //create cluster objects
    CPurelyTemporalCluster TopCluster(gpClusterDataFactory, DataGateway, eIncludeClustersType, gDataHub);
    CPurelyTemporalCluster ClusterComparator(gpClusterDataFactory, DataGateway, eIncludeClustersType, gDataHub);
    //get new time intervals objects
    TimeIntervals.reset(GetNewTemporalDataEvaluatorObject(eIncludeClustersType, SUCCESSIVELY));
    //iterate through time intervals, finding top cluster
    TimeIntervals->CompareClusters(ClusterComparator, TopCluster);
    //if any interesting clusters found, add to top cluster array
    if (TopCluster.ClusterDefined())
      TopClustersContainer.Add(TopCluster);
  }
  catch (ZdException &x) {
    x.AddCallpath("FindTopClusters()","CPurelyTemporalAnalysis");
    throw;
  }
}

/** Returns maximized value for Monte Carlo replication. Depending on the parameter settings,
    value returned might either be the maximizing value or full loglikelihood ratio/test statistic. */
double CPurelyTemporalAnalysis::MonteCarlo(tract_t, const AbstractDataSetGateway&) {
  gClusterData->InitializeData();
  //iterate through time intervals, finding top cluster
  double dMaximumingValue = gTimeIntervals->ComputeMaximizingValue(*gClusterData);
  if (gDataHub.GetDataSetHandler().GetNumDataSets() == 1)
    dMaximumingValue = gpLikelihoodCalculator->CalculateFullStatistic(dMaximumingValue);
  return std::max(0.0, dMaximumingValue);  
}

/** Returns log likelihood ratio for Monte Carlo replication. */
double CPurelyTemporalAnalysis::MonteCarlo(const DataSetInterface&) {
  gMeasureList->Reset();
  macroRunTimeStartFocused(FocusRunTimeComponent::MeasureListScanningAdding);
  gTimeIntervals->CompareMeasures(*gClusterData, *gMeasureList);
  macroRunTimeStopFocused(FocusRunTimeComponent::MeasureListScanningAdding);
  return gMeasureList->GetMaximumLogLikelihoodRatio();
}

