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
  CPurelyTemporalCluster              * pTopCluster=0;
  IncludeClustersType                   eIncludeClustersType;

  try {
    // if Prospective Space-Time then Alive Clusters Only.
    if (m_pParameters->GetAnalysisType() == PROSPECTIVEPURELYTEMPORAL)
      eIncludeClustersType = ALIVECLUSTERS;
    else
      eIncludeClustersType = m_pParameters->GetIncludeClustersType();

    gpPrintDirection->SatScanPrintf("Get Top P.T. Cluster.\n");
    pTopCluster = new CPurelyTemporalCluster(eIncludeClustersType, *m_pData, *gpPrintDirection);
    CPurelyTemporalCluster thisCluster(eIncludeClustersType, *m_pData, *gpPrintDirection);
    
    pTopCluster->CCluster::SetLogLikelihood(m_pData->m_pModel->GetLogLikelihoodForTotal());
    thisCluster.SetCenter(nCenter);
    thisCluster.SetRate(m_pParameters->GetAreaScanRateType());
    thisCluster.CompareTopCluster(*pTopCluster, *m_pData);
    pTopCluster->SetRatioAndDates(*m_pData);
  }
  catch (ZdException & x) {
    delete pTopCluster;
    x.AddCallpath("GetTopCluster()", "CPurelyTemporalAnalysis");
    throw;
  }
  return pTopCluster;
}

/** Returns loglikelihood for Monte Carlo replication. */
double CPurelyTemporalAnalysis::MonteCarlo() {
  CMeasureList        * pMeasureList=0;
  double                dMaxLogLikelihoodRatio;

  try {
    CPurelyTemporalCluster C(m_pParameters->GetIncludeClustersType(), *m_pData, *gpPrintDirection);
    C.SetRate(m_pParameters->GetAreaScanRateType());
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
    C.ComputeBestMeasures(m_pData->m_pPTSimCases, m_pData->m_pPTMeasure, *pMeasureList);

    dMaxLogLikelihoodRatio = pMeasureList->GetMaximumLogLikelihoodRatio();
    delete pMeasureList;
  }
  catch (ZdException & x) {
    delete pMeasureList;
    x.AddCallpath("MonteCarlo()", "CPurelyTemporalAnalysis");
    throw;
  }
  return dMaxLogLikelihoodRatio;
}

/** For purely temporal analysis, prospective monte carlo is the same as monte carlo. */
double CPurelyTemporalAnalysis::MonteCarloProspective() {
  CMeasureList                * pMeasureList=0;
  double                        dMaxLogLikelihoodRatio;

  try {
    CPurelyTemporalCluster C(ALLCLUSTERS, *m_pData, *gpPrintDirection);
    C.SetRate(m_pParameters->GetAreaScanRateType());
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
    C.ComputeBestMeasures(m_pData->m_pPTSimCases, m_pData->m_pPTMeasure, *pMeasureList);

    dMaxLogLikelihoodRatio = pMeasureList->GetMaximumLogLikelihoodRatio();
    delete pMeasureList;
  }
  catch (ZdException & x) {
    delete pMeasureList;
    x.AddCallpath("MonteCarloProspective()", "CPurelyTemporalAnalysis");
    throw;
  }
  return dMaxLogLikelihoodRatio;
}
