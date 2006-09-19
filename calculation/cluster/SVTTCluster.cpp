//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "SVTTCluster.h"

/** constructor */
SVTTClusterSetData::SVTTClusterSetData(int unsigned iAllocationSize) : ClusterSetData(), giAllocationSize(iAllocationSize) {
  try {
    Init();
    Setup();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","SVTTClusterSetData");
    throw;
  }
}

/** copy constructor */
SVTTClusterSetData::SVTTClusterSetData(const SVTTClusterSetData& rhs) {
  try {
    Init();
    gpCasesInsideCluster = new count_t[rhs.giAllocationSize];
    gpCasesOutsideCluster = new count_t[rhs.giAllocationSize];
    gpMeasureInsideCluster = new measure_t[rhs.giAllocationSize];
    gpMeasureOutsideCluster = new measure_t[rhs.giAllocationSize];
    *this = rhs;
  }
  catch (ZdException &x) {
    delete[] gpCasesInsideCluster;
    delete[] gpCasesOutsideCluster;
    delete[] gpMeasureInsideCluster;
    delete[] gpMeasureOutsideCluster;
    x.AddCallpath("constructor()","SVTTClusterSetData");
    throw;
  }
}

/** destructor */
SVTTClusterSetData::~SVTTClusterSetData() {
  try {
    delete[] gpCasesInsideCluster;
    delete[] gpCasesOutsideCluster;
    delete[] gpMeasureInsideCluster;
    delete[] gpMeasureOutsideCluster;
  }
  catch(...){}
}

SVTTClusterSetData & SVTTClusterSetData::operator=(const SVTTClusterSetData& rhs) {
  gCases = rhs.gCases;
  gMeasure = rhs.gMeasure;
//  gSqMeasure = rhs.gSqMeasure;
  giAllocationSize = rhs.giAllocationSize;
  memcpy(gpCasesInsideCluster, rhs.gpCasesInsideCluster, giAllocationSize * sizeof(count_t));
  memcpy(gpMeasureInsideCluster, rhs.gpMeasureInsideCluster, giAllocationSize * sizeof(measure_t));
  gtTotalCasesInsideCluster = rhs.gtTotalCasesInsideCluster;
  gtTotalMeasureInsideCluster = rhs.gtTotalMeasureInsideCluster;
  memcpy(gpCasesOutsideCluster, rhs.gpCasesOutsideCluster, giAllocationSize * sizeof(count_t));
  memcpy(gpMeasureOutsideCluster, rhs.gpMeasureOutsideCluster, giAllocationSize * sizeof(measure_t));
  gtTotalCasesOutsideCluster = rhs.gtTotalCasesOutsideCluster;
  gTimeTrendInside = rhs.gTimeTrendInside;
  gTimeTrendOutside = rhs.gTimeTrendOutside;
   return *this;
}

SVTTClusterSetData * SVTTClusterSetData::Clone() const {
  return new SVTTClusterSetData(*this);
}

/** internal initialization function */
void SVTTClusterSetData::Init() {
  gCases=0;
  gMeasure=0;
//  gSqMeasure=0;
  gtTotalCasesInsideCluster=0;
  gtTotalMeasureInsideCluster=0;
  gtTotalCasesOutsideCluster=0;
  gpCasesInsideCluster=0;
  gpCasesOutsideCluster=0;
  gpMeasureInsideCluster=0;
  gpMeasureOutsideCluster=0;
}

/** re-intialize data */
void SVTTClusterSetData::InitializeSVTTData(const DataSetInterface & Interface) {
  memset(gpCasesInsideCluster, 0, giAllocationSize * sizeof(count_t));
  memset(gpMeasureInsideCluster, 0, giAllocationSize * sizeof(measure_t));
  gtTotalCasesInsideCluster=0;
  gtTotalMeasureInsideCluster=0;
  memcpy(gpCasesOutsideCluster, Interface.GetPTCaseArray(), giAllocationSize * sizeof(count_t));
  memcpy(gpMeasureOutsideCluster, Interface.GetPTMeasureArray(), giAllocationSize * sizeof(measure_t));
  gtTotalCasesOutsideCluster = Interface.GetTotalCasesCount();

  gTimeTrendInside.Initialize();
  gTimeTrendOutside.Initialize();
}

/** internal setup function */
void SVTTClusterSetData::Setup() {
  try {
    gpCasesInsideCluster = new count_t[giAllocationSize];
    gpCasesOutsideCluster = new count_t[giAllocationSize];
    gpMeasureInsideCluster = new measure_t[giAllocationSize];
    gpMeasureOutsideCluster = new measure_t[giAllocationSize];
  }
  catch (ZdException &x) {
    delete[] gpCasesInsideCluster;
    delete[] gpCasesOutsideCluster;
    delete[] gpMeasureInsideCluster;
    delete[] gpMeasureOutsideCluster;
    x.AddCallpath("Setup()","SVTTClusterSetData");
    throw;
  }
}

