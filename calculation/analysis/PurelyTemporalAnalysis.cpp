#include "SaTScan.h"
#pragma hdrstop
#include "PurelyTemporalAnalysis.h"

/** Constructor */
CPurelyTemporalAnalysis::CPurelyTemporalAnalysis(CParameters*  pParameters, CSaTScanData* pData, BasePrint *pPrintDirection)
                        :CAnalysis(pParameters, pData, pPrintDirection) {
  Init();
}

/** Destructor */
CPurelyTemporalAnalysis::~CPurelyTemporalAnalysis() {
  try {
    delete gpTopCluster;
  }
  catch (...){}
}

/** Returns cluster centered at grid point nCenter, with the greatest loglikelihood.
    Caller is responsible for deleting returned cluster. */
void CPurelyTemporalAnalysis::CalculateTopCluster(tract_t tCenter, const DataStreamGateway & DataGateway, bool bSimulation) {
  IncludeClustersType                   eIncludeClustersType;

  try {
    // if Prospective Space-Time then Alive Clusters Only.
    if (m_pParameters->GetAnalysisType() == PROSPECTIVEPURELYTEMPORAL)
      eIncludeClustersType = (bSimulation ? ALLCLUSTERS : ALIVECLUSTERS);
    else
      eIncludeClustersType = m_pParameters->GetIncludeClustersType();

    gpTopCluster = new CPurelyTemporalCluster(DataGateway, eIncludeClustersType, *m_pData, *gpPrintDirection);
    CPurelyTemporalCluster thisCluster(DataGateway, eIncludeClustersType, *m_pData, *gpPrintDirection);
    
    thisCluster.SetRate(m_pParameters->GetAreaScanRateType());
    thisCluster.CompareTopCluster(*gpTopCluster, *m_pData);
  }
  catch (ZdException &x) {
    delete gpTopCluster; gpTopCluster=0;
    x.AddCallpath("CalculateTopCluster()","CPurelyTemporalAnalysis");
    throw;
  }
}

/** Finds top cluster. */
bool CPurelyTemporalAnalysis::FindTopClusters() {
  DataStreamGateway   * pDataStreamGateway=0;

  try {
    gpPrintDirection->SatScanPrintf("Finding the most likely clusters\n");
    pDataStreamGateway = m_pData->GetDataStreamHandler().GetNewDataGateway();
    CalculateTopCluster(0, *pDataStreamGateway, false);
    if (gpTopCluster->ClusterDefined()) {
      ++m_nClustersRetained;
      m_pTopClusters[0] = gpTopCluster->Clone();
      m_pTopClusters[0]->SetStartAndEndDates(m_pData->GetTimeIntervalStartTimes(), m_pData->m_nTimeIntervals);
    }
    else {
      gpTopCluster; gpTopCluster=0;
    }
    delete pDataStreamGateway; pDataStreamGateway=0;
  }
  catch (ZdException &x) {
    delete pDataStreamGateway;
    x.AddCallpath("FindTopClusters()","CPurelyTemporalAnalysis");
    throw;
  }
  return true;
}

/** Returns loglikelihood for Monte Carlo replication. */
double CPurelyTemporalAnalysis::MonteCarlo(const DataStreamInterface & Interface) {
  CMeasureList        * pMeasureList=0;
  double                dMaxLogLikelihoodRatio;

  try {
    CPurelyTemporalCluster C(Interface, m_pParameters->GetIncludeClustersType(), *m_pData, *gpPrintDirection);
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
    C.ComputeBestMeasures(*m_pData, *pMeasureList);

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
double CPurelyTemporalAnalysis::MonteCarloProspective(const DataStreamInterface & Interface) {
  CMeasureList                * pMeasureList=0;
  double                        dMaxLogLikelihoodRatio;

  try {
    CPurelyTemporalCluster C(Interface, ALLCLUSTERS, *m_pData, *gpPrintDirection);
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
    C.ComputeBestMeasures(*m_pData, *pMeasureList);

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
