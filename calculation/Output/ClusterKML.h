//******************************************************************************
#ifndef __ClusterKML_H
#define __ClusterKML_H
//******************************************************************************
#include<vector>
#include "SimulationVariables.h"
#include "MostLikelyClustersContainer.h"

class CSaTScanData;

class ClusterKML {
  typedef std::vector<std::pair<double, double> > points_t;

  protected:      
      static const char                 * KML_FILE_EXT;
      const CSaTScanData                & _dataHub;
      const MostLikelyClustersContainer & _clusters;
      const SimulationVariables         & _simVars;
	  bool                                _includeLocations;
      bool                                _visibleLocations;

      std::string                       & getClusterLegend(const CCluster& cluster, int iCluster, std::string& legend) const;
      void                                writeCluster(std::ofstream& outKML, const CCluster& cluster, int iCluster) const;

  public:
      ClusterKML(const CSaTScanData& dataHub, const MostLikelyClustersContainer& clusters, const SimulationVariables& simVars);
      ~ClusterKML() {}

      void                                renderKML();

      static std::string                & changeColor(std::string& s, long i, RandomNumberGenerator & rng);

};
//******************************************************************************
#endif
