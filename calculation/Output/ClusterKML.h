//******************************************************************************
#ifndef __ClusterKML_H
#define __ClusterKML_H
//******************************************************************************
#include<vector>
#include "SimulationVariables.h"
#include "MostLikelyClustersContainer.h"
#include "GisUtils.h"

class CSaTScanData;

class BaseClusterKML {
  protected:      
      typedef std::vector<FileName> file_collection_t;
      static const char * KMZ_FILE_EXT;
      static const char * KML_FILE_EXT;
      static double _minRatioToReport;

      const CSaTScanData & _dataHub;
      bool _visibleLocations;
      bool _separateLocationsKML;
      mutable boost::dynamic_bitset<> _cluster_locations;

      unsigned int addClusters(const MostLikelyClustersContainer& clusters, const SimulationVariables& simVars, std::ofstream& outKML, file_collection_t& fileCollection, unsigned int clusterOffset=0);
      void createKMZ(const file_collection_t& fileCollection, bool removefiles=true);
      std::string & encode(const std::string& data, std::string& encoded) const;
      std::string & getClusterBalloonTemplate(const CCluster& cluster, std::string& buffer) const;
      std::string & getClusterExtendedData(const CCluster& cluster, int iCluster, std::string& buffer) const;
      std::string & getClusterStyleTags(const CCluster& cluster, int iCluster, std::string& styleString, bool isHighRate) const;
      void writeCloseBlockKML(std::ofstream& outKML) const;
      void writeCluster(file_collection_t& fileCollection, std::ofstream& outKML, const CCluster& cluster, int iCluster, const SimulationVariables& simVars) const;
      void writeOpenBlockKML(std::ofstream& outKML) const;

  public:
      BaseClusterKML(const CSaTScanData& dataHub);
      virtual ~BaseClusterKML() {}
};

class ClusterKML : public BaseClusterKML {
protected:
    file_collection_t _fileCollection;
    std::ofstream _kml_out;
    unsigned int _clusters_written;
    unsigned int _locations_written;

public:
    ClusterKML(const CSaTScanData& dataHub);
    virtual ~ClusterKML() {}

    void add(const MostLikelyClustersContainer& clusters, const SimulationVariables& simVars);
    void finalize();
};
//******************************************************************************
#endif
