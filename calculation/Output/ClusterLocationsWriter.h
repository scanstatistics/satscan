//******************************************************************************
#ifndef ClusterLocationsWriterH
#define ClusterLocationsWriterH
//******************************************************************************
#include "AbstractDataFileWriter.h"
#include "WeightedNormalRandomizer.h"
#include "SimulationVariables.h"

class CCluster;       /** forward class declaration */
class CSaTScanData;   /** forward class declaration */

class LocationInformationWriter : public AbstractDataFileWriter {
  protected:
      static const char       * AREA_SPECIFIC_FILE_EXT;
      static const char       * LOC_OBS_FIELD;
      static const char       * LOC_EXP_FIELD;
      static const char       * LOC_MEAN_FIELD;
      static const char       * LOC_WEIGHTED_MEAN_FIELD;
      static const char       * LOC_OBS_DIV_EXP_FIELD;
      static const char       * LOC_REL_RISK_FIELD;
      static const char       * LOC_TIME_TREND_FIELD;
      static const char       * CLU_OBS_FIELD;
      static const char       * CLU_EXP_FIELD;
      static const char       * CLU_OBS_DIV_EXP_FIELD;
      static const char       * CLU_REL_RISK_FIELD;
      static const char       * CLU_MEAN_IN_FIELD;
      static const char       * CLU_MEAN_OUT_FIELD;
      static const char       * CLU_WEIGHTED_MEAN_IN_FIELD;
      static const char       * CLU_WEIGHTED_MEAN_OUT_FIELD;
      static const char       * CLU_TIME_TREND_IN_FIELD;
      static const char       * CLU_TIME_TREND_OUT_FIELD;
      static const char       * CLU_TIME_TREND_DIFF_FIELD;

      // TODO: These statements are for testing and will be removed eventually.  
      static const char       * CLU_ALPHA_IN_FIELD;
      static const char       * CLU_BETA1_IN_FIELD;
      static const char       * CLU_BETA2_IN_FIELD;
      static const char       * CLU_ALPHA_OUT_FIELD;
      static const char       * CLU_BETA1_OUT_FIELD;
      static const char       * CLU_BETA2_OUT_FIELD;
      static const char       * CLU_ALPHA_GLOBAL_FIELD;
      static const char       * CLU_BETA1_GLOBAL_FIELD;
      static const char       * CLU_BETA2_GLOBAL_FIELD;

      AbstractWeightedNormalRandomizer::ClusterLocationStatistics gStatistics;

      void                      DefineFields(const CSaTScanData& DataHub);

  public:
    LocationInformationWriter(const CSaTScanData& DataHub, bool bAppend=false);
    virtual ~LocationInformationWriter();

      virtual void              Write(const CCluster& theCluster, const CSaTScanData& theData,
                                      int iClusterNumber, tract_t tTract, const SimulationVariables& simVars);
      void                      WritePrep(const CCluster& theCluster, const CSaTScanData& DataHub);
};
//******************************************************************************
#endif
