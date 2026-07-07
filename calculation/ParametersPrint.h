//******************************************************************************
#ifndef ParametersPrintH
#define ParametersPrintH
//******************************************************************************
#include "Parameters.h"
#include "AsciiPrintFormat.h"

class DataSetHandler; /* forward class declaration */
class AnalysisResultsWriter; /* forward class declaration */

/** Provides methods to print CParameters in an organized manner to the main output
    file of an analysis. */
class ParametersPrint {
  private:
    typedef std::vector<std::pair<std::string,std::string>> SettingContainer_t;
    const CParameters & _parameters;

    SettingContainer_t& getAdditionalOutputFiles(SettingContainer_t& settings) const;
    SettingContainer_t& getInputParameters(SettingContainer_t& settings) const;
    SettingContainer_t& getAnalysisParameters(SettingContainer_t& settings) const;
    SettingContainer_t& getOutputParameters(SettingContainer_t& settings) const;
    SettingContainer_t& getPolygonParameters(SettingContainer_t& settings) const;
    SettingContainer_t& getMultipleDataSetParameters(SettingContainer_t& settings) const;
    SettingContainer_t& getDataCheckingParameters(SettingContainer_t& settings) const;
    SettingContainer_t& getSpatialNeighborsParameters(SettingContainer_t& settings) const;
    SettingContainer_t& getTemporalWindowParameters(SettingContainer_t& settings) const;
    SettingContainer_t& getClusterRestrictionsParameters(SettingContainer_t& settings) const;
    SettingContainer_t& getSpaceAndTimeAdjustmentsParameters(SettingContainer_t& settings) const;
    SettingContainer_t& getInferenceParameters(SettingContainer_t& settings) const;
    SettingContainer_t& getDrilldownParameters(SettingContainer_t& settings, bool isDrilldown) const;
    SettingContainer_t& getLocationNetworkParameters(SettingContainer_t& settings) const;
    SettingContainer_t& getMiscellaneousAnalysisParameters(SettingContainer_t& settings) const;
    SettingContainer_t& getPowerEvaluationsParameters(SettingContainer_t& settings) const;
    SettingContainer_t& getSpatialOutputParameters(SettingContainer_t& settings) const;
    SettingContainer_t& getTemporalOutputParameters(SettingContainer_t& settings) const;
    SettingContainer_t& getOtherOutputParameters(SettingContainer_t& settings) const;
    SettingContainer_t& getNotificationsParameters(SettingContainer_t& settings) const;
    SettingContainer_t& getEllipticScanParameters(SettingContainer_t& settings) const;
    SettingContainer_t& getPowerSimulationsParameters(SettingContainer_t& settings) const;
    SettingContainer_t& getRunOptionsParameters(SettingContainer_t& settings) const;
    SettingContainer_t& getSpatialWindowParameters(SettingContainer_t& settings) const;
    SettingContainer_t& getSystemParameters(SettingContainer_t& settings) const;
    void                writeSettingPairsHTML(const SettingContainer_t& settings, const std::string& section, const std::string& sectionClass, std::stringstream& out, bool isDrilldown) const;
    void                writeSettingPairsTextFile(const SettingContainer_t& settings, const std::string& section, FILE* fp, unsigned int margin=2) const;

  public:
    ParametersPrint(const CParameters& Parameters) : _parameters(Parameters) {}
    ~ParametersPrint() {}

    const char        * GetAnalysisTypeAsString() const;
    const char        * GetTimeAggregationTypeAsString() const;
    const char        * GetAreaScanRateTypeAsString() const;
    const char        * GetProbabilityModelTypeAsString() const;
    const char        * getPowerEvaluationMethodAsString() const;
    void                Print(BasePrint& printDirection, FILE* fp, std::ofstream * fstream=0, bool isDrilldown=false) const;
    void                Print(AnalysisResultsWriter& resultsWriter, bool isDrilldown = false) const;
    void                PrintAnalysisSummary(FILE* fp, const DataSetHandler& SetHandler) const;
    std::vector<std::string>& getAnalysisSummaryStatements(std::vector<std::string>& statements, const DataSetHandler& SetHandler) const;
    std::string         getCalculatedTimeTrendAsString(const DataSetHandler& SetHandler) const;
};
//******************************************************************************
#endif

