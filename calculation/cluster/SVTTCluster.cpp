#include "SaTScan.h"
#pragma hdrstop
#include "SVTTCluster.h"

/** constructor for DataStreamGateway - used with calculating most likely clusters */
CSVTTCluster::CSVTTCluster(const DataStreamGateway & DataGateway, int iNumTimeIntervals, BasePrint *pPrintDirection)
             :CCluster(pPrintDirection){
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
CSVTTCluster::CSVTTCluster(const DataStreamInterface & Interface, int iNumTimeIntervals, BasePrint *pPrintDirection)
             :CCluster(pPrintDirection){
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
CSVTTCluster::CSVTTCluster(const CSVTTCluster & rhs): CCluster(rhs.gpPrintDirection) {
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
    DeallocateArrays();
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
  m_nStartDate                  = rhs.m_nStartDate;
  m_nEndDate                    = rhs.m_nEndDate;
  giTotalIntervals              = rhs.giTotalIntervals;
  *(gpCasesInsideCluster)       = *(rhs.gpCasesInsideCluster);
  *(gpCasesOutsideCluster)      = *(rhs.gpCasesOutsideCluster);
  *(gpMeasureInsideCluster)     = *(rhs.gpMeasureInsideCluster);
  *(gpMeasureOutsideCluster)    = *(rhs.gpMeasureOutsideCluster);
  memcpy(gpTotalCasesInsideCluster, rhs.gpTotalCasesInsideCluster, rhs.giNumDataStream * sizeof(count_t));
  memcpy(gpTotalCasesOutsideCluster, rhs.gpTotalCasesOutsideCluster, rhs.giNumDataStream * sizeof(count_t));
  memcpy(gpTotalMeasureInsideCluster, rhs.gpTotalMeasureInsideCluster, rhs.giNumDataStream * sizeof(measure_t));
  m_nSteps                      = rhs.m_nSteps;
  m_bClusterDefined             = rhs.m_bClusterDefined;
  m_nClusterType                = rhs.m_nClusterType;
  gTimeTrendInside              = rhs.gTimeTrendInside;
  gTimeTrendOutside             = rhs.gTimeTrendOutside;
  giNumDataStream               = rhs.giNumDataStream;
  return *this;
}

/** add neighbor tract data from DataGateway */
void CSVTTCluster::AddNeighbor(tract_t tNeighbor, const DataStreamGateway & DataGateway) {
  ++m_nTracts;
  for (size_t tStream=0; tStream < DataGateway.GetNumInterfaces(); ++tStream)
    AddNeighbor(tNeighbor, DataGateway.GetDataStreamInterface(tStream), tStream);
}

/** add neighbor tract data from DataStreamInterface */
void CSVTTCluster::AddNeighbor(tract_t tNeighbor, const DataStreamInterface & Interface, size_t tStream) {
  measure_t  ** ppMeasureNC(Interface.GetNCMeasureArray());
  count_t    ** ppCasesNC(Interface.GetNCCaseArray());

  for (int i=0; i < giTotalIntervals; ++i) {
    gpCasesInsideCluster->GetArray()[tStream][i] += ppCasesNC[i][tNeighbor];
    gpMeasureInsideCluster->GetArray()[tStream][i] += ppMeasureNC[i][tNeighbor];
    gpCasesOutsideCluster->GetArray()[tStream][i] -= ppCasesNC[i][tNeighbor];
    gpMeasureOutsideCluster->GetArray()[tStream][i] -= ppMeasureNC[i][tNeighbor];
    gpTotalCasesInsideCluster[tStream] += ppCasesNC[i][tNeighbor];
    gpTotalMeasureInsideCluster[tStream] += ppMeasureNC[i][tNeighbor];
    gpTotalCasesOutsideCluster[tStream] -= ppCasesNC[i][tNeighbor];
  }
  //NOTE: Reporting of cluster information curently uses CCluster::m_nCases and
  //      CCluster::m_nMeasure but SVTT cluster does not need to collect this
  //      information in this manner anymore. It may be better to calculate these
  //      values from the arrays that replaced them when reporting.
//  for (int i=0; tStream == 0 && i < giTotalIntervals; ++i) {
//    m_nCases += ppCasesNC[i][tNeighbor];
//    m_nMeasure += ppMeasureNC[i][tNeighbor];
//  }

  m_bClusterDefined = true; // KR990421 - What about this? PS-Yes, ST-No?
}

/** allocates class arrays */
void CSVTTCluster::AllocateArrays() {
  try {
    gpCasesInsideCluster = new TwoDimCountArray_t(giNumDataStream, giTotalIntervals);
    gpCasesOutsideCluster = new TwoDimCountArray_t(giNumDataStream, giTotalIntervals);
    gpMeasureInsideCluster = new TwoDimMeasureArray_t(giNumDataStream, giTotalIntervals);
    gpMeasureOutsideCluster = new TwoDimMeasureArray_t(giNumDataStream, giTotalIntervals);
    gpTotalCasesInsideCluster = new count_t[giNumDataStream];
    gpTotalMeasureInsideCluster = new measure_t[giNumDataStream];
    gpTotalCasesOutsideCluster = new count_t[giNumDataStream];
  }
  catch (ZdException &x) {
    DeallocateArrays();
    x.AddCallpath("AllocateArrays()","CSVTTCluster");
    throw;
  }
}

/** returns newly cloned CSVTTCluster */
CSVTTCluster * CSVTTCluster::Clone() const {
  return new CSVTTCluster(*this);
}

/** deallocates class arrays */
void CSVTTCluster::DeallocateArrays() {
  try {
    delete gpCasesInsideCluster;gpCasesInsideCluster=0;
    delete gpCasesOutsideCluster;gpCasesOutsideCluster=0;
    delete gpMeasureInsideCluster;gpMeasureInsideCluster=0;
    delete gpMeasureOutsideCluster;gpMeasureOutsideCluster=0;
    delete[] gpTotalCasesInsideCluster;gpTotalCasesInsideCluster=0;
    delete[] gpTotalMeasureInsideCluster;gpTotalMeasureInsideCluster=0;
    delete[] gpTotalCasesOutsideCluster;gpTotalCasesOutsideCluster=0;
  }
  catch (...){}
}

void CSVTTCluster::DisplayAnnualTimeTrendWithoutTitle(FILE* fp) {
  if (gTimeTrendInside.IsNegative())
    fprintf(fp, "     -");
  else
    fprintf(fp, "      ");

  fprintf(fp, "%.3f", gTimeTrendInside.GetAnnualTimeTrend());
}

void CSVTTCluster::DisplayTimeTrend(FILE* fp, char* szSpacesOnLeft) {
  fprintf(fp, "%sTime trend............: %f  (%.3f%% ",
              szSpacesOnLeft, gTimeTrendInside.m_nBeta,
              gTimeTrendInside.GetAnnualTimeTrend());

  if (gTimeTrendInside.IsNegative())
    fprintf(fp, "annual decrease)\n");
  else
    fprintf(fp, "annual increase)\n");
}

/** returns the number of cases for tract as defined by cluster
    NOTE: Hard coded to return the number of cases from first data stream.
          This will need modification when the reporting aspect of multiple
          data streams is hashed out.                                      */
count_t CSVTTCluster::GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data) const {
  return Data.GetDataStreamHandler().GetStream(0/*for now*/).GetCaseArray()[0][tTract];
}

/** Returns the measure for tract as defined by cluster.
    NOTE: Hard coded to return the measure from first data stream.
          This will need modification when the reporting aspect of multiple
          data streams is hashed out.                                      */
measure_t CSVTTCluster::GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const {
  return Data.GetMeasureAdjustment() * Data.GetDataStreamHandler().GetStream(0/*for now*/).GetMeasureArray()[0][tTract];
}

/** internal initialization function */
void CSVTTCluster::Init() {
  gpCasesInsideCluster=0;
  gpTotalCasesInsideCluster=0;
  gpMeasureInsideCluster=0;
  gpTotalMeasureInsideCluster=0;
  gpCasesOutsideCluster=0;
  gpTotalCasesOutsideCluster=0;
  gpMeasureOutsideCluster=0;
  giTotalIntervals=0;
  giNumDataStream=0;
}

/** re-initializes cluster data */
void CSVTTCluster::InitializeSVTT(tract_t nCenter, const DataStreamGateway & DataGateway) {
  unsigned int  iDim;

  CCluster::Initialize(nCenter);

  m_nSteps        = 1;
  m_nClusterType  = SPATIALVARTEMPTREND;

  gpCasesInsideCluster->Set(0);
  gpMeasureInsideCluster->Set(0);
  memset(gpTotalCasesInsideCluster, 0, giNumDataStream * sizeof(count_t));
  memset(gpTotalMeasureInsideCluster, 0, giNumDataStream * sizeof(measure_t));
  for (iDim=0; iDim < gpCasesOutsideCluster->Get1stDimension(); ++iDim)
    memcpy(gpCasesOutsideCluster->GetArray()[iDim], DataGateway.GetDataStreamInterface(iDim).GetPTCaseArray(), giTotalIntervals * sizeof(count_t));
  for (iDim=0; iDim < gpMeasureOutsideCluster->Get1stDimension(); ++iDim)
    memcpy(gpMeasureOutsideCluster->GetArray()[iDim], DataGateway.GetDataStreamInterface(iDim).GetPTMeasureArray(), giTotalIntervals * sizeof(measure_t));
  for (iDim=0; iDim < DataGateway.GetNumInterfaces(); ++iDim)
     gpTotalCasesOutsideCluster[iDim] = DataGateway.GetDataStreamInterface(iDim).GetTotalCasesCount();

  gTimeTrendInside.Initialize();
  gTimeTrendOutside.Initialize();
}

/** re-initializes cluster data */
void CSVTTCluster::InitializeSVTT(tract_t nCenter, const DataStreamInterface & Interface) {
  CCluster::Initialize(nCenter);

  m_nSteps        = 1;
  m_nClusterType  = SPATIALVARTEMPTREND;

  gpCasesInsideCluster->Set(0);
  gpMeasureInsideCluster->Set(0);
  gpTotalCasesInsideCluster[0] = 0;
  gpTotalMeasureInsideCluster[0] = 0;
  memcpy(gpCasesOutsideCluster->GetArray()[0], Interface.GetPTCaseArray(), giTotalIntervals * sizeof(count_t));
  memcpy(gpMeasureOutsideCluster->GetArray()[0], Interface.GetPTMeasureArray(), giTotalIntervals * sizeof(measure_t));
  gpTotalCasesOutsideCluster[0] = Interface.GetTotalCasesCount();

  gTimeTrendInside.Initialize();
  gTimeTrendOutside.Initialize();
}

void CSVTTCluster::SetStartAndEndDates(const Julian* pIntervalStartTimes, int nTimeIntervals) {
  m_nFirstInterval = 0;
  m_nLastInterval  = nTimeIntervals;
  m_nStartDate     = pIntervalStartTimes[m_nFirstInterval];
  m_nEndDate       = pIntervalStartTimes[m_nLastInterval]-1;
}

/** internal setup function for DataStreamGateway */
void CSVTTCluster::Setup(const DataStreamGateway & DataGateway, int iNumTimeIntervals) {
  try {
    giTotalIntervals = iNumTimeIntervals;
    giNumDataStream = DataGateway.GetNumInterfaces();
    AllocateArrays();
    InitializeSVTT(0, DataGateway);
  }
  catch (ZdException &x) {
    DeallocateArrays();
    x.AddCallpath("Setup()","CSVTTCluster");
    throw;
  }
}

/** internal setup function for DataStreamInterface */
void CSVTTCluster::Setup(const DataStreamInterface & Interface, int iNumTimeIntervals) {
  try {
    giTotalIntervals = iNumTimeIntervals;
    giNumDataStream = 1;
    AllocateArrays();
    InitializeSVTT(0, Interface);
  }
  catch (ZdException &x) {
    DeallocateArrays();
    x.AddCallpath("Setup()","CSVTTCluster");
    throw;
  }
}

/** internal setup function */
void CSVTTCluster::Setup(const CSVTTCluster& rhs) {
  try {
    giNumDataStream = rhs.giNumDataStream;
    giTotalIntervals = rhs.giTotalIntervals;
    AllocateArrays();
  }
  catch (ZdException &x) {
    DeallocateArrays();
    x.AddCallpath("Setup()","CSVTTCluster");
    throw;
  }
}

