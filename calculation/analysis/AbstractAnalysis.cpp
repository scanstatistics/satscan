//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "AbstractAnalysis.h"
#include "TimeIntervalRange.h"
#include "ClusterDataFactory.h"
#include "NormalClusterDataFactory.h"
#include "BatchedClusterDataFactory.h"
#include "CategoricalClusterDataFactory.h"
#include "UniformTimeClusterDataFactory.h"
#include "LikelihoodCalculation.h"
#include "PoissonLikelihoodCalculation.h"
#include "PoissonSVTTLikelihoodCalculation.h"
#include "BernoulliLikelihoodCalculation.h"
#include "WilcoxonLikelihoodCalculation.h"
#include "NormalLikelihoodCalculation.h"
#include "BatchedLikelihoodCalculation.h"
#include "OrdinalLikelihoodCalculation.h"
#include "WeightedNormalLikelihoodCalculation.h"
#include "WeightedNormalCovariatesLikelihoodCalculation.h"
#include "UniformTimeLikelihoodCalculation.h"
#include "MeasureList.h"
#include "SSException.h"

/** constructor */
AbstractAnalysis::AbstractAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
:_parameters(Parameters), _data_hub(DataHub), _print(PrintDirection), _likelihood_calculator(GetNewLikelihoodCalculator(_data_hub)), _replica_process_type(MeasureListEvaluation) {
    try {
        //createSetup cluster data factory
        if (_parameters.GetProbabilityModelType() == BATCHED) {
            _replica_process_type = ClusterEvaluation;
            if (_data_hub.GetNumDataSets() == 1)
                _cluster_data_factory.reset(new BatchedClusterDataFactory(_data_hub));
            else
                _cluster_data_factory.reset(new MultiSetBatchedClusterDataFactory(_data_hub));
        } else if (_parameters.GetProbabilityModelType() == NORMAL) {
            _replica_process_type = ClusterEvaluation;
            if (_data_hub.GetNumDataSets() == 1)
                _cluster_data_factory.reset(new BatchedClusterDataFactory(_data_hub));
            else
                _cluster_data_factory.reset(new MultiSetNormalClusterDataFactory(_data_hub));
        } else if (_parameters.GetProbabilityModelType() == ORDINAL || _parameters.GetProbabilityModelType() == CATEGORICAL) {
            _replica_process_type = ClusterEvaluation;
            if (_data_hub.GetNumDataSets() == 1)
                _cluster_data_factory.reset(new CategoricalClusterDataFactory());
            else
                _cluster_data_factory.reset(new MultiSetsCategoricalClusterDataFactory(_parameters));
        } else if (_parameters.GetProbabilityModelType() == UNIFORMTIME) {
            _replica_process_type = MeasureListEvaluation;
            if (_data_hub.GetNumDataSets() == 1)
                _cluster_data_factory.reset(new UniformTimeClusterDataFactory());
            else {
                _replica_process_type = ClusterEvaluation;
                _cluster_data_factory.reset(new MultiSetUniformTimeClusterDataFactory());
            }
        } else if (_data_hub.GetNumDataSets() > 1) {
            _cluster_data_factory.reset(new MultiSetClusterDataFactory(_parameters));
            _replica_process_type = ClusterEvaluation;
        } else {
            _cluster_data_factory.reset(new ClusterDataFactory());
            if (_parameters.GetAnalysisType() == SPATIALVARTEMPTREND || (_parameters.GetAnalysisType() == PURELYSPATIAL && _parameters.GetRiskType() == MONOTONERISK))
                _replica_process_type = ClusterEvaluation;
            else if (_parameters.GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION)
                _replica_process_type = ClusterEvaluation;
            else
                _replica_process_type = MeasureListEvaluation;
        }
    } catch (prg_exception& x) {
        x.addTrace("constructor()","AbstractAnalysis");
        throw;
    }
}

/** Returns newly allocated log likelihood ratio calculator based upon requested
    probability model. Caller is responsible for object deletion.
    - throws prg_error if model type is not known */
