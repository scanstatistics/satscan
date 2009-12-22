//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "AbstractAnalysis.h"
#include "TimeIntervalRange.h"
#include "ClusterDataFactory.h"
#include "NormalClusterDataFactory.h"
#include "CategoricalClusterDataFactory.h"
#include "LikelihoodCalculation.h"
#include "PoissonLikelihoodCalculation.h"
#include "PoissonSVTTLikelihoodCalculation.h"
#include "BernoulliLikelihoodCalculation.h"
#include "WilcoxonLikelihoodCalculation.h"
#include "NormalLikelihoodCalculation.h"
#include "OrdinalLikelihoodCalculation.h"
#include "WeightedNormalLikelihoodCalculation.h"
#include "WeightedNormalCovariatesLikelihoodCalculation.h"
#include "MeasureList.h"
#include "SSException.h"

/** constructor */
AbstractAnalysis::AbstractAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
                 :gParameters(Parameters), gDataHub(DataHub), gPrintDirection(PrintDirection),
                  gpClusterDataFactory(0), geReplicationsProcessType(MeasureListEvaluation), gpLikelihoodCalculator(0) {
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
    case POISSON              : if (DataHub.GetParameters().GetAnalysisType() == SPATIALVARTEMPTREND)
                                  return new PoissonSVTTLikelihoodCalculator(DataHub);
    case HOMOGENEOUSPOISSON   :
    case SPACETIMEPERMUTATION :
    case EXPONENTIAL          : return new PoissonLikelihoodCalculator(DataHub);
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
    case HIGH       : return new CMinMeasureList(gDataHub, *gpLikelihoodCalculator);
    case LOW        : return new CMaxMeasureList(gDataHub, *gpLikelihoodCalculator);
    case HIGHANDLOW : return new CMinMaxMeasureList(gDataHub, *gpLikelihoodCalculator);
    default         : throw prg_error("Unknown incidence rate specifier \"%d\".","GetNewMeasureListObject()",
                                      gParameters.GetExecuteScanRateType());
  }
}

/** Returns newly allocated CTimeIntervals derived object based upon parameter
    settings - caller is responsible for deletion. */
CTimeIntervals * AbstractAnalysis::GetNewTemporalDataEvaluatorObject(IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType) const {
  if (gParameters.GetProbabilityModelType() == NORMAL) {
    if (gParameters.GetNumDataSets() == 1)
      return new NormalTemporalDataEvaluator(gDataHub, *gpLikelihoodCalculator, eIncludeClustersType, eExecutionType);
    return new MultiSetNormalTemporalDataEvaluator(gDataHub, *gpLikelihoodCalculator, eIncludeClustersType);
  }
  else if (gParameters.GetProbabilityModelType() == ORDINAL || gParameters.GetProbabilityModelType() == CATEGORICAL) {
    if (gParameters.GetNumDataSets() == 1)
      return new CategoricalTemporalDataEvaluator(gDataHub, *gpLikelihoodCalculator, eIncludeClustersType, eExecutionType);
    return new MultiSetCategoricalTemporalDataEvaluator(gDataHub, *gpLikelihoodCalculator, eIncludeClustersType);
  }
  else if (gParameters.GetNumDataSets() > 1)
    return new MultiSetTemporalDataEvaluator(gDataHub, *gpLikelihoodCalculator, eIncludeClustersType);
  else
    return new TemporalDataEvaluator(gDataHub, *gpLikelihoodCalculator, eIncludeClustersType, eExecutionType);
}

/** internal setup function - allocates cluster data factory object and loglikelihood
    calculator object */
void AbstractAnalysis::Setup() {
  try {
    //create cluster data factory
    if (gParameters.GetProbabilityModelType() == NORMAL) {
      geReplicationsProcessType = ClusterEvaluation;
      if (gParameters.GetNumDataSets() == 1)
        gpClusterDataFactory = new NormalClusterDataFactory(gDataHub);
      else
        gpClusterDataFactory = new MultiSetNormalClusterDataFactory(gDataHub);
    }
    else if (gParameters.GetProbabilityModelType() == ORDINAL || gParameters.GetProbabilityModelType() == CATEGORICAL) {
      geReplicationsProcessType = ClusterEvaluation;
      if (gParameters.GetNumDataSets() == 1)
        gpClusterDataFactory = new CategoricalClusterDataFactory();
      else
        gpClusterDataFactory = new MultiSetsCategoricalClusterDataFactory(gParameters);
    }
    else if (gParameters.GetNumDataSets() > 1) {
      gpClusterDataFactory = new MultiSetClusterDataFactory(gParameters);
      geReplicationsProcessType = ClusterEvaluation;
    }
    else {
      gpClusterDataFactory = new ClusterDataFactory();
      if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND ||
          (gParameters.GetAnalysisType() == PURELYSPATIAL && gParameters.GetRiskType() == MONOTONERISK))
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

