#include "SaTScan.h"
#pragma hdrstop
#include "PurelySpatialProspectiveCluster.h"
#include "MeasureList.h"

/** Purely spatial cluster to be used in simulations of prospective analysis.*/

/** constructor */
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

/** copy constructor */
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

/** destructor */
CPurelySpatialProspectiveCluster::~CPurelySpatialProspectiveCluster() {
  try {
    free(m_pCumCases);
    free(m_pCumMeasure);
  }
  catch(...){}
}

/** overloaded assignment operator */
CPurelySpatialProspectiveCluster& CPurelySpatialProspectiveCluster::operator=(const CPurelySpatialProspectiveCluster& cluster) {
  m_Center                      = cluster.m_Center;
  m_nTracts                     = cluster.m_nTracts;
  m_nRatio                      = cluster.m_nRatio;
  m_nLogLikelihood              = cluster.m_nLogLikelihood;
  m_nRank                       = cluster.m_nRank;
  m_DuczmalCorrection           = cluster.m_DuczmalCorrection;
  m_nFirstInterval              = cluster.m_nFirstInterval;
  m_nLastInterval               = cluster.m_nLastInterval;
  m_nStartDate                  = cluster.m_nStartDate;
  m_nEndDate                    = cluster.m_nEndDate;
  m_nTotalIntervals             = cluster.m_nTotalIntervals;
  memcpy(m_pCumCases, cluster.m_pCumCases, m_nNumIntervals*sizeof(count_t));
  memcpy(m_pCumMeasure, cluster.m_pCumMeasure, m_nNumIntervals*sizeof(measure_t));
  if (cluster.m_pCumMeasureSquared)
    memcpy(m_pCumMeasureSquared, cluster.m_pCumMeasureSquared, m_nNumIntervals*sizeof(measure_t));
  m_nSteps                      = cluster.m_nSteps;
  m_bClusterDefined             = cluster.m_bClusterDefined;
  m_nClusterType                = cluster.m_nClusterType;
  m_iEllipseOffset              = cluster.m_iEllipseOffset;
  m_nProspectiveStartInterval   = cluster.m_nProspectiveStartInterval;
  m_nNumIntervals               = cluster.m_nNumIntervals;     
  return *this;
}

/** add neighbor tract data from DataStreamInterface */
void CPurelySpatialProspectiveCluster::AddNeighborData(tract_t tNeighbor, const DataStreamInterface & Interface) {
  int           i, j;
  count_t    ** ppCases = Interface.GetCaseArray();
  measure_t  ** ppMeasure = Interface.GetMeasureArray();

  ++m_nTracts;

  //set cases for entire period added by this neighbor
  m_pCumCases[0]   += ppCases[0][tNeighbor];                            
  m_pCumMeasure[0] += ppMeasure[0][tNeighbor];
  for (j=1, i=m_nProspectiveStartInterval; i < m_nTotalIntervals; ++j, ++i) {
      m_pCumCases[j]   += ppCases[i][tNeighbor];
      m_pCumMeasure[j] += ppMeasure[i][tNeighbor];
  }
}

/** add neighbor tract data from DataStreamInterface */
void CPurelySpatialProspectiveCluster::AddNeighborDataEx(tract_t tNeighbor, const DataStreamInterface & Interface) {
  int           i, j;
  count_t    ** ppCases = Interface.GetCaseArray();
  measure_t  ** ppMeasure = Interface.GetMeasureArray(),
             ** ppMeasureSquared = Interface.GetSqMeasureArray();

  ++m_nTracts;

  //set cases for entire period added by this neighbor
  m_pCumCases[0]   += ppCases[0][tNeighbor];                            
  m_pCumMeasure[0] += ppMeasure[0][tNeighbor];
  m_pCumMeasureSquared[0] += ppMeasureSquared[0][tNeighbor];
  for (j=1, i=m_nProspectiveStartInterval; i < m_nTotalIntervals; ++j, ++i) {
     m_pCumCases[j]   += ppCases[i][tNeighbor];
     m_pCumMeasure[j] += ppMeasure[i][tNeighbor];
     m_pCumMeasureSquared[j] += ppMeasureSquared[i][tNeighbor];
  }
}

