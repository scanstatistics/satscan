//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "PurelyTemporalCluster.h"
#include "SSException.h"

/** constructor */
CPurelyTemporalCluster::CPurelyTemporalCluster(const AbstractClusterDataFactory * pClusterFactory,
                                               const AbstractDataSetGateway & DataGateway,
                                               IncludeClustersType eIncludeClustersType,
                                               const CSaTScanData & Data)
                       :CCluster() {
  try {
    Init();
    Setup(pClusterFactory, DataGateway, eIncludeClustersType, Data);
  }
  catch (prg_exception& x) {
    x.addTrace("constructor()","CPurelyTemporalCluster");
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
  catch (prg_exception& x) {
    delete gpClusterData; gpClusterData=0;
    x.addTrace("copy constructor()","CPurelyTemporalCluster");
    throw;
  }
}

/** destructor */
CPurelyTemporalCluster::~CPurelyTemporalCluster() {
  try { delete gpClusterData; } catch(...){}
}

/** overloaded assignment operator */
CPurelyTemporalCluster& CPurelyTemporalCluster::operator=(const CPurelyTemporalCluster& rhs) {
  m_Center              = rhs.m_Center;
  m_MostCentralLocation = rhs.m_MostCentralLocation;
  m_nTracts             = rhs.m_nTracts;
  m_nRatio              = rhs.m_nRatio;
  m_nRank               = rhs.m_nRank;
  m_nFirstInterval      = rhs.m_nFirstInterval;
  m_nLastInterval       = rhs.m_nLastInterval;
  gpClusterData->Assign(*(rhs.gpClusterData));
  return *this;
}

void CPurelyTemporalCluster::CopyEssentialClassMembers(const CCluster& rhs) {
  m_Center              = ((CPurelyTemporalCluster&)rhs).m_Center;
  m_MostCentralLocation = ((CPurelyTemporalCluster&)rhs).m_MostCentralLocation;
  m_nTracts             = ((CPurelyTemporalCluster&)rhs).m_nTracts; m_nTracts=1;
  m_nRatio              = ((CPurelyTemporalCluster&)rhs).m_nRatio;
  m_nRank               = ((CPurelyTemporalCluster&)rhs).m_nRank;
  m_nFirstInterval      = ((CPurelyTemporalCluster&)rhs).m_nFirstInterval;
  m_nLastInterval       = ((CPurelyTemporalCluster&)rhs).m_nLastInterval;
  gpClusterData->CopyEssentialClassMembers(*(rhs.GetClusterData()));
}

/** returns newly cloned CPurelyTemporalCluster */
CPurelyTemporalCluster * CPurelyTemporalCluster::Clone() const {
  return new CPurelyTemporalCluster(*this);
}

void CPurelyTemporalCluster::DisplayCensusTracts(FILE* fp, const CSaTScanData&, const AsciiPrintFormat& PrintFormat) const {
  PrintFormat.PrintSectionLabel(fp, "Location IDs included", false, false);
  fprintf(fp, "All\n");
}

/** Returns the measure for tract as defined by cluster. */
measure_t CPurelyTemporalCluster::GetExpectedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex) const {
  measure_t      tMeasure,
              ** ppMeasure = Data.GetDataSetHandler().GetDataSet(tSetIndex).getMeasureData().GetArray();

  if (m_nLastInterval == Data.GetNumTimeIntervals())
    tMeasure = ppMeasure[m_nFirstInterval][tTractIndex];
  else
    tMeasure  = ppMeasure[m_nFirstInterval][tTractIndex] - ppMeasure[m_nLastInterval][tTractIndex];

  return tMeasure;
}

/** Returns collection of location indexes that define this cluster. If 'bAtomize' is true, breaks
    down meta locations into atomic indexes. */
std::vector<tract_t> & CPurelyTemporalCluster::getLocationIndexes(const CSaTScanData& DataHub, std::vector<tract_t>& indexes, bool bAtomize) const {
   indexes.clear();
   indexes.push_back(0);   
   return indexes;
}

/** Returns index of most central location. */
tract_t CPurelyTemporalCluster::GetMostCentralLocationIndex() const {
  throw prg_error("GetMostCentralLocationIndex() not implemented for CPurelyTemporalCluster.","GetMostCentralLocationIndex()");
}

/** returns the number of cases for tract as defined by cluster */
count_t CPurelyTemporalCluster::GetObservedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex) const {
  count_t      tCaseCount,
            ** ppCases = Data.GetDataSetHandler().GetDataSet(tSetIndex).getCaseData().GetArray();

  if (m_nLastInterval == Data.GetNumTimeIntervals())
    tCaseCount = ppCases[m_nFirstInterval][tTractIndex];
  else
    tCaseCount  = ppCases[m_nFirstInterval][tTractIndex] - ppCases[m_nLastInterval][tTractIndex];

  return tCaseCount;
}

/** re-initializes cluster data */
void CPurelyTemporalCluster::Initialize(tract_t nCenter) {
  CCluster::Initialize(nCenter);
  gpClusterData->InitializeData();
}

/** internal setup function */
void CPurelyTemporalCluster::Setup(const AbstractClusterDataFactory * pClusterFactory, const AbstractDataSetGateway & DataGateway, IncludeClustersType eIncludeClustersType, const CSaTScanData & Data) {
  try {
    gpClusterData = pClusterFactory->GetNewTemporalClusterData(DataGateway);
  }
  catch (prg_exception& x) {
    delete gpClusterData;
    x.addTrace("Setup()","CPurelyTemporalCluster");
    throw;
  }
}

