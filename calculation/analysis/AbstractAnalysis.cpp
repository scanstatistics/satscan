//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "AbstractAnalysis.h"
#include "TimeIntervalRange.h"
#include "ClusterDataFactory.h"
#include "NormalClusterDataFactory.h"
#include "CategoricalClusterDataFactory.h"
#include "UniformTimeClusterDataFactory.h"
#include "LikelihoodCalculation.h"
#include "PoissonLikelihoodCalculation.h"
#include "PoissonSVTTLikelihoodCalculation.h"
#include "BernoulliLikelihoodCalculation.h"
#include "WilcoxonLikelihoodCalculation.h"
#include "NormalLikelihoodCalculation.h"
#include "OrdinalLikelihoodCalculation.h"
#include "WeightedNormalLikelihoodCalculation.h"
#include "WeightedNormalCovariatesLikelihoodCalculation.h"
#include "UniformTimeLikelihoodCalculation.h"
#include "MeasureList.h"
#include "SSException.h"

/** constructor */
AbstractAnalysis::AbstractAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
                 :gParameters(Parameters), gDataHub(DataHub), gPrintDirection(PrintDirection),
                  gpClusterDataFactory(0), gpLikelihoodCalculator(0), geReplicationsProcessType(MeasureListEvaluation) {
  try {
    Setup();
  }
  catch (prg_exception& x) {
    x.addTrace("constructor()","AbstractAnalysis");
    throw;
  }
}

/** destructor */
AbstractAnalysis::~AbstractAnalysis() {
  try {
    delete gpClusterDataFactory;
    delete gpLikelihoodCalculator;
  }
  catch(...){}
}

/** Returns newly allocated log likelihood ratio calculator based upon requested
    probability model. Caller is responsible for object deletion.
    - throws prg_error if model type is not known */
AbstractLikelihoodCalculator * AbstractAnalysis::GetNewLikelihoodCalculator(const CSaTScanData& DataHub) {
  //create likelihood calculator
  switch (DataHub.GetParameters().GetProbabilityModelType()) {
	case POISSON              : if (DataHub.GetParameters().GetAnalysisType() == SPATIALVARTEMPTREND) {
		                          if (DataHub.GetParameters().getTimeTrendType() == QUADRATIC)
                                    return new PoissonQuadraticTrendLikelihoodCalculator(DataHub);
								  else
                                    return new PoissonLinearTrendLikelihoodCalculator(DataHub);
								}
    case HOMOGENEOUSPOISSON   :
    case SPACETIMEPERMUTATION :
    case EXPONENTIAL          : return new PoissonLikelihoodCalculator(DataHub);
    case UNIFORMTIME          : return new UniformTimeLikelihoodCalculator(DataHub);
    case BERNOULLI            : return new BernoulliLikelihoodCalculator(DataHub);
    case NORMAL               : if (DataHub.GetParameters().getIsWeightedNormal()) { 
                                    if (DataHub.GetParameters().getIsWeightedNormalCovariates()) 
                                       return new WeightedNormalCovariatesLikelihoodCalculator(DataHub);
                                    else 
                                       return new WeightedNormalLikelihoodCalculator(DataHub);
                                }
                                return new NormalLikelihoodCalculator(DataHub);
    case CATEGORICAL          : /*** may or may not implement separate class ***/
    case ORDINAL              : return new OrdinalLikelihoodCalculator(DataHub);
    case RANK                 : return new WilcoxonLikelihoodCalculator(DataHub);
    default                   :
     throw prg_error("Unknown probability model '%d'.", "GetNewLikelihoodCalculator()",
                      DataHub.GetParameters().GetProbabilityModelType());
  };
}

/** Returns newly allocated CMeasureList object - caller is responsible for deletion.
    - throws prg_error if type is not known */
