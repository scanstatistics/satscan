//*****************************************************************************
#ifndef RELATIVERISK_H
#define RELATIVERISK_H
//*****************************************************************************
#include "Parameters.h"
#include "stsOutputFileData.h"

class CSaTScanData;
class CSVTTData;

/** data model for the relative risk output file type */
class RelativeRiskData : public BaseOutputStorageClass {
   private:
      virtual void              SetupFields();

   protected:
      static const char       * REL_RISK_EXT;
      static const char       * TIME_TREND_FIELD;
      static const char       * DATASET_FIELD;
      static const char       * CATEGORY_FIELD;
      static const char       * OBSERVED_DIV_EXPECTED_FIELD;
      static const char       * RELATIVE_RISK_FIELD;
      const CParameters       & gParameters;

      ZdString                & GetLocationId(ZdString& sId, tract_t tTractIndex, const CSaTScanData& DataHub) const;
      void                      RecordRelativeRiskDataAsOrdinal(const CSaTScanData& DataHub);
      void                      RecordRelativeRiskDataStandard(const CSaTScanData& DataHub);

   public:
      RelativeRiskData(const CParameters& Parameters);
      virtual ~RelativeRiskData();

      virtual const char      * GetOutputExtension() const {return REL_RISK_EXT;}
      void                      RecordRelativeRiskData(const CSaTScanData& DataHub);
      void                      RecordRelativeRiskData(const CSVTTData& DataHub);
};
//*****************************************************************************
#endif
