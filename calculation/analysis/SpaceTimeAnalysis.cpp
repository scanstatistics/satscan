//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SpaceTimeAnalysis.h"
#include "MakeNeighbors.h"
#include "ClusterData.h"
#include "SSException.h"

/** Constructor */
CSpaceTimeAnalysis::CSpaceTimeAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
                   :CAnalysis(Parameters, DataHub, PrintDirection), gTopShapeClusters(DataHub) {}

/** Destructor */
CSpaceTimeAnalysis::~CSpaceTimeAnalysis() {}

/** Allocates objects used during simulations, instead of repeated allocations
    for each simulation. Which objects that are allocated depends on whether
    the simluations process uses same process as real data or uses measure list. */
void CSpaceTimeAnalysis::AllocateSimulationObjects(const AbstractDataSetGateway& DataGateway) {
  IncludeClustersType           eIncludeClustersType;

  try {
    //create new time intervals object - delete existing object used during real data process
    eIncludeClustersType = (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME ? ALLCLUSTERS : gParameters.GetIncludeClustersType());
    gTimeIntervals.reset(GetNewTemporalDataEvaluatorObject(eIncludeClustersType, SUCCESSIVELY));
    //create simulation objects based upon which process used to perform simulations
    if (geReplicationsProcessType == MeasureListEvaluation)
      gMeasureList.reset(GetNewMeasureListObject());
    gAbstractClusterData.reset(gpClusterDataFactory->GetNewSpaceTimeClusterData(DataGateway));
  }
  catch (prg_exception& x) {
    x.addTrace("AllocateSimulationObjects()","CSpaceTimeAnalysis");
    throw;
  }
}

/** Allocates objects used during calculation of most likely clusters, instead
    of repeated allocations for each grid point.                             */
void CSpaceTimeAnalysis::AllocateTopClustersObjects(const AbstractDataSetGateway& DataGateway) {
  IncludeClustersType           eIncludeClustersType;

  try {
    //create new time intervals object - delete existing object used during real data process
    eIncludeClustersType = (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME ? ALIVECLUSTERS : gParameters.GetIncludeClustersType());
    gTimeIntervals.reset(GetNewTemporalDataEvaluatorObject(eIncludeClustersType, SUCCESSIVELY));
    //create cluster object used as comparator when iterating over centroids and time intervals
    gClusterComparator.reset(new CSpaceTimeCluster(gpClusterDataFactory, DataGateway));
    //initialize list of top circle/ellipse clusters
    gTopShapeClusters.SetTopClusters(*gClusterComparator);
  }
  catch (prg_exception& x) {
    x.addTrace("AllocateTopClustersObjects()","CSpaceTimeAnalysis");
    throw;
  }
}

/** Returns cluster centered at grid point nCenter, with the greatest log
    likelihood ratio . Caller should not assume that returned reference is
    persistent, but should either call Clone() method or overloaded assignment
    operator. */
const CCluster& CSpaceTimeAnalysis::CalculateTopCluster(tract_t tCenter, const AbstractDataSetGateway& DataGateway) {
  tract_t               k;

  gTopShapeClusters.Reset(tCenter);
  gTimeIntervals->setIntervalRange(tCenter);
  //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
  for (k=0; k <= gParameters.GetNumTotalEllipses(); ++k) {
     CentroidNeighbors CentroidDef(k, gDataHub);
     CentroidDef.Set(tCenter);
     gClusterComparator->Initialize(tCenter);
     gClusterComparator->SetEllipseOffset(k, gDataHub);
     gClusterComparator->CalculateTopClusterAboutCentroidDefinition(DataGateway,
                                                                    CentroidDef,
                                                                    gTopShapeClusters.GetTopCluster(k),
                                                                    *gTimeIntervals);
  }
  return gTopShapeClusters.GetTopCluster();
}

/** Returns loglikelihood ratio for Monte Carlo replication using same algorithm as real data. */
double CSpaceTimeAnalysis::MonteCarlo(tract_t tCenter, const AbstractDataSetGateway& DataGateway) {
  tract_t                       t, tNumNeighbors, * pIntegerArray;
  unsigned short              * pUnsignedShortArray;
  double                        dMaximizingValue;
  std::vector<double>           vMaximizingValues(gParameters.GetNumTotalEllipses() + 1, -std::numeric_limits<double>::max());
  std::vector<double>::iterator itr, itr_end;

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

/** Returns log likelihood ratio for Monte Carlo replication. */
double CSpaceTimeAnalysis::MonteCarlo(const DataSetInterface& Interface) {
  tract_t               k, i;
  SpaceTimeData       * pSpaceTimeData = dynamic_cast<SpaceTimeData*>(gAbstractClusterData.get());

  gMeasureList->Reset();
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

