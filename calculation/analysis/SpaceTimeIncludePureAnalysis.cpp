#include "SaTScan.h"
#pragma hdrstop
#include "SpaceTimeIncludePureAnalysis.h"

/** Constructor */
C_ST_PS_PT_Analysis::C_ST_PS_PT_Analysis(CParameters*  pParameters, CSaTScanData* pData, BasePrint *pPrintDirection)
                    :C_ST_PS_Analysis(pParameters, pData, pPrintDirection) {}

/** Desctructor */                    
C_ST_PS_PT_Analysis::~C_ST_PS_PT_Analysis() {}

/** Finds top clusters. */
bool C_ST_PS_PT_Analysis::FindTopClusters() {
  try {
    if (! C_ST_PS_Analysis::FindTopClusters())  // KR-980327 Changed from CSpaceTimeAnalysis::
      return false;
      
    tract_t nLastClusterIndex = m_nClustersRetained;
    m_pTopClusters[nLastClusterIndex] = GetTopPTCluster();
    SortTopClusters();
  }
  catch (ZdException & x) {
    x.AddCallpath("FindTopClusters()", "C_ST_PS_PT_Analysis");
    throw;
  }
  return true;
}

/** Returns cluster centered at grid point nCenter, with the greatest loglikelihood.
    Caller is responsible for deleting returned cluster. */
CPurelyTemporalCluster* C_ST_PS_PT_Analysis::GetTopPTCluster() {
  CPurelyTemporalCluster              * pTopCluster=0;
  bool                                  bAliveCluster;

  try {
    // if Prospective Space-Time then Alive Clusters Only.
    if (m_pParameters->GetAnalysisType() == PROSPECTIVESPACETIME)
      bAliveCluster = true;
    else
      bAliveCluster = m_pParameters->GetAliveClustersOnly();

    pTopCluster = new CPurelyTemporalCluster(bAliveCluster, m_pData->m_nTimeIntervals,
                                            m_pData->m_nIntervalCut, gpPrintDirection);
    CPurelyTemporalCluster C_PT(bAliveCluster, m_pData->m_nTimeIntervals,
                                m_pData->m_nIntervalCut, gpPrintDirection);
    pTopCluster->SetLogLikelihood(m_pData->m_pModel->GetLogLikelihoodForTotal());

    C_PT.SetRate(m_pParameters->GetAreaScanRateType());
    C_PT.InitTimeIntervalIndeces();
    while (C_PT.SetNextTimeInterval(m_pData->m_pPTCases, m_pData->m_pPTMeasure)) {
         if (C_PT.RateIsOfInterest(m_pData->m_nTotalCases, m_pData->m_nTotalMeasure)) {
          C_PT.m_nLogLikelihood = m_pData->m_pModel->CalcLogLikelihood(C_PT.m_nCases, C_PT.m_nMeasure);
          if (C_PT.m_nLogLikelihood > pTopCluster->m_nLogLikelihood)
            *pTopCluster = C_PT;
        }
    }
    pTopCluster->SetRatioAndDates(*m_pData);
    m_nClustersRetained++;
  }
  catch (ZdException & x) {
    delete pTopCluster;
    x.AddCallpath("GetTopPTCluster()", "C_ST_PS_PT_Analysis");
    throw;
  }
  return pTopCluster;
}

/** Returns loglikelihood for Monte Carlo replication. */
double C_ST_PS_PT_Analysis::MonteCarlo() {
  CMeasureList                * pMeasureList=0;
  double                        dMaxLogLikelihoodRatio;
  int                           k;
  tract_t                       i, j;

  try {
    CPurelySpatialCluster C_PS(gpPrintDirection);
    CSpaceTimeCluster     C_ST(m_pParameters->GetAliveClustersOnly(), m_pData->m_nTimeIntervals,
                               m_pData->m_nIntervalCut, gpPrintDirection);
    CPurelyTemporalCluster C_PT(m_pParameters->GetAliveClustersOnly(), m_pData->m_nTimeIntervals,
                                m_pData->m_nIntervalCut, gpPrintDirection);
    C_PS.SetRate(m_pParameters->GetAreaScanRateType());
    C_ST.SetRate(m_pParameters->GetAreaScanRateType());
    switch (m_pParameters->GetAreaScanRateType()) {
      case HIGH       : pMeasureList = new CMinMeasureList(*m_pData, *gpPrintDirection);
                        break;
      case LOW        : pMeasureList = new CMaxMeasureList(*m_pData, *gpPrintDirection);
                        break;
      case HIGHANDLOW : pMeasureList = new CMinMaxMeasureList(*m_pData, *gpPrintDirection);
                        break;
      default         : ZdGenerateException("Unknown incidence rate specifier \"%d\".", "MonteCarlo()",
                                             m_pParameters->GetAreaScanRateType());
    }

    //Add measure values for purely space first - so that this cluster's values
    //will be calculated with circle's measure values.
    C_PT.Initialize(0);
    C_PT.SetRate(m_pParameters->GetAreaScanRateType());
    C_PT.InitTimeIntervalIndeces();
    while (C_PT.SetNextTimeInterval(m_pData->m_pPTSimCases, m_pData->m_pPTMeasure))
        pMeasureList->AddMeasure(C_PT.m_nCases, C_PT.m_nMeasure);

    //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
    for (k=0; k <= m_pParameters->GetNumTotalEllipses(); k++) {  
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
       pMeasureList->SetForNextIteration(k);
    }
    dMaxLogLikelihoodRatio = pMeasureList->GetMaximumLogLikelihoodRatio();
    delete pMeasureList;
  }
  catch (ZdException & x) {
    delete pMeasureList;
    x.AddCallpath("MonteCarlo()", "C_ST_PS_PT_Analysis");
    throw;
  }
  return dMaxLogLikelihoodRatio;
}