CMeasureList * AbstractAnalysis::GetNewMeasureListObject() const {
    switch (gParameters.GetExecuteScanRateType()) {
    case HIGH:
        if (gParameters.getRiskLimitHighClusters()) 
            return new RiskMinMeasureList(gDataHub, *gpLikelihoodCalculator, gParameters.getRiskThresholdHighClusters());
        if (gParameters.GetProbabilityModelType() == RANK)
            return new CMaxMeasureList(gDataHub, *gpLikelihoodCalculator);
        return new CMinMeasureList(gDataHub, *gpLikelihoodCalculator);
    case LOW: 
        if (gParameters.getRiskLimitLowClusters())
            return new RiskMaxMeasureList(gDataHub, *gpLikelihoodCalculator, gParameters.getRiskThresholdLowClusters());
        if (gParameters.GetProbabilityModelType() == RANK)
            return new CMinMeasureList(gDataHub, *gpLikelihoodCalculator);
        return new CMaxMeasureList(gDataHub, *gpLikelihoodCalculator);
    case HIGHANDLOW:
        if (gParameters.getRiskLimitHighClusters() && gParameters.getRiskLimitLowClusters())
            return new RiskMinMaxMeasureList(gDataHub, *gpLikelihoodCalculator, gParameters.getRiskThresholdLowClusters(), gParameters.getRiskThresholdHighClusters());
        else if (gParameters.getRiskLimitLowClusters())
            /* We're restricting the low clusters only -- pass 1.0 for high risk restriction, which will always pass for high rates. */
            return new RiskMinMaxMeasureList(gDataHub, *gpLikelihoodCalculator, gParameters.getRiskThresholdLowClusters(), 1.0);
        else if (gParameters.getRiskLimitHighClusters())
            /* We're restricting the high clusters only -- pass 1.0 for low risk restriction, which will always pass for low rates. */
            return new RiskMinMaxMeasureList(gDataHub, *gpLikelihoodCalculator, 1.0, gParameters.getRiskThresholdHighClusters());
        return new CMinMaxMeasureList(gDataHub, *gpLikelihoodCalculator);
    default : throw prg_error("Unknown incidence rate specifier '%d'.","GetNewMeasureListObject()", gParameters.GetExecuteScanRateType());
    }
}

/** Returns newly allocated CTimeIntervals derived object based upon parameter
    settings - caller is responsible for deletion. */
CTimeIntervals * AbstractAnalysis::GetNewTemporalDataEvaluatorObject(IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType) const {
    switch (gParameters.GetProbabilityModelType()) {
        case NORMAL:
            if (gParameters.GetAnalysisType() == SEASONALTEMPORAL) {
                if (gDataHub.GetNumDataSets() == 1)
                    return new ClosedLoopNormalTemporalDataEvaluator(gDataHub, *gpLikelihoodCalculator, eIncludeClustersType, eExecutionType);
                return new ClosedLoopMultiSetNormalTemporalDataEvaluator(gDataHub, *gpLikelihoodCalculator, eIncludeClustersType);
            }
            if (gDataHub.GetNumDataSets() == 1)
                return new NormalTemporalDataEvaluator(gDataHub, *gpLikelihoodCalculator, eIncludeClustersType, eExecutionType);
            return new MultiSetNormalTemporalDataEvaluator(gDataHub, *gpLikelihoodCalculator, eIncludeClustersType);
        case ORDINAL:
        case CATEGORICAL :
            if (gParameters.GetAnalysisType() == SEASONALTEMPORAL) {
                if (gDataHub.GetNumDataSets() == 1)
                    return new ClosedLoopCategoricalTemporalDataEvaluator(gDataHub, *gpLikelihoodCalculator, eIncludeClustersType, eExecutionType);
                return new ClosedLoopMultiSetCategoricalTemporalDataEvaluator(gDataHub, *gpLikelihoodCalculator, eIncludeClustersType);
            }
            if (gDataHub.GetNumDataSets() == 1)
                return new CategoricalTemporalDataEvaluator(gDataHub, *gpLikelihoodCalculator, eIncludeClustersType, eExecutionType);
            return new MultiSetCategoricalTemporalDataEvaluator(gDataHub, *gpLikelihoodCalculator, eIncludeClustersType);
        case UNIFORMTIME:
            if (gParameters.GetAnalysisType() == SEASONALTEMPORAL) {
                throw prg_error("Uniform time model not implemented for seasonal scan.", "GetNewTemporalDataEvaluatorObject()");
            }
            if (gDataHub.GetNumDataSets() == 1)
                return new UniformTimeTemporalDataEvaluator(gDataHub, *gpLikelihoodCalculator, eIncludeClustersType, eExecutionType);
            return new MultiSetUniformTimeTemporalDataEvaluator(gDataHub, *gpLikelihoodCalculator, eIncludeClustersType);
        case BERNOULLI:
            if (gParameters.GetSpatialAdjustmentType() == SPATIAL_STRATIFIED_RANDOMIZATION)
                return new BernoulliSpatialStratifiedTemporalDataEvaluator(gDataHub, *gpLikelihoodCalculator, eIncludeClustersType, eExecutionType);
        default :
            if (gParameters.GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION) {
                if (gDataHub.GetNumDataSets() == 1)
                    return new TimeStratifiedTemporalDataEvaluator(gDataHub, *gpLikelihoodCalculator, eIncludeClustersType, eExecutionType);
                return new MultiSetTimeStratifiedTemporalDataEvaluator(gDataHub, *gpLikelihoodCalculator, eIncludeClustersType);
            }

            if (gParameters.GetAnalysisType() == SEASONALTEMPORAL) {
                if (gDataHub.GetNumDataSets() == 1)
                    return new ClosedLoopTemporalDataEvaluator(gDataHub, *gpLikelihoodCalculator, eIncludeClustersType, eExecutionType);
                return new ClosedLoopMultiSetTemporalDataEvaluator(gDataHub, *gpLikelihoodCalculator, eIncludeClustersType);
            }
            if (gDataHub.GetNumDataSets() == 1)
                return new TemporalDataEvaluator(gDataHub, *gpLikelihoodCalculator, eIncludeClustersType, eExecutionType);
            return new MultiSetTemporalDataEvaluator(gDataHub, *gpLikelihoodCalculator, eIncludeClustersType);
    }
}

