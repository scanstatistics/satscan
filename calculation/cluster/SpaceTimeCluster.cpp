//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "SpaceTimeCluster.h"

/** constructor */
CSpaceTimeCluster::CSpaceTimeCluster(const AbstractClusterDataFactory * pClusterFactory, const AbtractDataStreamGateway & DataGateway)
                  :CCluster() {
  try {
    Init();
    Setup(pClusterFactory, DataGateway);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CSpaceTimeCluster");
    throw;
  }
}


/** copy constructor */
CSpaceTimeCluster::CSpaceTimeCluster(const CSpaceTimeCluster& rhs)
                  : CCluster() {
  try {
    Init();
    gpClusterData = rhs.gpClusterData->Clone();
    *this = rhs;
  }
  catch (ZdException &x) {
    delete gpClusterData;
    x.AddCallpath("copy constructor()","CSpaceTimeCluster");
    throw;
  }
}

/** destructor */
CSpaceTimeCluster::~CSpaceTimeCluster() {
  try {
    delete gpClusterData;
  }
  catch(...){}
}

/** overloaded assignment operator */
CSpaceTimeCluster& CSpaceTimeCluster::operator =(const CSpaceTimeCluster& rhs) {
  m_Center                      = rhs.m_Center;
  m_nTracts                     = rhs.m_nTracts;
  m_nRatio                      = rhs.m_nRatio;
  m_nRank                       = rhs.m_nRank;
  m_DuczmalCorrection           = rhs.m_DuczmalCorrection;
  m_nFirstInterval              = rhs.m_nFirstInterval;
  m_nLastInterval               = rhs.m_nLastInterval;
  m_iEllipseOffset              = rhs.m_iEllipseOffset;
  gpClusterData->Assign(*(rhs.gpClusterData));
  return *this;
}

/** add neighbor tract data from DataGateway */
void CSpaceTimeCluster::AddNeighborDataAndCompare(tract_t tEllipseOffset,
                                                  tract_t tCentroid,
                                                  const AbtractDataStreamGateway & DataGateway,
                                                  const CSaTScanData * pData,
                                                  CSpaceTimeCluster & TopCluster,
                                                  CTimeIntervals * pTimeIntervals) {
                                                  
  tract_t       t, tNumNeighbors = pData->GetNeighborCountArray()[tEllipseOffset][tCentroid];

  for (t=1; t <= tNumNeighbors; ++t) {
    ++m_nTracts;
    gpClusterData->AddNeighborData(pData->GetNeighbor(tEllipseOffset, tCentroid, t), DataGateway);
    pTimeIntervals->CompareClusters(*this, TopCluster);
  }
}

/** returns newly cloned CSpaceTimeCluster */
CSpaceTimeCluster * CSpaceTimeCluster::Clone() const {
  return new CSpaceTimeCluster(*this);
}

/** returns the number of cases for tract as defined by cluster */
count_t CSpaceTimeCluster::GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data, unsigned int iStream) const {
  count_t      tCaseCount,
            ** ppCases = Data.GetDataStreamHandler().GetStream(iStream).GetCaseArray();

  if (m_nLastInterval == Data.GetNumTimeIntervals())
    tCaseCount = ppCases[m_nFirstInterval][tTract];
  else
    tCaseCount  = ppCases[m_nFirstInterval][tTract] - ppCases[m_nLastInterval][tTract];

  return tCaseCount;
}

/** Returns the measure for tract as defined by cluster. */
measure_t CSpaceTimeCluster::GetMeasureForTract(tract_t tTract, const CSaTScanData& Data, unsigned int iStream) const {
  measure_t      tMeasure,
              ** ppMeasure = Data.GetDataStreamHandler().GetStream(iStream).GetMeasureArray();

  if (m_nLastInterval == Data.GetNumTimeIntervals())
    tMeasure = ppMeasure[m_nFirstInterval][tTract];
  else
    tMeasure  = ppMeasure[m_nFirstInterval][tTract] - ppMeasure[m_nLastInterval][tTract];

  return Data.GetMeasureAdjustment(iStream) * tMeasure;
}

/** re-initializes cluster data */
void CSpaceTimeCluster::Initialize(tract_t nCenter) {
  m_Center = nCenter;
  m_nTracts = 0;
  m_nRatio = 0;
  gpClusterData->InitializeData();
}

/** internal setup function */
void CSpaceTimeCluster::Setup(const AbstractClusterDataFactory * pClusterFactory, const AbtractDataStreamGateway & DataGateway) {
  try {
    gpClusterData = pClusterFactory->GetNewSpaceTimeClusterData(DataGateway);
  }
  catch (ZdException &x) {
    delete gpClusterData;
    x.AddCallpath("Setup()","CSpaceTimeCluster");
    throw;
  }
}

