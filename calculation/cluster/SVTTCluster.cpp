#include "SaTScan.h"
#pragma hdrstop
#include "SVTTCluster.h"

/** constructor */
CSVTTCluster::CSVTTCluster(const CSaTScanData& Data, const count_t* pCasesByTimeInt, BasePrint *pPrintDirection)
             :CCluster(pPrintDirection){
  try {
    Init();
    Setup(Data, pCasesByTimeInt);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CSVTTCluster");
    throw;
  }
}

/** copy constructor */
CSVTTCluster::CSVTTCluster(const CSVTTCluster & rhs): CCluster(rhs.gpPrintDirection) {
  try {
    Init();
    Setup(rhs);
    *this = rhs;
  }
  catch (ZdException &x) {
    x.AddCallpath("copy constructor()","CSVTTCluster");
    throw;
  }
}

/** destructor */
CSVTTCluster::~CSVTTCluster() {
  try {
    free(m_pCumCases);
    free(m_pCumMeasure);
    free(m_pRemCases);
    free(m_pRemMeasure);
  }
  catch (...){}
}

/** overloaded assignment operator */
CSVTTCluster& CSVTTCluster::operator =(const CSVTTCluster& rhs) {
  if (this == &rhs)
    return *this;
  m_Center         = rhs.m_Center;
  m_nCases         = rhs.m_nCases;
  m_nMeasure       = rhs.m_nMeasure;
  m_nTracts        = rhs.m_nTracts;
  m_nRatio         = rhs.m_nRatio;
  m_nLogLikelihood = rhs.m_nLogLikelihood;
  m_nRank          = rhs.m_nRank;
  m_nFirstInterval = rhs.m_nFirstInterval;
  m_nLastInterval  = rhs.m_nLastInterval;
  m_nStartDate     = rhs.m_nStartDate;
  m_nEndDate       = rhs.m_nEndDate;
  m_nTotalIntervals = rhs.m_nTotalIntervals;
  m_nRemCases       = rhs.m_nRemCases;
  memcpy(m_pCumCases, rhs.m_pCumCases, m_nTotalIntervals * sizeof(count_t));
  memcpy(m_pCumMeasure, rhs.m_pCumMeasure, m_nTotalIntervals * sizeof(measure_t));
  memcpy(m_pRemCases, rhs.m_pRemCases, m_nTotalIntervals * sizeof(count_t));
  memcpy(m_pRemMeasure, rhs.m_pRemMeasure, m_nTotalIntervals * sizeof(measure_t));
  m_nSteps           = rhs.m_nSteps;
  m_bClusterInit   = rhs.m_bClusterInit;
  m_bClusterDefined= rhs.m_bClusterDefined;
  m_bClusterSet    = rhs.m_bClusterSet;
  m_bLogLSet       = rhs.m_bLogLSet;
  m_bRatioSet      = rhs.m_bRatioSet;
  m_nClusterType   = rhs.m_nClusterType;
  m_nTimeTrend     = rhs.m_nTimeTrend;
  m_nRemTimeTrend  = rhs.m_nRemTimeTrend;

  return *this;
}

void CSVTTCluster::AddNeighbor(int iEllipse, const CSaTScanData& Data, count_t** pCases, tract_t n) {
  m_nTracts = n;
  tract_t nNeighbor = Data.GetNeighbor(iEllipse, m_Center, n);

  for (int i=0; i<m_nTotalIntervals; i++)  {
    m_pCumCases[i]   += pCases[i][nNeighbor];
    m_pCumMeasure[i] += Data.m_pMeasure_NC[i][nNeighbor];
    m_pRemCases[i]   -= pCases[i][nNeighbor];
    m_pRemMeasure[i] -= Data.m_pMeasure_NC[i][nNeighbor];

    m_nCases         += pCases[i][nNeighbor];
    m_nMeasure       += Data.m_pMeasure_NC[i][nNeighbor];
    m_nRemCases      -= pCases[i][nNeighbor];
  }

  m_bClusterDefined = true; // KR990421 - What about this? PS-Yes, ST-No?
}

/** returns newly cloned CSVTTCluster */
CSVTTCluster * CSVTTCluster::Clone() const {
  return new CSVTTCluster(*this);
}

void CSVTTCluster::DisplayAnnualTimeTrendWithoutTitle(FILE* fp) {
  if (m_nTimeTrend.IsNegative())
    fprintf(fp, "     -");
  else
    fprintf(fp, "      ");

  fprintf(fp, "%.3f", m_nTimeTrend.GetAnnualTimeTrend());
}