/** constructor for DataSetGateway - used with calculating most likely clusters */
CSVTTCluster::CSVTTCluster(const AbstractDataSetGateway & DataGateway, int iNumTimeIntervals)
             :CCluster(){
  try {
    Init();
    Setup(DataGateway, iNumTimeIntervals);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CSVTTCluster");
    throw;
  }
}

/** constructor for DataSetInterface - used with calculating loglikelihood ratios */
CSVTTCluster::CSVTTCluster(const DataSetInterface & Interface, int iNumTimeIntervals)
             :CCluster(){
  try {
    Init();
    Setup(Interface, iNumTimeIntervals);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CSVTTCluster");
    throw;
  }
}


/** copy constructor */
CSVTTCluster::CSVTTCluster(const CSVTTCluster & rhs): CCluster() {
  try {
    Init();
    Setup(rhs);
    *this = rhs;
  }
  catch (ZdException &x) {
    x.AddCallpath("copy constructor()","CSVTTCluster");
    throw;
  }
}

/** destructor */
CSVTTCluster::~CSVTTCluster() {}

/** overloaded assignment operator */
CSVTTCluster& CSVTTCluster::operator=(const CSVTTCluster& rhs) {
  if (this == &rhs)
    return *this;
  m_Center                      = rhs.m_Center;
  m_MostCentralLocation         = rhs.m_MostCentralLocation;
//  m_nCases                      = rhs.m_nCases;
//  m_nMeasure                    = rhs.m_nMeasure;
  m_nTracts                     = rhs.m_nTracts;
  m_CartesianRadius             = rhs.m_CartesianRadius;  
  m_nRatio                      = rhs.m_nRatio;
  m_nLogLikelihood              = rhs.m_nLogLikelihood;
  m_nRank                       = rhs.m_nRank;
  m_nFirstInterval              = rhs.m_nFirstInterval;
  m_nLastInterval               = rhs.m_nLastInterval;
  giTotalIntervals              = rhs.giTotalIntervals;
  gvSetData                     = rhs.gvSetData;
  return *this;
}

/** add neighbor tract data from DataGateway */
void CSVTTCluster::AddNeighbor(tract_t tNeighbor, const AbstractDataSetGateway & DataGateway) {
  ++m_nTracts;
  for (size_t tSetIndex=0; tSetIndex < DataGateway.GetNumInterfaces(); ++tSetIndex)
    AddNeighbor(tNeighbor, DataGateway.GetDataSetInterface(tSetIndex), tSetIndex);
}

/** add neighbor tract data from DataSetInterface */
void CSVTTCluster::AddNeighbor(tract_t tNeighbor, const DataSetInterface & Interface, size_t tSetIndex) {
  measure_t  ** ppMeasureNC(Interface.GetNCMeasureArray());
  count_t    ** ppCasesNC(Interface.GetNCCaseArray());

  SVTTClusterSetData & SetData = gvSetData[tSetIndex];
  for (int i=0; i < giTotalIntervals; ++i) {
    SetData.gpCasesInsideCluster[i] += ppCasesNC[i][tNeighbor];
    SetData.gpMeasureInsideCluster[i] += ppMeasureNC[i][tNeighbor];
    SetData.gpCasesOutsideCluster[i] -= ppCasesNC[i][tNeighbor];
    SetData.gpMeasureOutsideCluster[i] -= ppMeasureNC[i][tNeighbor];
    SetData.gtTotalCasesInsideCluster += ppCasesNC[i][tNeighbor];
    SetData.gtTotalMeasureInsideCluster += ppMeasureNC[i][tNeighbor];
    SetData.gtTotalCasesOutsideCluster -= ppCasesNC[i][tNeighbor];
  }
  //NOTE: Reporting of cluster information curently uses CCluster::m_nCases and
  //      CCluster::m_nMeasure but SVTT cluster does not need to collect this
  //      information in this manner anymore. It may be better to calculate these
  //      values from the arrays that replaced them when reporting.
//  for (int i=0; tSetIndex == 0 && i < giTotalIntervals; ++i) {
//    m_nCases += ppCasesNC[i][tNeighbor];
//    m_nMeasure += ppMeasureNC[i][tNeighbor];
//  }
}

/** returns newly cloned CSVTTCluster */
CSVTTCluster * CSVTTCluster::Clone() const {
  return new CSVTTCluster(*this);
}

