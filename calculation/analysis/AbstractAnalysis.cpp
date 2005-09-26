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
#include "BernoulliLikelihoodCalculation.h"
#include "WilcoxonLikelihoodCalculation.h"
#include "NormalLikelihoodCalculation.h"
#include "OrdinalLikelihoodCalculation.h"
#include "MeasureList.h"

/** constructor */
AbstractAnalysis::AbstractAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
                 :gParameters(Parameters), gDataHub(DataHub), gPrintDirection(PrintDirection),
                  gpClusterDataFactory(0), geReplicationsProcessType(MeasureListEvaluation), gpLikelihoodCalculator(0) {
  try {
    Setup();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","AbstractAnalysis");
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
    - throws ZdException if model type is not known */
AbstractLikelihoodCalculator * AbstractAnalysis::GetNewLikelihoodCalculator(const CSaTScanData& DataHub) {
  //create likelihood calculator
  switch (DataHub.GetParameters().GetProbabilityModelType()) {
    case POISSON              :
    case SPACETIMEPERMUTATION :
    case EXPONENTIAL          : return new PoissonLikelihoodCalculator(DataHub);
    case BERNOULLI            : return new BernoulliLikelihoodCalculator(DataHub);
    case NORMAL               : return new NormalLikelihoodCalculator(DataHub);
    case ORDINAL              : return new OrdinalLikelihoodCalculator(DataHub);
    case RANK                 : return new WilcoxonLikelihoodCalculator(DataHub);
    default                   :
     ZdGenerateException("Unknown probability model '%d'.", "GetNewLikelihoodCalculator()",
                         DataHub.GetParameters().GetProbabilityModelType());
  };
  return 0;
}

/** Returns newly allocated CMeasureList object - caller is responsible for deletion.
    - throws ZdException if type is not known */
CMeasureList * AbstractAnalysis::GetNewMeasureListObject() const {
  switch (gParameters.GetAreaScanRateType()) {
    case HIGH       : return new CMinMeasureList(gDataHub, *gpLikelihoodCalculator);
    case LOW        : return new CMaxMeasureList(gDataHub, *gpLikelihoodCalculator);
    case HIGHANDLOW : return new CMinMaxMeasureList(gDataHub, *gpLikelihoodCalculator);
    default         : ZdGenerateException("Unknown incidence rate specifier \"%d\".","GetNewMeasureListObject()",
                                          gParameters.GetAreaScanRateType());
  }
  return 0;
}

/** Returns newly allocated CTimeIntervals derived object based upon parameter
    settings - caller is responsible for deletion. */
CTimeIntervals * AbstractAnalysis::GetNewTemporalDataEvaluatorObject(IncludeClustersType eType) const {
  if (gParameters.GetProbabilityModelType() == NORMAL)
    return new NormalTemporalDataEvaluator(gDataHub, *gpLikelihoodCalculator, eType);
  else if (gParameters.GetProbabilityModelType() == ORDINAL) {
    if (gParameters.GetNumDataSets() == 1)
      return new CategoricalTemporalDataEvaluator(gDataHub, *gpLikelihoodCalculator, eType);
    else
      return new MultiSetCategoricalTemporalDataEvaluator(gDataHub, *gpLikelihoodCalculator, eType);
  }
  else if (gParameters.GetNumDataSets() > 1)
    return new MultiSetTemporalDataEvaluator(gDataHub, *gpLikelihoodCalculator, eType);
  else
    return new TemporalDataEvaluator(gDataHub, *gpLikelihoodCalculator, eType);
}

/** internal setup function - allocates cluster data factory object and loglikelihood
    calculator object */
void AbstractAnalysis::Setup() {
  try {
    //create cluster data factory
    if (gParameters.GetProbabilityModelType() == NORMAL) {
      gpClusterDataFactory = new NormalClusterDataFactory();
      geReplicationsProcessType = ClusterEvaluation;
    }
    else if (gParameters.GetProbabilityModelType() == ORDINAL) {
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
      geReplicationsProcessType = MeasureListEvaluation;
    }
    //create likelihood calculator
    gpLikelihoodCalculator = GetNewLikelihoodCalculator(gDataHub);
  }
  catch (ZdException &x) {
    delete gpClusterDataFactory;
    delete gpLikelihoodCalculator;
    x.AddCallpath("Setup()","AbstractAnalysis");
    throw;
  }
}

