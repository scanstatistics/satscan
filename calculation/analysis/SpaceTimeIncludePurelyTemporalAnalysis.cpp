#include "SaTScan.h"
#pragma hdrstop
#include "SpaceTimeIncludePurelyTemporalAnalysis.h"

/** Constructor */
C_ST_PT_Analysis::C_ST_PT_Analysis(CParameters*  pParameters, CSaTScanData* pData, BasePrint *pPrintDirection)
                 :CSpaceTimeAnalysis(pParameters, pData, pPrintDirection) {}

/** Destructor */
C_ST_PT_Analysis::~C_ST_PT_Analysis() {}

/** Returns maximum number of clusters. */
void C_ST_PT_Analysis::SetMaxNumClusters() {
  m_nMaxClusters = m_pData->m_nGridTracts+1;
}

/** Finds top clusters for each grid point. */
bool C_ST_PT_Analysis::FindTopClusters() {
  try {
    if (!CSpaceTimeAnalysis::FindTopClusters())
      return false;

    tract_t nLastClusterIndex = m_nClustersRetained;
    m_pTopClusters[nLastClusterIndex] = GetTopPTCluster();
    SortTopClusters();
  }
  catch (ZdException & x) {
    x.AddCallpath("FindTopClusters()", "C_ST_PT_Analysis");
    throw;
  }
  return true;
}

/** Returns temporal cluster with the greatest loglikelihood.
    Caller is responsible for deleting returned cluster. */
CPurelyTemporalCluster* C_ST_PT_Analysis::GetTopPTCluster() {
  CPurelyTemporalCluster      * pTopCluster=0;
  IncludeClustersType           eIncludeClustersType;

  try {
    // if Prospective Space-Time then Alive Clusters Only.
    if (m_pParameters->GetAnalysisType() == PROSPECTIVESPACETIME)
      eIncludeClustersType = ALIVECLUSTERS;
    else
      eIncludeClustersType = m_pParameters->GetIncludeClustersType();

    pTopCluster = new CPurelyTemporalCluster(eIncludeClustersType, *m_pData, *gpPrintDirection);
    CPurelyTemporalCluster C_PT(eIncludeClustersType, *m_pData, *gpPrintDirection);

    pTopCluster->SetLogLikelihood(m_pData->m_pModel->GetLogLikelihoodForTotal());
    C_PT.SetRate(m_pParameters->GetAreaScanRateType());
    C_PT.CompareTopCluster(*pTopCluster, *m_pData);

    if (pTopCluster->ClusterDefined()) {
      pTopCluster->SetRatio(m_pData->m_pModel->GetLogLikelihoodForTotal());
      pTopCluster->SetStartAndEndDates(m_pData->m_pIntervalStartTimes, m_pData->m_nTimeIntervals);
    }
    m_nClustersRetained++;
  }
  catch (ZdException & x) {
    delete pTopCluster;
    x.AddCallpath("GetTopPTCluster()", "C_ST_PT_Analysis");
    throw;
  }
  return pTopCluster;
}

/** Returns loglikelihood for Monte Carlo replication. */
double C_ST_PT_Analysis::MonteCarlo() {
  CMeasureList        * pMeasureList=0;
  double                dMaxLogLikelihoodRatio;
  int                   k;
  tract_t               i, j;

  try {
    CPurelyTemporalCluster C_PT(m_pParameters->GetIncludeClustersType(), *m_pData, *gpPrintDirection);
    CSpaceTimeCluster C_ST(m_pParameters->GetIncludeClustersType(), *m_pData, *gpPrintDirection);
    
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
    C_PT.ComputeBestMeasures(m_pData->m_pPTSimCases, m_pData->m_pPTMeasure, *pMeasureList);

    //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
    for (k=0; k <= m_pParameters->GetNumTotalEllipses(); k++) {
       for (i=0; i < m_pData->m_nGridTracts; i++) {
          C_ST.Initialize(i);
          for (j=1; j <= m_pData->m_NeighborCounts[k][i]; j++) {
             C_ST.AddNeighbor(k, *m_pData, m_pData->m_pSimCases, j);
             C_ST.ComputeBestMeasures(*pMeasureList);
          }
       }
       pMeasureList->SetForNextIteration(k);
    }
    dMaxLogLikelihoodRatio = pMeasureList->GetMaximumLogLikelihoodRatio();
    delete pMeasureList;
  }
  catch (ZdException & x) {
    delete pMeasureList;
    x.AddCallpath("MonteCarlo()", "C_ST_PT_Analysis");
    throw;
  }
  return dMaxLogLikelihoodRatio;
}

/** Returns loglikelihood for Monte Carlo Prospective replication. */
double C_ST_PT_Analysis::MonteCarloProspective() {
  CMeasureList                * pMeasureList=0;
  double                        dMaxLogLikelihoodRatio;
  long                          lTime;
  Julian                        jCurrentDate;
  int                           iThisStartInterval, n, m, k;
  tract_t                       i, j;

  try {
    //for prospective Space-Time, m_bAliveClustersOnly should be false..
    //m_bAliveClustersOnly is the first parameter into the CSpaceTimeCluster class
    CPurelyTemporalCluster C_PT(ALLCLUSTERS, *m_pData, *gpPrintDirection);
    CSpaceTimeCluster C_ST(ALLCLUSTERS, *m_pData, *gpPrintDirection);

    C_ST.SetRate(m_pParameters->GetAreaScanRateType());
    switch (m_pParameters->GetAreaScanRateType()) {
      case HIGH       : pMeasureList = new CMinMeasureList(*m_pData, *gpPrintDirection);
                        break;
      case LOW        : pMeasureList = new CMaxMeasureList(*m_pData, *gpPrintDirection);
                        break;
      case HIGHANDLOW : pMeasureList = new CMinMaxMeasureList(*m_pData, *gpPrintDirection);
                        break;
      default        :  ZdGenerateException("Unknown incidence rate specifier \"%d\".", "MonteCarloProspective()",
                                             m_pParameters->GetAreaScanRateType());
    }

    //Add measure values for purely space first - so that this cluster's values
    //will be calculated with circle's measure values.
    C_PT.Initialize(0);
    C_PT.SetRate(m_pParameters->GetAreaScanRateType());
    C_PT.ComputeBestMeasures(m_pData->m_pPTSimCases, m_pData->m_pPTMeasure, *pMeasureList);

    //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
    for (k=0; k <= m_pParameters->GetNumTotalEllipses(); k++) {
       for (i=0; i < m_pData->m_nGridTracts; i++) {
          C_ST.Initialize(i);
          for (tract_t j=1; j<=m_pData->m_NeighborCounts[k][i]; j++) {
             C_ST.AddNeighbor(k, *m_pData, m_pData->m_pSimCases, j);
             C_ST.ComputeBestMeasures(*pMeasureList);
          }
       }
       pMeasureList->SetForNextIteration(k);
    }
    dMaxLogLikelihoodRatio = pMeasureList->GetMaximumLogLikelihoodRatio();
    delete pMeasureList;
  }
  catch (ZdException & x) {
    delete pMeasureList;
    x.AddCallpath("MonteCarloProspective()", "C_ST_PT_Analysis");
    throw;
  }
  return dMaxLogLikelihoodRatio;
}