/** Returns loglikelihood for Monte Carlo Prospective replication. */
double C_ST_PS_PT_Analysis::MonteCarloProspective() {
  CMeasureList                * pMeasureList=0;
  double                        dMaxLogLikelihoodRatio;
  long                          lTime;
  Julian                        jCurrentDate;
  int                           iThisStartInterval, n, m, k;
  tract_t                       i, j;

  try {
    //for prospective Space-Time, GetAliveClustersOnly() should be false..
    //m_bAliveClustersOnly is the first parameter into the CSpaceTimeCluster class
    CPurelySpatialCluster C_PS(gpPrintDirection);
    CSpaceTimeCluster     C_ST(false, m_pData->m_nTimeIntervals, m_pData->m_nIntervalCut, gpPrintDirection);
    CPurelyTemporalCluster C_PT(false, m_pData->m_nTimeIntervals, m_pData->m_nIntervalCut, gpPrintDirection);
    
    C_PS.SetRate(m_pParameters->GetAreaScanRateType());
    C_ST.SetRate(m_pParameters->GetAreaScanRateType());
    switch (m_pParameters->GetAreaScanRateType()) {
     case HIGH       : pMeasureList = new CMinMeasureList(*m_pData, *gpPrintDirection);
                       break;
     case LOW        : pMeasureList = new CMaxMeasureList(*m_pData, *gpPrintDirection);
                       break;
     case HIGHANDLOW : pMeasureList = new CMinMaxMeasureList(*m_pData, *gpPrintDirection);
                       break;
     default         : ZdGenerateException("Unknown incidence rate specifier \"%d\".", "MonteCarloProspective()",
                                           m_pParameters->GetAreaScanRateType());
    }

    //Add measure values for purely space first - so that this cluster's values
    //will be calculated with circle's measure values.
    C_PT.Initialize(0);
    C_PT.SetRate(m_pParameters->GetAreaScanRateType());
    C_PT.InitTimeIntervalIndeces();
    while (C_PT.SetNextTimeInterval(m_pData->m_pPTSimCases,  m_pData->m_pPTMeasure))
         pMeasureList->AddMeasure(C_PT.m_nCases, C_PT.m_nMeasure);

    //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
    for (k=0; k <= m_pParameters->GetNumTotalEllipses(); ++k) {
       for (i=0; i < m_pData->m_nGridTracts; i++) {
          C_PS.Initialize(i);
          C_ST.Initialize(i);
          for (tract_t j=1; j<=m_pData->m_NeighborCounts[k][i]; ++j) {
             C_PS.AddNeighbor(k, *m_pData, m_pData->m_pSimCases, j);
             pMeasureList->AddMeasure(C_PS.m_nCases, C_PS.m_nMeasure);
             C_ST.AddNeighbor(k, *m_pData, m_pData->m_pSimCases, j);
             //Need to keep track of the current date as you loop through intervals
             jCurrentDate = m_pData->m_nEndDate;
             // Loop from study end date back to Prospective start date -- loop by interval
             for (n=m_pData->m_nTimeIntervals; n >= m_pData->m_nProspectiveIntervalStart; --n) {
                //Need to re-compute duration due to by using current date (whatever date loop "n" is at)
                //and the Begin Study Date
                iThisStartInterval = m_pData->ComputeNewCutoffInterval(m_pData->m_nStartDate,jCurrentDate);
                //Now compute a new Current Date by subtracting the interval duration
                jCurrentDate = DecrementDate(jCurrentDate, m_pParameters->GetTimeIntervalUnitsType(), m_pParameters->GetTimeIntervalLength());
                C_ST.InitTimeIntervalIndeces(iThisStartInterval, n);
                while (C_ST.SetNextProspTimeInterval())
                     pMeasureList->AddMeasure(C_ST.m_nCases, C_ST.m_nMeasure);
             }
          }
       }
       pMeasureList->SetForNextIteration(k);
    }
    dMaxLogLikelihoodRatio = pMeasureList->GetMaximumLogLikelihoodRatio();
    delete pMeasureList;
  }
  catch (ZdException & x) {
    delete pMeasureList;
    x.AddCallpath("MonteCarloProspective()", "C_ST_PS_PT_Analysis");
    throw;
  }
  return dMaxLogLikelihoodRatio;
}

