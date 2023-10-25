//******************************************************************************
#ifndef LocationRiskEstimateWriterH
#define LocationRiskEstimateWriterH
//******************************************************************************
#include "AbstractDataFileWriter.h"
#include "LocationRelevance.h"
#include "MultipleDimensionArrayHandler.h"

class CSaTScanData;  /** forward class declaration */
class CSVTTData;     /** forward class declaration */
class Identifier;
class Location;

class LocationRiskEstimateWriter : public AbstractDataFileWriter {
  public:
      typedef MinimalGrowthArray<tract_t> IndentifierList_t;
      typedef std::map<const Location*, IndentifierList_t> LocationToIdentifiers_t;
      typedef std::vector<const Location*> LocationIdentifiers_t;

      static const char       * REL_RISK_EXT;
      static const char       * TREND_IN_FIELD;
      static const char       * TREND_OUT_FIELD;
      static const char       * ALPHA_IN_FIELD;
      static const char       * BETA1_IN_FIELD;
      static const char       * BETA2_IN_FIELD;
      static const char       * ALPHA_OUT_FIELD;
      static const char       * BETA1_OUT_FIELD;
      static const char       * BETA2_OUT_FIELD;
      static const char       * WEIGHTED_MEAN_VALUE_FIELD;
      static const char       * OLIVEIRA_F_MLC_FIELD;
      static const char       * OLIVEIRA_F_HIERARCHICAL_FIELD;

  protected:
      void                      DefineFields(const CSaTScanData& DataHub);
      void                      getLocationInfo(unsigned int locIdx, const CSaTScanData& DataHub, std::vector<tract_t>& identifierIndexes, std::string& name, unsigned int nameMax) const;

      std::pair<double, double> getNormalMeanForIdentifiers(const std::vector<tract_t>& identifiers, count_t * pCases, measure_t * pMeasure, measure_t * pMeasureAux) const;

      void                      RecordRelativeRiskDataAsOrdinal(const CSaTScanData& DataHub);
      void                      RecordRelativeRiskDataAsWeightedNormal(const CSaTScanData& DataHub);
      void                      RecordRelativeRiskDataStandard(const CSaTScanData& DataHub, const LocationRelevance& location_relevance);

      LocationToIdentifiers_t   _location_to_identifiers;
      boost::dynamic_bitset<>   _report_locations;


  public:
    LocationRiskEstimateWriter(const CSaTScanData& DataHub);
    virtual ~LocationRiskEstimateWriter();
    
      void                      Write(const CSaTScanData& DataHub, const LocationRelevance& location_relevance);
      void                      Write(const CSVTTData& DataHub);
};
//******************************************************************************
#endif
