#include "SaTScan.h"
#pragma hdrstop
#include "PurelySpatialCluster.h"

/** constructor */
CPurelySpatialCluster::CPurelySpatialCluster(const AbstractClusterDataFactory * pClusterFactory, const AbtractDataStreamGateway & DataGateway,
                                             int iRate, BasePrint *pPrintDirection)
                      :CCluster(pPrintDirection) {
  try {
    gpClusterData = pClusterFactory->GetNewSpatialClusterData(DataGateway, iRate);
  }
  catch (ZdException &x) {
    delete gpClusterData;
    x.AddCallpath("constructor()","CPurelySpatialCluster");
    throw;
  }
}

/** constructor */
CPurelySpatialCluster::CPurelySpatialCluster(const AbstractClusterDataFactory * pClusterFactory, const DataStreamInterface & Interface,
                                             int iRate, BasePrint *pPrintDirection)
                      :CCluster(pPrintDirection) {
  try {
    gpClusterData = pClusterFactory->GetNewSpatialClusterData(Interface, iRate);
  }
  catch (ZdException &x) {
    delete gpClusterData;
    x.AddCallpath("constructor()","CPurelySpatialCluster");
    throw;
  }
}

/** copy constructor */
CPurelySpatialCluster::CPurelySpatialCluster(const CPurelySpatialCluster& rhs)
                      :CCluster(rhs.gpPrintDirection) {
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
CPurelySpatialCluster::~CPurelySpatialCluster() {}

/** overloaded assignment operator */
CPurelySpatialCluster& CPurelySpatialCluster::operator=(const CPurelySpatialCluster& rhs) {
  m_Center              = rhs.m_Center;
  m_nTracts             = rhs.m_nTracts;
  m_nRatio              = rhs.m_nRatio;
  m_nRank               = rhs.m_nRank;
  m_DuczmalCorrection   = rhs.m_DuczmalCorrection;
  m_nFirstInterval      = rhs.m_nFirstInterval;
  m_nLastInterval       = rhs.m_nLastInterval;
  m_nStartDate          = rhs.m_nStartDate;
  m_nEndDate            = rhs.m_nEndDate;
  m_iEllipseOffset      = rhs.m_iEllipseOffset;
  gpClusterData->Assign(*(rhs.gpClusterData));  
  return *this;
}

/** add neighbor tract data from DataGateway */
void CPurelySpatialCluster::AddNeighborDataAndCompare(const AbtractDataStreamGateway & DataGateway,
                                                      const CSaTScanData * pData,
                                                      CPurelySpatialCluster & TopCluster,
                                                      AbstractLikelihoodCalculator & Calculator) {
                                                      
  tract_t       t, tNumNeighbors = pData->GetImpliedNeighborCount();

  for (t=1; t <= tNumNeighbors; ++t) {
    //update cluster data
    ++m_nTracts;
    gpClusterData->AddNeighborData(pData->GetNeighborTractIndex(t), DataGateway);
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

void CPurelySpatialCluster::DisplayTimeFrame(FILE* fp, char* szSpacesOnLeft, int nAnalysisType) {
  ZdString      sStart, sEnd;

  if (nAnalysisType == SPACETIME)
    fprintf(fp, "%sTime frame............: %s - %s\n", szSpacesOnLeft,
            JulianToString(sStart, m_nStartDate).GetCString(), JulianToString(sEnd, m_nEndDate).GetCString());
}

/** returns the number of cases for tract as defined by cluster
    NOTE: Hard coded to return the number of cases from first data stream.
          This will need modification when the reporting aspect of multiple
          data streams is hashed out.                                      */
count_t CPurelySpatialCluster::GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data) const {
  return Data.GetDataStreamHandler().GetStream(0/*for now*/).GetCaseArray()[0][tTract];
}

/** Returns the measure for tract as defined by cluster.
    NOTE: Hard coded to return the measure from first data stream.
          This will need modification when the reporting aspect of multiple
          data streams is hashed out.                                      */
measure_t CPurelySpatialCluster::GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const {
  return Data.GetMeasureAdjustment() * Data.GetDataStreamHandler().GetStream(0/*for now*/).GetMeasureArray()[0][tTract];
}

/** re-initializes cluster data */
void CPurelySpatialCluster::Initialize(tract_t nCenter) {
  //CCluster::Initialize(nCenter);
  m_Center = nCenter;
  m_nTracts = 0;
  m_nRatio = 0;
  gpClusterData->InitializeData();
}

void CPurelySpatialCluster::SetStartAndEndDates(const Julian* pIntervalStartTimes, int nTimeIntervals) {
  m_nFirstInterval = 0;
  m_nLastInterval  = nTimeIntervals;
  m_nStartDate     = pIntervalStartTimes[m_nFirstInterval];
  m_nEndDate       = pIntervalStartTimes[m_nLastInterval]-1;
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

