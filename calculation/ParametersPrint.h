//******************************************************************************
#ifndef ParametersPrintH
#define ParametersPrintH
//******************************************************************************
#include "Parameters.h"
#include "AsciiPrintFormat.h"

class DataSetHandler; /* forward class declaration */

/** Provides methods to print CParameters in an organized manner to the main output
    file of an analysis. */
class ParametersPrint {
  private:
    typedef std::vector<std::pair<std::string,std::string>> SettingContainer_t;
    const CParameters & _parameters;

    void                PrintAdditionalOutputFiles(FILE* fp) const;
    void                PrintAnalysisParameters(FILE* fp) const;
    void                PrintMiscellaneousAnalysisParameters(FILE* fp) const;
    void                PrintSpatialOutputParameters(FILE* fp) const;
    void                PrintDataCheckingParameters(FILE* fp) const;
    void                PrintDrilldownParameters(FILE* fp, bool isDrilldown) const;
    void                PrintEllipticScanParameters(FILE* fp) const;
    void                PrintNotificationsParameters(FILE* fp) const;
    void                PrintClusterRestrictionsParameters(FILE* fp) const;
    void                PrintInferenceParameters(FILE* fp) const;
    void                PrintInputParameters(FILE* fp) const;
    void                PrintMultipleDataSetParameters(FILE* fp) const;
    void                PrintOtherOutputParameters(FILE* fp) const;
    void                PrintOutputParameters(FILE* fp) const;
    void                PrintPowerEvaluationsParameters(FILE* fp) const;
    void                PrintPowerSimulationsParameters(FILE* fp) const;
    void                PrintPolygonParameters(FILE* fp) const;
    void                PrintRunOptionsParameters(FILE* fp) const;
    void                PrintSpaceAndTimeAdjustmentsParameters(FILE* fp) const;
    void                PrintSpatialNeighborsParameters(FILE* fp) const;
    void                PrintLocationNetworkParameters(FILE* fp) const;
    void                PrintSpatialWindowParameters(FILE* fp) const;
    void                PrintSystemParameters(FILE* fp) const;
    void                PrintTemporalWindowParameters(FILE* fp) const;
    void                PrintTemporalOutputParameters(FILE* fp) const;

    void                WriteSettingsContainer(const SettingContainer_t& settings, const std::string& section, FILE* fp, unsigned int margin=2) const;

  public:
    ParametersPrint(const CParameters& Parameters) : _parameters(Parameters) {}
    ~ParametersPrint() {}

    const char        * GetAnalysisTypeAsString() const;
    const char        * GetTimeAggregationTypeAsString() const;
    const char        * GetAreaScanRateTypeAsString() const;
    const char        * GetProbabilityModelTypeAsString() const;
    const char        * getPowerEvaluationMethodAsString() const;
    void                Print(FILE* fp, bool isDrilldown=false) const;
    void                PrintAnalysisSummary(FILE* fp, const DataSetHandler& SetHandler) const;
    std::string         getCalculatedTimeTrendAsString(const DataSetHandler& SetHandler) const;
};
//******************************************************************************
#endif

