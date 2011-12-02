//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "PurelySpatialBruteForceAnalysis.h"
#include "HomogeneousPoissonRandomizer.h"

#include "ClusterData.h"
#include "SSException.h"

/** Constructor */
CPurelySpatialBruteForceAnalysis::CPurelySpatialBruteForceAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
                       :AbstractBruteForceAnalysis(Parameters, DataHub, PrintDirection), _topClusters(DataHub) {}

/** Destructor */
CPurelySpatialBruteForceAnalysis::~CPurelySpatialBruteForceAnalysis(){}

/** Allocates objects used during Monte Carlo simulations instead of repeated
    allocations for each simulation. This method must be called prior to MonteCarlo(). */
void CPurelySpatialBruteForceAnalysis::AllocateSimulationObjects(const AbstractDataSetGateway& DataGateway) {
  try {
    //create simulation objects based upon which process used to perform simulations
    if (geReplicationsProcessType == MeasureListEvaluation)
      gMeasureList.reset(GetNewMeasureListObject());
    gAbstractClusterData.reset(new SpatialHomogeneousData(DataGateway));
  }
  catch (prg_exception& x) {
    x.addTrace("AllocateSimulationObjects()","CPurelySpatialBruteForceAnalysis");
    throw;
  }
}

/** Allocates objects used during calculation of most likely clusters, instead
    of repeated allocations. This method must be called prior to CalculateTopCluster(). */
void CPurelySpatialBruteForceAnalysis::AllocateTopClustersObjects(const AbstractDataSetGateway& DataGateway) {
  try {
    gClusterComparator.reset(new PurelySpatialHomogeneousPoissonCluster(DataGateway));
    _topClusters.setTopClusters(*gClusterComparator);
    gCentroidCalculator.reset(new CentroidNeighborCalculator(gDataHub, gPrintDirection));
  }
  catch (prg_exception& x) {
    x.addTrace("AllocateTopClustersObjects()","CPurelySpatialBruteForceAnalysis");
    throw;
  }
}

/** Returns cluster centered at grid point nCenter, with the greatest loglikelihood ratio.
    Caller should not assume that returned reference is persistent, but should either call
    Clone() method or overloaded assignment operator. */
const SharedClusterVector_t CPurelySpatialBruteForceAnalysis::CalculateTopClusters(tract_t tCenter, const AbstractDataSetGateway& DataGateway) {
  _topClusters.reset(tCenter);
  gNeighborInfo.killAll();
  const CentroidNeighborCalculator::LocationDistContainer_t& locDist = gCentroidCalculator->getLocationDistances();
  for (int j=0; j <= gParameters.GetNumTotalEllipses(); ++j) {
     gNeighborInfo.push_back(new CentroidNeighbors());
     gCentroidCalculator->CalculateNeighborsAboutCentroid(j, tCenter, *gNeighborInfo.back());
     gClusterComparator->Initialize(tCenter);
     gClusterComparator->SetEllipseOffset(j, gDataHub);
     gClusterComparator->CalculateTopClusterAboutCentroidDefinition(DataGateway, *gNeighborInfo.back(), locDist, _topClusters.getClusterSet(j), *gpLikelihoodCalculator);
  }
  SharedClusterVector_t topClusters;
  return _topClusters.getTopClusters(topClusters);
}

/** Returns loglikelihood ratio for Monte Carlo replication using same algorithm as real data. */
double CPurelySpatialBruteForceAnalysis::MonteCarlo(tract_t tCenter, const AbstractDataSetGateway & DataGateway) {
    return MonteCarlo(DataGateway.GetDataSetInterface(0));
}

/** Returns loglikelihood ratio for Monte Carlo replication utilizing measure list structure. */
double CPurelySpatialBruteForceAnalysis::MonteCarlo(const DataSetInterface& Interface) {
  tract_t       k, i;
  SpatialHomogeneousData * pSpatialData = gAbstractClusterData.get();

  gMeasureList->Reset();
  const CentroidNeighborCalculator::LocationDistContainer_t& locDist = gCentroidCalculator->getLocationDistances();
  for (k=0; k <= gParameters.GetNumTotalEllipses(); ++k) {
     for (i=0; i < gDataHub.m_nGridTracts; ++i) {
        CentroidNeighbors centroidDef;
        gCentroidCalculator->CalculateNeighborsAboutCentroid(k, i, centroidDef);
        centroidDef.SetMaximumClusterSize_SimulatedData();
        pSpatialData->AddMeasureList(centroidDef, locDist,Interface, gMeasureList.get());
     }
     gMeasureList->SetForNextIteration(k);
  }
  return gMeasureList->GetMaximumLogLikelihoodRatio();
}

/** Allocates additional objects used during randomization. */
void CPurelySpatialBruteForceAnalysis::AllocateAdditionalSimulationObjects(RandomizerContainer_t& Container) {
   try {
     //Ensure that only one randomizer exists in container -- only one data set implemented for H. Poisson.
     if (Container.size() != 1)
        throw prg_error("Expecting exactly one randomizer.","AllocateAdditionalSimulationObjects()");

     //Cast container object to expected HomogeneousPoissonRandomizer.
     HomogeneousPoissonRandomizer * pRandomizer = dynamic_cast<HomogeneousPoissonRandomizer*>(*Container.begin());
     if (!pRandomizer)
        throw prg_error("Randomizer cound not be cast to HomogeneousPoissonRandomizer type.","AllocateAdditionalSimulationObjects()");

     //If using grid file, get handler from data hub; else get separate object held by randomizer.
     const GInfo& ginfo = (gParameters.UseSpecialGrid() ? *gDataHub.GetGInfo() : pRandomizer->getCentroidHandler());
     //Allocate CentroidNeighborCalculator object used during randomization.
     gCentroidCalculator.reset(new CentroidNeighborCalculator(gDataHub, pRandomizer->getTractHandler(), ginfo, gPrintDirection));
   }
   catch (prg_exception& x) {
     x.addTrace("AllocateAdditionalSimulationObjects()","CPurelySpatialBruteForceAnalysis");
     throw;
   }
}
