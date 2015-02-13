//******************************************************************************
#ifndef LocationRiskEstimateWriterH
#define LocationRiskEstimateWriterH
//******************************************************************************
#include "AbstractDataFileWriter.h"
#include "LocationRelevance.h"

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
      static const char       * OLIVEIRA_F_MLC_FIELD;
      static const char       * OLIVEIRA_F_HIERARCHICAL_FIELD;
      /* We're disabling the gini portion for the time being: https://www.squishlist.com/ims/satscan/66323/
      static const char       * OLIVEIRA_F_GINI_OPTIMAL_FIELD;
      static const char       * OLIVEIRA_F_GINI_MAXIMA_FIELD;
      static const char       * OLIVEIRA_F_HIERARCHICAL_GINI_OPTIMAL_FIELD;
      static const char       * OLIVEIRA_F_HIERARCHICAL_GINI_MAXIMA_FIELD;
      */

  protected:
      void                      DefineFields(const CSaTScanData& DataHub);
      std::string             & getLocationId(std::string& sId, tract_t tTractIndex, const CSaTScanData& DataHub) const;
      void                      RecordRelativeRiskDataAsOrdinal(const CSaTScanData& DataHub);
      void                      RecordRelativeRiskDataStandard(const CSaTScanData& DataHub, const LocationRelevance& location_relevance);
      void                      RecordRelativeRiskDataAsWeightedNormal(const CSaTScanData& DataHub);

  public:
    LocationRiskEstimateWriter(const CSaTScanData& DataHub);
    virtual ~LocationRiskEstimateWriter();
    
      void                      Write(const CSaTScanData& DataHub, const LocationRelevance& location_relevance);
      void                      Write(const CSVTTData& DataHub);
};
//******************************************************************************
#endif
