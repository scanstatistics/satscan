#include "SaTScan.h"
#pragma hdrstop
#include "SpaceTimeCluster.h"

/** constructor */
CSpaceTimeCluster::CSpaceTimeCluster(IncludeClustersType eIncludeClustersType, const CSaTScanData & Data, BasePrint & PrintDirection)
                  :CCluster(&PrintDirection) {
  try {
    Init();
    Setup(eIncludeClustersType, Data);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CSpaceTimeCluster");
    throw;
  }
}

/** copy constructor */
CSpaceTimeCluster::CSpaceTimeCluster(const CSpaceTimeCluster& rhs)
                  : CCluster(rhs.gpPrintDirection) {
  try {
    Init();
    Setup(rhs);
    *this = rhs;
  }
  catch (ZdException &x) {
    x.AddCallpath("copy constructor()","CSpaceTimeCluster");
    throw;
  }
}

/** destructor */
CSpaceTimeCluster::~CSpaceTimeCluster() {
  try {
    delete TI;
    DeAllocCumulativeCounts();
  }
  catch(...){}
} 

CSpaceTimeCluster& CSpaceTimeCluster::operator =(const CSpaceTimeCluster& cluster) {
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
  m_nIntervalCut    = cluster.m_nIntervalCut;
  m_nTIType         = cluster.m_nTIType;
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
  *TI = *(cluster.TI);
  return *this;
}

void CSpaceTimeCluster::AddNeighbor(int iEllipse, const CSaTScanData& Data, count_t** pCases, tract_t n) {
  int i;

  m_nTracts = n;
  tract_t nNeighbor = Data.GetNeighbor(iEllipse, m_Center, n);

  for (i=0; i<m_nTotalIntervals; i++) {
      m_pCumCases[i]   += pCases[i][nNeighbor];
      m_pCumMeasure[i] += Data.m_pMeasure[i][nNeighbor];
  }
}

/** returns newly cloned CSpaceTimeCluster */
CSpaceTimeCluster * CSpaceTimeCluster::Clone() const {
  return new CSpaceTimeCluster(*this);;
}

void CSpaceTimeCluster::CompareTopCluster(CSpaceTimeCluster & TopShapeCluster, const CSaTScanData & Data) {
  m_bClusterDefined = true;
  TI->CompareClusters(*this, TopShapeCluster, Data, m_pCumCases, m_pCumMeasure);
}

void CSpaceTimeCluster::ComputeBestMeasures(CMeasureList & MeasureList) {
  TI->ComputeBestMeasures(m_pCumCases, m_pCumMeasure,MeasureList);
}

void CSpaceTimeCluster::DeAllocCumulativeCounts()
{
  if (m_pCumCases != NULL)
  {
    free(m_pCumCases);
    m_pCumCases = NULL;
  }

  if (m_pCumMeasure != NULL)
  {
    free(m_pCumMeasure);
    m_pCumMeasure = NULL;
  }
}

/** Returns the number of case for tract as defined by cluster. */
count_t CSpaceTimeCluster::GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data) const
{
  return TI->GetCaseCountForTract(*this, tTract, Data.m_pCases);
}

/** Returns the measure for tract as defined by cluster. */
measure_t CSpaceTimeCluster::GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const
{
  return Data.GetMeasureAdjustment() * TI->GetMeasureForTract(*this, tTract, Data.m_pMeasure);
}

void CSpaceTimeCluster::Initialize(tract_t nCenter = 0) {
  CCluster::Initialize(nCenter);
  m_nClusterType = SPACETIME;
  memset(m_pCumCases, 0, sizeof(count_t) * m_nTotalIntervals);
  memset(m_pCumMeasure, 0, sizeof(measure_t) * m_nTotalIntervals);
}

/** internal setup function */
void CSpaceTimeCluster::Setup(IncludeClustersType eIncludeClustersType, const CSaTScanData & Data) {
  try {
    m_nTotalIntervals = Data.m_nTimeIntervals;
    m_nIntervalCut = Data.m_nIntervalCut;
    m_nTIType = eIncludeClustersType;

    switch (m_nTIType) {
      case ALLCLUSTERS     : //TI = new CTIAll(m_nTotalIntervals, m_nIntervalCut);   break;
      case CLUSTERSINRANGE : TI = new TimeIntervalRange(Data); break;
      case ALIVECLUSTERS   : TI = new CTIAlive(m_nTotalIntervals, m_nIntervalCut); break;
      default : ZdGenerateException("Unknown clusters type: '%d'.","Setup()", m_nTIType);
    }
    m_pCumCases   = (count_t*) Smalloc((m_nTotalIntervals)*sizeof(count_t), gpPrintDirection);
    m_pCumMeasure = (measure_t*) Smalloc((m_nTotalIntervals)*sizeof(measure_t), gpPrintDirection);
    Initialize(0);
  }
  catch (ZdException &x) {
    delete TI;
    free(m_pCumCases);
    free(m_pCumMeasure);
    x.AddCallpath("Setup()","CSpaceTimeCluster");
    throw;
  }
}

/** internal setup function */
void CSpaceTimeCluster::Setup(const CSpaceTimeCluster& rhs) {
  try {
    TI = rhs.TI->Clone();
    m_pCumCases   = (count_t*) Smalloc((rhs.m_nTotalIntervals)*sizeof(count_t), rhs.gpPrintDirection);
    m_pCumMeasure = (measure_t*) Smalloc((rhs.m_nTotalIntervals)*sizeof(measure_t), rhs.gpPrintDirection);
  }
  catch (ZdException &x) {
    delete TI;
    free(m_pCumCases);
    free(m_pCumMeasure);
    x.AddCallpath("Setup()","CSpaceTimeCluster");
    throw;
  }
}

//------------------------------------------------------------------------------
/*void CSpaceTimeCluster::Display(FILE* pFile)
{
  fprintf(pFile, "\nCenter Grid Point #%i\n", m_Center);
  fprintf(pFile, "Cases=%i, Measure=%f, Tracts=%i, Ratio=%f, Rank=%i\n",
          m_nCases, m_nMeasure, m_nTracts, m_nRatio, m_nRank);
  fprintf(pFile, "Start Interval=%i, Last Interval=%i\n\n",
          m_nFirstInterval, m_nLastInterval);
}
*/
