//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "SpaceTimeIncludePurelySpatialAnalysis.h"
#include "ClusterData.h"

/** Constructor */
C_ST_PS_Analysis::C_ST_PS_Analysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
                 :CSpaceTimeAnalysis(Parameters, DataHub, PrintDirection), gPSTopShapeClusters(DataHub), gPSPTopShapeClusters(DataHub) {}

/** Destructor */
C_ST_PS_Analysis::~C_ST_PS_Analysis() {}

/** Allocates objects used during simulations, instead of repeated allocations
    for each simulation. Which objects that are allocated depends on whether
    the simluations process uses same process as real data or uses measure list. */
void C_ST_PS_Analysis::AllocateSimulationObjects(const AbstractDataSetGateway & DataGateway) {
  try {
    CSpaceTimeAnalysis::AllocateSimulationObjects(DataGateway);
    //create simulation objects based upon which process used to perform simulations
    if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)
      gAbstractPSPClusterData.reset(gpClusterDataFactory->GetNewProspectiveSpatialClusterData(gDataHub, DataGateway));
    else
      gAbstractPSClusterData.reset(gpClusterDataFactory->GetNewSpatialClusterData(DataGateway));
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateSimulationObjects()","C_ST_PS_Analysis");
    throw;
  }
}

/** Allocates objects used during calculation of most likely clusters, instead
    of repeated allocations for each simulation.                                */
void C_ST_PS_Analysis::AllocateTopClustersObjects(const AbstractDataSetGateway & DataGateway) {
  try {
    //create top cluster objects for space-time portion
    CSpaceTimeAnalysis::AllocateTopClustersObjects(DataGateway);
    //create comparator cluster for purely spatial cluster
    gPSClusterComparator.reset(new CPurelySpatialCluster(gpClusterDataFactory, DataGateway));
    gPSTopShapeClusters.SetTopClusters(*gPSClusterComparator);
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateTopClustersObjects()","C_ST_PS_Analysis");
    throw;
  }
}

/** Returns cluster centered at grid point nCenter, with the greatest log likelihood
    ratio. Caller should not assume that returned reference is persistent, but should
    either call Clone() method or overloaded assignment operator. */
const CCluster& C_ST_PS_Analysis::CalculateTopCluster(tract_t tCenter, const AbstractDataSetGateway & DataGateway) {
  int                   j;

  //re-initialize clusters
  if (gPSClusterComparator.get()) gPSTopShapeClusters.Reset(tCenter);
  if (gPSPClusterComparator.get()) gPSPTopShapeClusters.Reset(tCenter);
  gTopShapeClusters.Reset(tCenter);

  for (j=0 ;j <= gParameters.GetNumTotalEllipses(); ++j) {
     CentroidNeighbors CentroidDef(j, gDataHub);
     CentroidDef.Set(tCenter);
     gClusterComparator->Initialize(tCenter);
     gClusterComparator->SetEllipseOffset(j, gDataHub);
     if (gPSClusterComparator.get()) {
       gPSClusterComparator->Initialize(tCenter);
       gPSClusterComparator->SetEllipseOffset(j, gDataHub);
       gPSClusterComparator->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, gPSTopShapeClusters.GetTopCluster(j), *gpLikelihoodCalculator);
     }
     else {
       gPSPClusterComparator->Initialize(tCenter);
       gPSPClusterComparator->SetEllipseOffset(j, gDataHub);
       gPSPClusterComparator->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, gPSPTopShapeClusters.GetTopCluster(j), *gpLikelihoodCalculator);
     }
     gClusterComparator->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, gTopShapeClusters.GetTopCluster(j), *gTimeIntervals);
  }
  return GetTopCalculatedCluster();
}

/** returns top cluster calculated during 'FindTopClsuters()' routine */
const CCluster & C_ST_PS_Analysis::GetTopCalculatedCluster() {
  try {
    CCluster& PSCluster = gPSClusterComparator.get() ? (CCluster&)gPSTopShapeClusters.GetTopCluster() : (CCluster&)gPSPTopShapeClusters.GetTopCluster();
    CCluster& STCluster = (CCluster&)gTopShapeClusters.GetTopCluster();
    if (!PSCluster.ClusterDefined())
      return STCluster;
    else if (std::fabs(PSCluster.m_nRatio - STCluster.m_nRatio) > DBL_CMP_TOLERANCE && PSCluster.m_nRatio > STCluster.m_nRatio)
      return PSCluster;
    else
      return STCluster;
  }
  catch (ZdException &x) {
    x.AddCallpath("GetTopCalculatedCluster()","C_ST_PS_Analysis");
    throw;
  }
}

/** Returns loglikelihood for Monte Carlo replication. */
double C_ST_PS_Analysis::MonteCarlo(const DataSetInterface & Interface) {
  if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)
    return MonteCarloProspective(Interface);

  tract_t               i, k;
  SpatialData         * pSpatialData = dynamic_cast<SpatialData*>(gAbstractPSClusterData.get());
  SpaceTimeData       * pSpaceTimeData = dynamic_cast<SpaceTimeData*>(gAbstractClusterData.get());

  gMeasureList->Reset();
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

/** Returns cluster centered at grid point nCenter, with the greatest log
    likelihood ratio . Caller should not assume that returned reference is
    persistent, but should either call Clone() method or overloaded assignment
    operator. */
double C_ST_PS_Analysis::MonteCarlo(tract_t tCenter, const AbstractDataSetGateway& DataGateway) {
  if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)
    return MonteCarloProspective(tCenter, DataGateway);
    
  tract_t                       t, tNumNeighbors, * pIntegerArray;
  unsigned short              * pUnsignedShortArray;
  double                        dMaximizingValue;
  std::vector<double>           vMaximizingValues(gParameters.GetNumTotalEllipses() + 1, -std::numeric_limits<double>::max());
  std::vector<double>::iterator itr, itr_end;

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
double C_ST_PS_Analysis::MonteCarloProspective(const DataSetInterface & Interface) {
  tract_t                  k, i;
  ProspectiveSpatialData * pPSSpatialData = dynamic_cast<ProspectiveSpatialData*>(gAbstractPSPClusterData.get());
  SpaceTimeData          * pSpaceTimeData = dynamic_cast<SpaceTimeData*>(gAbstractClusterData.get());

  gMeasureList->Reset();
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

/** Returns cluster centered at grid point nCenter, with the greatest log
    likelihood ratio . Caller should not assume that returned reference is
    persistent, but should either call Clone() method or overloaded assignment
    operator. */
double C_ST_PS_Analysis::MonteCarloProspective(tract_t tCenter, const AbstractDataSetGateway& DataGateway) {
  tract_t                       t, tNumNeighbors, * pIntegerArray;
  unsigned short              * pUnsignedShortArray;
  double                        dMaximizingValue;
  std::vector<double>           vMaximizingValues(gParameters.GetNumTotalEllipses() + 1, -std::numeric_limits<double>::max());
  std::vector<double>::iterator itr, itr_end;
  AbstractProspectiveSpatialClusterData * pPSPSpatialData = dynamic_cast<AbstractProspectiveSpatialClusterData*>(gAbstractPSPClusterData.get());

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

