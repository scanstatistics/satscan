//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "SpaceTimeIncludePurelySpatialAnalysis.h"

/** Constructor */
C_ST_PS_Analysis::C_ST_PS_Analysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
                 :CSpaceTimeAnalysis(Parameters, DataHub, PrintDirection) {
  try {
    Init();
    Setup();
  }
  catch (ZdException & x) {
    x.AddCallpath("constructor()", "C_ST_PS_Analysis");
    throw;
  }
}

/** Destructor */
C_ST_PS_Analysis::~C_ST_PS_Analysis() {
  try {
    delete gpPSTopShapeClusters;
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
void C_ST_PS_Analysis::AllocateSimulationObjects(const AbtractDataStreamGateway & DataGateway) {
  try {
    CSpaceTimeAnalysis::AllocateSimulationObjects(DataGateway);
    //delete object used during real data
    delete gpPSPClusterComparator; gpPSPClusterComparator=0;
    delete gpPSClusterComparator; gpPSClusterComparator=0;
    //create simulation objects based upon which process used to perform simulations
    if (gbMeasureListReplications) {
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
        gpPSTopShapeClusters->SetTopClusters(*gpPSPClusterComparator);
      }
      else {
        //create cluster object used as comparator when iterating over centroids and time intervals
//        gpPSClusterComparator = new CPurelySpatialCluster(gpClusterDataFactory, DataGateway, (int)m_pParameters->GetAreaScanRateType());
        //initialize list of top circle/ellipse clusters
        gpPSTopShapeClusters->SetTopClusters(*gpPSClusterComparator);
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
void C_ST_PS_Analysis::AllocateTopClustersObjects(const AbtractDataStreamGateway & DataGateway) {
  IncludeClustersType           eIncludeClustersType;

  try {
    //create top cluster objects for space-time portion
    CSpaceTimeAnalysis::AllocateTopClustersObjects(DataGateway);
    //create comparator cluster for purely spatial cluster
    gpPSClusterComparator = new CPurelySpatialCluster(gpClusterDataFactory, DataGateway, gParameters.GetAreaScanRateType());
    gpPSTopShapeClusters->SetTopClusters(*gpPSClusterComparator);
  }
  catch (ZdException &x) {
    delete gpPSClusterComparator; gpPSClusterComparator=0;
    x.AddCallpath("AllocateTopClustersObjects()","C_ST_PS_Analysis");
    throw;
  }
}

/** Returns cluster centered at grid point nCenter, with the greatest loglikelihood.
    Caller is responsible for deleting returned cluster. */
const CCluster& C_ST_PS_Analysis::CalculateTopCluster(tract_t tCenter, const AbtractDataStreamGateway & DataGateway) {
  int           j;

  //re-initialize clusters
  gpPSTopShapeClusters->Reset(tCenter);
  gpTopShapeClusters->Reset(tCenter);

  for (j=0 ;j <= gParameters.GetNumTotalEllipses(); ++j) {
     gpClusterComparator->Initialize(tCenter);
     gpClusterComparator->SetRate(gParameters.GetAreaScanRateType());
     gpClusterComparator->SetEllipseOffset(j);
     gpClusterComparator->SetDuczmalCorrection((j == 0 || !gParameters.GetDuczmalCorrectEllipses() ? 1 : gDataHub.GetShapesArray()[j - 1]));
     CSpaceTimeCluster & Top_ST_ShapeCluster = (CSpaceTimeCluster&)(gpTopShapeClusters->GetTopCluster(j));
     if (gpPSClusterComparator) {
       gpPSClusterComparator->Initialize(tCenter);
       gpPSClusterComparator->SetRate(gParameters.GetAreaScanRateType());
       gpPSClusterComparator->SetEllipseOffset(j);
       gpPSClusterComparator->SetDuczmalCorrection((j == 0 || !gParameters.GetDuczmalCorrectEllipses() ? 1 : gDataHub.GetShapesArray()[j - 1]));
       CPurelySpatialCluster & Top_PS_ShapeCluster = (CPurelySpatialCluster&)(gpPSTopShapeClusters->GetTopCluster(j));
       gpPSClusterComparator->AddNeighborDataAndCompare(j, tCenter, DataGateway, &gDataHub, Top_PS_ShapeCluster, *gpLikelihoodCalculator);
     }
     else {
       gpPSPClusterComparator->Initialize(tCenter);
       gpPSPClusterComparator->SetRate(gParameters.GetAreaScanRateType());
       gpPSPClusterComparator->SetEllipseOffset(j);
       gpPSPClusterComparator->SetDuczmalCorrection((j == 0 || !gParameters.GetDuczmalCorrectEllipses() ? 1 : gDataHub.GetShapesArray()[j - 1]));
       CPurelySpatialProspectiveCluster & Top_PSP_ShapeCluster = (CPurelySpatialProspectiveCluster&)(gpPSTopShapeClusters->GetTopCluster(j));
       gpPSPClusterComparator->AddNeighborAndCompare(j, tCenter, DataGateway, &gDataHub, Top_PSP_ShapeCluster, *gpLikelihoodCalculator);
     }
     gpClusterComparator->AddNeighborDataAndCompare(j, tCenter, DataGateway, &gDataHub, Top_ST_ShapeCluster, gpTimeIntervals);
  }
  return GetTopCalculatedCluster();
}

/** returns top cluster calculated during 'FindTopClsuters()' routine */
const CCluster & C_ST_PS_Analysis::GetTopCalculatedCluster() {
  try {
    CCluster& PSCluster = (CCluster&)(gpPSTopShapeClusters->GetTopCluster());
    CCluster& STCluster = (CCluster&)(gpTopShapeClusters->GetTopCluster());
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

/** internal initialization */
void C_ST_PS_Analysis::Init() {
  gpPSTopShapeClusters=0;
  gpPSClusterComparator=0;
  gpPSPClusterComparator=0;
  gpPSClusterData=0;
  gpPSPClusterData=0;
}

/** Returns loglikelihood for Monte Carlo replication. */
double C_ST_PS_Analysis::MonteCarlo(const DataStreamInterface & Interface) {
  double                        dMaxLogLikelihoodRatio;
  tract_t                       k, i;

  if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)
    return MonteCarloProspective(Interface);

  gpMeasureList->Reset();
  //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
  for (k=0; k <= gParameters.GetNumTotalEllipses(); ++k) {
     for (i=0; i < gDataHub.m_nGridTracts; ++i) {
        gpPSClusterData->AddMeasureList(k, i, Interface, gpMeasureList, &gDataHub);
        gpClusterData->AddNeighborDataAndCompare(k, i, Interface, &gDataHub, gpTimeIntervals, gpMeasureList);
     }
     gpMeasureList->SetForNextIteration(k);
  }
  return gpMeasureList->GetMaximumLogLikelihoodRatio();
}


/** Returns loglikelihood for Monte Carlo Prospective replication. */
double C_ST_PS_Analysis::MonteCarloProspective(const DataStreamInterface & Interface) {
  double                        dMaxLogLikelihoodRatio;
  tract_t                       k, i;

  gpMeasureList->Reset();
  //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
  for (k=0; k <= gParameters.GetNumTotalEllipses(); ++k) {
     for (i=0; i < gDataHub.m_nGridTracts; ++i) {
        gpPSPClusterData->AddMeasureList(k, i, Interface, gpMeasureList, &gDataHub);
        gpClusterData->AddNeighborDataAndCompare(k, i, Interface, &gDataHub, gpTimeIntervals, gpMeasureList);
     }
     gpMeasureList->SetForNextIteration(k);
  }
  return gpMeasureList->GetMaximumLogLikelihoodRatio();
}

void C_ST_PS_Analysis::Setup() {
  try {
    gpPSTopShapeClusters = new TopClustersContainer(gDataHub);
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()", "C_ST_PS_Analysis");
    delete gpPSTopShapeClusters;
    throw;
  }
}

