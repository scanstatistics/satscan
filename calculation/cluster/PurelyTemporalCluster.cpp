//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "PurelyTemporalCluster.h"

/** constructor */
CPurelyTemporalCluster::CPurelyTemporalCluster(const AbstractClusterDataFactory * pClusterFactory,
                                               const AbtractDataStreamGateway & DataGateway,
                                               IncludeClustersType eIncludeClustersType,
                                               const CSaTScanData & Data)
                       :CCluster() {
  try {
    Init();
    Setup(pClusterFactory, DataGateway, eIncludeClustersType, Data);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CPurelyTemporalCluster");
    throw;
  }
}

/** copy constructor */
CPurelyTemporalCluster::CPurelyTemporalCluster(const CPurelyTemporalCluster& rhs)
                       : CCluster() {
  try {
    Init();
    gpClusterData = rhs.gpClusterData->Clone();
    *this = rhs;
  }
  catch (ZdException &x) {
    delete gpClusterData; gpClusterData=0;
    x.AddCallpath("copy constructor()","CPurelyTemporalCluster");
    throw;
  }
}

/** destructor */
CPurelyTemporalCluster::~CPurelyTemporalCluster() {
  try {
    delete gpClusterData;
  }
  catch(...){}
}

/** overloaded assignment operator */
CPurelyTemporalCluster& CPurelyTemporalCluster::operator=(const CPurelyTemporalCluster& rhs) {
  m_Center              = rhs.m_Center;
  m_nTracts             = rhs.m_nTracts;
  m_nRatio              = rhs.m_nRatio;
  m_nRank               = rhs.m_nRank;
  m_nFirstInterval      = rhs.m_nFirstInterval;
  m_nLastInterval       = rhs.m_nLastInterval;
  gpClusterData->Assign(*(rhs.gpClusterData));
  return *this;
}

/** returns newly cloned CPurelyTemporalCluster */
CPurelyTemporalCluster * CPurelyTemporalCluster::Clone() const {
  return new CPurelyTemporalCluster(*this);
}

void CPurelyTemporalCluster::DisplayCensusTracts(FILE* fp, const CSaTScanData&, measure_t, const AsciiPrintFormat& PrintFormat) const {
  PrintFormat.PrintSectionLabel(fp, "Location IDs included", false, false);
  fprintf(fp, "All\n");
}

/** re-initializes cluster data */
void CPurelyTemporalCluster::Initialize(tract_t nCenter) {
  CCluster::Initialize(nCenter);
  gpClusterData->InitializeData();
}

count_t CPurelyTemporalCluster::GetCaseCount(unsigned int iStream) const {
  return gpClusterData->GetCaseCount(iStream);
}

measure_t CPurelyTemporalCluster::GetMeasure(unsigned int iStream) const {
  return gpClusterData->GetMeasure(iStream);
}

/** returns the number of cases for tract as defined by cluster
    NOTE: Hard coded to return the number of cases from first data stream.
          This will need modification when the reporting aspect of multiple
          data streams is hashed out.                                        */
count_t CPurelyTemporalCluster::GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data, unsigned int iStream) const {
  count_t      tCaseCount,
            ** ppCases = Data.GetDataStreamHandler().GetStream(iStream).GetCaseArray();

  if (m_nLastInterval == Data.GetNumTimeIntervals())
    tCaseCount = ppCases[m_nFirstInterval][tTract];
  else
    tCaseCount  = ppCases[m_nFirstInterval][tTract] - ppCases[m_nLastInterval][tTract];

  return tCaseCount;
}

/** Returns the measure for tract as defined by cluster.
    NOTE: Hard coded to return the measure from first data stream.
          This will need modification when the reporting aspect of multiple
          data streams is hashed out.                                       */
measure_t CPurelyTemporalCluster::GetMeasureForTract(tract_t tTract, const CSaTScanData& Data, unsigned int iStream) const {
  measure_t      tMeasure,
              ** ppMeasure = Data.GetDataStreamHandler().GetStream(iStream).GetMeasureArray();

  if (m_nLastInterval == Data.GetNumTimeIntervals())
    tMeasure = ppMeasure[m_nFirstInterval][tTract];
  else
    tMeasure  = ppMeasure[m_nFirstInterval][tTract] - ppMeasure[m_nLastInterval][tTract];

  return tMeasure;
}

/** internal setup function */
void CPurelyTemporalCluster::Setup(const AbstractClusterDataFactory * pClusterFactory, const AbtractDataStreamGateway & DataGateway, IncludeClustersType eIncludeClustersType, const CSaTScanData & Data) {
  try {
    gpClusterData = pClusterFactory->GetNewTemporalClusterData(DataGateway);
  }
  catch (ZdException &x) {
    delete gpClusterData;
    x.AddCallpath("Setup()","CPurelyTemporalCluster");
    throw;
  }
}