AbstractLikelihoodCalculator * AbstractAnalysis::GetNewLikelihoodCalculator(const CSaTScanData& DataHub) {
    //create likelihood calculator
    switch (DataHub.GetParameters().GetProbabilityModelType()) {
	    case POISSON            : if (DataHub.GetParameters().GetAnalysisType() == SPATIALVARTEMPTREND) {
		                            if (DataHub.GetParameters().getTimeTrendType() == QUADRATIC)
                                        return new PoissonQuadraticTrendLikelihoodCalculator(DataHub);
								    else
                                        return new PoissonLinearTrendLikelihoodCalculator(DataHub);
								}
        case HOMOGENEOUSPOISSON   :
        case SPACETIMEPERMUTATION :
        case EXPONENTIAL          : return new PoissonLikelihoodCalculator(DataHub);
        case BATCHED              : return new BatchedLikelihoodCalculator(DataHub);
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
            throw prg_error("Unknown probability model '%d'.", "GetNewLikelihoodCalculator()", DataHub.GetParameters().GetProbabilityModelType());
    };
}

/** Returns newly allocated CMeasureList object - caller is responsible for deletion.
    - throws prg_error if type is not known */
CMeasureList * AbstractAnalysis::GetNewMeasureListObject() const {
    switch (_parameters.GetExecuteScanRateType()) {
        case HIGH:
            if (_parameters.getRiskLimitHighClusters()) 
                return new RiskMinMeasureList(_data_hub, *_likelihood_calculator, _parameters.getRiskThresholdHighClusters());
            if (_parameters.GetProbabilityModelType() == RANK)
                return new CMaxMeasureList(_data_hub, *_likelihood_calculator);
            return new CMinMeasureList(_data_hub, *_likelihood_calculator);
        case LOW: 
            if (_parameters.getRiskLimitLowClusters())
                return new RiskMaxMeasureList(_data_hub, *_likelihood_calculator, _parameters.getRiskThresholdLowClusters());
            if (_parameters.GetProbabilityModelType() == RANK)
                return new CMinMeasureList(_data_hub, *_likelihood_calculator);
            return new CMaxMeasureList(_data_hub, *_likelihood_calculator);
        case HIGHANDLOW:
            if (_parameters.getRiskLimitHighClusters() && _parameters.getRiskLimitLowClusters())
                return new RiskMinMaxMeasureList(_data_hub, *_likelihood_calculator, _parameters.getRiskThresholdLowClusters(), _parameters.getRiskThresholdHighClusters());
            else if (_parameters.getRiskLimitLowClusters())
                /* We're restricting the low clusters only -- pass 1.0 for high risk restriction, which will always pass for high rates. */
                return new RiskMinMaxMeasureList(_data_hub, *_likelihood_calculator, _parameters.getRiskThresholdLowClusters(), 1.0);
            else if (_parameters.getRiskLimitHighClusters())
                /* We're restricting the high clusters only -- pass 1.0 for low risk restriction, which will always pass for low rates. */
                return new RiskMinMaxMeasureList(_data_hub, *_likelihood_calculator, 1.0, _parameters.getRiskThresholdHighClusters());
            return new CMinMaxMeasureList(_data_hub, *_likelihood_calculator);
        default : throw prg_error("Unknown incidence rate specifier '%d'.","GetNewMeasureListObject()", _parameters.GetExecuteScanRateType());
    }
}

/** Returns newly allocated CTimeIntervals derived object based upon parameter
    settings - caller is responsible for deletion. */
