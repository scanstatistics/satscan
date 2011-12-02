//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "Analysis.h"
#include "cluster.h"
#include "MostLikelyClustersContainer.h"

/** constructor */
CAnalysis::CAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
          :AbstractAnalysis(Parameters, DataHub, PrintDirection) {}

/** destructor */
CAnalysis::~CAnalysis() {}

/** Executes simulation. Calls MonteCarlo() for analyses that can utilize
    TMeasureList class or perform simulations by the same algorithm as the real data. */
double CAnalysis::ExecuteSimulation(const AbstractDataSetGateway& DataGateway) {

  if (geReplicationsProcessType == MeasureListEvaluation)
    return MonteCarlo(DataGateway.GetDataSetInterface(0));

  if (gParameters.GetIsPurelyTemporalAnalysis())
    return MonteCarlo(0, DataGateway);

  double dMaxLogLikelihoodRatio=0;
  //calculate greatest loglikelihood ratio about each centroid
  for (int i=0; i < gDataHub.m_nGridTracts && !gPrintDirection.GetIsCanceled(); ++i)
    dMaxLogLikelihoodRatio = std::max(MonteCarlo(i, DataGateway), dMaxLogLikelihoodRatio);
  return dMaxLogLikelihoodRatio;
}

/** Given data gate way, calculates and collects most likely clusters about
    each grid point. Collection of clusters are sorted by loglikelihood ratio
    and condensed based upon overlapping geographical areas.                */
void CAnalysis::FindTopClusters(const AbstractDataSetGateway& DataGateway, MLC_Collections_t& TopClustersContainers) {
  boost::posix_time::ptime StartTime = ::GetCurrentTime_HighResolution();//start clock which will help determine how long this process will take
  try {
    //calculate top cluster about each centroid(grid point) and store copy in top cluster array
    for (int i=0; i < gDataHub.m_nGridTracts && !gPrintDirection.GetIsCanceled(); ++i) {
       const SharedClusterVector_t topClusters(CalculateTopClusters(i, DataGateway));
       for (size_t t=0; t < topClusters.size(); ++t) {
         const CCluster& TopCluster = *topClusters[t];
         const_cast<CCluster&>(TopCluster).SetCartesianRadius(gDataHub);
         const_cast<CCluster&>(TopCluster).SetMostCentralLocationIndex(gDataHub);
         TopClustersContainers.at(t).Add(TopCluster);
       }
       if (i==9)
         ReportTimeEstimate(StartTime, gDataHub.m_nGridTracts, i+1, &gPrintDirection);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("FindTopClusters()","CAnalysis");
    throw;
  }
}

