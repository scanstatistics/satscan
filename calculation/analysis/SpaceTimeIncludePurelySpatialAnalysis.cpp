#include "SaTScan.h"
#pragma hdrstop
#include "SpaceTimeIncludePurelySpatialAnalysis.h"

/** Constructor */
C_ST_PS_Analysis::C_ST_PS_Analysis(CParameters*  pParameters, CSaTScanData* pData, BasePrint *pPrintDirection)
                 :CSpaceTimeAnalysis(pParameters, pData, pPrintDirection) {}

/** Destructor */                 
C_ST_PS_Analysis::~C_ST_PS_Analysis() {}

/** Returns cluster centered at grid point nCenter, with the greatest loglikelihood.
    Caller is responsible for deleting returned cluster. */
CCluster* C_ST_PS_Analysis::GetTopCluster(tract_t nCenter) {
  int                           j;
  tract_t                       i;
  CCluster                    * MaxCluster=0;
  CPurelySpatialCluster       * C_PS_MAX=0;
  CSpaceTimeCluster           * C_ST_MAX=0;
  CPurelySpatialCluster       * C_PS=0;
  CSpaceTimeCluster           * C_ST=0;
  CCluster                    * OrigCluster=0;
  bool                          bAliveCluster;

  try {
    MaxCluster = new CCluster(gpPrintDirection);
    OrigCluster = MaxCluster;

    // if Prospective Space-Time then Alive Clusters Only.
    if (m_pParameters->m_nAnalysisType == PROSPECTIVESPACETIME)
      bAliveCluster = true;
    else
      bAliveCluster = m_pParameters->m_bAliveClustersOnly;

    C_PS_MAX = new CPurelySpatialCluster(gpPrintDirection);
    C_ST_MAX = new CSpaceTimeCluster(bAliveCluster, m_pData->m_nTimeIntervals,
                                     m_pData->m_nIntervalCut, gpPrintDirection);

    MaxCluster->SetLogLikelihood(m_pData->m_pModel->GetLogLikelihoodForTotal());
    for (j=0 ;j <= m_pParameters->m_lTotalNumEllipses; j++) {
        CPurelySpatialCluster C_PS(gpPrintDirection);
        C_PS.SetCenter(nCenter);
        C_PS.SetRate(m_pParameters->m_nAreas);
        C_PS.SetEllipseOffset(j);  
        C_PS.SetDuczmalCorrection((j == 0 || !m_pParameters->m_bDuczmalCorrectEllipses ? 1 : m_pData->mdE_Shapes[j - 1]));
        CSpaceTimeCluster C_ST(bAliveCluster, m_pData->m_nTimeIntervals, m_pData->m_nIntervalCut, gpPrintDirection);
        C_ST.SetCenter(nCenter);
        C_ST.SetRate(m_pParameters->m_nAreas);
        C_ST.SetEllipseOffset(j);
        C_ST.SetDuczmalCorrection((j == 0 || !m_pParameters->m_bDuczmalCorrectEllipses ? 1 : m_pData->mdE_Shapes[j - 1]));
        for (i=1; i <= m_pData->m_NeighborCounts[j][nCenter]; i++) {                // update this later
           C_PS.AddNeighbor(j, *m_pData, m_pData->m_pCases, i);                          // update this later
           if (C_PS.RateIsOfInterest(m_pData->m_nTotalCases, m_pData->m_nTotalMeasure)) {
             C_PS.m_nLogLikelihood = m_pData->m_pModel->CalcLogLikelihood(C_PS.m_nCases, C_PS.m_nMeasure);
             if (C_PS.m_nLogLikelihood * C_PS.m_DuczmalCorrection > MaxCluster->m_nLogLikelihood) {
               *C_PS_MAX  = C_PS;
               MaxCluster = C_PS_MAX;
             }
           }
           C_ST.AddNeighbor(j, *m_pData, m_pData->m_pCases, i);                            // update this later
           C_ST.InitTimeIntervalIndeces();
           while (C_ST.SetNextTimeInterval()) {
                if (C_ST.RateIsOfInterest(m_pData->m_nTotalCases, m_pData->m_nTotalMeasure)) {
                  C_ST.m_nLogLikelihood = m_pData->m_pModel->CalcLogLikelihood(C_ST.m_nCases, C_ST.m_nMeasure);
                  if (C_ST.m_nLogLikelihood * C_ST.m_DuczmalCorrection > MaxCluster->m_nLogLikelihood) {
                    *C_ST_MAX  = C_ST;
                    MaxCluster = C_ST_MAX;
                  }
                }
           }
        }
    }

    if (MaxCluster == OrigCluster) {
      delete C_PS_MAX;
      delete C_ST_MAX;
    }
    else {
      delete OrigCluster;
      if (MaxCluster == C_PS_MAX)
        delete C_ST_MAX;
      else
        delete C_PS_MAX;
    }

    MaxCluster->SetRatioAndDates(*m_pData);
  }
  catch (ZdException & x) {
    // do not need to delete MaxCluster... is one of the objects below
    delete C_PS; C_PS = 0;
    delete C_ST; C_ST = 0;
    delete C_PS_MAX;C_PS_MAX=0;
    delete C_ST_MAX;C_ST_MAX=0;
    delete OrigCluster;
    x.AddCallpath("GetTopCluster()", "C_ST_PS_Analysis");
    throw;
  }
  return MaxCluster;
}

