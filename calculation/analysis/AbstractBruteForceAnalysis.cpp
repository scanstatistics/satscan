//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "AbstractBruteForceAnalysis.h"
#include "cluster.h"
#include "MostLikelyClustersContainer.h"
#include "MakeNeighbors.h"

/** constructor */
AbstractBruteForceAnalysis::AbstractBruteForceAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
          :CAnalysis(Parameters, DataHub, PrintDirection) {}

/** destructor */
AbstractBruteForceAnalysis::~AbstractBruteForceAnalysis() {}

/** Given data gate way, calculates and collects most likely clusters about
    each grid point. Collection of clusters are sorted by loglikelihood ratio
    and condensed based upon overlapping geographical areas.                */
void AbstractBruteForceAnalysis::FindTopClusters(const AbstractDataSetGateway& DataGateway, MostLikelyClustersContainer& TopClustersContainer) {
  int                   i;
  boost::posix_time::ptime StartTime = ::GetCurrentTime_HighResolution();//start clock which will help determine how long this process will take

  try {
    //calculate top cluster about each centroid(grid point) and store copy in top cluster array
    for (i=0; i < gDataHub.m_nGridTracts && !gPrintDirection.GetIsCanceled(); ++i) {
       const CCluster& TopCluster = CalculateTopCluster(i, DataGateway);
       if (TopCluster.ClusterDefined()) {
         std::auto_ptr<CentroidNeighbors> pNeighbors(gNeighborInfo[TopCluster.GetEllipseOffset()]);
         gNeighborInfo[TopCluster.GetEllipseOffset()] = 0;
         const_cast<CSaTScanData&>(gDataHub).addStore(pNeighbors);
         const_cast<CCluster&>(TopCluster).SetCartesianRadius(gDataHub);
         const_cast<CCluster&>(TopCluster).SetMostCentralLocationIndex(gDataHub);       
         TopClustersContainer.Add(TopCluster);
       }
       if (i==9)
         ReportTimeEstimate(StartTime, gDataHub.m_nGridTracts, i+1, &gPrintDirection);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("FindTopClusters()","AbstractBruteForceAnalysis");
    throw;
  }
}

