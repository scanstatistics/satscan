//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "Analysis.h"
#include "PoissonLikelihoodCalculation.h"
#include "BernoulliLikelihoodCalculation.h"
#include "WilcoxonLikelihoodCalculation.h"
#include "NormalLikelihoodCalculation.h"
#include "OrdinalLikelihoodCalculation.h"
#include "MeasureList.h"

/** constructor */
TopClustersContainer::TopClustersContainer(const CSaTScanData & Data)
                     :gData(Data){}

/** constructor */
TopClustersContainer::~TopClustersContainer(){}

/** Returns reference to top cluster for shape offset. This function is tightly
    coupled to the presumption that CAnalysis::GetTopCluster() iterates over
    circles and ellispes in such a way that an offset of zero is a circle, and
    anything greater offset corresponds to an particular rotation of an ellipse shape.
    NOTE: Caller should never assume that reference will remain after return from
          get CAnalysis::GetTopCluster(), as the vector is reset for each iteration
          of GetTopClusters(). Instead, a copy should be made, using operator= method
          and appropriate casting, to store cluster information. */
CCluster & TopClustersContainer::GetTopCluster(int iShapeOffset) {
  CCluster    * pTopCluster=0;
  int           iEllipse, iBoundry=0;

  if (iShapeOffset && gData.GetParameters().GetNumRequestedEllipses() && gData.GetParameters().GetNonCompactnessPenalty())
    for (iEllipse=0; iEllipse < gData.GetParameters().GetNumRequestedEllipses() && !pTopCluster; ++iEllipse) {
       //Get the number of angles this ellipse shape rotates through.
       iBoundry += gData.GetParameters().GetEllipseRotations()[iEllipse];
       if (iShapeOffset <= iBoundry)
         pTopCluster = gvTopShapeClusters[iEllipse + 1];
    }
  else
    pTopCluster = gvTopShapeClusters[0];

  return *pTopCluster;
}

/** Returns the top cluster for all shapes, taking into account the option of
    non-compactness penalizing for ellispes.
    NOTE: Caller should never assume that reference will remain after return from
          get CAnalysis::GetTopCluster(), as the vector is reset for each iteration
          of GetTopClusters(). Instead a copy should be made, using operator= method
          and appropriate casting, to store cluster information. 
    NOTE: This function should only be called after all iterations in
          CAnalysis::GetTopCluster() are completed. */
CCluster & TopClustersContainer::GetTopCluster() {
  CCluster    * pTopCluster;

  //set the maximum cluster to the circle shape initially
  pTopCluster = gvTopShapeClusters[0];
  //apply compactness correction
  pTopCluster->m_nRatio *= pTopCluster->GetNonCompactnessPenalty();
  //if the there are ellipses, compare current top cluster against them
  //note: we don't have to be concerned with whether we are comparing circles and ellipses,
  //     the adjusted loglikelihood ratio for a circle is just the loglikelihood ratio
  for (size_t t=1; t < gvTopShapeClusters.size(); t++) {
     if (gvTopShapeClusters[t]->ClusterDefined()) {
       //apply compactness correction
       gvTopShapeClusters[t]->m_nRatio *= gvTopShapeClusters[t]->GetNonCompactnessPenalty();
       //compare against current top cluster
       if (gvTopShapeClusters[t]->m_nRatio > pTopCluster->m_nRatio)
         pTopCluster = gvTopShapeClusters[t];
     }    
  }
  return *pTopCluster;
}

void TopClustersContainer::Reset(int iCenter) {
  for (size_t t=0; t < gvTopShapeClusters.size(); ++t)
     gvTopShapeClusters[t]->Initialize(iCenter);
}

/** Initialzies the vector of top clusters to cloned copies of cluster,
    taking into account whether spatial shape will be a factor in analysis. */
void TopClustersContainer::SetTopClusters(const CCluster& InitialCluster) {
  int   i, iNumTopClusters;

  try {
    gvTopShapeClusters.DeleteAllElements();
    //if there are ellipses and non-compactness penalty is true, then we need
    //a top cluster for the circle and each ellipse shape
    if (gData.GetParameters().GetNumRequestedEllipses() && gData.GetParameters().GetNonCompactnessPenalty())
      iNumTopClusters = gData.GetParameters().GetNumRequestedEllipses() + 1;
    else
    //else there is only one top cluster - regardless of whether there are ellipses
      iNumTopClusters = 1;

    for (i=0; i < iNumTopClusters; i++)
       gvTopShapeClusters.push_back(InitialCluster.Clone());
  }
  catch (ZdException &x) {
    x.AddCallpath("SetTopClusters()","TopClustersContainer");
    throw;
  }
}

/** constructor */
CAnalysis::CAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
          :gParameters(Parameters), gDataHub(DataHub), gPrintDirection(PrintDirection) {
  try {
    Init();
    Setup();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CAnalysis");
    throw;
  }
}

/** destructor */
CAnalysis::~CAnalysis() {
  try {
    delete gpClusterDataFactory;
    delete gpLikelihoodCalculator;
  }
  catch(...){}
}

/** allocates likelihood calculator object
    - this function should not be called prior to calculation of total cases
      and total measure                                                       */
