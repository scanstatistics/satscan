#include "SaTScan.h"
#pragma hdrstop
#include "SpaceTimeIncludePureAnalysis.h"

/** Constructor */
C_ST_PS_PT_Analysis::C_ST_PS_PT_Analysis(CParameters*  pParameters, CSaTScanData* pData, BasePrint *pPrintDirection)
                    :C_ST_PS_Analysis(pParameters, pData, pPrintDirection) {
  Init();                  
}

/** Desctructor */
C_ST_PS_PT_Analysis::~C_ST_PS_PT_Analysis() {
  try {
    delete gpTopCluster;
  }
  catch(...){}  
}

void C_ST_PS_PT_Analysis::CalculatePurelyTemporalCluster(const DataStreamGateway & DataGateway, bool bSimulation) {
  IncludeClustersType                   eIncludeClustersType;

  try {
    // if Prospective Space-Time then Alive Clusters Only.
    if (m_pParameters->GetAnalysisType() == PROSPECTIVESPACETIME)
      eIncludeClustersType = (bSimulation ? ALLCLUSTERS : ALIVECLUSTERS);
    else
      eIncludeClustersType = m_pParameters->GetIncludeClustersType();

    delete gpTopCluster; gpTopCluster=0;
    gpTopCluster = new CPurelyTemporalCluster(DataGateway, eIncludeClustersType, *m_pData, *gpPrintDirection);
    CPurelyTemporalCluster C_PT(DataGateway, eIncludeClustersType, *m_pData, *gpPrintDirection);

    C_PT.SetRate(m_pParameters->GetAreaScanRateType());
    C_PT.CompareTopCluster(*gpTopCluster, *m_pData);
    gpTopCluster->SetStartAndEndDates(m_pData->GetTimeIntervalStartTimes(), m_pData->m_nTimeIntervals);
  }
  catch (ZdException & x) {
    delete gpTopCluster;
    x.AddCallpath("CalculateTopCluster()", "C_ST_PS_PT_Analysis");
    throw;
  }
}

/** Returns cluster centered at grid point nCenter, with the greatest loglikelihood.
    Caller is responsible for deleting returned cluster. */
void C_ST_PS_PT_Analysis::CalculateTopCluster(tract_t tCenter, const DataStreamGateway & DataGateway, bool bSimulation) {
  try {
    C_ST_PS_Analysis::CalculateTopCluster(tCenter, DataGateway, bSimulation);
    CalculatePurelyTemporalCluster(DataGateway, bSimulation);
  }
  catch (ZdException & x) {
    x.AddCallpath("CalculateTopCluster()", "C_ST_PS_PT_Analysis");
    throw;
  }
}

/** Finds top clusters. */
bool C_ST_PS_PT_Analysis::FindTopClusters() {
  tract_t               tLastClusterIndex;
  int                   i;
  clock_t               tStartTime;
  DataStreamGateway   * pDataStreamGateway=0;

  try {
    DisplayFindClusterHeading();
    pDataStreamGateway = m_pData->GetDataStreamHandler().GetNewDataGateway();
    tStartTime = clock();

    C_ST_PS_Analysis::SetTopClusters(*pDataStreamGateway, false);
    for (i=0; i < m_pData->m_nGridTracts && !gpPrintDirection->GetIsCanceled(); ++i) {
       C_ST_PS_Analysis::CalculateTopCluster(i, *pDataStreamGateway, false);
       m_pTopClusters[i] = C_ST_PS_Analysis::GetTopCalculatedCluster().Clone();
       m_pTopClusters[i]->SetStartAndEndDates(m_pData->GetTimeIntervalStartTimes(), m_pData->m_nTimeIntervals);
       ++m_nClustersRetained;
       if (i==9)
         ReportTimeEstimate(tStartTime, m_pData->m_nGridTracts, i+1, gpPrintDirection);
    }

    if (gpPrintDirection->GetIsCanceled())
      return false;

    RankTopClusters();

    tLastClusterIndex = m_nClustersRetained;
    CalculatePurelyTemporalCluster(*pDataStreamGateway, false);
    if (gpTopCluster->ClusterDefined()) {
      m_pTopClusters[tLastClusterIndex] = gpTopCluster->Clone();
      ++m_nClustersRetained;
      SortTopClusters();
    }
    else {
      delete gpTopCluster; gpTopCluster=0;
    }

    delete pDataStreamGateway; pDataStreamGateway=0;
  }
  catch (ZdException &x) {
    delete pDataStreamGateway;
    x.AddCallpath("FindTopClusters()","C_ST_PS_PT_Analysis");
    throw;
  }
  return true;
}

