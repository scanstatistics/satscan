//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "SpaceTimeIncludePurelySpatialAnalysis.h"
#include "ClusterData.h"

/** Constructor */
C_ST_PS_Analysis::C_ST_PS_Analysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
                 :CSpaceTimeAnalysis(Parameters, DataHub, PrintDirection), gPSTopShapeClusters(DataHub), gPSPTopShapeClusters(DataHub),
                  gpPSClusterComparator(0), gpPSPClusterComparator(0), gpPSClusterData(0), gpPSPClusterData(0) {}

/** Destructor */
C_ST_PS_Analysis::~C_ST_PS_Analysis() {
  try {
    delete gpPSClusterComparator;
    delete gpPSPClusterComparator;
    delete gpPSClusterData;
    delete gpPSPClusterData;
  }
  catch(...){}
}

/** Allocates objects used during simulations, instead of repeated allocations
    for each simulation. Which objects that are allocated depends on whether
    the simluations process uses same process as real data or uses measure list. */
void C_ST_PS_Analysis::AllocateSimulationObjects(const AbstractDataSetGateway & DataGateway) {
  try {
    CSpaceTimeAnalysis::AllocateSimulationObjects(DataGateway);
    //delete object used during real data
    delete gpPSPClusterComparator; gpPSPClusterComparator=0;
    delete gpPSClusterComparator; gpPSClusterComparator=0;
    //create simulation objects based upon which process used to perform simulations
    if (geReplicationsProcessType == MeasureListEvaluation) {
      if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)
        gpPSPClusterData = new ProspectiveSpatialData(gDataHub, DataGateway);
      else
        gpPSClusterData = new SpatialData(DataGateway, gParameters.GetAreaScanRateType());
    }
    else {
      if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME) {
        //create cluster object used as comparator when iterating over centroids and time intervals
        gpPSPClusterComparator = new CPurelySpatialProspectiveCluster(gpClusterDataFactory, DataGateway, gDataHub);
        //initialize list of top circle/ellipse clusters
        gPSPTopShapeClusters.SetTopClusters(*gpPSPClusterComparator);
      }
      else {
        //create cluster object used as comparator when iterating over centroids and time intervals
        gpPSClusterComparator = new CPurelySpatialCluster(gpClusterDataFactory, DataGateway, (int)gParameters.GetAreaScanRateType());
        //initialize list of top circle/ellipse clusters
        gPSTopShapeClusters.SetTopClusters(*gpPSClusterComparator);
      }
    }
  }
  catch (ZdException &x) {
    delete gpPSClusterData; gpPSClusterData=0;
    delete gpPSPClusterData; gpPSPClusterData=0;
    delete gpPSPClusterComparator; gpPSPClusterComparator=0;
    delete gpPSClusterComparator; gpPSClusterComparator=0;
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
    gpPSClusterComparator = new CPurelySpatialCluster(gpClusterDataFactory, DataGateway, gParameters.GetAreaScanRateType());
    gPSTopShapeClusters.SetTopClusters(*gpPSClusterComparator);
  }
  catch (ZdException &x) {
    delete gpPSClusterComparator; gpPSClusterComparator=0;
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
  if (gpPSClusterComparator) gPSTopShapeClusters.Reset(tCenter);
  if (gpPSPClusterComparator) gPSPTopShapeClusters.Reset(tCenter);
  gTopShapeClusters.Reset(tCenter);

  for (j=0 ;j <= gParameters.GetNumTotalEllipses(); ++j) {
     CentroidNeighbors CentroidDef(j, gDataHub);
     CentroidDef.Set(tCenter);
     gpClusterComparator->Initialize(tCenter);
     gpClusterComparator->SetEllipseOffset(j, gDataHub);
     if (gpPSClusterComparator) {
       gpPSClusterComparator->Initialize(tCenter);
       gpPSClusterComparator->SetEllipseOffset(j, gDataHub);
       gpPSClusterComparator->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, gPSTopShapeClusters.GetTopCluster(j), *gpLikelihoodCalculator);
     }
     else {
       gpPSPClusterComparator->Initialize(tCenter);
       gpPSPClusterComparator->SetEllipseOffset(j, gDataHub);
       gpPSPClusterComparator->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, gPSPTopShapeClusters.GetTopCluster(j), *gpLikelihoodCalculator);
     }
     gpClusterComparator->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, gTopShapeClusters.GetTopCluster(j), *gpTimeIntervals);
  }
  return GetTopCalculatedCluster();
}

/** returns top cluster calculated during 'FindTopClsuters()' routine */
const CCluster & C_ST_PS_Analysis::GetTopCalculatedCluster() {
  try {
    CCluster& PSCluster = gpPSClusterComparator ? (CCluster&)gPSTopShapeClusters.GetTopCluster() : (CCluster&)gPSPTopShapeClusters.GetTopCluster();
    CCluster& STCluster = (CCluster&)gTopShapeClusters.GetTopCluster();
    if (!PSCluster.ClusterDefined())
      return STCluster;
    else if (PSCluster.m_nRatio > STCluster.m_nRatio)
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

  gpMeasureList->Reset();
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
double C_ST_PS_Analysis::MonteCarloProspective(const DataSetInterface & Interface) {
  tract_t               k, i;

  gpMeasureList->Reset();
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