void CSVTTCluster::DisplayTimeTrend(FILE* fp, char* szSpacesOnLeft) {
  fprintf(fp, "%sTime trend............: %f  (%.3f%% ",
              szSpacesOnLeft, m_nTimeTrend.m_nBeta,
              m_nTimeTrend.GetAnnualTimeTrend());

  if (m_nTimeTrend.IsNegative())
    fprintf(fp, "annual decrease)\n");
  else
    fprintf(fp, "annual increase)\n");
}

/** Returns the number of case for tract as defined by cluster. */
count_t CSVTTCluster::GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data) const
{
  return Data.m_pCases[0][tTract];
}

/** Returns the measure for tract as defined by cluster. */
measure_t CSVTTCluster::GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const {
  return Data.GetMeasureAdjustment() * Data.m_pMeasure[0][tTract];
}

void CSVTTCluster::InitializeSVTT(tract_t nCenter, const CSaTScanData& Data, const count_t* pCasesByTimeInt) {
  CCluster::Initialize(nCenter);

  m_nSteps        = 1;
  m_nClusterType  = SPATIALVARTEMPTREND;

  memset(m_pCumCases, 0, m_nTotalIntervals * sizeof(count_t));
  memset(m_pCumMeasure, 0, m_nTotalIntervals * sizeof(measure_t));
  memcpy(m_pRemCases, pCasesByTimeInt, m_nTotalIntervals * sizeof(count_t));
  memcpy(m_pRemMeasure, Data.m_pMeasure_TotalByTimeInt, m_nTotalIntervals * sizeof(measure_t));

  m_nCases    = 0;
  m_nMeasure  = 0.0;
  m_nRemCases = 0;
  m_nRemCases = Data.m_nTotalCases;

  m_nTimeTrend.Initialize();
  m_nRemTimeTrend.Initialize();
}

void CSVTTCluster::SetStartAndEndDates(const Julian* pIntervalStartTimes, int nTimeIntervals) {
  m_nFirstInterval = 0;
  m_nLastInterval  = nTimeIntervals;
  m_nStartDate     = pIntervalStartTimes[m_nFirstInterval];
  m_nEndDate       = pIntervalStartTimes[m_nLastInterval]-1;
}

/** internal setup function */
void CSVTTCluster::Setup(const CSaTScanData& Data, const count_t* pCasesByTimeInt) {
  try {
    m_nTotalIntervals = Data.m_nTimeIntervals;
    m_pCumCases   = (count_t*) Smalloc((m_nTotalIntervals)*sizeof(count_t), gpPrintDirection);
    m_pCumMeasure = (measure_t*) Smalloc((m_nTotalIntervals)*sizeof(measure_t), gpPrintDirection);
    m_pRemCases   = (count_t*) Smalloc((m_nTotalIntervals)*sizeof(count_t), gpPrintDirection);
    m_pRemMeasure = (measure_t*) Smalloc((m_nTotalIntervals)*sizeof(measure_t), gpPrintDirection);
    InitializeSVTT(0, Data, pCasesByTimeInt);
  }
  catch (ZdException &x) {
    free(m_pCumCases);
    free(m_pCumMeasure);
    free(m_pRemCases);
    free(m_pRemMeasure);
    x.AddCallpath("Setup()","CSVTTCluster");
    throw;
  }
}

/** internal setup function */
void CSVTTCluster::Setup(const CSVTTCluster& rhs) {
  try {
    m_pCumCases   = (count_t*) Smalloc((rhs.m_nTotalIntervals)*sizeof(count_t), gpPrintDirection);
    m_pCumMeasure = (measure_t*) Smalloc((rhs.m_nTotalIntervals)*sizeof(measure_t), gpPrintDirection);
    m_pRemCases   = (count_t*) Smalloc((rhs.m_nTotalIntervals)*sizeof(count_t), gpPrintDirection);
    m_pRemMeasure = (measure_t*) Smalloc((rhs.m_nTotalIntervals)*sizeof(measure_t), gpPrintDirection);
  }
  catch (ZdException &x) {
    free(m_pCumCases);
    free(m_pCumMeasure);
    free(m_pRemCases);
    free(m_pRemMeasure);
    x.AddCallpath("Setup()","CSVTTCluster");
    throw;
  }
}
