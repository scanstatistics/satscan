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
    if (m_pTopClusters[nLastClusterIndex]->ClusterDefined()) {
      m_pTopClusters[nLastClusterIndex]->SetStartAndEndDates(m_pData->GetTimeIntervalStartTimes(), m_pData->m_nTimeIntervals);
      m_nClustersRetained++;
      SortTopClusters();
    }
    else {
      delete m_pTopClusters[nLastClusterIndex]; m_pTopClusters[nLastClusterIndex]=0;
    }
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
  DataStreamGateway           * pDataStreamGateway=0;

  try {
    pDataStreamGateway = m_pData->GetDataStreamHandler().GetNewDataGateway();

    // if Prospective Space-Time then Alive Clusters Only.
    if (m_pParameters->GetAnalysisType() == PROSPECTIVESPACETIME)
      eIncludeClustersType = ALIVECLUSTERS;
    else
      eIncludeClustersType = m_pParameters->GetIncludeClustersType();

    pTopCluster = new CPurelyTemporalCluster(*pDataStreamGateway, eIncludeClustersType, *m_pData, *gpPrintDirection);
    CPurelyTemporalCluster C_PT(*pDataStreamGateway, eIncludeClustersType, *m_pData, *gpPrintDirection);

    C_PT.SetRate(m_pParameters->GetAreaScanRateType());
    C_PT.CompareTopCluster(*pTopCluster, *m_pData);
    delete pDataStreamGateway; pDataStreamGateway=0;
  }
  catch (ZdException & x) {
    delete pTopCluster;
    delete pDataStreamGateway;
    x.AddCallpath("GetTopPTCluster()", "C_ST_PT_Analysis");
    throw;
  }
  return pTopCluster;
}

/** Returns loglikelihood for Monte Carlo replication. */
double C_ST_PT_Analysis::MonteCarlo(const DataStreamInterface & Interface) {
  CMeasureList        * pMeasureList=0;
  double                dMaxLogLikelihoodRatio;
  int                   k;
  tract_t               i, j, iNumNeighbors;

  try {
    CPurelyTemporalCluster C_PT(Interface, m_pParameters->GetIncludeClustersType(), *m_pData, *gpPrintDirection);
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
    C_PT.ComputeBestMeasures(*m_pData, *pMeasureList);

    //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
    for (k=0; k <= m_pParameters->GetNumTotalEllipses(); k++) {
       for (i=0; i < m_pData->m_nGridTracts; i++) {
          C_ST.Initialize(i);
          iNumNeighbors = m_pData->GetNeighborCountArray()[k][i];
          for (j=1; j <= iNumNeighbors; j++) {
             C_ST.AddNeighborData(m_pData->GetNeighbor(k, i, j), Interface);
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
double C_ST_PT_Analysis::MonteCarloProspective(const DataStreamInterface & Interface) {
  CMeasureList        * pMeasureList=0;
  double                dMaxLogLikelihoodRatio;
  int                   k;
  tract_t               i, j, iNumNeighbors;

  try {
    //for prospective Space-Time, m_bAliveClustersOnly should be false..
    //m_bAliveClustersOnly is the first parameter into the CSpaceTimeCluster class
    CPurelyTemporalCluster C_PT(Interface, ALLCLUSTERS, *m_pData, *gpPrintDirection);
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
    C_PT.ComputeBestMeasures(*m_pData, *pMeasureList);

    //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
    for (k=0; k <= m_pParameters->GetNumTotalEllipses(); k++) {
       for (i=0; i < m_pData->m_nGridTracts; i++) {
          C_ST.Initialize(i);
          iNumNeighbors = m_pData->GetNeighborCountArray()[k][i];
          for (tract_t j=1; j <= iNumNeighbors ; j++) {
             C_ST.AddNeighborData(m_pData->GetNeighbor(k, i, j), Interface);
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
