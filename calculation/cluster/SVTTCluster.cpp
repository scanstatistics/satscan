//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "SVTTCluster.h"

/** constructor */
SVTTClusterStreamData::SVTTClusterStreamData(int unsigned iAllocationSize) : ClusterStreamData(), giAllocationSize(iAllocationSize) {
  try {
    Init();
    Setup();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","SVTTClusterStreamData");
    throw;
  }
}

/** copy constructor */
SVTTClusterStreamData::SVTTClusterStreamData(const SVTTClusterStreamData& rhs) {
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
    x.AddCallpath("constructor()","SVTTClusterStreamData");
    throw;
  }
}

/** destructor */
SVTTClusterStreamData::~SVTTClusterStreamData() {
  try {
    delete[] gpCasesInsideCluster;
    delete[] gpCasesOutsideCluster;
    delete[] gpMeasureInsideCluster;
    delete[] gpMeasureOutsideCluster;
  }
  catch(...){}
}

SVTTClusterStreamData & SVTTClusterStreamData::operator=(const SVTTClusterStreamData& rhs) {
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

SVTTClusterStreamData * SVTTClusterStreamData::Clone() const {
  return new SVTTClusterStreamData(*this);
}

/** internal initialization function */
void SVTTClusterStreamData::Init() {
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

/** re-intialize stream data */
void SVTTClusterStreamData::InitializeSVTTData(const DataStreamInterface & Interface) {
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
void SVTTClusterStreamData::Setup() {
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
    x.AddCallpath("Setup()","SVTTClusterStreamData");
    throw;
  }
}

/** constructor for DataStreamGateway - used with calculating most likely clusters */
CSVTTCluster::CSVTTCluster(const AbtractDataStreamGateway & DataGateway, int iNumTimeIntervals)
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

/** constructor for DataStreamInterface - used with calculating loglikelihood ratios */
CSVTTCluster::CSVTTCluster(const DataStreamInterface & Interface, int iNumTimeIntervals)
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
CSVTTCluster::~CSVTTCluster() {
  try {
  }
  catch (...){}
}

/** overloaded assignment operator */
CSVTTCluster& CSVTTCluster::operator=(const CSVTTCluster& rhs) {
  if (this == &rhs)
    return *this;
  m_Center                      = rhs.m_Center;
//  m_nCases                      = rhs.m_nCases;
//  m_nMeasure                    = rhs.m_nMeasure;
  m_nTracts                     = rhs.m_nTracts;
  m_nRatio                      = rhs.m_nRatio;
  m_nLogLikelihood              = rhs.m_nLogLikelihood;
  m_nRank                       = rhs.m_nRank;
  m_nFirstInterval              = rhs.m_nFirstInterval;
  m_nLastInterval               = rhs.m_nLastInterval;
  giTotalIntervals              = rhs.giTotalIntervals;
  gvStreamData                  = rhs.gvStreamData;
  return *this;
}

/** add neighbor tract data from DataGateway */
void CSVTTCluster::AddNeighbor(tract_t tNeighbor, const AbtractDataStreamGateway & DataGateway) {
  ++m_nTracts;
  for (size_t tStream=0; tStream < DataGateway.GetNumInterfaces(); ++tStream)
    AddNeighbor(tNeighbor, DataGateway.GetDataStreamInterface(tStream), tStream);
}

/** add neighbor tract data from DataStreamInterface */
void CSVTTCluster::AddNeighbor(tract_t tNeighbor, const DataStreamInterface & Interface, size_t tStream) {
  measure_t  ** ppMeasureNC(Interface.GetNCMeasureArray());
  count_t    ** ppCasesNC(Interface.GetNCCaseArray());

  SVTTClusterStreamData & StreamData = gvStreamData[tStream];
  for (int i=0; i < giTotalIntervals; ++i) {
    StreamData.gpCasesInsideCluster[i] += ppCasesNC[i][tNeighbor];
    StreamData.gpMeasureInsideCluster[i] += ppMeasureNC[i][tNeighbor];
    StreamData.gpCasesOutsideCluster[i] -= ppCasesNC[i][tNeighbor];
    StreamData.gpMeasureOutsideCluster[i] -= ppMeasureNC[i][tNeighbor];
    StreamData.gtTotalCasesInsideCluster += ppCasesNC[i][tNeighbor];
    StreamData.gtTotalMeasureInsideCluster += ppMeasureNC[i][tNeighbor];
    StreamData.gtTotalCasesOutsideCluster -= ppCasesNC[i][tNeighbor];
  }
  //NOTE: Reporting of cluster information curently uses CCluster::m_nCases and
  //      CCluster::m_nMeasure but SVTT cluster does not need to collect this
  //      information in this manner anymore. It may be better to calculate these
  //      values from the arrays that replaced them when reporting.
//  for (int i=0; tStream == 0 && i < giTotalIntervals; ++i) {
//    m_nCases += ppCasesNC[i][tNeighbor];
//    m_nMeasure += ppMeasureNC[i][tNeighbor];
//  }
}

/** returns newly cloned CSVTTCluster */
CSVTTCluster * CSVTTCluster::Clone() const {
  return new CSVTTCluster(*this);
}

void CSVTTCluster::DisplayAnnualTimeTrendWithoutTitle(FILE* fp) const {
  if (gvStreamData[0].gTimeTrendInside.IsNegative())
    fprintf(fp, "     -");
  else
    fprintf(fp, "      ");

  fprintf(fp, "%.3f", gvStreamData[0].gTimeTrendInside.GetAnnualTimeTrend());
}

void CSVTTCluster::DisplayTimeTrend(FILE* fp, const AsciiPrintFormat& PrintFormat) const {
  ZdString      sBuffer;

  PrintFormat.PrintSectionLabel(fp, "Time trend", false, true);
  sBuffer.printf("%f  (%.3f%% %s",
               gvStreamData[0].gTimeTrendInside.GetBeta(),
               gvStreamData[0].gTimeTrendInside.GetAnnualTimeTrend(),
               (gvStreamData[0].gTimeTrendInside.IsNegative() ? "annual decrease)\n" : "annual increase)"));
  PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
}

/** returns the number of cases for tract as defined by cluster */
count_t CSVTTCluster::GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data, unsigned int iStream) const {
  return Data.GetDataStreamHandler().GetStream(iStream).GetCaseArray()[0][tTract];
}


AbstractClusterData * CSVTTCluster::GetClusterData() {
 ZdGenerateException("GetClusterData() not implemented.","CSVTTCluster");
 return 0;
}

/** returns end date of defined cluster as formated string */
ZdString& CSVTTCluster::GetEndDate(ZdString& sDateString, const CSaTScanData& DataHub) const {
  return JulianToString(sDateString, DataHub.GetTimeIntervalStartTimes()[DataHub.GetNumTimeIntervals()] - 1);
}

/** Returns the measure for tract as defined by cluster. */
measure_t CSVTTCluster::GetMeasureForTract(tract_t tTract, const CSaTScanData& Data, unsigned int iStream) const {
  return Data.GetMeasureAdjustment(iStream) * Data.GetDataStreamHandler().GetStream(iStream).GetMeasureArray()[0][tTract];
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
void CSVTTCluster::InitializeSVTT(tract_t nCenter, const AbtractDataStreamGateway & DataGateway) {
  CCluster::Initialize(nCenter);
  for (size_t t=0; t < gvStreamData.size(); ++t)
     gvStreamData[t].InitializeSVTTData(DataGateway.GetDataStreamInterface(t));
}

/** re-initializes cluster data */
void CSVTTCluster::InitializeSVTT(tract_t nCenter, const DataStreamInterface & Interface) {
  CCluster::Initialize(nCenter);
  gvStreamData.back().InitializeSVTTData(Interface);
}

void CSVTTCluster::SetTimeTrend(DatePrecisionType eDatePrecision, double nIntervalLen) {
  for (size_t t=0; t < gvStreamData.size(); ++t)
     gvStreamData[t].gTimeTrendInside.SetAnnualTimeTrend(eDatePrecision, nIntervalLen);
}

/** internal setup function for DataStreamGateway */
void CSVTTCluster::Setup(const AbtractDataStreamGateway & DataGateway, int iNumTimeIntervals) {
  try {
    giTotalIntervals = iNumTimeIntervals;
    gvStreamData.resize(DataGateway.GetNumInterfaces(), SVTTClusterStreamData(iNumTimeIntervals));
    InitializeSVTT(0, DataGateway);
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","CSVTTCluster");
    throw;
  }
}

/** internal setup function for DataStreamInterface */
void CSVTTCluster::Setup(const DataStreamInterface & Interface, int iNumTimeIntervals) {
  try {
    giTotalIntervals = iNumTimeIntervals;
    gvStreamData.push_back(SVTTClusterStreamData(iNumTimeIntervals));
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

