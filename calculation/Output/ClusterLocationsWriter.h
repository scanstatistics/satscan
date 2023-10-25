//******************************************************************************
#ifndef ClusterLocationsWriterH
#define ClusterLocationsWriterH
//******************************************************************************
#include "AbstractDataFileWriter.h"
#include "WeightedNormalRandomizer.h"
#include "SimulationVariables.h"
#include "ShapeFileWriter.h"
#include "LocationRelevance.h"
#include "LocationNetwork.h"

class CCluster;       /** forward class declaration */
class CSaTScanData;   /** forward class declaration */

class LocationInformationWriter : public AbstractDataFileWriter {
  public:
      static const char       * AREA_SPECIFIC_FILE_EXT;
      static const char       * LOC_OBS_FIELD;
      static const char       * LOC_EXP_FIELD;
      static const char       * LOC_MEAN_FIELD;
      static const char       * LOC_WEIGHTED_MEAN_FIELD;
      static const char       * LOC_OBS_DIV_EXP_FIELD;
      static const char       * LOC_REL_RISK_FIELD;
      static const char       * LOC_POPULATION_FIELD;
      static const char       * LOC_TIME_TREND_FIELD;
      static const char       * LOC_COORD_LAT_FIELD;
      static const char       * LOC_COORD_LONG_FIELD;
      static const char       * LOC_COORD_X_FIELD;
      static const char       * LOC_COORD_Y_FIELD;
      static const char       * LOC_COORD_Z_FIELD;

      static const char       * CLU_OBS_FIELD;
      static const char       * CLU_EXP_FIELD;
      static const char       * CLU_OBS_DIV_EXP_FIELD;
      static const char       * CLU_REL_RISK_FIELD;
      static const char       * CLU_POPULATION_FIELD;
      static const char       * CLU_MEAN_IN_FIELD;
      static const char       * CLU_MEAN_OUT_FIELD;
      static const char       * CLU_WEIGHTED_MEAN_IN_FIELD;
      static const char       * CLU_WEIGHTED_MEAN_OUT_FIELD;
      static const char       * CLU_TIME_TREND_IN_FIELD;
      static const char       * CLU_TIME_TREND_OUT_FIELD;
      static const char       * CLU_TIME_TREND_DIFF_FIELD;

      static const char        * GINI_CLUSTER_FIELD;
      static const char        * OLIVEIRA_F_MLC_FIELD;
      static const char        * OLIVEIRA_F_HIERARCHICAL_FIELD;
      typedef std::vector<std::pair<const Location*, MinimalGrowthArray<size_t>>> LocationToIdentifiers_t;

  protected:
      AbstractWeightedNormalRandomizer::ClusterLocationStatistics _weighted_nornal_statistics;
      ShapeDataFileWriter * gpShapeDataFileWriter;
      boost::shared_ptr<NetworkLocationContainer_t> _clusterNetwork;
      LocationToIdentifiers_t _location_to_identifiers;

      void DefineFields(const CSaTScanData& DataHub);
      std::pair<double, double> getWeightedNormalMeanForIdentifiers(const MinimalGrowthArray<size_t>& identifiers) const;
      double getRelativeRiskForIdentifiers(const CSaTScanData& DataHub, const CCluster& cluster, const MinimalGrowthArray<size_t>& identifiers) const;
      boost::shared_ptr<AbstractTimeTrend> getTimeTrendForIdentifiers(const CSaTScanData& DataHub, const MinimalGrowthArray<size_t>& identifiers) const;

  public:
    LocationInformationWriter(const CSaTScanData& DataHub, bool bAppend=false);
    virtual ~LocationInformationWriter();

    virtual void WriteClusterLocations(
        const CCluster& theCluster, const CSaTScanData& theData, int iClusterNumber, const SimulationVariables& simVars, const LocationRelevance& location_relevance
    );
    void WritePrep(const CCluster& theCluster, const CSaTScanData& DataHub);
};
//******************************************************************************
#endif
