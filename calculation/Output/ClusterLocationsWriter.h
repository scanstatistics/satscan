//******************************************************************************
#ifndef ClusterLocationsWriterH
#define ClusterLocationsWriterH
//******************************************************************************
#include "AbstractDataFileWriter.h"

class CCluster;       /** forward class declaration */
class CSaTScanData;   /** forward class declaration */

class LocationInformationWriter : public AbstractDataFileWriter {
  protected:
      static const char       * AREA_SPECIFIC_FILE_EXT;
      static const char       * LOC_OBS_FIELD;
      static const char       * LOC_EXP_FIELD;
      static const char       * LOC_MEAN_FIELD;
      static const char       * LOC_OBS_DIV_EXP_FIELD;
      static const char       * LOC_REL_RISK_FIELD;
      static const char       * CLU_OBS_FIELD;
      static const char       * CLU_EXP_FIELD;
      static const char       * CLU_OBS_DIV_EXP_FIELD;
      static const char       * CLU_REL_RISK_FIELD;
      static const char       * CLU_MEAN_IN_FIELD;
      static const char       * CLU_MEAN_OUT_FIELD;
      static const char       * CLU_VARIANCE_FIELD;
      static const char       * CLU_DEVIATION_FIELD;
      bool                      gbExcludePValueField;

      void                      DefineFields();

  public:
    LocationInformationWriter(const CParameters& Parameters, bool bExcludePValueField, bool bAppend=false);
    virtual ~LocationInformationWriter();

      virtual void              Write(const CCluster& theCluster, const CSaTScanData& theData,
                                      int iClusterNumber, tract_t tTract, unsigned int iNumSimsCompleted);
};
//******************************************************************************
#endif