/** internal setup function - allocates cluster data factory object and loglikelihood
    calculator object */
void AbstractAnalysis::Setup() {
  try {
    //create cluster data factory
    if (gParameters.GetProbabilityModelType() == NORMAL) {
      geReplicationsProcessType = ClusterEvaluation;
      if (gDataHub.GetNumDataSets() == 1)
        gpClusterDataFactory = new NormalClusterDataFactory(gDataHub);
      else
        gpClusterDataFactory = new MultiSetNormalClusterDataFactory(gDataHub);
    } else if (gParameters.GetProbabilityModelType() == ORDINAL || gParameters.GetProbabilityModelType() == CATEGORICAL) {
      geReplicationsProcessType = ClusterEvaluation;
      if (gDataHub.GetNumDataSets() == 1)
        gpClusterDataFactory = new CategoricalClusterDataFactory();
      else
        gpClusterDataFactory = new MultiSetsCategoricalClusterDataFactory(gParameters);
    } else if (gParameters.GetProbabilityModelType() == UNIFORMTIME) {
        geReplicationsProcessType = MeasureListEvaluation;
        if (gDataHub.GetNumDataSets() == 1)
            gpClusterDataFactory = new UniformTimeClusterDataFactory();
        else {
            geReplicationsProcessType = ClusterEvaluation;
            gpClusterDataFactory = new MultiSetUniformTimeClusterDataFactory();
        }
    } else if (gDataHub.GetNumDataSets() > 1) {
      gpClusterDataFactory = new MultiSetClusterDataFactory(gParameters);
      geReplicationsProcessType = ClusterEvaluation;
    } else {
      gpClusterDataFactory = new ClusterDataFactory();
      if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND || (gParameters.GetAnalysisType() == PURELYSPATIAL && gParameters.GetRiskType() == MONOTONERISK))
        geReplicationsProcessType = ClusterEvaluation;
      else if (gParameters.GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION)
        geReplicationsProcessType = ClusterEvaluation;
      else
        geReplicationsProcessType = MeasureListEvaluation;
    }
    //create likelihood calculator
    gpLikelihoodCalculator = GetNewLikelihoodCalculator(gDataHub);
  }
  catch (prg_exception& x) {
    delete gpClusterDataFactory;
    delete gpLikelihoodCalculator;
    x.addTrace("Setup()","AbstractAnalysis");
    throw;
  }
}

