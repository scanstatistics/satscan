//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "SpaceTimeIncludePureAnalysis.h"
#include "ClusterData.h"
#include "MostLikelyClustersContainer.h"

/** Constructor */
C_ST_PS_PT_Analysis::C_ST_PS_PT_Analysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
                    :C_ST_PS_Analysis(Parameters, DataHub, PrintDirection) {}

/** Desctructor */
C_ST_PS_PT_Analysis::~C_ST_PS_PT_Analysis() {}

/** Allocates objects used during simulations, instead of repeated allocations
    for each simulation. Which objects that are allocated depends on whether
    the simluations process uses same process as real data or uses measure list. */
void C_ST_PS_PT_Analysis::AllocateSimulationObjects(const AbstractDataSetGateway& DataGateway) {
  IncludeClustersType           eIncludeClustersType;

  try {
    //allocate objects for space-time part of simulations
    C_ST_PS_Analysis::AllocateSimulationObjects(DataGateway);
    eIncludeClustersType = (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME ? ALLCLUSTERS : gParameters.GetIncludeClustersType());
    gPTClusterData.reset(gpClusterDataFactory->GetNewTemporalClusterData(DataGateway));
  }
  catch (ZdException &x) {
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
  eIncludeClustersType = (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME ? ALIVECLUSTERS : gParameters.GetIncludeClustersType());
  //create cluster objects
  CPurelyTemporalCluster TopCluster(gpClusterDataFactory, DataGateway, eIncludeClustersType, gDataHub);
  CPurelyTemporalCluster ClusterComparator(gpClusterDataFactory, DataGateway, eIncludeClustersType, gDataHub);
  //iterate through time intervals - looking for top purely temporal cluster
  gTimeIntervals->CompareClusters(ClusterComparator, TopCluster);
  TopClustersContainer.Add(TopCluster);
}

/** Returns loglikelihood for Monte Carlo replication. */
double C_ST_PS_PT_Analysis::MonteCarlo(const DataSetInterface & Interface) {
  if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)
    return MonteCarloProspective(Interface);

  tract_t               k, i;
  SpatialData         * pSpatialData = dynamic_cast<SpatialData*>(gAbstractPSClusterData.get());
  SpaceTimeData       * pSpaceTimeData = dynamic_cast<SpaceTimeData*>(gAbstractClusterData.get());

  gMeasureList->Reset();
  //Add measure values for purely space first - so that this cluster's values
  //will be calculated with circle's measure values.
  macroRunTimeStartFocused(FocusRunTimeComponent::MeasureListScanningAdding);
  gTimeIntervals->CompareMeasures(*gPTClusterData, *gMeasureList);
  macroRunTimeStopFocused(FocusRunTimeComponent::MeasureListScanningAdding);
  //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
  for (k=0; k <= gParameters.GetNumTotalEllipses(); ++k) {
     CentroidNeighbors CentroidDef(k, gDataHub);
     for (i=0; i < gDataHub.m_nGridTracts; ++i) {
        CentroidDef.Set(i);
        pSpatialData->AddMeasureList(CentroidDef, Interface, gMeasureList.get());
        pSpaceTimeData->AddNeighborDataAndCompare(CentroidDef, Interface, *gTimeIntervals, *gMeasureList);
     }
     gMeasureList->SetForNextIteration(k);
  }
  return gMeasureList->GetMaximumLogLikelihoodRatio();
}