void CSVTTCluster::DisplayAnnualTimeTrendWithoutTitle(FILE* fp) const {
  if (gvSetData[0].gTimeTrendInside.IsNegative())
    fprintf(fp, "     -");
  else
    fprintf(fp, "      ");

  fprintf(fp, "%.3f", gvSetData[0].gTimeTrendInside.GetAnnualTimeTrend());
}

void CSVTTCluster::DisplayTimeTrend(FILE* fp, const AsciiPrintFormat& PrintFormat) const {
  ZdString      sBuffer;

  PrintFormat.PrintSectionLabel(fp, "Time trend", false, true);
  sBuffer.printf("%f  (%.3f%% %s",
               gvSetData[0].gTimeTrendInside.GetBeta(),
               gvSetData[0].gTimeTrendInside.GetAnnualTimeTrend(),
               (gvSetData[0].gTimeTrendInside.IsNegative() ? "annual decrease)\n" : "annual increase)"));
  PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
}

AbstractClusterData * CSVTTCluster::GetClusterData() {
 ZdGenerateException("GetClusterData() not implemented.","CSVTTCluster");
 return 0;
}

const AbstractClusterData * CSVTTCluster::GetClusterData() const {
 ZdGenerateException("GetClusterData() not implemented.","CSVTTCluster");
 return 0;
}

/** returns end date of defined cluster as formated string */
ZdString& CSVTTCluster::GetEndDate(ZdString& sDateString, const CSaTScanData& DataHub) const {
  return JulianToString(sDateString, DataHub.GetTimeIntervalStartTimes()[DataHub.GetNumTimeIntervals()] - 1);
}

/** Returns number of expcected cases in accumulated data. */
measure_t CSVTTCluster::GetExpectedCount(const CSaTScanData& DataHub, size_t tSetIndex) const {
  return DataHub.GetMeasureAdjustment(tSetIndex) * gvSetData[tSetIndex].gtTotalMeasureInsideCluster;
}

/** Returns the measure for tract as defined by cluster. */
measure_t CSVTTCluster::GetExpectedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex) const {
  return Data.GetMeasureAdjustment(tSetIndex) * Data.GetDataSetHandler().GetDataSet(tSetIndex).getMeasureData().GetArray()[0][tTractIndex];
}

/** returns the number of cases for tract as defined by cluster */
count_t CSVTTCluster::GetObservedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex) const {
  return Data.GetDataSetHandler().GetDataSet(tSetIndex).getCaseData().GetArray()[0][tTractIndex];
}

/** returns start date of defined cluster as formated string */
ZdString& CSVTTCluster::GetStartDate(ZdString& sDateString, const CSaTScanData& DataHub) const {
  return JulianToString(sDateString, DataHub.GetTimeIntervalStartTimes()[0]);
}

/** internal initialization function */
void CSVTTCluster::Init() {
  giTotalIntervals=0;
}

/** re-initializes cluster data */
void CSVTTCluster::InitializeSVTT(tract_t nCenter, const AbstractDataSetGateway & DataGateway) {
  CCluster::Initialize(nCenter);
  for (size_t t=0; t < gvSetData.size(); ++t)
     gvSetData[t].InitializeSVTTData(DataGateway.GetDataSetInterface(t));
}

/** re-initializes cluster data */
void CSVTTCluster::InitializeSVTT(tract_t nCenter, const DataSetInterface & Interface) {
  CCluster::Initialize(nCenter);
  gvSetData.back().InitializeSVTTData(Interface);
}

void CSVTTCluster::SetTimeTrend(DatePrecisionType eDatePrecision, double nIntervalLen) {
  for (size_t t=0; t < gvSetData.size(); ++t)
     gvSetData[t].gTimeTrendInside.SetAnnualTimeTrend(eDatePrecision, nIntervalLen);
}

/** internal setup function for DataSetGateway */
void CSVTTCluster::Setup(const AbstractDataSetGateway & DataGateway, int iNumTimeIntervals) {
  try {
    giTotalIntervals = iNumTimeIntervals;
    gvSetData.resize(DataGateway.GetNumInterfaces(), SVTTClusterSetData(iNumTimeIntervals));
    InitializeSVTT(0, DataGateway);
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","CSVTTCluster");
    throw;
  }
}

/** internal setup function for DataSetInterface */
void CSVTTCluster::Setup(const DataSetInterface & Interface, int iNumTimeIntervals) {
  try {
    giTotalIntervals = iNumTimeIntervals;
    gvSetData.push_back(SVTTClusterSetData(iNumTimeIntervals));
    InitializeSVTT(0, Interface);
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","CSVTTCluster");
    throw;
  }
}

/** internal setup function */
void CSVTTCluster::Setup(const CSVTTCluster& rhs) {
  try {
    giTotalIntervals = rhs.giTotalIntervals;
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","CSVTTCluster");
    throw;
  }
}

