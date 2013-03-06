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
    typedef std::vector< std::pair<std::string, std::string> > SettingContainer_t;
    const CParameters & gParameters;

    void                PrintAdditionalOutputParameters(FILE* fp) const;
    void                PrintAnalysisParameters(FILE* fp) const;
    void                PrintClustersReportedParameters(FILE* fp) const;
    void                PrintDataCheckingParameters(FILE* fp) const;
    void                PrintEllipticScanParameters(FILE* fp) const;
    void                PrintInferenceParameters(FILE* fp) const;
    void                PrintInputParameters(FILE* fp) const;
    void                PrintIsotonicScanParameters(FILE* fp) const;
    void                PrintMultipleDataSetParameters(FILE* fp) const;
    void                PrintOutputParameters(FILE* fp) const;
    void                PrintPowerEvaluationsParameters(FILE* fp) const;
    void                PrintPowerSimulationsParameters(FILE* fp) const;
    void                PrintPolygonParameters(FILE* fp) const;
    void                PrintRunOptionsParameters(FILE* fp) const;
    void                PrintSpaceAndTimeAdjustmentsParameters(FILE* fp) const;
    void                PrintSpatialNeighborsParameters(FILE* fp) const;
    void                PrintSpatialWindowParameters(FILE* fp) const;
    void                PrintSystemParameters(FILE* fp) const;
    void                PrintTemporalWindowParameters(FILE* fp) const;

    void                WriteSettingsContainer(const SettingContainer_t& settings, const std::string& section, FILE* fp) const;

  public:
    ParametersPrint(const CParameters& Parameters);
    ~ParametersPrint();

    const char        * GetAnalysisTypeAsString() const;
    const char        * GetAreaScanRateTypeAsString() const;
    const char        * GetProbabilityModelTypeAsString() const;
    const char        * getPowerEvaluationMethodAsString() const;
    void                Print(FILE* fp) const;
    void                PrintAdjustments(FILE* fp, const DataSetHandler& SetHandler) const;
    void                PrintAnalysisSummary(FILE* fp) const;
    void                PrintCalculatedTimeTrend(FILE* fp, const DataSetHandler& SetHandler) const;
};
//******************************************************************************
#endif

