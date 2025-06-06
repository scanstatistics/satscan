//******************************************************************************
#ifndef ParametersValidateH
#define ParametersValidateH
//******************************************************************************
#include "Parameters.h"
#include "BasePrint.h"

/** Validates CParameters settings in relation to program and each other, as well as available functionality in program. */
class ParametersValidate {
  public: 
      static const char * MSG_INVALID_PARAM;

  private:
    const CParameters & gParameters;

    bool                checkFileExists(const std::string& filename, const std::string& filetype, BasePrint& PrintDirection, bool writeCheck=false) const;

    bool                ValidateBorderAnalysisParameters(BasePrint & printdirection) const;
    bool                ValidateClosedLoopAnalysisParameters(BasePrint & printdirection) const;
    bool                ValidateSpatialOutputParameters(BasePrint & PrintDirection) const;
    bool                ValidateContinuousPoissonParameters(BasePrint & PrintDirection) const;
    bool                ValidateDateParameters(BasePrint& PrintDirection) const;
    bool                ValidateDateString(BasePrint& PrintDirection, ParameterType eParameterType, const std::string& value) const;
    bool                ValidateDrilldownParameters(BasePrint & PrintDirection, bool isDilldown) const;
    bool                ValidateEllipseParameters(BasePrint & PrintDirection) const;
    bool                ValidateNotificationParameters(BasePrint & PrintDirection) const;
    bool                ValidateExecutionTypeParameters(BasePrint & PrintDirection) const;
    bool                ValidateFileParameters(BasePrint & PrintDirection) const;
    bool                ValidateInferenceParameters(BasePrint & PrintDirection) const;
    bool                ValidateInputSource(const CParameters::InputSource * source, const std::string& filename, const std::string& verbosename, BasePrint& PrintDirection) const;
    bool                ValidateIterativeScanParameters(BasePrint & PrintDirection) const;
    bool                ValidateLinelistParameters(BasePrint& PrintDirection) const;
    bool                ValidateMonotoneRisk(BasePrint& PrintDirection) const;
    bool                ValidateLocationNetworkParameters(BasePrint & PrintDirection) const;
    bool                ValidateOtherOutputOptionParameters(BasePrint & PrintDirection, bool isDrilldown) const;
    bool                ValidateOutputOptionParameters(BasePrint & PrintDirection) const;
    bool                ValidatePowerEvaluationsParameters(BasePrint & PrintDirection) const;
    bool                ValidateRandomizationSeed(BasePrint& PrintDirection) const;
    bool                ValidateRangeParameters(BasePrint & PrintDirection) const;
    bool                ValidateSimulationDataParameters(BasePrint & PrintDirection) const;
    bool                ValidateSpatialParameters(BasePrint & PrintDirection) const;
    bool                ValidateStudyPeriodEndDate(BasePrint& PrintDirection) const;
    bool                ValidateStudyPeriodStartDate(BasePrint& PrintDirection) const;
    bool                ValidateSVTTAnalysisSettings(BasePrint& PrintDirection) const;
    bool                ValidateTemporalClusterSize(BasePrint& PrintDirection) const;
    bool                ValidateTemporalParameters(BasePrint & PrintDirection) const;
    bool                ValidateTimeAggregationUnits(BasePrint & PrintDirection) const;
    bool                ValidateTemporalOutputParameters(BasePrint& PrintDirection, bool isDrilldown) const;

  public:
    ParametersValidate(const CParameters& Parameters): gParameters(Parameters) {}

    bool                Validate(BasePrint& PrintDirection, bool excludeFileValidation=false, bool isDrilldown=false) const;
};
//******************************************************************************
#endif

