//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "PurelyTemporalCluster.h"
#include "SSException.h"
#include "ClosedLoopData.h"

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
  _central_identifier   = rhs._central_identifier;
  _num_identifiers      = rhs._num_identifiers;
  m_nRatio              = rhs.m_nRatio;
  _ratio_sets           = rhs._ratio_sets;
  m_nRank               = rhs.m_nRank;
  m_nFirstInterval      = rhs.m_nFirstInterval;
  m_nLastInterval       = rhs.m_nLastInterval;
  gpClusterData->Assign(*(rhs.gpClusterData));
  return *this;
}

void CPurelyTemporalCluster::CopyEssentialClassMembers(const CCluster& rhs) {
  m_Center              = ((CPurelyTemporalCluster&)rhs).m_Center;
  _central_identifier   = ((CPurelyTemporalCluster&)rhs)._central_identifier;
  _num_identifiers      = ((CPurelyTemporalCluster&)rhs)._num_identifiers; _num_identifiers=1;
  m_nRatio              = ((CPurelyTemporalCluster&)rhs).m_nRatio;
  _ratio_sets           = ((const CPurelyTemporalCluster&)rhs)._ratio_sets;
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
measure_t CPurelyTemporalCluster::GetExpectedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex, bool adjusted) const {
  measure_t tMeasure, ** ppMeasure = Data.GetDataSetHandler().GetDataSet(tSetIndex).getMeasureData().GetArray();

  if (m_nLastInterval == Data.GetNumTimeIntervals())
    tMeasure = ppMeasure[m_nFirstInterval][tTractIndex];
  else
    tMeasure  = ppMeasure[m_nFirstInterval][tTractIndex] - ppMeasure[m_nLastInterval][tTractIndex];

  return tMeasure * Data.GetMeasureAdjustment(tSetIndex);
}

/** returns end date of defined cluster as formated string */
std::string& CPurelyTemporalCluster::GetEndDate(std::string& sDateString, const CSaTScanData& DataHub, const char * sep) const {
    DatePrecisionType eDatePrint = DataHub.GetParameters().GetPrecisionOfTimesType();
    if (DataHub.GetParameters().GetAnalysisType() == SEASONALTEMPORAL) {
        const ClosedLoopData* closedloop = dynamic_cast<const ClosedLoopData*>(&DataHub);
        if (!closedloop) throw prg_error("Unable to cast to ClosedLoopData object.","GetEndDate()");

		int intervalIdx = m_nLastInterval <= closedloop->getExtendedPeriodStart() ? m_nLastInterval : (m_nLastInterval - closedloop->getExtendedPeriodStart());
        return JulianToString(sDateString, closedloop->getExtendedTimeIntervalStartTimes()[intervalIdx] - 1, eDatePrint, sep, true, true);
    } else
        return JulianToString(sDateString, DataHub.GetTimeIntervalStartTimes()[m_nLastInterval] - 1, eDatePrint, sep);
}

/** returns start date of defined cluster as formated string */
std::string& CPurelyTemporalCluster::GetStartDate(std::string& sDateString, const CSaTScanData& DataHub, const char * sep) const {
    DatePrecisionType eDatePrint = DataHub.GetParameters().GetPrecisionOfTimesType();
    if (DataHub.GetParameters().GetAnalysisType() == SEASONALTEMPORAL) {
        const ClosedLoopData* closedloop = dynamic_cast<const ClosedLoopData*>(&DataHub);
        if (!closedloop) throw prg_error("Unable to cast to ClosedLoopData object.","GetEndDate()");
        return JulianToString(sDateString, closedloop->getExtendedTimeIntervalStartTimes()[m_nFirstInterval], eDatePrint, sep, false, true);
    } else
        return JulianToString(sDateString, DataHub.GetTimeIntervalStartTimes()[m_nFirstInterval], eDatePrint, sep);
}


/** Returns collection of location indexes that define this cluster. If 'bAtomize' is true, breaks
    down meta locations into atomic indexes. */
std::vector<tract_t> & CPurelyTemporalCluster::getIdentifierIndexes(const CSaTScanData& DataHub, std::vector<tract_t>& indexes, bool bAtomize) const {
   indexes.clear();
   indexes.push_back(0);
   return indexes;
}

/** Returns index of most central identifier. */
tract_t CPurelyTemporalCluster::mostCentralIdentifierIdx() const {
  throw prg_error("mostCentralIdentifierIdx() not implemented for CPurelyTemporalCluster.","mostCentralIdentifierIdx()");
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

