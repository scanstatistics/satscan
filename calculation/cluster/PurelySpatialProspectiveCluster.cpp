#include "SaTScan.h"
#pragma hdrstop
#include "PurelySpatialProspectiveCluster.h"
#include "MeasureList.h"

/** Purely spatial cluster to be used in simulations of prospective analysis.*/


CPurelySpatialProspectiveCluster::CPurelySpatialProspectiveCluster(const CSaTScanData & Data, BasePrint *pPrintDirection)
                                 :CCluster(pPrintDirection) {
  try {
    Init();
    Setup(Data);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CPurelySpatialProspectiveCluster");
    throw;
  }
}

CPurelySpatialProspectiveCluster::CPurelySpatialProspectiveCluster(const CPurelySpatialProspectiveCluster& rhs)
                                 :CCluster(rhs.gpPrintDirection) {
  try {
    Init();
    Setup(rhs);
    *this = rhs;
  }
  catch (ZdException &x) {
    x.AddCallpath("copy constructor()","CPurelySpatialProspectiveCluster");
    throw;
  }
}

CPurelySpatialProspectiveCluster::~CPurelySpatialProspectiveCluster() {
  try {
    free(m_pCumCases);
    free(m_pCumMeasure);
  }
  catch(...){}
}

CPurelySpatialProspectiveCluster& CPurelySpatialProspectiveCluster::operator=(const CPurelySpatialProspectiveCluster& cluster) {
  m_Center         = cluster.m_Center;
  m_nCases         = cluster.m_nCases ;
  m_nMeasure       = cluster.m_nMeasure;
  m_nTracts        = cluster.m_nTracts;
  m_nRatio         = cluster.m_nRatio;
  m_nLogLikelihood = cluster.m_nLogLikelihood;
  m_nRank          = cluster.m_nRank;
  m_DuczmalCorrection = cluster.m_DuczmalCorrection;
  m_nFirstInterval = cluster.m_nFirstInterval;
  m_nLastInterval  = cluster.m_nLastInterval;
  m_nStartDate     = cluster.m_nStartDate;
  m_nEndDate       = cluster.m_nEndDate;
  m_nTotalIntervals = cluster.m_nTotalIntervals;
  memcpy(m_pCumCases, cluster.m_pCumCases, m_nNumIntervals*sizeof(count_t));
  memcpy(m_pCumMeasure, cluster.m_pCumMeasure, m_nNumIntervals*sizeof(measure_t));
  m_nSteps           = cluster.m_nSteps;
  m_bClusterInit   = cluster.m_bClusterInit;
  m_bClusterDefined= cluster.m_bClusterDefined;
  m_bClusterSet    = cluster.m_bClusterSet;
  m_bLogLSet       = cluster.m_bLogLSet;
  m_bRatioSet      = cluster.m_bRatioSet;
  m_nClusterType   = cluster.m_nClusterType;
  m_iEllipseOffset = cluster.m_iEllipseOffset;
  m_nProspectiveStartInterval = cluster.m_nProspectiveStartInterval;
  m_nNumIntervals  = cluster.m_nNumIntervals;     
  return *this;
}

void CPurelySpatialProspectiveCluster::AddNeighbor(int iEllipse, const CSaTScanData& Data, count_t** pCases, tract_t n) {
  int   i, j;

  m_nTracts = n;
  tract_t nNeighbor = Data.GetNeighbor(iEllipse, m_Center, n);

  //set cases for entire period added by this neighbor
  m_pCumCases[0]   += pCases[0][nNeighbor];
  m_pCumMeasure[0] += Data.m_pMeasure[0][nNeighbor];

  for (j=1, i=m_nProspectiveStartInterval; i < m_nTotalIntervals; j++, i++) {
      m_pCumCases[j]   += pCases[i][nNeighbor];
      m_pCumMeasure[j] += Data.m_pMeasure[i][nNeighbor];
  }
}

/** returns newly cloned CPurelySpatialCluster */
CPurelySpatialProspectiveCluster * CPurelySpatialProspectiveCluster::Clone() const {
  return new CPurelySpatialProspectiveCluster(*this);
}

void CPurelySpatialProspectiveCluster::ComputeBestMeasures(CMeasureList & MeasureList) {
  int   iWindowEnd;

  for (iWindowEnd=1; iWindowEnd < m_nNumIntervals; ++iWindowEnd)
     MeasureList.AddMeasure(m_pCumCases[0] - m_pCumCases[iWindowEnd], m_pCumMeasure[0] - m_pCumMeasure[iWindowEnd]);

  MeasureList.AddMeasure(m_pCumCases[0], m_pCumMeasure[0]);
}

/** Returns the number of case for tract as defined by cluster. */
count_t CPurelySpatialProspectiveCluster::GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data) const {
  ZdGenerateException("GetCaseCountForTract() not implemented.","CPurelySpatialProspectiveCluster");
  return 0;
}

/** Returns the measure for tract as defined by cluster. */
measure_t CPurelySpatialProspectiveCluster::GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const {
  ZdGenerateException("GetMeasureForTract() not implemented.","CPurelySpatialProspectiveCluster");
  return 0;
}

void CPurelySpatialProspectiveCluster::Initialize(tract_t nCenter = 0) {
  CCluster::Initialize(nCenter);
  m_nClusterType = PURELYSPATIAL/*PROSPECTIVE*/;
  memset(m_pCumCases, 0, sizeof(count_t) * m_nNumIntervals);
  memset(m_pCumMeasure, 0, sizeof(measure_t) * m_nNumIntervals);
}

void CPurelySpatialProspectiveCluster::SetStartAndEndDates(const Julian* pIntervalStartTimes, int nTimeIntervals) {
  ZdGenerateException("SetStartAndEndDates() not implemented.","CPurelySpatialProspectiveCluster");
}

/** internal setup function */
void CPurelySpatialProspectiveCluster::Setup(const CSaTScanData & Data) {
  try {
    m_nNumIntervals = 1 + Data.m_nTimeIntervals - Data.m_nProspectiveIntervalStart;
    m_nTotalIntervals = Data.m_nTimeIntervals;
    m_nProspectiveStartInterval = Data.m_nProspectiveIntervalStart;
    m_pCumCases   = (count_t*) Smalloc((m_nNumIntervals)*sizeof(count_t), gpPrintDirection);
    m_pCumMeasure = (measure_t*) Smalloc((m_nNumIntervals)*sizeof(measure_t), gpPrintDirection);
    Initialize(0);
  }
  catch (ZdException &x) {
    free(m_pCumCases);
    free(m_pCumMeasure);
    x.AddCallpath("Setup()","CPurelySpatialProspectiveCluster");
    throw;
  }
}

/** internal setup function */
void CPurelySpatialProspectiveCluster::Setup(const CPurelySpatialProspectiveCluster& rhs) {
  try {
    m_pCumCases   = (count_t*) Smalloc((rhs.m_nNumIntervals)*sizeof(count_t), rhs.gpPrintDirection);
    m_pCumMeasure = (measure_t*) Smalloc((rhs.m_nNumIntervals)*sizeof(measure_t), rhs.gpPrintDirection);
  }
  catch (ZdException &x) {
    free(m_pCumCases);
    free(m_pCumMeasure);
    x.AddCallpath("Setup()","CPurelySpatialProspectiveCluster");
    throw;
  }
}