/** returns newly cloned CPurelySpatialCluster */
CPurelySpatialProspectiveCluster * CPurelySpatialProspectiveCluster::Clone() const {
  return new CPurelySpatialProspectiveCluster(*this);
}

/** modifies measure list given this cluster definition */
void CPurelySpatialProspectiveCluster::ComputeBestMeasures(CMeasureList & MeasureList) {
  int   iWindowEnd;

  for (iWindowEnd=1; iWindowEnd < m_nNumIntervals; ++iWindowEnd)
     MeasureList.AddMeasure(m_pCumCases[0] - m_pCumCases[iWindowEnd], gMeasure_(0, iWindowEnd, m_pCumMeasure, m_pCumMeasureSquared));
  MeasureList.AddMeasure(m_pCumCases[0], g_Measure_(0, m_pCumMeasure, m_pCumMeasureSquared));
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

/** re-initializes cluster data */
void CPurelySpatialProspectiveCluster::Initialize(tract_t nCenter = 0) {
  CCluster::Initialize(nCenter);
  m_nClusterType = PURELYSPATIAL/*PROSPECTIVE*/;
  memset(m_pCumCases, 0, sizeof(count_t) * m_nNumIntervals);
  memset(m_pCumMeasure, 0, sizeof(measure_t) * m_nNumIntervals);
  if (m_pCumMeasureSquared)
    memcpy(m_pCumMeasureSquared, m_pCumMeasureSquared, m_nNumIntervals*sizeof(measure_t));
}

void CPurelySpatialProspectiveCluster::SetStartAndEndDates(const Julian* pIntervalStartTimes, int nTimeIntervals) {
  ZdGenerateException("SetStartAndEndDates() not implemented.","CPurelySpatialProspectiveCluster");
}

/** internal setup function */
void CPurelySpatialProspectiveCluster::Setup(const CSaTScanData & Data) {
  try {
    //set AddNeihbor function pointer - for Normal model we will set to AddNeighborDataEx
    fAddNeighborData = &CPurelySpatialProspectiveCluster::AddNeighborData;

    m_nNumIntervals = 1 + Data.m_nTimeIntervals - Data.m_nProspectiveIntervalStart;
    m_nTotalIntervals = Data.m_nTimeIntervals;
    m_nProspectiveStartInterval = Data.m_nProspectiveIntervalStart;
    m_pCumCases   = (count_t*) Smalloc((m_nNumIntervals)*sizeof(count_t), gpPrintDirection);
    m_pCumMeasure = (measure_t*) Smalloc((m_nNumIntervals)*sizeof(measure_t), gpPrintDirection);
    if (this == 0x0/*normal model*/)
      m_pCumMeasureSquared = (measure_t*) Smalloc((m_nNumIntervals)*sizeof(measure_t), gpPrintDirection);
    Initialize(0);
  }
  catch (ZdException &x) {
    free(m_pCumCases);
    free(m_pCumMeasure);
    free(m_pCumMeasureSquared);
    x.AddCallpath("Setup()","CPurelySpatialProspectiveCluster");
    throw;
  }
}

/** internal setup function */
void CPurelySpatialProspectiveCluster::Setup(const CPurelySpatialProspectiveCluster& rhs) {
  try {
    m_pCumCases   = (count_t*) Smalloc((rhs.m_nNumIntervals)*sizeof(count_t), rhs.gpPrintDirection);
    m_pCumMeasure = (measure_t*) Smalloc((rhs.m_nNumIntervals)*sizeof(measure_t), rhs.gpPrintDirection);
    if (rhs.m_pCumMeasureSquared)
      m_pCumMeasureSquared = (measure_t*) Smalloc((rhs.m_nNumIntervals)*sizeof(measure_t), rhs.gpPrintDirection);
  }
  catch (ZdException &x) {
    free(m_pCumCases);
    free(m_pCumMeasure);
    free(m_pCumMeasureSquared);
    x.AddCallpath("Setup()","CPurelySpatialProspectiveCluster");
    throw;
  }
}
