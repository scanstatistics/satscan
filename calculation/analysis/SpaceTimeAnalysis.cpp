#include "SaTScan.h"
#pragma hdrstop
#include "SpaceTimeAnalysis.h"

/** Constructor */
CSpaceTimeAnalysis::CSpaceTimeAnalysis(CParameters*  pParameters, CSaTScanData* pData, BasePrint *pPrintDirection)
                   :CAnalysis(pParameters, pData, pPrintDirection) {}

/** Destructor */
CSpaceTimeAnalysis::~CSpaceTimeAnalysis() {}

/** Returns cluster centered at grid point nCenter, with the greatest loglikelihood.
    Caller is responsible for deleting returned cluster. */
CCluster* CSpaceTimeAnalysis::GetTopCluster(tract_t nCenter) {
  CSpaceTimeCluster   * MaxCluster = 0;
  bool                  bAliveCluster;
  int                   k;
  tract_t               i;

  try {
    // if Prospective Space-Time then Alive Clusters Only.
    if (m_pParameters->m_nAnalysisType == PROSPECTIVESPACETIME)
      bAliveCluster = true;
    else
      bAliveCluster = m_pParameters->m_bAliveClustersOnly;

    MaxCluster = new CSpaceTimeCluster(bAliveCluster, m_pData->m_nTimeIntervals, m_pData->m_nIntervalCut, gpPrintDirection);
    MaxCluster->CCluster::SetLogLikelihood(m_pData->m_pModel->GetLogLikelihoodForTotal());

    //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
    for (k=0; k <= m_pParameters->m_lTotalNumEllipses; k++) {
       CSpaceTimeCluster thisCluster(bAliveCluster, m_pData->m_nTimeIntervals, m_pData->m_nIntervalCut, gpPrintDirection);
       thisCluster.SetCenter(nCenter);
       thisCluster.SetRate(m_pParameters->m_nAreas);
       thisCluster.SetEllipseOffset(k);
       thisCluster.SetDuczmalCorrection((k == 0 || !m_pParameters->m_bDuczmalCorrectEllipses ? 1 : m_pData->mdE_Shapes[k - 1]));
       for (i=1; i <= m_pData->m_NeighborCounts[k][nCenter]; i++) {
          thisCluster.AddNeighbor(k, *m_pData, m_pData->m_pCases, i);
          thisCluster.InitTimeIntervalIndeces();
          while (thisCluster.SetNextTimeInterval()) {
               if (thisCluster.RateIsOfInterest(m_pData->m_nTotalCases, m_pData->m_nTotalMeasure)) {
                 thisCluster.m_nLogLikelihood = m_pData->m_pModel->CalcLogLikelihood(thisCluster.m_nCases, thisCluster.m_nMeasure);
                 if (thisCluster.m_nLogLikelihood * thisCluster.m_DuczmalCorrection > MaxCluster->m_nLogLikelihood)
                   *MaxCluster = thisCluster;
               }
          }
       }
    }
    MaxCluster->SetRatioAndDates(*m_pData);
  }
  catch (ZdException & x) {
    delete MaxCluster;
    x.AddCallpath("GetTopCluster()", "CSpaceTimeAnalysis");
    throw;
  }
  return MaxCluster;
}

