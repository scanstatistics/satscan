//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SpaceTimeIncludePurelyTemporalAnalysis.h"
#include "ClusterData.h"
#include "MostLikelyClustersContainer.h"
#include "SSException.h"

/** Constructor */
C_ST_PT_Analysis::C_ST_PT_Analysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
                 :CSpaceTimeAnalysis(Parameters, DataHub, PrintDirection) {}

/** Destructor */
C_ST_PT_Analysis::~C_ST_PT_Analysis() {}

/** Allocates objects used during simulations, instead of repeated allocations
    for each simulation. Which objects that are allocated depends on whether
    the simluations process uses same process as real data or uses measure list. */
void C_ST_PT_Analysis::AllocateSimulationObjects(const AbstractDataSetGateway & DataGateway) {
  try {
    //allocate objects for space-time part of simulations
    CSpaceTimeAnalysis::AllocateSimulationObjects(DataGateway);
    //create simulation objects based upon which process used to perform simulations
    gAbstractPTClusterData.reset(gpClusterDataFactory->GetNewTemporalClusterData(DataGateway));
  }
  catch (prg_exception& x) {
    x.addTrace("AllocateSimulationObjects()","C_ST_PT_Analysis");
    throw;
  }
}

/** calculates top cluster about each centroid/grid point - iterating through
    all possible time intervals - populates top cluster array with most likely
    cluster about each grid point plus , possible, one more for purely temporal
    cluster. */
void C_ST_PT_Analysis::FindTopClusters(const AbstractDataSetGateway & DataGateway, MLC_Collections_t& TopClustersContainers) {
  IncludeClustersType           eIncludeClustersType;

  try {
    //calculate top cluster over all space-time
    CSpaceTimeAnalysis::FindTopClusters(DataGateway, TopClustersContainers);
    //detect user cancellation
    if (gPrintDirection.GetIsCanceled())
      return;
    eIncludeClustersType = (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME ? ALIVECLUSTERS : gParameters.GetIncludeClustersType());
    //create comparator cluster
    CPurelyTemporalCluster ClusterComparator(gpClusterDataFactory, DataGateway, eIncludeClustersType, gDataHub);
    CClusterSet clusterSet;
    CClusterObject clusterObject(ClusterComparator);
    clusterSet.add(clusterObject);
    gTimeIntervals->resetIntervalRange();
    gTimeIntervals->CompareClusterSet(ClusterComparator, clusterSet);
    for (MLC_Collections_t::iterator itr = TopClustersContainers.begin(); itr != TopClustersContainers.end(); ++itr)
       itr->Add(clusterSet.get(0).getCluster());
  }
  catch (prg_exception& x) {
    x.addTrace("FindTopClusters()","C_ST_PT_Analysis");
    throw;
  }
}

/** Returns loglikelihood ratio for Monte Carlo replication using same algorithm as real data. */
double C_ST_PT_Analysis::MonteCarlo(tract_t tCenter, const AbstractDataSetGateway & DataGateway) {
  tract_t                       t, tNumNeighbors, * pIntegerArray;
  unsigned short              * pUnsignedShortArray;
  double                        dMaximizingValue;
  std::vector<double>           vMaximizingValues(gParameters.GetNumTotalEllipses() + 1, -std::numeric_limits<double>::max());
  std::vector<double>::iterator itr, itr_end;

  gAbstractPTClusterData->InitializeData();
  //iterate through time intervals, finding top maximizing value
  gTimeIntervals->resetIntervalRange();
  dMaximizingValue = gTimeIntervals->ComputeMaximizingValue(*gAbstractPTClusterData);
  if (dMaximizingValue > vMaximizingValues[0]) vMaximizingValues[0] = dMaximizingValue;

  gTimeIntervals->setIntervalRange(tCenter);
  for (int j=0; j <= gParameters.GetNumTotalEllipses(); ++j) {
     double& dShapeMaxValue = vMaximizingValues[j];
     gAbstractClusterData->InitializeData();
     CentroidNeighbors CentroidDef(j, gDataHub);
     CentroidDef.Set(tCenter);
     tNumNeighbors = CentroidDef.GetNumNeighbors();
     pUnsignedShortArray = CentroidDef.GetRawUnsignedShortArray();
     pIntegerArray = CentroidDef.GetRawIntegerArray();
     for (t=0; t < tNumNeighbors; ++t) {
        gAbstractClusterData->AddNeighborData((pUnsignedShortArray ? (tract_t)pUnsignedShortArray[t] : pIntegerArray[t]), DataGateway);
        dMaximizingValue = gTimeIntervals->ComputeMaximizingValue(*gAbstractClusterData);
        if (dMaximizingValue > dShapeMaxValue) dShapeMaxValue = dMaximizingValue;
     }   
  }
  //if maximizing value is not a ratio/test statistic, convert them now
  if (gDataHub.GetDataSetHandler().GetNumDataSets() == 1)
    for (itr=vMaximizingValues.begin(),itr_end=vMaximizingValues.end(); itr != itr_end; ++itr)
      *itr = gpLikelihoodCalculator->CalculateFullStatistic(*itr);
  //determine which ratio/test statistic is the greatest, be sure to apply compactness correction
  double dPenalty = gDataHub.GetParameters().GetNonCompactnessPenaltyPower();
  dMaximizingValue = vMaximizingValues.front() * CalculateNonCompactnessPenalty(gDataHub.GetEllipseShape(0), dPenalty);
  for (t=1,itr=vMaximizingValues.begin()+1,itr_end=vMaximizingValues.end(); itr != itr_end; ++itr, ++t) {
     *itr *= CalculateNonCompactnessPenalty(gDataHub.GetEllipseShape(t), dPenalty);
     dMaximizingValue = std::max(*itr, dMaximizingValue);
  }
  return dMaximizingValue;
}

/** Returns loglikelihood for Monte Carlo replication. */
double C_ST_PT_Analysis::MonteCarlo(const DataSetInterface & Interface) {
  tract_t               k, i;
  SpaceTimeData       * pSpaceTimeData = dynamic_cast<SpaceTimeData*>(gAbstractClusterData.get());

  gMeasureList->Reset();
  //compare purely temporal cluster in same ratio correction as circle
  macroRunTimeStartFocused(FocusRunTimeComponent::MeasureListScanningAdding);
  gTimeIntervals->resetIntervalRange();
  gTimeIntervals->CompareMeasures(*gAbstractPTClusterData, *gMeasureList);
  macroRunTimeStopFocused(FocusRunTimeComponent::MeasureListScanningAdding);
  //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
  for (k=0; k <= gParameters.GetNumTotalEllipses(); ++k) {
     CentroidNeighbors CentroidDef(k, gDataHub);
     for (i=0; i < gDataHub.m_nGridTracts; ++i) {
        CentroidDef.Set(i);
        gTimeIntervals->setIntervalRange(i);
        pSpaceTimeData->AddNeighborDataAndCompare(CentroidDef, Interface, *gTimeIntervals, *gMeasureList);
     }
     gMeasureList->SetForNextIteration(k);
  }
  return gMeasureList->GetMaximumLogLikelihoodRatio();
}