/** Returns loglikelihood for Monte Carlo replication. */
double C_ST_PS_Analysis::MonteCarlo() {
  CMeasureList                * pMeasureList = 0;
  CPurelySpatialCluster         C_PS(gpPrintDirection);
  double                        dMaxLogLikelihood;
  int                           k;
  tract_t                       i, j;

  try {
    CSpaceTimeCluster C_ST(m_pParameters->m_bAliveClustersOnly, m_pData->m_nTimeIntervals,
                           m_pData->m_nIntervalCut, gpPrintDirection);
    C_PS.SetRate(m_pParameters->m_nAreas);
    C_ST.SetRate(m_pParameters->m_nAreas);
    dMaxLogLikelihood = m_pData->m_pModel->GetLogLikelihoodForTotal();
    switch (m_pParameters->m_nAreas) {
      case HIGH       : pMeasureList = new CMinMeasureList(*m_pData, *gpPrintDirection);
                        break;
      case LOW        : pMeasureList = new CMaxMeasureList(*m_pData, *gpPrintDirection);
                        break;
      case HIGHANDLOW : pMeasureList = new CMinMaxMeasureList(*m_pData, *gpPrintDirection);
                        break;
      default         : ZdGenerateException("Unknown incidence rate specifier \"%d\".", "MonteCarlo()",
                                             m_pParameters->m_nAreas);
    }

    //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
    for (k=0; k <= m_pParameters->m_lTotalNumEllipses; k++) {  
       for (i=0; i < m_pData->m_nGridTracts; i++) {
          C_PS.Initialize(i);
          C_ST.Initialize(i);
          for (j=1; j <= m_pData->m_NeighborCounts[k][i]; j++) {
            C_PS.AddNeighbor(k, *m_pData, m_pData->m_pSimCases, j);
            pMeasureList->AddMeasure(C_PS.m_nCases, C_PS.m_nMeasure);
            C_ST.AddNeighbor(k, *m_pData, m_pData->m_pSimCases, j);
            C_ST.InitTimeIntervalIndeces();
            while (C_ST.SetNextTimeInterval())
                 pMeasureList->AddMeasure(C_ST.m_nCases, C_ST.m_nMeasure);
          }
       }
       pMeasureList->SetForNextIteration(k, dMaxLogLikelihood);
    }
    delete pMeasureList;
  }
  catch (ZdException & x) {
    delete pMeasureList;
    x.AddCallpath("MonteCarlo()", "C_ST_PS_Analysis");
    throw;
  }
  return (dMaxLogLikelihood - m_pData->m_pModel->GetLogLikelihoodForTotal());
}


