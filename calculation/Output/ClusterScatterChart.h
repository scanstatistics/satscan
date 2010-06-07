//******************************************************************************
#ifndef __ClusterScatterChart_H
#define __ClusterScatterChart_h
//******************************************************************************
#include<vector>
#include "SimulationVariables.h"
#include "MostLikelyClustersContainer.h"

class CSaTScanData;

class ClusterScatterChart {
  typedef std::vector<std::pair<double, double> > points_t;

  protected:      
      const CSaTScanData                & _dataHub;
      const MostLikelyClustersContainer & _clusters;
      const SimulationVariables         & _simVars;

      std::string                       & getClusterLegend(const CCluster& cluster, int iCluster, std::string& legend) const;

  public:
      ClusterScatterChart(const CSaTScanData& dataHub, const MostLikelyClustersContainer& clusters, const SimulationVariables& simVars);
      ~ClusterScatterChart() {}

      void renderScatterChart();

      static std::string                & changeColor(std::string& s, long i, RandomNumberGenerator & rng);

};
//******************************************************************************
#endif