CCluster & C_ST_PS_PT_Analysis::GetTopCalculatedCluster() {
  CCluster & ST_PS_PTCLuster = C_ST_PS_Analysis::GetTopCalculatedCluster();
  if (!gpTopCluster->ClusterDefined())
    return ST_PS_PTCLuster;
  else if (gpTopCluster->m_nRatio > ST_PS_PTCLuster.m_nRatio)
    return *gpTopCluster;
  else
    return ST_PS_PTCLuster;
}

/** Returns loglikelihood for Monte Carlo replication. */
double C_ST_PS_PT_Analysis::MonteCarlo(const DataStreamInterface & Interface) {
  CMeasureList        * pMeasureList=0;
  double                dMaxLogLikelihoodRatio;
  int                   k;
  tract_t               i, j, iNumNeighbors;

  try {
    CPurelySpatialCluster C_PS(*m_pData, gpPrintDirection);
    CSpaceTimeCluster     C_ST(m_pParameters->GetIncludeClustersType(), *m_pData, *gpPrintDirection);
    CPurelyTemporalCluster C_PT(Interface, m_pParameters->GetIncludeClustersType(), *m_pData, *gpPrintDirection);
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
    C_PT.ComputeBestMeasures(*m_pData, *pMeasureList);

    //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
    for (k=0; k <= m_pParameters->GetNumTotalEllipses(); k++) {  
       for (i=0; i < m_pData->m_nGridTracts; i++) {
          C_PS.Initialize(i);
          C_ST.Initialize(i);
          iNumNeighbors = m_pData->GetNeighborCountArray()[k][i];
          for (j=1; j <= iNumNeighbors; j++) {
             C_PS.AddNeighborData(m_pData->GetNeighbor(k, i, j), Interface);
             C_PS.ComputeBestMeasures(*pMeasureList);
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
    x.AddCallpath("MonteCarlo()", "C_ST_PS_PT_Analysis");
    throw;
  }
  return dMaxLogLikelihoodRatio;
}

/** Returns loglikelihood for Monte Carlo Prospective replication. */
double C_ST_PS_PT_Analysis::MonteCarloProspective(const DataStreamInterface & Interface) {
  CMeasureList        * pMeasureList=0;
  double                dMaxLogLikelihoodRatio;
  int                   k;
  tract_t               i, j, iNumNeighbors;

  try {
    //for prospective Space-Time, GetAliveClustersOnly() should be false..
    //m_bAliveClustersOnly is the first parameter into the CSpaceTimeCluster class
    CPurelySpatialProspectiveCluster C_PS(*m_pData, gpPrintDirection);
    CSpaceTimeCluster     C_ST(ALLCLUSTERS, *m_pData, *gpPrintDirection);
    CPurelyTemporalCluster C_PT(Interface, ALLCLUSTERS, *m_pData, *gpPrintDirection);
    
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
    C_PT.ComputeBestMeasures(*m_pData, *pMeasureList);

    //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
    for (k=0; k <= m_pParameters->GetNumTotalEllipses(); ++k) {
       for (i=0; i < m_pData->m_nGridTracts; i++) {
          C_PS.Initialize(i);
          C_ST.Initialize(i);
          iNumNeighbors = m_pData->GetNeighborCountArray()[k][i]; 
          for (tract_t j=1; j <= iNumNeighbors ; ++j) {
             C_PS.AddNeighborData(m_pData->GetNeighbor(k, i, j), Interface);
             C_PS.ComputeBestMeasures(*pMeasureList);
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
    x.AddCallpath("MonteCarloProspective()", "C_ST_PS_PT_Analysis");
    throw;
  }
  return dMaxLogLikelihoodRatio;
}

