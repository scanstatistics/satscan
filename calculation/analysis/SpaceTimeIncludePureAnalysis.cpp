//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "SpaceTimeIncludePureAnalysis.h"
#include "ClusterData.h"
#include "MostLikelyClustersContainer.h"

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
void C_ST_PS_PT_Analysis::AllocateSimulationObjects(const AbstractDataSetGateway& DataGateway) {
  IncludeClustersType           eIncludeClustersType;

  try {
    //allocate objects for space-time part of simulations
    C_ST_PS_Analysis::AllocateSimulationObjects(DataGateway);
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
void C_ST_PS_PT_Analysis::FindTopClusters(const AbstractDataSetGateway& DataGateway, MostLikelyClustersContainer& TopClustersContainer) {
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
  TopClustersContainer.Add(TopCluster);
}

/** Returns greatest calculated log likelihood ratio. */
double C_ST_PS_PT_Analysis::FindTopRatio(const AbstractDataSetGateway& DataGateway) {
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
double C_ST_PS_PT_Analysis::MonteCarlo(const DataSetInterface & Interface) {
  if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)
    return MonteCarloProspective(Interface);

  tract_t               k, i;

  gpMeasureList->Reset();
  //Add measure values for purely space first - so that this cluster's values
  //will be calculated with circle's measure values.
  macroRunTimeStartFocused(FocusRunTimeComponent::MeasureListScanningAdding);
  gpTimeIntervals->CompareMeasures(*gpPTClusterData, *gpMeasureList);
  macroRunTimeStopFocused(FocusRunTimeComponent::MeasureListScanningAdding);
  //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
  for (k=0; k <= gParameters.GetNumTotalEllipses(); ++k) {
     CentroidNeighbors CentroidDef(k, gDataHub);
     for (i=0; i < gDataHub.m_nGridTracts; ++i) {
        CentroidDef.Set(i);
        gpPSClusterData->AddMeasureList(CentroidDef, Interface, gpMeasureList);
        gpClusterData->AddNeighborDataAndCompare(CentroidDef, Interface, *gpTimeIntervals, *gpMeasureList);
     }
     gpMeasureList->SetForNextIteration(k);
  }
  return gpMeasureList->GetMaximumLogLikelihoodRatio();
}

/** Returns loglikelihood for Monte Carlo Prospective replication. */
double C_ST_PS_PT_Analysis::MonteCarloProspective(const DataSetInterface & Interface) {
  tract_t                 k, i;

  gpMeasureList->Reset();
  //Add measure values for purely space first - so that this cluster's values
  //will be calculated with circle's measure values.
  macroRunTimeStartFocused(FocusRunTimeComponent::MeasureListScanningAdding);
  gpTimeIntervals->CompareMeasures(*gpPTClusterData, *gpMeasureList);
  macroRunTimeStopFocused(FocusRunTimeComponent::MeasureListScanningAdding);
  //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
  for (k=0; k <= gParameters.GetNumTotalEllipses(); ++k) {
     CentroidNeighbors CentroidDef(k, gDataHub);
     for (i=0; i < gDataHub.m_nGridTracts; ++i) {
        CentroidDef.Set(i);
        gpPSPClusterData->AddMeasureList(CentroidDef, Interface, gpMeasureList);
        gpClusterData->AddNeighborDataAndCompare(CentroidDef, Interface, *gpTimeIntervals, *gpMeasureList);
     }
     gpMeasureList->SetForNextIteration(k);
  }
  return gpMeasureList->GetMaximumLogLikelihoodRatio();
}

