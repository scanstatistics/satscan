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
      static const char       * DATASTREAM_FIELD;
      const CParameters       & gParameters;

      ZdString                & GetLocationId(ZdString& sId, tract_t tTractIndex, const CSaTScanData& DataHub);

   public:
      RelativeRiskData(const CParameters& Parameters);
      virtual ~RelativeRiskData();

      virtual const char      * GetOutputExtension() const {return REL_RISK_EXT;}
      void                      RecordRelativeRiskData(const CSaTScanData& DataHub);
      void                      RecordRelativeRiskData(const CSVTTData& DataHub);
};
//*****************************************************************************
#endif