void CAnalysis::AllocateLikelihoodObject() {
  try {
    //create likelihood calculator
    switch (gParameters.GetProbabilityModelType()) {
      case POISSON :
      case SPACETIMEPERMUTATION :
      case EXPONENTIAL :
        gpLikelihoodCalculator = new PoissonLikelihoodCalculator(gDataHub); break;
      case BERNOULLI :
       gpLikelihoodCalculator = new BernoulliLikelihoodCalculator(gDataHub); break;
      case NORMAL :
       gpLikelihoodCalculator = new NormalLikelihoodCalculator(gDataHub); break;
      case ORDINAL :
       gpLikelihoodCalculator = new OrdinalLikelihoodCalculator(gDataHub); break;
      case RANK :
       gpLikelihoodCalculator = new WilcoxonLikelihoodCalculator(gDataHub); break;
      default :
       ZdGenerateException("Unknown probability model '%d'.", "AllocateLikelihoodObject()", gParameters.GetProbabilityModelType());
    };
  }
  catch (ZdException &x) {
    delete gpLikelihoodCalculator;
    x.AddCallpath("AllocateLikelihoodObject()","CAnalysis");
    throw;
  }
}

/** Executes simulation. Calls MonteCarlo() for analyses that can utilize
    TMeasureList class or FindTopRatio() for analyses which must perform
    simulations by the same algorithm as the real data. */
double CAnalysis::ExecuteSimulation(const AbtractDataSetGateway& DataGateway) {
  if (gbMeasureListReplications)
    return MonteCarlo(DataGateway.GetDataSetInterface(0));
  else
    return FindTopRatio(DataGateway);
}

/** Given data gate way, calculates and collects most likely clusters about
    each grid point. Collection of clusters are sorted by loglikelihood ratio
    and condensed based upon overlapping geographical areas.                */
void CAnalysis::FindTopClusters(const AbtractDataSetGateway& DataGateway, MostLikelyClustersContainer& TopClustersContainer) {
  int                   i;
  clock_t               tStartTime;

  try {
    //start clock which will help determine how long this process will take
    tStartTime = clock();
    //calculate top cluster about each centroid(grid point) and store copy in top cluster array
    for (i=0; i < gDataHub.m_nGridTracts && !gPrintDirection.GetIsCanceled(); ++i) {
       TopClustersContainer.Add(CalculateTopCluster(i, DataGateway));
       if (i==9)
         ReportTimeEstimate(tStartTime, gDataHub.m_nGridTracts, i+1, &gPrintDirection);
    }
    if (!gPrintDirection.GetIsCanceled())
      TopClustersContainer.RankTopClusters(gParameters, gDataHub);
  }
  catch (ZdException &x) {
    x.AddCallpath("FindTopClusters()","CAnalysis");
    throw;
  }
}

/** calculates greatest loglikelihood ratio about each centroid(grid point) */
double CAnalysis::FindTopRatio(const AbtractDataSetGateway & DataGateway) {
  int                   i;
  double                dMaxLogLikelihoodRatio=0;

  //calculate greatest loglikelihood ratio about each centroid
  for (i=0; i < gDataHub.m_nGridTracts && !gPrintDirection.GetIsCanceled(); ++i)
    dMaxLogLikelihoodRatio = std::max(CalculateTopCluster(i, DataGateway).m_nRatio, dMaxLogLikelihoodRatio);
  return dMaxLogLikelihoodRatio;
}

CMeasureList * CAnalysis::GetNewMeasureListObject() const {
  switch (gParameters.GetAreaScanRateType()) {
    case HIGH       : return new CMinMeasureList(gDataHub, *gpLikelihoodCalculator);
    case LOW        : return new CMaxMeasureList(gDataHub, *gpLikelihoodCalculator);
    case HIGHANDLOW : return new CMinMaxMeasureList(gDataHub, *gpLikelihoodCalculator);
    default         : ZdGenerateException("Unknown incidence rate specifier \"%d\".","MonteCarlo()",
                                          gParameters.GetAreaScanRateType());
  }
  return 0;
}

/** Returns newly allocated CTimeIntervals derived object based upon parameter
    settings. Note that caller is responsible for deletion of object. */
CTimeIntervals * CAnalysis::GetNewTemporalDataEvaluatorObject(IncludeClustersType eType) const {
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

/** internal initialization */
void CAnalysis::Init() {
  gpClusterDataFactory=0;
  gbMeasureListReplications=true;
  gpLikelihoodCalculator=0;
}

/** internal setup function */
void CAnalysis::Setup() {
  try {
    //create cluster data factory
    if (gParameters.GetProbabilityModelType() == NORMAL) {
      gpClusterDataFactory = new NormalClusterDataFactory();
      gbMeasureListReplications = false;
    }
    else if (gParameters.GetProbabilityModelType() == ORDINAL) {
      gbMeasureListReplications = false;
      if (gParameters.GetNumDataSets() == 1)
        gpClusterDataFactory = new CategoricalClusterDataFactory();
      else
        gpClusterDataFactory = new MultiSetsCategoricalClusterDataFactory(gParameters);
    }
    else if (gParameters.GetNumDataSets() > 1) {
      gpClusterDataFactory = new MultiSetClusterDataFactory(gParameters);
      gbMeasureListReplications = false;
    }
    else {
      gpClusterDataFactory = new ClusterDataFactory();
      gbMeasureListReplications = true;
    }
    AllocateLikelihoodObject();
  }
  catch (ZdException &x) {
    delete gpClusterDataFactory;
    x.AddCallpath("Setup()","CAnalysis");
    throw;
  }
}

