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

/** A helper class which assists with identifying locations for which we report a relative risk. */
class LocationsReportHelper {
    public:
        typedef MinimalGrowthArray<tract_t> IndentifierList_t;
        typedef std::map<const Location*, IndentifierList_t> LocationToIdentifiers_t;
        typedef std::vector<const Location*> LocationIdentifiers_t;
        typedef std::map<size_t, std::vector<FieldValue>> RptLocationData_t;

    private:
        const CSaTScanData& _data_hub;
        LocationToIdentifiers_t _location_to_identifiers;
        boost::dynamic_bitset<> _report_locations;
        RptLocationData_t _location_riskdata;

    public:
        LocationsReportHelper(const CSaTScanData& DataHub);

        void addRptLocationRiskData(size_t loc_idx, const std::vector<FieldValue>& riskdata);
        const std::vector<FieldValue>& getRptLocationRiskData(size_t loc_idx) { return _location_riskdata[loc_idx]; }
        void getLocationInfo(unsigned int locIdx, std::vector<tract_t>& identifierIndexes, std::string& name, unsigned int nameMax) const;
        const LocationToIdentifiers_t& getLocationToIdentifiers() const { return _location_to_identifiers; }
        const boost::dynamic_bitset<>& getReportLocations() const { return _report_locations; }
};

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
      static const char       * WEIGHTED_MEAN_VALUE_FIELD;
      static const char       * OLIVEIRA_F_MLC_FIELD;
      static const char       * OLIVEIRA_F_HIERARCHICAL_FIELD;

  protected:
      void                      DefineFields(const CSaTScanData& DataHub);
      std::pair<double, double> getNormalMeanForIdentifiers(const std::vector<tract_t>& identifiers, count_t * pCases, measure_t * pMeasure, measure_t * pMeasureAux) const;
      void                      writeGeneric(const CSaTScanData& DataHub, const LocationRelevance& location_relevance);
      void                      writeOrdinal(const CSaTScanData& DataHub);
      void                      writeSTP(const CSaTScanData& DataHub, const MostLikelyClustersContainer& mlc);
      void                      writeWeightedNormal(const CSaTScanData& DataHub);

  public:
    LocationRiskEstimateWriter(const CSaTScanData& DataHub);
    virtual ~LocationRiskEstimateWriter() {}
    
      void                      Write(const CSaTScanData& DataHub, const LocationRelevance& location_relevance, const MostLikelyClustersContainer& mlc);
      void                      Write(const CSVTTData& DataHub);
};
//******************************************************************************
#endif
