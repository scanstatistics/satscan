//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "PurelySpatialCluster.h"

/** constructor */
CPurelySpatialCluster::CPurelySpatialCluster(const AbstractClusterDataFactory * pClusterFactory,
                                             const AbtractDataStreamGateway & DataGateway,
                                             int iRate)
                      :CCluster() {
  try {
    gpClusterData = pClusterFactory->GetNewSpatialClusterData(DataGateway, iRate);
    //for a purely spatial cluster, the last time interval is the last time calculated interval index
    m_nLastInterval = DataGateway.GetDataStreamInterface(0).GetNumTimIntervals();
  }
  catch (ZdException &x) {
    delete gpClusterData;
    x.AddCallpath("constructor()","CPurelySpatialCluster");
    throw;
  }
}

/** constructor */
CPurelySpatialCluster::CPurelySpatialCluster(const AbstractClusterDataFactory * pClusterFactory,
                                             const DataStreamInterface & Interface,
                                             int iRate)
                      :CCluster() {
  try {
    gpClusterData = pClusterFactory->GetNewSpatialClusterData(Interface, iRate);
    //for a purely spatial cluster, the last time interval is the last time calculated interval index
    m_nLastInterval = Interface.GetNumTimIntervals();
  }
  catch (ZdException &x) {
    delete gpClusterData;
    x.AddCallpath("constructor()","CPurelySpatialCluster");
    throw;
  }
}

/** copy constructor */
CPurelySpatialCluster::CPurelySpatialCluster(const CPurelySpatialCluster& rhs)
                      :CCluster() {
  try {
    gpClusterData = rhs.gpClusterData->Clone();
    *this = rhs;
  }
  catch (ZdException &x) {
    delete gpClusterData; gpClusterData=0;
    x.AddCallpath("constructor()","CPurelySpatialCluster");
    throw;
  }
}

/** destructor */
CPurelySpatialCluster::~CPurelySpatialCluster() {
  try {
    delete gpClusterData;
  }
  catch (...){}  
}

/** overloaded assignment operator */
CPurelySpatialCluster& CPurelySpatialCluster::operator=(const CPurelySpatialCluster& rhs) {
  m_Center                = rhs.m_Center;
  m_nTracts               = rhs.m_nTracts;
  m_nRatio                = rhs.m_nRatio;
  m_nRank                 = rhs.m_nRank;
  m_NonCompactnessPenalty = rhs.m_NonCompactnessPenalty;
  m_nFirstInterval        = rhs.m_nFirstInterval;
  m_nLastInterval         = rhs.m_nLastInterval;
  m_iEllipseOffset        = rhs.m_iEllipseOffset;
  gpClusterData->Assign(*(rhs.gpClusterData));  
  return *this;
}

/** add neighbor tract data from DataGateway */
void CPurelySpatialCluster::AddNeighborDataAndCompare(tract_t tEllipseOffset,
                                                      tract_t tCentroid,
                                                      const AbtractDataStreamGateway & DataGateway,
                                                      const CSaTScanData * pData,
                                                      CPurelySpatialCluster & TopCluster,
                                                      AbstractLikelihoodCalculator & Calculator) {
                                                      
  tract_t       t, tNumNeighbors = pData->GetNeighborCountArray()[tEllipseOffset][tCentroid];

  for (t=1; t <= tNumNeighbors; ++t) {
    //update cluster data
    ++m_nTracts;
    gpClusterData->AddNeighborData(pData->GetNeighbor(tEllipseOffset, tCentroid, t), DataGateway);
    //calculate loglikehood ratio and compare against current top cluster
    m_nRatio = gpClusterData->CalculateLoglikelihoodRatio(Calculator);
    if (m_nRatio > TopCluster.m_nRatio)
      TopCluster = *this;
  }    
}

/** returns newly cloned CPurelySpatialCluster */
CPurelySpatialCluster * CPurelySpatialCluster::Clone() const {
  return new CPurelySpatialCluster(*this);;
}

void CPurelySpatialCluster::DisplayTimeFrame(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  if (DataHub.GetParameters().GetIsSpaceTimeAnalysis())
    CCluster::DisplayTimeFrame(fp, DataHub, PrintFormat);
}

/** returns the number of cases for tract as defined by cluster */
count_t CPurelySpatialCluster::GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data, unsigned int iStream) const {
  return Data.GetDataStreamHandler().GetStream(iStream).GetCaseArray()[0][tTract];
}

/** returns end date of defined cluster as formated string */
ZdString& CPurelySpatialCluster::GetEndDate(ZdString& sDateString, const CSaTScanData& DataHub) const {
  return JulianToString(sDateString, DataHub.GetTimeIntervalStartTimes()[DataHub.GetNumTimeIntervals()] - 1);
}

/** Returns the measure for tract as defined by cluster. */
measure_t CPurelySpatialCluster::GetMeasureForTract(tract_t tTract, const CSaTScanData& Data, unsigned int iStream) const {
  return Data.GetMeasureAdjustment(iStream) * Data.GetDataStreamHandler().GetStream(iStream).GetMeasureArray()[0][tTract];
}

/** returns start date of defined cluster as formated string */
ZdString& CPurelySpatialCluster::GetStartDate(ZdString& sDateString, const CSaTScanData& DataHub) const {
  return JulianToString(sDateString, DataHub.GetTimeIntervalStartTimes()[0]);
}

/** re-initializes cluster data */
void CPurelySpatialCluster::Initialize(tract_t nCenter) {
  //CCluster::Initialize(nCenter);
  m_Center = nCenter;
  m_nTracts = 0;
  m_nRatio = 0;
//  m_nLastInterval = 1;
  gpClusterData->InitializeData();
}

/** internal setup routine */
void CPurelySpatialCluster::Setup(const CSaTScanData & Data) {
  try {
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","CPurelySpatialCluster");
    throw;
  }
}

/** internal setup routine for copy constructor */
void CPurelySpatialCluster::Setup(const CPurelySpatialCluster& rhs) {
  try {
    Initialize(0);
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","CPurelySpatialCluster");
    throw;
  }
}