double C_ST_PS_PT_Analysis::MonteCarlo(tract_t tCenter, const AbstractDataSetGateway & DataGateway) {
  if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)
    return MonteCarloProspective(tCenter, DataGateway);

  tract_t                       t, tNumNeighbors, * pIntegerArray;
  unsigned short              * pUnsignedShortArray;
  double                        dMaximizingValue;
  std::vector<double>           vMaximizingValues(gParameters.GetNumTotalEllipses() + 1, -std::numeric_limits<double>::max());
  std::vector<double>::iterator itr, itr_end;

  gPTClusterData->InitializeData();
  //iterate through time intervals, finding top cluster
  dMaximizingValue = gTimeIntervals->ComputeMaximizingValue(*gPTClusterData);
  if (dMaximizingValue > vMaximizingValues[0]) vMaximizingValues[0] = dMaximizingValue;
  for (int j=0; j <= gParameters.GetNumTotalEllipses(); ++j) {
     double& dShapeMaxValue = vMaximizingValues[j];
     gAbstractPSClusterData->InitializeData();
     gAbstractClusterData->InitializeData();
     CentroidNeighbors CentroidDef(j, gDataHub);
     CentroidDef.Set(tCenter);
     tNumNeighbors = CentroidDef.GetNumNeighbors();
     pUnsignedShortArray = CentroidDef.GetRawUnsignedShortArray();
     pIntegerArray = CentroidDef.GetRawIntegerArray();
     for (t=0; t < tNumNeighbors; ++t) {
        gAbstractPSClusterData->AddNeighborData((pUnsignedShortArray ? (tract_t)pUnsignedShortArray[t] : pIntegerArray[t]), DataGateway);
        dMaximizingValue = gAbstractPSClusterData->GetMaximizingValue(*gpLikelihoodCalculator);
        if (dMaximizingValue > dShapeMaxValue) dShapeMaxValue = dMaximizingValue;
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

/** Returns loglikelihood for Monte Carlo Prospective replication. */
double C_ST_PS_PT_Analysis::MonteCarloProspective(const DataSetInterface & Interface) {
  tract_t                  k, i;
  ProspectiveSpatialData * pPSSpatialData = dynamic_cast<ProspectiveSpatialData*>(gAbstractPSPClusterData.get());
  SpaceTimeData          * pSpaceTimeData = dynamic_cast<SpaceTimeData*>(gAbstractClusterData.get());

  gMeasureList->Reset();
  //Add measure values for purely space first - so that this cluster's values
  //will be calculated with circle's measure values.
  macroRunTimeStartFocused(FocusRunTimeComponent::MeasureListScanningAdding);
  gTimeIntervals->CompareMeasures(*gPTClusterData, *gMeasureList);
  macroRunTimeStopFocused(FocusRunTimeComponent::MeasureListScanningAdding);
  //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
  for (k=0; k <= gParameters.GetNumTotalEllipses(); ++k) {
     CentroidNeighbors CentroidDef(k, gDataHub);
     for (i=0; i < gDataHub.m_nGridTracts; ++i) {
        CentroidDef.Set(i);
        pPSSpatialData->AddMeasureList(CentroidDef, Interface, gMeasureList.get());
        pSpaceTimeData->AddNeighborDataAndCompare(CentroidDef, Interface, *gTimeIntervals, *gMeasureList);
     }
     gMeasureList->SetForNextIteration(k);
  }
  return gMeasureList->GetMaximumLogLikelihoodRatio();
}

double C_ST_PS_PT_Analysis::MonteCarloProspective(tract_t tCenter, const AbstractDataSetGateway & DataGateway) {
  tract_t                       t, tNumNeighbors, * pIntegerArray;
  unsigned short              * pUnsignedShortArray;
  double                        dMaximizingValue;
  std::vector<double>           vMaximizingValues(gParameters.GetNumTotalEllipses() + 1, -std::numeric_limits<double>::max());
  std::vector<double>::iterator itr, itr_end;
  AbstractProspectiveSpatialClusterData * pPSPSpatialData = dynamic_cast<AbstractProspectiveSpatialClusterData*>(gAbstractPSPClusterData.get());

  gPTClusterData->InitializeData();
  //iterate through time intervals, finding top cluster
  dMaximizingValue = gTimeIntervals->ComputeMaximizingValue(*gPTClusterData);
  if (dMaximizingValue > vMaximizingValues[0]) vMaximizingValues[0] = dMaximizingValue;  
  for (int j=0; j <= gParameters.GetNumTotalEllipses(); ++j) {
     double& dShapeMaxValue = vMaximizingValues[j];
     gAbstractPSPClusterData->InitializeData();
     gAbstractClusterData->InitializeData();
     CentroidNeighbors CentroidDef(j, gDataHub);
     CentroidDef.Set(tCenter);
     tNumNeighbors = CentroidDef.GetNumNeighbors();
     pUnsignedShortArray = CentroidDef.GetRawUnsignedShortArray();
     pIntegerArray = CentroidDef.GetRawIntegerArray();
     for (t=0; t < tNumNeighbors; ++t) {
        gAbstractPSPClusterData->AddNeighborData((pUnsignedShortArray ? (tract_t)pUnsignedShortArray[t] : pIntegerArray[t]), DataGateway);
        dMaximizingValue = pPSPSpatialData->GetMaximizingValue(*gpLikelihoodCalculator);
        if (dMaximizingValue > dShapeMaxValue) dShapeMaxValue = dMaximizingValue;
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


