#include "SaTScan.h"
#pragma hdrstop
#include "PurelySpatialAnalysis.h"

/** Constructor */
CPurelySpatialAnalysis::CPurelySpatialAnalysis(CParameters*  pParameters, CSaTScanData* pData, BasePrint *pPrintDirection)
                       :CAnalysis(pParameters, pData, pPrintDirection) {
  try {
    Init();
    Setup();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CPurelySpatialAnalysis");
    throw;
  }
}

/** Desctructor */
CPurelySpatialAnalysis::~CPurelySpatialAnalysis(){
  try {
    delete gpTopShapeClusters;
  }
  catch(...){}
}

/** Returns cluster centered at grid point nCenter, with the greatest loglikelihood.
    Caller is responsible for deleting returned cluster. */
CCluster* CPurelySpatialAnalysis::GetTopCluster(tract_t nCenter) {
  int                           i, j;
  CPurelySpatialCluster       * pTopCluster=0;
  count_t                    ** ppCases(m_pData->GetCasesArray());
  tract_t                       iNumNeighbors;
  CModel                      & ProbModel(m_pData->GetProbabilityModel());

  try {
    pTopCluster = new CPurelySpatialCluster(gpPrintDirection);
    pTopCluster->SetLogLikelihood(ProbModel.GetLogLikelihoodForTotal());
    gpTopShapeClusters->SetTopClusters(*pTopCluster);

    for (j=0; j <= m_pParameters->GetNumTotalEllipses(); j++) {   //circle is 0 offset... (always there)
       CPurelySpatialCluster thisCluster(gpPrintDirection);
       thisCluster.SetCenter(nCenter);
       thisCluster.SetRate(m_pParameters->GetAreaScanRateType());
       thisCluster.SetEllipseOffset(j);                       // store the ellipse link in the cluster obj
       thisCluster.SetDuczmalCorrection((j == 0 || !m_pParameters->GetDuczmalCorrectEllipses() ? 1 : m_pData->GetShapesArray()[j - 1]));
       CPurelySpatialCluster & TopShapeCluster = (CPurelySpatialCluster&)(gpTopShapeClusters->GetTopCluster(j));       
       iNumNeighbors = m_pData->GetNeighborCountArray()[j][nCenter];
       for (i=1; i <= iNumNeighbors; i++) {
          thisCluster.AddNeighbor(j, *m_pData, ppCases, i);
          if (thisCluster.RateIsOfInterest(m_pData->GetNumCases(), m_pData->GetTotalMeasure())) {
            thisCluster.m_nLogLikelihood = ProbModel.CalcLogLikelihood(thisCluster.m_nCases, thisCluster.m_nMeasure);
            if (thisCluster.m_nLogLikelihood > TopShapeCluster.m_nLogLikelihood)
              TopShapeCluster = thisCluster;
          }
       }
    }
    //get copy of best cluster over all iterations
    *pTopCluster = (CPurelySpatialCluster&)(gpTopShapeClusters->GetTopCluster());
  }
  catch (ZdException & x) {
    x.AddCallpath("GetTopCluster()", "CPurelySpatialAnalysis");
    delete pTopCluster;
    throw;
  }
  return pTopCluster;
}

/** Returns loglikelihood for Monte Carlo replication. */
double CPurelySpatialAnalysis::MonteCarlo() {
  CMeasureList                  * pMeasureList=0;
  CPurelySpatialCluster           C(gpPrintDirection);
  double                          dMaxLogLikelihoodRatio;
  tract_t                         i, j, iNumNeighbors;
  int                             k;
  count_t                      ** ppSimCases(m_pData->GetSimCasesArray());
  tract_t                      ** ppNeighbors(m_pData->GetNeighborCountArray());

  try {
    C.SetRate(m_pParameters->GetAreaScanRateType());
    switch (m_pParameters->GetAreaScanRateType()) {
     case HIGH       : pMeasureList = new CMinMeasureList(*m_pData, *gpPrintDirection);
                       break;
     case LOW        : pMeasureList = new CMaxMeasureList(*m_pData, *gpPrintDirection);
                       break;
     case HIGHANDLOW : pMeasureList = new CMinMaxMeasureList(*m_pData, *gpPrintDirection);
                       break;
     default         : ZdGenerateException("Unknown incidence rate specifier \"%d\".","MonteCarlo()",
                                           m_pParameters->GetAreaScanRateType());
    }

    for (k=0; k <= m_pParameters->GetNumTotalEllipses(); k++) { //circle is 0 offset... (always there)
       for (i=0; i < m_pData->m_nGridTracts; i++) {
          C.Initialize(i);
          iNumNeighbors = ppNeighbors[k][i];
          for (j=1; j <= iNumNeighbors; j++) {
             C.AddNeighbor(k, *m_pData, ppSimCases, j);
             pMeasureList->AddMeasure(C.m_nCases, C.m_nMeasure);
          }
       }
       pMeasureList->SetForNextIteration(k);
    }
    dMaxLogLikelihoodRatio = pMeasureList->GetMaximumLogLikelihoodRatio();
    delete pMeasureList;
  }
  catch (ZdException & x) {
    delete pMeasureList;
    x.AddCallpath("MonteCarlo()", "CPurelySpatialAnalysis");
    throw;
  }
  return dMaxLogLikelihoodRatio;
}

/** Prospective monte carlo not valid for purely spatial analysis. */
double CPurelySpatialAnalysis::MonteCarloProspective() {
  ZdGenerateException("MonteCarloProspective() not implemented for CPurelySpatialAnalysis.","MonteCarloProspective()");
  return 0;
}

void CPurelySpatialAnalysis::Setup() {
  try {
    gpTopShapeClusters = new TopClustersContainer(*m_pData);
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()", "CPurelySpatialAnalysis");
    delete gpTopShapeClusters;
    throw;
  }
}

