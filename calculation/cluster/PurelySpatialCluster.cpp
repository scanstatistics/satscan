#include "SaTScan.h"
#pragma hdrstop
#include "PurelySpatialCluster.h"


CPurelySpatialCluster::CPurelySpatialCluster(BasePrint *pPrintDirection)
                   :CCluster(pPrintDirection)
{
  Initialize(0);
}

CPurelySpatialCluster::~CPurelySpatialCluster() {}

/** returns newly cloned CPurelySpatialCluster */
CPurelySpatialCluster * CPurelySpatialCluster::Clone() const {
  //Note: Replace this code with copy constructor...
  CPurelySpatialCluster * pClone = new CPurelySpatialCluster(gpPrintDirection);
  *pClone = *this;
  return pClone;
}

void CPurelySpatialCluster::AddNeighbor(int iEllipse, const CSaTScanData& Data, count_t** pCases, tract_t n)
{
  tract_t nNeighbor = Data.GetNeighbor(iEllipse, m_Center, n);

  m_nTracts++;
  m_nCases   += pCases[0][nNeighbor];                        // the first dimension [0] applies to the time interval...
  m_nMeasure += Data.m_pMeasure[0][nNeighbor];               // the first dimension [0] applies to the time interval...

  m_bClusterDefined = true;
}

void CPurelySpatialCluster::DisplayTimeFrame(FILE* fp, char* szSpacesOnLeft, int nAnalysisType)
{
  char szStartDt[MAX_DT_STR];
  char szEndDt[MAX_DT_STR];

  if (nAnalysisType == SPACETIME)
    fprintf(fp, "%sTime frame............: %s - %s\n",
                 szSpacesOnLeft,
                 JulianToChar(szStartDt, m_nStartDate),
                 JulianToChar(szEndDt, m_nEndDate));
}

/** Returns the number of case for tract as defined by cluster. */
count_t CPurelySpatialCluster::GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data) const
{
  return Data.m_pCases[0][tTract];
}

/** Returns the measure for tract as defined by cluster. */
measure_t CPurelySpatialCluster::GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const
{
  return Data.GetMeasureAdjustment() * Data.m_pMeasure[0][tTract];
}

void CPurelySpatialCluster::Initialize(tract_t nCenter=0)
{
  CCluster::Initialize(nCenter);

  m_nSteps     = 1;
  m_nClusterType = PURELYSPATIAL;
}

void CPurelySpatialCluster::SetStartAndEndDates(const Julian* pIntervalStartTimes, int nTimeIntervals)
{
  m_nFirstInterval = 0;
  m_nLastInterval  = nTimeIntervals;
  m_nStartDate     = pIntervalStartTimes[m_nFirstInterval];
  m_nEndDate       = pIntervalStartTimes[m_nLastInterval]-1;
}
