#include "SaTScan.h"
#pragma hdrstop
#include "PurelySpatialProspectiveCluster.h"

/** constructor */
CPurelySpatialProspectiveCluster::CPurelySpatialProspectiveCluster(BasePrint *pPrintDirection, int nTotalIntervals)
                   :CCluster(pPrintDirection), m_nTotalIntervals(nTotalIntervals) {
  try {
    Init();
    Setup();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CPurelySpatialProspectiveCluster");
    throw;
  }
}

/** destructor */
CPurelySpatialProspectiveCluster::~CPurelySpatialProspectiveCluster() {
  try {
    free(m_pCumCases);
    free(m_pCumMeasure);
  }
  catch(...){}  	
}

/** returns newly cloned CPurelySpatialCluster */
CPurelySpatialProspectiveCluster * CPurelySpatialProspectiveCluster::Clone() const {
  //Note: Replace this code with copy constructor...
  CPurelySpatialProspectiveCluster* pClone = new CPurelySpatialProspectiveCluster(gpPrintDirection, m_nTotalIntervals);
  *pClone = *this;
  return pClone;
}

/** overloaded assignment operator */
CPurelySpatialProspectiveCluster& CPurelySpatialProspectiveCluster::operator =(const CPurelySpatialProspectiveCluster& cluster) {
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
  memcpy(m_pCumCases, cluster.m_pCumCases, m_nTotalIntervals*sizeof(count_t));
  memcpy(m_pCumMeasure, cluster.m_pCumMeasure, m_nTotalIntervals*sizeof(measure_t));
  m_nSteps           = cluster.m_nSteps;
  m_bClusterInit   = cluster.m_bClusterInit;
  m_bClusterDefined= cluster.m_bClusterDefined;
  m_bClusterSet    = cluster.m_bClusterSet;
  m_bLogLSet       = cluster.m_bLogLSet;
  m_bRatioSet      = cluster.m_bRatioSet;
  m_nClusterType   = cluster.m_nClusterType;
  m_iEllipseOffset = cluster.m_iEllipseOffset;
  return *this;
}

/** Adds cases and measure for neighbor at tract_t to cumulated arrays. */
void CPurelySpatialProspectiveCluster::AddNeighbor(int iEllipse, const CSaTScanData& Data, count_t** pCases, tract_t n) {
  int i;

  m_nTracts = n;
  tract_t nNeighbor = Data.GetNeighbor(iEllipse, m_Center, n);
  for (i=0; i<m_nTotalIntervals; i++) {
     m_pCumCases[i]   += pCases[i][nNeighbor];
     m_pCumMeasure[i] += Data.m_pMeasure[i][nNeighbor];
  }
  m_bClusterDefined = true;
}

/** prints time window*/
void CPurelySpatialProspectiveCluster::DisplayTimeFrame(FILE* fp, char* szSpacesOnLeft, int nAnalysisType) {
  ZdGenerateException("DisplayTimeFrame() not implemented. CPurelySpatialProspectiveCluster's current implementation\n"
                      "only meant for use in simulations.","CPurelySpatialProspectiveCluster");
}

/** Returns the number of case for tract as defined by cluster. */
count_t CPurelySpatialProspectiveCluster::GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data) const {
  ZdGenerateException("GetCaseCountForTract() not implemented. CPurelySpatialProspectiveCluster's current implementation\n"
                      "only meant for use in simulations.","CPurelySpatialProspectiveCluster");
  return 0;
}

/** Returns the measure for tract as defined by cluster. */
measure_t CPurelySpatialProspectiveCluster::GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const {
  ZdGenerateException("GetMeasureForTract() not implemented. CPurelySpatialProspectiveCluster's current implementation\n"
                      "only meant for use in simulations.","CPurelySpatialProspectiveCluster");
  return 0;
}

/** initializes cluster for centroid. */
void CPurelySpatialProspectiveCluster::Initialize(tract_t nCenter=0) {
  CCluster::Initialize(nCenter);

  m_nSteps     = 1;
  m_nClusterType = PURELYSPATIAL;
  memset(m_pCumCases, 0, sizeof(count_t) * m_nTotalIntervals);
  memset(m_pCumMeasure, 0, sizeof(measure_t) * m_nTotalIntervals);  
}

void CPurelySpatialProspectiveCluster::SetStartAndEndDates(const Julian* pIntervalStartTimes, int nTimeIntervals) {
  ZdGenerateException("SetStartAndEndDates() not implemented. CPurelySpatialProspectiveCluster's current implementation\n"
                      "only meant for use in simulations.","CPurelySpatialProspectiveCluster");
}

/** determines cases and measure for cluster for current end date */
void CPurelySpatialProspectiveCluster::SetTimeIntervalEndDate(int iTimeIntervalEndDateIndex) {
  if (iTimeIntervalEndDateIndex == m_nTotalIntervals) {
    m_nCases   = m_pCumCases[0];
    m_nMeasure = m_pCumMeasure[0];
  }
  else {
    m_nCases   = m_pCumCases[0] - m_pCumCases[iTimeIntervalEndDateIndex];
    m_nMeasure = m_pCumMeasure[0] - m_pCumMeasure[iTimeIntervalEndDateIndex];
  }
}

/** internal setup function */
void CPurelySpatialProspectiveCluster::Setup() {
  try {
    m_pCumCases   = (count_t*) Smalloc((m_nTotalIntervals)*sizeof(count_t), gpPrintDirection);
    m_pCumMeasure = (measure_t*) Smalloc((m_nTotalIntervals)*sizeof(measure_t), gpPrintDirection);
    Initialize(0);
  }
  catch (ZdException &x) {
    free(m_pCumCases); m_pCumCases=0;
    free(m_pCumMeasure); m_pCumMeasure=0;
    x.AddCallpath("Setup()","CPurelySpatialProspectiveCluster");
    throw;
  }
}
