//******************************************************************************
#ifndef __ClusterKML_H
#define __ClusterKML_H
//******************************************************************************
#include<vector>
#include "SimulationVariables.h"
#include "MostLikelyClustersContainer.h"
#include "GisUtils.h"

class CSaTScanData;

class ClusterKML {
  typedef std::vector<FileName> file_collection_t;

  protected:      
      static const char                 * KMZ_FILE_EXT;
      static const char                 * KML_FILE_EXT;
      static double                       _minRatioToReport;

      const CSaTScanData                & _dataHub;
      const MostLikelyClustersContainer & _clusters;
      const SimulationVariables         & _simVars;
      bool                                _visibleLocations;
      bool                                _separateLocationsKML;

      file_collection_t                 & createKMLFiles(file_collection_t& fileCollection) const;
      std::string                       & getBalloonTemplate(std::string& buffer) const;
      std::string                       & getClusterLegend(const CCluster& cluster, int iCluster, std::string& legend) const;
      void                                writeCluster(file_collection_t& fileCollection, std::ofstream& outKML, const CCluster& cluster, int iCluster) const;

  public:
      ClusterKML(const CSaTScanData& dataHub, const MostLikelyClustersContainer& clusters, const SimulationVariables& simVars);
      ~ClusterKML() {}

      void                                generateKML();
      static std::string                & changeColor(std::string& s, long i, RandomNumberGenerator & rng);
};
//******************************************************************************
#endif
