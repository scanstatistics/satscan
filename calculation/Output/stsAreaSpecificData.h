//***************************************************************************
#ifndef stsAreaSpecificData_h
#define stsAreaSpecificData_h
//***************************************************************************
#include "stsOutputFileData.h"
#include "Parameters.h"

class CCluster;
class CSaTScanData;

/** This class is responsible for the storage of the area specific data for output.
    This class defines the fields which make up the the data contained therein.
    The class is derived from the base class BaseOutputStorageClass.            */
class stsAreaSpecificData : public BaseOutputStorageClass {
   private:
      virtual void              SetupFields();

   protected:
      static const char       * AREA_SPECIFIC_FILE_EXT;
      static const char       * LOC_OBS_FIELD;
      static const char       * LOC_EXP_FIELD;
      static const char       * LOC_OBS_DIV_EXP_FIELD;
      static const char       * CLU_OBS_FIELD;
      static const char       * CLU_EXP_FIELD;
      static const char       * CLU_OBS_DIV_EXP_FIELD;
      const CParameters       & gParameters;
      bool                      gbExcludePValueField;

   public:
      stsAreaSpecificData(const CParameters& Parameters, bool bExcludePValueField);
      virtual ~stsAreaSpecificData();

      virtual const char      * GetOutputExtension() const {return AREA_SPECIFIC_FILE_EXT;}
      virtual void              RecordClusterData(const CCluster& theCluster, const CSaTScanData& theData,
                                                  int iClusterNumber, tract_t tTract, unsigned int iNumSimsCompleted);
};
//***************************************************************************
#endif