/** Returns loglikelihood for Monte Carlo Prospective replication. */
double C_ST_PS_Analysis::MonteCarloProspective() {
  CMeasureList                * pMeasureList = 0;
  CPurelySpatialCluster         C_PS(gpPrintDirection);
  double                        dMaxLogLikelihood;
  long                          lTime;
  Julian                        jCurrentDate;
  int                           iThisStartInterval, n, m, k;
  tract_t                       i, j;

  try {
    //for prospective Space-Time, m_bAliveClustersOnly should be false..
    //m_bAliveClustersOnly is the first parameter into the CSpaceTimeCluster class
    CSpaceTimeCluster     C_ST(false, m_pData->m_nTimeIntervals, m_pData->m_nIntervalCut, gpPrintDirection);
    C_PS.SetRate(m_pParameters->m_nAreas);
    C_ST.SetRate(m_pParameters->m_nAreas);
    dMaxLogLikelihood = m_pData->m_pModel->GetLogLikelihoodForTotal();
    switch (m_pParameters->m_nAreas) {
      case HIGH       : pMeasureList = new CMinMeasureList(*m_pData, *gpPrintDirection);
                        break;
      case LOW        : pMeasureList = new CMaxMeasureList(*m_pData, *gpPrintDirection);
                        break;
      case HIGHANDLOW : pMeasureList = new CMinMaxMeasureList(*m_pData, *gpPrintDirection);
                        break;
       default        : ZdGenerateException("Unknown incidence rate specifier \"%d\".", "MonteCarloProspective()",
                                             m_pParameters->m_nAreas);
    }

    //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
    for (k=0; k <= m_pParameters->m_lTotalNumEllipses; k++) {  
       for (i=0; i < m_pData->m_nGridTracts; i++) {
          C_PS.Initialize(i);
          C_ST.Initialize(i);
           for (tract_t j=1; j<=m_pData->m_NeighborCounts[k][i]; j++) {
              C_PS.AddNeighbor(k, *m_pData, m_pData->m_pSimCases, j);
              pMeasureList->AddMeasure(C_PS.m_nCases, C_PS.m_nMeasure);
              C_ST.AddNeighbor(k, *m_pData, m_pData->m_pSimCases, j);    // k use to be "0"
              //Need to keep track of the current date as you loop through intervals
              jCurrentDate = m_pData->m_nEndDate;
              // Loop from study end date back to Prospective start date -- loop by interval
              for (n = m_pData->m_nTimeIntervals; n >= m_pData->m_nProspectiveIntervalStart; n--) {
                //Need to re-compute duration due to by using current date (whatever date loop "n" is at)
                //and the Begin Study Date
                iThisStartInterval = m_pData->ComputeNewCutoffInterval(m_pData->m_nStartDate,jCurrentDate);
                //Now compute a new Current Date by subtracting the interval duration
                jCurrentDate = DecrementDate(jCurrentDate, m_pParameters->m_nIntervalUnits, m_pParameters->m_nIntervalLength);
                C_ST.InitTimeIntervalIndeces(iThisStartInterval, n);
                while (C_ST.SetNextProspTimeInterval())
                     pMeasureList->AddMeasure(C_ST.m_nCases, C_ST.m_nMeasure);
              }
           }
       }
       pMeasureList->SetForNextIteration(k, dMaxLogLikelihood);
    }
    delete pMeasureList;
  }
  catch (ZdException & x) {
    delete pMeasureList;
    x.AddCallpath("MonteCarloProspective()", "C_ST_PS_Analysis");
    throw;
  }
  return (dMaxLogLikelihood - m_pData->m_pModel->GetLogLikelihoodForTotal());
}


