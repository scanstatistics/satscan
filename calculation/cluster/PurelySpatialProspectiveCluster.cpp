//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "PurelySpatialProspectiveCluster.h"
#include "MeasureList.h"

/** Purely spatial cluster to be used in simulations of prospective analysis.*/

/** constructor */
CPurelySpatialProspectiveCluster::CPurelySpatialProspectiveCluster(const AbstractClusterDataFactory * pClusterFactory,
                                                                   const AbtractDataStreamGateway & DataGateway,
                                                                   const CSaTScanData & Data)
                                 :CCluster() {
  try {
    Init();
    Setup(pClusterFactory, DataGateway, Data);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CPurelySpatialProspectiveCluster");
    throw;
  }
}

/** copy constructor */
CPurelySpatialProspectiveCluster::CPurelySpatialProspectiveCluster(const CPurelySpatialProspectiveCluster& rhs)
                                 :CCluster() {
  try {
    Init();
    gpClusterData = rhs.gpClusterData->Clone();
    *this = rhs;
  }
  catch (ZdException &x) {
    delete gpClusterData;
    x.AddCallpath("copy constructor()","CPurelySpatialProspectiveCluster");
    throw;
  }
}

/** destructor */
CPurelySpatialProspectiveCluster::~CPurelySpatialProspectiveCluster() {
  try {
    delete gpClusterData;
  }
  catch(...){}
}

/** overloaded assignment operator */
CPurelySpatialProspectiveCluster& CPurelySpatialProspectiveCluster::operator=(const CPurelySpatialProspectiveCluster& cluster) {
  m_Center                      = cluster.m_Center;
  m_nTracts                     = cluster.m_nTracts;
  m_nRatio                      = cluster.m_nRatio;
  m_nRank                       = cluster.m_nRank;
  m_DuczmalCorrection           = cluster.m_DuczmalCorrection;
  m_nFirstInterval              = cluster.m_nFirstInterval;
  m_nLastInterval               = cluster.m_nLastInterval;
  m_nStartDate                  = cluster.m_nStartDate;
  m_nEndDate                    = cluster.m_nEndDate;
  m_iEllipseOffset              = cluster.m_iEllipseOffset;
  gpClusterData->Assign(*cluster.gpClusterData);
  return *this;
}

/** add neighbor tract data from DataGateway */
void CPurelySpatialProspectiveCluster::AddNeighborAndCompare(const AbtractDataStreamGateway & DataGateway,
                                                             const CSaTScanData * pData,
                                                             CPurelySpatialProspectiveCluster & TopCluster,
                                                             AbstractLikelihoodCalculator & Calculator) {
  tract_t       t, tNumNeighbors = pData->GetImpliedNeighborCount();

  for (t=1; t <= tNumNeighbors; ++t) {
    //update cluster data
    ++m_nTracts;
    gpClusterData->AddNeighborData(pData->GetNeighborTractIndex(t), DataGateway);
    //calculate loglikehood ratio and compare against current top cluster
    m_nRatio = gpClusterData->CalculateLoglikelihoodRatio(Calculator);
    if (m_nRatio && m_nRatio > TopCluster.m_nRatio)
      TopCluster.AssignAsType(*this);
  }    
}

///** add neighbor tract data from DataStreamInterface */
//void CPurelySpatialProspectiveCluster::AddNeighbor(tract_t tNeighbor, const AbtractDataStreamGateway & DataGateway) {
// ++m_nTracts;
//  gpClusterData->AddNeighborData(tNeighbor, DataGateway);
//}

///** add neighbor tract data from DataStreamInterface */
//void CPurelySpatialProspectiveCluster::AddNeighborData(tract_t tNeighbor, const DataStreamInterface & Interface) {
//  gpClusterData->AddNeighborData(tNeighbor, Interface);
//}

/** returns newly cloned CPurelySpatialCluster */
CPurelySpatialProspectiveCluster * CPurelySpatialProspectiveCluster::Clone() const {
  return new CPurelySpatialProspectiveCluster(*this);
}

/** modifies measure list given this cluster definition */
/*void CPurelySpatialProspectiveCluster::ComputeBestMeasures(CMeasureList & MeasureList) {
  int   iWindowEnd;

  for (iWindowEnd=1; iWindowEnd < m_nNumIntervals; ++iWindowEnd)
     MeasureList.AddMeasure(m_pCumCases[0] - m_pCumCases[iWindowEnd], m_pCumMeasure[0] - m_pCumMeasure[iWindowEnd]);
  MeasureList.AddMeasure(m_pCumCases[0], m_pCumMeasure[0]);
} */

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
  gpClusterData->InitializeData();
}

void CPurelySpatialProspectiveCluster::SetStartAndEndDates(const Julian* pIntervalStartTimes, int nTimeIntervals) {
//  ZdGenerateException("SetStartAndEndDates() not implemented.","CPurelySpatialProspectiveCluster");
}

void CPurelySpatialProspectiveCluster::Setup(const AbstractClusterDataFactory * pClusterFactory, const AbtractDataStreamGateway & DataGateway, const CSaTScanData & Data) {
  try {
    gpClusterData = pClusterFactory->GetNewProspectiveSpatialClusterData(Data, DataGateway);
  }
  catch (ZdException &x) {
    delete gpClusterData;
    x.AddCallpath("Setup()","CPurelySpatialProspectiveCluster");
    throw;
  }
}

