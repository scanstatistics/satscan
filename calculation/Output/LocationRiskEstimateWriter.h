//******************************************************************************
#ifndef LocationRiskEstimateWriterH
#define LocationRiskEstimateWriterH
//******************************************************************************
#include "AbstractDataFileWriter.h"

class CSaTScanData;  /** forward class declaration */
class CSVTTData;     /** forward class declaration */

class LocationRiskEstimateWriter : public AbstractDataFileWriter {
  public:
      static const char       * REL_RISK_EXT;
      static const char       * TREND_IN_FIELD;
      static const char       * TREND_OUT_FIELD;
      static const char       * ALPHA_IN_FIELD;
      static const char       * BETA1_IN_FIELD;
      static const char       * BETA2_IN_FIELD;
      static const char       * ALPHA_OUT_FIELD;
      static const char       * BETA1_OUT_FIELD;
      static const char       * BETA2_OUT_FIELD;
      //static const char       * FUNC_ALPHA_IN_FIELD;
      //static const char       * FUNC_ALPHA_OUT_FIELD;
      static const char       * WEIGHTED_MEAN_VALUE_FIELD;
      static const char        * OLIVIERA_F_FIELD;

  protected:
      void                      DefineFields(const CSaTScanData& DataHub);
      std::string             & getLocationId(std::string& sId, tract_t tTractIndex, const CSaTScanData& DataHub) const;
      void                      RecordRelativeRiskDataAsOrdinal(const CSaTScanData& DataHub);
      void                      RecordRelativeRiskDataStandard(const CSaTScanData& DataHub, const Relevance_Container_t& location_relevance);
      void                      RecordRelativeRiskDataAsWeightedNormal(const CSaTScanData& DataHub);

  public:
    LocationRiskEstimateWriter(const CSaTScanData& DataHub);
    virtual ~LocationRiskEstimateWriter();
    
      void                      Write(const CSaTScanData& DataHub, const Relevance_Container_t& location_relevance);
      void                      Write(const CSVTTData& DataHub);
};
//******************************************************************************
#endif