/** Returns loglikelihood for Monte Carlo replication. */
double CSpaceTimeAnalysis::MonteCarlo() {
  CMeasureList  * pMeasureList=0;
  double          dMaxLogLikelihood;
  int             k;
  tract_t         i, j;

  try {
    CSpaceTimeCluster C(m_pParameters->m_bAliveClustersOnly, m_pData->m_nTimeIntervals,
                        m_pData->m_nIntervalCut, gpPrintDirection);
    C.SetRate(m_pParameters->m_nAreas);
    dMaxLogLikelihood = m_pData->m_pModel->GetLogLikelihoodForTotal();
    switch (m_pParameters->m_nAreas) {
     case HIGH      : pMeasureList = new CMinMeasureList(*m_pData, *gpPrintDirection);
                      break;
     case LOW       : pMeasureList = new CMaxMeasureList(*m_pData, *gpPrintDirection);
                      break;
     case HIGHANDLOW: pMeasureList = new CMinMaxMeasureList(*m_pData, *gpPrintDirection);
                      break;
     default        : ZdGenerateException("Unknown incidence rate specifier \"%d\".","MonteCarlo()",
                                          m_pParameters->m_nAreas);
    }
    
    //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
    for (k=0; k <= m_pParameters->m_lTotalNumEllipses; k++) {
       for (i=0; i < m_pData->m_nGridTracts; i++) {
          C.Initialize(i);
          for (j=1; j<=m_pData->m_NeighborCounts[k][i]; j++) {
             C.AddNeighbor(k, *m_pData, m_pData->m_pSimCases, j);
             C.InitTimeIntervalIndeces();
             while (C.SetNextTimeInterval())
               pMeasureList->AddMeasure(C.m_nCases, C.m_nMeasure);
          }
       }
       pMeasureList->SetForNextIteration(k, dMaxLogLikelihood);
    }
    delete pMeasureList;
  }
  catch (ZdException & x) {
    x.AddCallpath("MonteCarlo()", "CSpaceTimeAnalysis");
    delete pMeasureList;
    throw;
  }
  return (dMaxLogLikelihood - m_pData->m_pModel->GetLogLikelihoodForTotal());
}

double CSpaceTimeAnalysis::MonteCarloProspective() {
  CMeasureList        * pMeasureList=0;
  double                dMaxLogLikelihood;
  long                  lTime;
  Julian                jCurrentDate;
  int                   iThisStartInterval, n, m, k;

  try {
    //for prospective Space-Time, m_bAliveClustersOnly should be false..
    //m_bAliveClustersOnly is the first parameter into the CSpaceTimeCluster class
    CSpaceTimeCluster C(false, m_pData->m_nTimeIntervals, m_pData->m_nIntervalCut, gpPrintDirection);
    C.SetRate(m_pParameters->m_nAreas);
    dMaxLogLikelihood = m_pData->m_pModel->GetLogLikelihoodForTotal();

    switch (m_pParameters->m_nAreas) {
     case HIGH       : pMeasureList = new CMinMeasureList(*m_pData, *gpPrintDirection);
                       break;
     case LOW        : pMeasureList = new CMaxMeasureList(*m_pData, *gpPrintDirection);
                       break;
     case HIGHANDLOW : pMeasureList = new CMinMaxMeasureList(*m_pData, *gpPrintDirection);
                       break;
     default         : ZdGenerateException("Unknown incidence rate specifier \"%d\".","MonteCarloProspective()",
                       m_pParameters->m_nAreas);
    }

    for (k=0; k <= m_pParameters->m_lTotalNumEllipses; k++) {  //circle is 0 offset... (always there)
       for (tract_t i = 0; i<m_pData->m_nGridTracts; i++) {
          C.Initialize(i);
          for (tract_t j=1; j<=m_pData->m_NeighborCounts[k][i]; j++) {
             C.AddNeighbor(k, *m_pData, m_pData->m_pSimCases, j);
             //Need to keep track of the current date as you loop through intervals
             jCurrentDate = m_pData->m_nEndDate;
             // Loop from study end date back to Prospective start date -- loop by interval
             for (n = m_pData->m_nTimeIntervals; n >= m_pData->m_nProspectiveIntervalStart; n--) {
                //Need to re-compute duration by using current date (whatever date loop "n" is at)
                //and the Begin Study Date.  Must use time percent specified by user...
                iThisStartInterval = m_pData->ComputeNewCutoffInterval(m_pData->m_nStartDate,jCurrentDate);
                //Now compute a new Current Date by subtracting the interval duration
                jCurrentDate = DecrementDate(jCurrentDate, m_pParameters->m_nIntervalUnits, m_pParameters->m_nIntervalLength);
                C.InitTimeIntervalIndeces(iThisStartInterval, n);
                while (C.SetNextProspTimeInterval())
                     pMeasureList->AddMeasure(C.m_nCases, C.m_nMeasure);
             }
          }
       }
       pMeasureList->SetForNextIteration(k, dMaxLogLikelihood);
    }
    delete pMeasureList;
  }
  catch (ZdException & x) {
    x.AddCallpath("MonteCarloProspective()", "CSpaceTimeAnalysis");
    delete pMeasureList;
    throw;
  }
  return (dMaxLogLikelihood - m_pData->m_pModel->GetLogLikelihoodForTotal());
}


