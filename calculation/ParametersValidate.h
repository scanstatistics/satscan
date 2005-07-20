//******************************************************************************
#ifndef ParametersValidateH
#define ParametersValidateH
//******************************************************************************
#include "Parameters.h"
#include "BasePrint.h"

/** CParameters settings validator. */
class ParametersValidate {
  private:
    const CParameters & gParameters;

    bool                ValidateDateParameters(BasePrint& PrintDirection) const;
    bool                ValidateEllipseParameters(BasePrint & PrintDirection) const;
    bool                ValidateFileParameters(BasePrint & PrintDirection) const;
    bool                ValidateMaximumTemporalClusterSize(BasePrint& PrintDirection) const;
    bool                ValidatePowerCalculationParameters(BasePrint & PrintDirection) const;
    bool                ValidateProspectiveDate(BasePrint& PrintDirection) const;
    bool                ValidateRandomizationSeed(BasePrint& PrintDirection) const;
    bool                ValidateRangeParameters(BasePrint & PrintDirection) const;
    bool                ValidateSequentialScanParameters(BasePrint & PrintDirection) const;
    bool                ValidateSimulationDataParameters(BasePrint & PrintDirection) const;
    bool                ValidateSpatialParameters(BasePrint & PrintDirection) const;
    bool                ValidateStudyPeriodEndDate(BasePrint& PrintDirection) const;
    bool                ValidateStudyPeriodStartDate(BasePrint& PrintDirection) const;
    bool                ValidateTemporalParameters(BasePrint & PrintDirection) const;
    bool                ValidateTimeAggregationUnits(BasePrint & PrintDirection) const;

  public:
    ParametersValidate(const CParameters& Parameters);
    ~ParametersValidate();

    bool                Validate(BasePrint& PrintDirection) const;
};
//******************************************************************************
#endif
