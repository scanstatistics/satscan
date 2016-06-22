//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "AbstractBruteForceAnalysis.h"
#include "cluster.h"
#include "MostLikelyClustersContainer.h"
#include "MakeNeighbors.h"
#include "PrintQueue.h"

/** Given data gate way, calculates and collects most likely clusters about
    each grid point. Collection of clusters are sorted by loglikelihood ratio
    and condensed based upon overlapping geographical areas.                */
void AbstractBruteForceAnalysis::FindTopClusters(const AbstractDataSetGateway& DataGateway, MLC_Collections_t& TopClustersContainers) {
  boost::posix_time::ptime StartTime = ::GetCurrentTime_HighResolution();//start clock which will help determine how long this process will take
  try {
    // assert pre-condition for this analysis, not implemented for elliptical windows
    assert(gParameters.GetNumTotalEllipses() == 0);

    bool frequent_estimations = false;
    int modulas = std::max(1, static_cast<int>(std::floor(gDataHub.m_nGridTracts * STANDARD_MODULAS_PERCENT)));
    //calculate top cluster about each centroid(grid point) and store copy in top cluster array
    for (int i=0; i < gDataHub.m_nGridTracts && !gPrintDirection.GetIsCanceled(); ++i) {
       const SharedClusterVector_t topClusters(CalculateTopClusters(i, DataGateway));
       for (size_t t=0; t < topClusters.size(); ++t) {
            const CCluster& TopCluster = *topClusters[t];
            if (TopCluster.ClusterDefined()) {
                if (gNeighborInfo[TopCluster.GetEllipseOffset()]) { // if neighbor info still set in class variable, add to store and clear
                    std::auto_ptr<CentroidNeighbors> pNeighbors(gNeighborInfo[TopCluster.GetEllipseOffset()]); gNeighborInfo[TopCluster.GetEllipseOffset()] = 0;
                    const_cast<CSaTScanData&>(gDataHub).addStore(pNeighbors);
                }
                const_cast<CCluster&>(TopCluster).SetCartesianRadius(gDataHub);
                const_cast<CCluster&>(TopCluster).SetMostCentralLocationIndex(gDataHub);   
                TopClustersContainers.at(t).Add(TopCluster);
            }
       }
       if (i == 9 || (frequent_estimations && ((i + 1) % modulas == 0)))
           frequent_estimations = ReportTimeEstimate(StartTime, gDataHub.m_nGridTracts, i + 1, gPrintDirection, false, i != 9) > FREQUENT_ESTIMATES_SECONDS;
    }
  } catch (prg_exception& x) {
    x.addTrace("FindTopClusters()","AbstractBruteForceAnalysis");
    throw;
  }
}

