//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SpaceTimeAnalysis.h"
#include "MakeNeighbors.h"
#include "ClusterData.h"

/** Constructor */
CSpaceTimeAnalysis::CSpaceTimeAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
                   :CAnalysis(Parameters, DataHub, PrintDirection), gTopShapeClusters(DataHub),
                    gpClusterComparator(0), gpClusterData(0), gpMeasureList(0), gpTimeIntervals(0) {}

/** Destructor */
CSpaceTimeAnalysis::~CSpaceTimeAnalysis() {
  try {
    delete gpClusterComparator;
    delete gpClusterData;
    delete gpMeasureList;
    delete gpTimeIntervals;
  }
  catch(...){}
}

/** Allocates objects used during simulations, instead of repeated allocations
    for each simulation. Which objects that are allocated depends on whether
    the simluations process uses same process as real data or uses measure list. */
void CSpaceTimeAnalysis::AllocateSimulationObjects(const AbstractDataSetGateway& DataGateway) {
  IncludeClustersType           eIncludeClustersType;

  try {
    //delete object used during real data
    delete gpClusterComparator; gpClusterComparator=0;
    //create new time intervals object - delete existing object used during real data process
    delete gpTimeIntervals; gpTimeIntervals=0;
    if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)
      //for prospective analyses, allocate time object with ALLCLUSTERS for simulations
      eIncludeClustersType = ALLCLUSTERS;
    else
      eIncludeClustersType = gParameters.GetIncludeClustersType();
    gpTimeIntervals = GetNewTemporalDataEvaluatorObject(eIncludeClustersType);

    //create simulation objects based upon which process used to perform simulations
    if (geReplicationsProcessType == MeasureListEvaluation) {
      //create new cluster data object
      gpClusterData = new SpaceTimeData(DataGateway);
      //create new measure list object
      gpMeasureList = GetNewMeasureListObject();
    }
    else { //simulations performed using same process as real data set
      //create cluster object used as comparator when iterating over centroids and time intervals
      gpClusterComparator = new CSpaceTimeCluster(gpClusterDataFactory, DataGateway);
      //initialize list of top circle/ellipse clusters
      gTopShapeClusters.SetTopClusters(*gpClusterComparator);
    }
  }
  catch (ZdException &x) {
    delete gpClusterData; gpClusterData=0;
    delete gpMeasureList; gpMeasureList=0;
    delete gpTimeIntervals; gpTimeIntervals=0;
    delete gpClusterComparator; gpClusterComparator=0;
    x.AddCallpath("AllocateSimulationObjects()","CSpaceTimeAnalysis");
    throw;
  }
}

/** Allocates objects used during calculation of most likely clusters, instead
    of repeated allocations for each grid point.                             */
void CSpaceTimeAnalysis::AllocateTopClustersObjects(const AbstractDataSetGateway& DataGateway) {
  IncludeClustersType           eIncludeClustersType;

  try {
    //create new time intervals object - delete existing object used during real data process
    delete gpTimeIntervals; gpTimeIntervals=0;
    if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)
      //for prospective analyses, allocate time object with ALLCLUSTERS for simulations
      eIncludeClustersType = ALIVECLUSTERS;
    else
      eIncludeClustersType = gParameters.GetIncludeClustersType();
    gpTimeIntervals = GetNewTemporalDataEvaluatorObject(eIncludeClustersType);
    //create cluster object used as comparator when iterating over centroids and time intervals
    gpClusterComparator = new CSpaceTimeCluster(gpClusterDataFactory, DataGateway);
    //initialize list of top circle/ellipse clusters
    gTopShapeClusters.SetTopClusters(*gpClusterComparator);
  }
  catch (ZdException &x) {
    delete gpClusterComparator; gpClusterComparator=0;
    delete gpTimeIntervals; gpTimeIntervals=0;
    x.AddCallpath("AllocateTopClustersObjects()","CSpaceTimeAnalysis");
    throw;
  }
}

/** Returns cluster centered at grid point nCenter, with the greatest log
    likelihood ratio . Caller should not assume that returned reference is
    persistent, but should either call Clone() method or overloaded assignment
    operator. */
const CCluster& CSpaceTimeAnalysis::CalculateTopCluster(tract_t tCenter, const AbstractDataSetGateway& DataGateway) {
  tract_t               k;
  CentroidNeighbors     CentroidDef;

  gTopShapeClusters.Reset(tCenter);
  //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
  for (k=0; k <= gParameters.GetNumTotalEllipses(); ++k) {
     gpClusterComparator->Initialize(tCenter);
     gpClusterComparator->SetEllipseOffset(k, gDataHub);
     gpClusterComparator->CalculateTopClusterAboutCentroidDefinition(DataGateway,
                                                                     CentroidDef.Set(k, tCenter, gDataHub),
                                                                     gTopShapeClusters.GetTopCluster(k),
                                                                     *gpTimeIntervals);
  }
  return gTopShapeClusters.GetTopCluster();
}


/** Returns log likelihood ratio for Monte Carlo replication. */
double CSpaceTimeAnalysis::MonteCarlo(const DataSetInterface& Interface) {
  tract_t               k, i;
  CentroidNeighbors     CentroidDef;

  gpMeasureList->Reset();
  //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
  for (k=0; k <= gParameters.GetNumTotalEllipses(); ++k) {
     for (i=0; i < gDataHub.m_nGridTracts; ++i)
        gpClusterData->AddNeighborDataAndCompare(CentroidDef.Set(k, i, gDataHub), Interface, *gpTimeIntervals, *gpMeasureList);
     gpMeasureList->SetForNextIteration(k);
  }
  return gpMeasureList->GetMaximumLogLikelihoodRatio();
}

