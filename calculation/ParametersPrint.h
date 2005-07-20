//******************************************************************************
#ifndef ParametersPrintH
#define ParametersPrintH
//******************************************************************************
#include "Parameters.h"
#include "AsciiPrintFormat.h"

class DataSetHandler; /* forward class declaration */

class ParametersPrint {
  private:
    const CParameters & gParameters;

    void                PrintAnalysisParameters(FILE* fp) const;
    void                PrintClustersReportedParameters(FILE* fp) const;
    void                PrintEllipticScanParameters(FILE* fp) const;
    void                PrintInferenceParameters(FILE* fp) const;
    void                PrintInputParameters(FILE* fp) const;
    void                PrintIsotonicScanParameters(FILE* fp) const;
    void                PrintMultipleDataSetParameters(FILE* fp) const;
    void                PrintOutputParameters(FILE* fp) const;
    void                PrintPowerSimulationsParameters(FILE* fp) const;
    void                PrintRunOptionsParameters(FILE* fp) const;
    void                PrintSequentialScanParameters(FILE* fp) const;
    void                PrintSpaceAndTimeAdjustmentsParameters(FILE* fp) const;
    void                PrintSpatialWindowParameters(FILE* fp) const;
    void                PrintSystemParameters(FILE* fp) const;
    void                PrintTemporalWindowParameters(FILE* fp) const;

  public:
    ParametersPrint(const CParameters& Parameters);
    ~ParametersPrint();

    void                Print(FILE* fp) const;
    void                PrintAdjustments(FILE* fp, const DataSetHandler& SetHandler) const;
    void                PrintAnalysisSummary(FILE* fp) const;
    void                PrintCalculatedTimeTrend(FILE* fp, const DataSetHandler& SetHandler) const;
};
//******************************************************************************
#endif
