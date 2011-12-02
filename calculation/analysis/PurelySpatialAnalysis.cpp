//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "PurelySpatialAnalysis.h"
#include "ClusterData.h"
#include "SSException.h"

/** Constructor */
CPurelySpatialAnalysis::CPurelySpatialAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
                       :CAnalysis(Parameters, DataHub, PrintDirection), _topClusters(DataHub) {}

/** Desctructor */
CPurelySpatialAnalysis::~CPurelySpatialAnalysis(){}

/** Allocates objects used during Monte Carlo simulations instead of repeated
    allocations for each simulation. This method must be called prior to MonteCarlo(). */
void CPurelySpatialAnalysis::AllocateSimulationObjects(const AbstractDataSetGateway& DataGateway) {
  try {
    //create simulation objects based upon which process used to perform simulations
    if (geReplicationsProcessType == MeasureListEvaluation)
      gMeasureList.reset(GetNewMeasureListObject());
    gAbstractClusterData.reset(gpClusterDataFactory->GetNewSpatialClusterData(DataGateway));
  }
  catch (prg_exception& x) {
    x.addTrace("AllocateSimulationObjects()","CPurelySpatialAnalysis");
    throw;
  }
}

/** Allocates objects used during calculation of most likely clusters, instead
    of repeated allocations. This method must be called prior to CalculateTopCluster(). */
void CPurelySpatialAnalysis::AllocateTopClustersObjects(const AbstractDataSetGateway& DataGateway) {
  try {
    gClusterComparator.reset(new CPurelySpatialCluster(gpClusterDataFactory, DataGateway));
    _topClusters.setTopClusters(*gClusterComparator.get());
  }
  catch (prg_exception& x) {
    x.addTrace("AllocateTopClustersObjects()","CPurelySpatialAnalysis");
    throw;
  }
}

/** Calculates the top clusters about centroid and returns collection for each spatial expansion stop. */
const SharedClusterVector_t CPurelySpatialAnalysis::CalculateTopClusters(tract_t tCenter, const AbstractDataSetGateway& DataGateway) {
  _topClusters.reset(tCenter); // re-initialize top clusters objects
  for (int j=0; j <= gParameters.GetNumTotalEllipses(); ++j) {
     CentroidNeighbors CentroidDef(j, gDataHub, tCenter);
     _topClusters.resetNeighborCounts(j);
     gClusterComparator->Initialize(tCenter);
     gClusterComparator->SetEllipseOffset(j, gDataHub);
     gClusterComparator->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, _topClusters.getClusterSet(j), *gpLikelihoodCalculator);
  }
  SharedClusterVector_t topClusters;
  return _topClusters.getTopClusters(topClusters);
}


/** Returns loglikelihood ratio for Monte Carlo replication using same algorithm as real data. */
double CPurelySpatialAnalysis::MonteCarlo(tract_t tCenter, const AbstractDataSetGateway & DataGateway) {
  tract_t                       t, tNumNeighbors, * pIntegerArray;
  unsigned short              * pUnsignedShortArray;
  double                        dMaximizingValue;
  std::vector<double>           vMaximizingValues(gParameters.GetNumTotalEllipses() + 1, -std::numeric_limits<double>::max());
  std::vector<double>::iterator itr, itr_end;

  // for weighted normal model with covariates, we need to reload matrix data at the beginning of each simulation
  if (tCenter == 0 && gParameters.getIsWeightedNormalCovariates()) 
      gAbstractClusterData->InitializeData(DataGateway);

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
        dMaximizingValue = gAbstractClusterData->GetMaximizingValue(*gpLikelihoodCalculator);
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

/** Returns loglikelihood ratio for Monte Carlo replication utilizing measure list structure. */
double CPurelySpatialAnalysis::MonteCarlo(const DataSetInterface& Interface) {
  tract_t       k, i;
  SpatialData * pSpatialData = dynamic_cast<SpatialData*>(gAbstractClusterData.get());

  gMeasureList->Reset();
  for (k=0; k <= gParameters.GetNumTotalEllipses(); ++k) {
     CentroidNeighbors CentroidDef(k, gDataHub);
     for (i=0; i < gDataHub.m_nGridTracts; ++i) {
        CentroidDef.Set(i);
        pSpatialData->AddMeasureList(CentroidDef, Interface, gMeasureList.get());
     }
     gMeasureList->SetForNextIteration(k);
  }
  return gMeasureList->GetMaximumLogLikelihoodRatio();
}

