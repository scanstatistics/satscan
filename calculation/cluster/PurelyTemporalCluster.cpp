#include "SaTScan.h"
#pragma hdrstop
#include "PurelyTemporalCluster.h"

/** constructor */
CPurelyTemporalCluster::CPurelyTemporalCluster(IncludeClustersType eIncludeClustersType, const CSaTScanData & Data, BasePrint & PrintDirection)
                       :CCluster(&PrintDirection) {
  try {
    Init();
    Setup(eIncludeClustersType, Data);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CPurelyTemporalCluster");
    throw;
  }
}

/** copy constructor */
CPurelyTemporalCluster::CPurelyTemporalCluster(const CPurelyTemporalCluster& rhs)
                       : CCluster(rhs.gpPrintDirection) {
  try {
    Init();
    Setup(rhs);
    *this = rhs;
  }
  catch (ZdException &x) {
    x.AddCallpath("copy constructor()","CPurelyTemporalCluster");
    throw;
  }
}

/** desctructor */
CPurelyTemporalCluster::~CPurelyTemporalCluster() {
  try {
    delete m_TI;
  }
  catch(...){}
}

CPurelyTemporalCluster& CPurelyTemporalCluster::operator=(const CPurelyTemporalCluster& cluster) {
  m_Center         = cluster.m_Center;
  m_nCases         = cluster.m_nCases ;
  m_nMeasure       = cluster.m_nMeasure;
  m_nTracts        = cluster.m_nTracts;
  m_nRatio         = cluster.m_nRatio;
  m_nLogLikelihood = cluster.m_nLogLikelihood;
  m_nRank          = cluster.m_nRank;
  m_nFirstInterval = cluster.m_nFirstInterval;
  m_nLastInterval  = cluster.m_nLastInterval;
  m_nStartDate     = cluster.m_nStartDate;
  m_nEndDate       = cluster.m_nEndDate;
  m_nTotalIntervals = cluster.m_nTotalIntervals;
  m_nIntervalCut    = cluster.m_nIntervalCut;
  m_nTIType         = cluster.m_nTIType;
  m_nSteps          = cluster.m_nSteps;
  m_bClusterInit   = cluster.m_bClusterInit;
  m_bClusterDefined= cluster.m_bClusterDefined;
  m_bClusterSet    = cluster.m_bClusterSet;
  m_bLogLSet       = cluster.m_bLogLSet;
  m_bRatioSet      = cluster.m_bRatioSet;
  m_nClusterType   = cluster.m_nClusterType;
  *m_TI            = *(cluster.m_TI);
  return *this;
}

/** returns newly cloned CPurelyTemporalCluster */
CPurelyTemporalCluster * CPurelyTemporalCluster::Clone() const {
  return new CPurelyTemporalCluster(*this);
}

void CPurelyTemporalCluster::CompareTopCluster(CPurelyTemporalCluster & TopShapeCluster, const CSaTScanData & Data) {
  m_bClusterDefined = true;
  m_TI->CompareClusters(*this, TopShapeCluster, Data, Data.m_pPTCases, Data.m_pPTMeasure);
}

void CPurelyTemporalCluster::ComputeBestMeasures(const count_t* pCases, const measure_t* pMeasure, CMeasureList & MeasureList) {
  m_TI->ComputeBestMeasures(pCases, pMeasure, MeasureList);
}

void CPurelyTemporalCluster::DisplayCensusTracts(FILE* fp, const CSaTScanData& Data, int nCluster,
                                                 measure_t nMinMeasure, int nReplicas,
                                                 long lReportHistoryRunNumber, bool bIncludeRelRisk,
                                                 bool bIncludePVal, int nLeftMargin, int nRightMargin,
                                                 char cDeliminator, char* szSpacesOnLeft, bool bFormat) {
  //const char * szTID;

  try {
    if (nLeftMargin > 0)
      fprintf(fp, "included.: All\n");
    // Code to print tracts for purely temporal clusters
    //else {
    //  for (int i=0; i<Data.m_nTracts; i++) {
    //       szTID = tiGetTid(i);
    //       fprintf(fp, "%i         ", nCluster);
    //       fprintf(fp, "%s\n", szTID);
    //  }
    //}
  }
  catch (ZdException &x) {
    x.AddCallpath("DisplayCensusTracts()","CPurelyTemporalCluster");
    throw;
  }
}

void CPurelyTemporalCluster::Initialize(tract_t nCenter) {
  CCluster::Initialize(nCenter);
  m_nClusterType = PURELYTEMPORAL;
}

/** Returns the number of case for tract as defined by cluster. */
count_t CPurelyTemporalCluster::GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data) const {
  return m_TI->GetCaseCountForTract(*this, tTract, Data.m_pCases);
}

/** Returns the measure for tract as defined by cluster. */
measure_t CPurelyTemporalCluster::GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const {
  return m_TI->GetMeasureForTract(*this, tTract, Data.m_pMeasure);
}

/** internal setup function */
void CPurelyTemporalCluster::Setup(IncludeClustersType eIncludeClustersType, const CSaTScanData & Data) {
  try {
    m_nTotalIntervals = Data.m_nTimeIntervals;
    m_nIntervalCut = Data.m_nIntervalCut;
    m_nTIType = eIncludeClustersType;

    switch (m_nTIType) {
      case ALLCLUSTERS     : //TI = new CTIAll(m_nTotalIntervals, m_nIntervalCut);   break;
      case CLUSTERSINRANGE : m_TI = new TimeIntervalRange(Data); break;
      case ALIVECLUSTERS   : m_TI = new CTIAlive(m_nTotalIntervals, m_nIntervalCut); break;
      default : ZdGenerateException("Unknown clusters type: '%d'.","Setup()", m_nTIType);
    }
    Initialize(0);
  }
  catch (ZdException &x) {
    delete m_TI;
    x.AddCallpath("Setup()","CPurelyTemporalCluster");
    throw;
  }
}

/** internal setup function */
void CPurelyTemporalCluster::Setup(const CPurelyTemporalCluster& rhs) {
  try {
    m_TI = rhs.m_TI->Clone();
  }
  catch (ZdException &x) {
    delete m_TI;
    x.AddCallpath("Setup()","CPurelyTemporalCluster");
    throw;
  }
}