CTimeIntervals * AbstractAnalysis::GetNewTemporalDataEvaluatorObject(IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType) const {
    switch (_parameters.GetProbabilityModelType()) {
        case BATCHED:
            if (_parameters.GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION) {
                if (_data_hub.GetNumDataSets() == 1) {
                    if (_parameters.isTimeStratifiedWithLargerAdjustmentLength())
                        return new TimeStratifiedBatchedTemporalDataEvaluatorEnhanced(_data_hub, *_likelihood_calculator, eIncludeClustersType, eExecutionType);
                    return new TimeStratifiedBatchedTemporalDataEvaluator(_data_hub, *_likelihood_calculator, eIncludeClustersType, eExecutionType);
                }
                if (_parameters.isTimeStratifiedWithLargerAdjustmentLength())
                    return new MultiSetTimeStratifiedBatchedTemporalDataEvaluatorEnhanced(_data_hub, *_likelihood_calculator, eIncludeClustersType, eExecutionType);
                return new MultiSetTimeStratifiedBatchedTemporalDataEvaluator(_data_hub, *_likelihood_calculator, eIncludeClustersType, eExecutionType);
            }
            if (_parameters.GetAnalysisType() == SEASONALTEMPORAL) {
                if (_data_hub.GetNumDataSets() == 1)
                    return new ClosedLoopBatchedTemporalDataEvaluator(_data_hub, *_likelihood_calculator, eIncludeClustersType, eExecutionType);
                return new ClosedLoopMultiSetBatchedTemporalDataEvaluator(_data_hub, *_likelihood_calculator, eIncludeClustersType);
            }
            if (_data_hub.GetNumDataSets() == 1)
                return new BatchedTemporalDataEvaluator(_data_hub, *_likelihood_calculator, eIncludeClustersType, eExecutionType);
            return new MultiSetBatchedTemporalDataEvaluator(_data_hub, *_likelihood_calculator, eIncludeClustersType);
        case NORMAL:
            if (_parameters.GetAnalysisType() == SEASONALTEMPORAL) {
                if (_data_hub.GetNumDataSets() == 1)
                    return new ClosedLoopNormalTemporalDataEvaluator(_data_hub, *_likelihood_calculator, eIncludeClustersType, eExecutionType);
                return new ClosedLoopMultiSetNormalTemporalDataEvaluator(_data_hub, *_likelihood_calculator, eIncludeClustersType);
            }
            if (_data_hub.GetNumDataSets() == 1)
                return new NormalTemporalDataEvaluator(_data_hub, *_likelihood_calculator, eIncludeClustersType, eExecutionType);
            return new MultiSetNormalTemporalDataEvaluator(_data_hub, *_likelihood_calculator, eIncludeClustersType);
        case ORDINAL:
        case CATEGORICAL :
            if (_parameters.GetAnalysisType() == SEASONALTEMPORAL) {
                if (_data_hub.GetNumDataSets() == 1)
                    return new ClosedLoopCategoricalTemporalDataEvaluator(_data_hub, *_likelihood_calculator, eIncludeClustersType, eExecutionType);
                return new ClosedLoopMultiSetCategoricalTemporalDataEvaluator(_data_hub, *_likelihood_calculator, eIncludeClustersType);
            }
            if (_data_hub.GetNumDataSets() == 1)
                return new CategoricalTemporalDataEvaluator(_data_hub, *_likelihood_calculator, eIncludeClustersType, eExecutionType);
            return new MultiSetCategoricalTemporalDataEvaluator(_data_hub, *_likelihood_calculator, eIncludeClustersType);
        case UNIFORMTIME:
            if (_parameters.GetAnalysisType() == SEASONALTEMPORAL) {
                throw prg_error("Uniform time model not implemented for seasonal scan.", "GetNewTemporalDataEvaluatorObject()");
            }
            if (_data_hub.GetNumDataSets() == 1)
                return new UniformTimeTemporalDataEvaluator(_data_hub, *_likelihood_calculator, eIncludeClustersType, eExecutionType);
            return new MultiSetUniformTimeTemporalDataEvaluator(_data_hub, *_likelihood_calculator, eIncludeClustersType);
        case BERNOULLI:
            if (_parameters.GetSpatialAdjustmentType() == SPATIAL_STRATIFIED_RANDOMIZATION)
                return new BernoulliSpatialStratifiedTemporalDataEvaluator(_data_hub, *_likelihood_calculator, eIncludeClustersType, eExecutionType);
        default :
            if (_parameters.GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION) {
                if (_data_hub.GetNumDataSets() == 1)
                    return new TimeStratifiedTemporalDataEvaluator(_data_hub, *_likelihood_calculator, eIncludeClustersType, eExecutionType);
                return new MultiSetTimeStratifiedTemporalDataEvaluator(_data_hub, *_likelihood_calculator, eIncludeClustersType);
            }

            if (_parameters.GetAnalysisType() == SEASONALTEMPORAL) {
                if (_data_hub.GetNumDataSets() == 1)
                    return new ClosedLoopTemporalDataEvaluator(_data_hub, *_likelihood_calculator, eIncludeClustersType, eExecutionType);
                return new ClosedLoopMultiSetTemporalDataEvaluator(_data_hub, *_likelihood_calculator, eIncludeClustersType);
            }
            if (_data_hub.GetNumDataSets() == 1)
                return new TemporalDataEvaluator(_data_hub, *_likelihood_calculator, eIncludeClustersType, eExecutionType);
            return new MultiSetTemporalDataEvaluator(_data_hub, *_likelihood_calculator, eIncludeClustersType);
    }
}
