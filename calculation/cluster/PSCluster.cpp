// CPSCluster.cpp

#include <stdio.h>
#include "PSCluster.h"
#include "param.h"
#include "display.h"

CPurelySpatialCluster::CPurelySpatialCluster(BasePrint *pPrintDirection)
                   :CCluster(pPrintDirection)
{
  Initialize(0);
}

CPurelySpatialCluster::~CPurelySpatialCluster()
{
}

void CPurelySpatialCluster::Initialize(tract_t nCenter=0)
{
  CCluster::Initialize(nCenter);

  m_nSteps     = 1;
  m_nClusterType = PURELYSPATIAL;
}

void CPurelySpatialCluster::AddNeighbor(int iEllipse, const CSaTScanData& Data, count_t** pCases, tract_t n)
{
   try
      {
      //  printf("Add neighbor.\n");
      tract_t nNeighbor = Data.GetNeighbor(iEllipse, m_Center, n);

       m_nTracts++;
       m_nCases   += pCases[0][nNeighbor];                        // the first dimension [0] applies to the time interval...
       m_nMeasure += Data.m_pMeasure[0][nNeighbor];               // the first dimension [0] applies to the time interval...

       m_bClusterDefined = true;
       }
   catch (SSException & x)
      {
      x.AddCallpath("AddNeighbor()", "CPurelySpatialCluster");
      throw;
      }
}

void CPurelySpatialCluster::SetStartAndEndDates(const Julian* pIntervalStartTimes,
                                                int nTimeIntervals)
{
  m_nFirstInterval = 0;
  m_nLastInterval  = nTimeIntervals;
  m_nStartDate     = pIntervalStartTimes[m_nFirstInterval];
  m_nEndDate       = pIntervalStartTimes[m_nLastInterval]-1;
}

void CPurelySpatialCluster::DisplayTimeFrame(FILE* fp, char* szSpacesOnLeft,
                                             int nAnalysisType)
{
  char szStartDt[MAX_DT_STR];
  char szEndDt[MAX_DT_STR];

  if (nAnalysisType == SPACETIME)
    fprintf(fp, "%sTime frame............: %s - %s\n",
                 szSpacesOnLeft,
                 JulianToChar(szStartDt, m_nStartDate),
                 JulianToChar(szEndDt, m_nEndDate));
}

