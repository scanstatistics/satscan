//******************************************************************************
#ifndef LocationRiskEstimateWriterH
#define LocationRiskEstimateWriterH
//******************************************************************************
#include "AbstractDataFileWriter.h"

class CSaTScanData;  /** forward class declaration */
class CSVTTData;     /** forward class declaration */

class LocationRiskEstimateWriter : public AbstractDataFileWriter {
  protected:
      static const char       * REL_RISK_EXT;
      static const char       * TIME_TREND_FIELD;
      static const char       * DATASET_FIELD;
      static const char       * CATEGORY_FIELD;
      static const char       * OBSERVED_DIV_EXPECTED_FIELD;
      static const char       * RELATIVE_RISK_FIELD;

      void                      DefineFields();
      ZdString                & GetLocationId(ZdString& sId, tract_t tTractIndex, const CSaTScanData& DataHub) const;
      void                      RecordRelativeRiskDataAsOrdinal(const CSaTScanData& DataHub);
      void                      RecordRelativeRiskDataStandard(const CSaTScanData& DataHub);

  public:
    LocationRiskEstimateWriter(const CParameters& Parameters);
    virtual ~LocationRiskEstimateWriter();
    
      void                      Write(const CSaTScanData& DataHub);
      void                      Write(const CSVTTData& DataHub);
};
//******************************************************************************
#endif
