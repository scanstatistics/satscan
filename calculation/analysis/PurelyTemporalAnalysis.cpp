#include "SaTScan.h"
#pragma hdrstop
#include "PurelyTemporalAnalysis.h"

/** Constructor */
CPurelyTemporalAnalysis::CPurelyTemporalAnalysis(CParameters*  pParameters, CSaTScanData* pData, BasePrint *pPrintDirection)
                        :CAnalysis(pParameters, pData, pPrintDirection) {}

/** Destructor */
CPurelyTemporalAnalysis::~CPurelyTemporalAnalysis() {}

/** Finds top cluster. */
bool CPurelyTemporalAnalysis::FindTopClusters() {
  bool  bOk = true;

  try {
    gpPrintDirection->SatScanPrintf("Find top cluster.\n");
    m_pTopClusters[0] = GetTopCluster(0);
    if (m_pTopClusters[0]->ClusterDefined())
      m_nClustersRetained++;
    else {
      delete m_pTopClusters[0];
      m_pTopClusters[0] = NULL;
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("FindTopClusters()", "CPurelyTemporalAnalysis");
    throw;
  }
  return bOk;
}

/** Returns cluster centered at grid point nCenter, with the greatest loglikelihood.
    Caller is responsible for deleting returned cluster. */
CCluster* CPurelyTemporalAnalysis::GetTopCluster(tract_t nCenter) {
  CPurelyTemporalCluster              * pMaxCluster=0;
  bool                                  bAliveCluster;

  try {
    // if Prospective Space-Time then Alive Clusters Only.
    if (m_pParameters->GetAnalysisType() == PROSPECTIVESPACETIME)
      bAliveCluster = true;
    else
      bAliveCluster = m_pParameters->GetAliveClustersOnly();

    gpPrintDirection->SatScanPrintf("Get Top P.T. Cluster.\n");
    pMaxCluster = new CPurelyTemporalCluster(bAliveCluster, m_pData->m_nTimeIntervals,
                                             m_pData->m_nIntervalCut, gpPrintDirection);
    CPurelyTemporalCluster thisCluster(bAliveCluster, m_pData->m_nTimeIntervals,
                                       m_pData->m_nIntervalCut, gpPrintDirection);
    
    pMaxCluster->CCluster::SetLogLikelihood(m_pData->m_pModel->GetLogLikelihoodForTotal());
    thisCluster.SetCenter(nCenter);
    thisCluster.SetRate(m_pParameters->GetAreaScanRateType());
    thisCluster.InitTimeIntervalIndeces();
    
    while (thisCluster.SetNextTimeInterval(((CPurelyTemporalData*)(m_pData))->m_pPTCases,
                                           ((CPurelyTemporalData*)(m_pData))->m_pPTMeasure)) {
         if (thisCluster.RateIsOfInterest(m_pData->m_nTotalCases, m_pData->m_nTotalMeasure)) {
           thisCluster.m_nLogLikelihood = m_pData->m_pModel->CalcLogLikelihood(thisCluster.m_nCases,
                                                                               thisCluster.m_nMeasure);
           if (thisCluster.m_nLogLikelihood > pMaxCluster->m_nLogLikelihood)
             *pMaxCluster = thisCluster;
         }
    
    }
    pMaxCluster->SetRatioAndDates(*m_pData);
  }
  catch (ZdException & x) {
    delete pMaxCluster;
    x.AddCallpath("GetTopCluster()", "CPurelyTemporalAnalysis");
    throw;
  }
  return pMaxCluster;
}

/** Returns loglikelihood for Monte Carlo replication. */
double CPurelyTemporalAnalysis::MonteCarlo() {
  CMeasureList        * pMeasureList=0;
  double                dMaxLogLikelihood;

  try {
    CPurelyTemporalCluster C(m_pParameters->GetAliveClustersOnly(), m_pData->m_nTimeIntervals,
                             m_pData->m_nIntervalCut, gpPrintDirection);
    C.SetRate(m_pParameters->GetAreaScanRateType());
    dMaxLogLikelihood = m_pData->m_pModel->GetLogLikelihoodForTotal();
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

    C.Initialize(0);
    C.InitTimeIntervalIndeces();
    while (C.SetNextTimeInterval(m_pData->m_pPTSimCases, m_pData->m_pPTMeasure))
        pMeasureList->AddMeasure(C.m_nCases, C.m_nMeasure);
    dMaxLogLikelihood = pMeasureList->GetMaxLogLikelihood(dMaxLogLikelihood);

    delete pMeasureList;
  }
  catch (ZdException & x) {
    delete pMeasureList;
    x.AddCallpath("MonteCarlo()", "CPurelyTemporalAnalysis");
    throw;
  }
  return (dMaxLogLikelihood - m_pData->m_pModel->GetLogLikelihoodForTotal());
}

/** For purely temporal analysis, prospective monte carlo is the same as monte carlo. */
double CPurelyTemporalAnalysis::MonteCarloProspective() {
  CMeasureList                * pMeasureList=0;
  double                        dMaxLogLikelihood;

  try {
    CPurelyTemporalCluster C(m_pParameters->GetAliveClustersOnly(), m_pData->m_nTimeIntervals,
                             m_pData->m_nIntervalCut, gpPrintDirection);
    C.SetRate(m_pParameters->GetAreaScanRateType());
    dMaxLogLikelihood = m_pData->m_pModel->GetLogLikelihoodForTotal();
    switch (m_pParameters->GetAreaScanRateType()) {
      case HIGH       : pMeasureList = new CMinMeasureList(*m_pData, *gpPrintDirection);
                        break;
      case LOW        : pMeasureList = new CMaxMeasureList(*m_pData, *gpPrintDirection);
                        break;
      case HIGHANDLOW : pMeasureList = new CMinMaxMeasureList(*m_pData, *gpPrintDirection);
                        break;
      default        : ZdGenerateException("Unknown incidence rate specifier \"%d\".", "MonteCarloProspective()",
                                             m_pParameters->GetAreaScanRateType());
    }
    
    C.Initialize(0);
    C.InitTimeIntervalIndeces();
    while (C.SetNextTimeInterval(m_pData->m_pPTSimCases, m_pData->m_pPTMeasure))
         pMeasureList->AddMeasure(C.m_nCases, C.m_nMeasure);

    dMaxLogLikelihood = pMeasureList->GetMaxLogLikelihood(dMaxLogLikelihood);
    delete pMeasureList;
  }
  catch (ZdException & x) {
    delete pMeasureList;
    x.AddCallpath("MonteCarloProspective()", "CPurelyTemporalAnalysis");
    throw;
  }
  return (dMaxLogLikelihood - m_pData->m_pModel->GetLogLikelihoodForTotal());
}
