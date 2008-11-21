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
      static const char       * WEIGHTED_MEAN_VALUE_FIELD;

      void                      DefineFields(const CSaTScanData& DataHub);
      std::string             & getLocationId(std::string& sId, tract_t tTractIndex, const CSaTScanData& DataHub) const;
      void                      RecordRelativeRiskDataAsOrdinal(const CSaTScanData& DataHub);
      void                      RecordRelativeRiskDataStandard(const CSaTScanData& DataHub);
      void                      RecordRelativeRiskDataAsWeightedNormal(const CSaTScanData& DataHub);

  public:
    LocationRiskEstimateWriter(const CSaTScanData& DataHub);
    virtual ~LocationRiskEstimateWriter();
    
      void                      Write(const CSaTScanData& DataHub);
      void                      Write(const CSVTTData& DataHub);
};
//******************************************************************************
#endif
