#include "SaTScan.h"
#pragma hdrstop
#include "PurelySpatialCluster.h"

/** constructor */
CPurelySpatialCluster::CPurelySpatialCluster(const CSaTScanData & Data, BasePrint *pPrintDirection)
                   :CCluster(pPrintDirection) {
  try {
    Setup(Data);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CPurelySpatialCluster");
    throw;
  }
}

/** copy constructor */
CPurelySpatialCluster::CPurelySpatialCluster(const CPurelySpatialCluster& rhs)
                      :CCluster(rhs.gpPrintDirection) {
  try {
    Setup(rhs);
    *this = rhs;
  }
  catch (ZdException &x) {
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
  m_nLogLikelihood      = rhs.m_nLogLikelihood;
  m_nRank               = rhs.m_nRank;
  m_DuczmalCorrection   = rhs.m_DuczmalCorrection;
  m_nFirstInterval      = rhs.m_nFirstInterval;
  m_nLastInterval       = rhs.m_nLastInterval;
  m_nStartDate          = rhs.m_nStartDate;
  m_nEndDate            = rhs.m_nEndDate;
  m_nSteps              = rhs.m_nSteps;
  m_bClusterDefined     = rhs.m_bClusterDefined;
  m_nClusterType        = rhs.m_nClusterType;
  m_iEllipseOffset      = rhs.m_iEllipseOffset;
  gStreamData           = rhs.gStreamData;
  
  return *this;
}

/** add neighbor tract data from DataGateway */
void CPurelySpatialCluster::AddNeighbor(tract_t tNeighbor, const DataStreamGateway & DataGateway) {
  ++m_nTracts;

  for (size_t tStream=0; tStream < DataGateway.GetNumInterfaces(); ++tStream)
    AddNeighbor(tNeighbor, DataGateway.GetDataStreamInterface(tStream), tStream);
  m_bClusterDefined = true;
}

/** add neighbor tract data from DataStreamInterface */
void CPurelySpatialCluster::AddNeighbor(tract_t tNeighbor, const DataStreamInterface & Interface, size_t tStream) {
  ClusterStreamData & StreamData = gStreamData[tStream];
  StreamData.gCases += Interface.GetPSCaseArray()[tNeighbor];
  StreamData.gMeasure += Interface.GetPSMeasureArray()[tNeighbor];
  if (Interface.IsSqMeasureArray())
    StreamData.gSqMeasure += Interface.GetPSSqMeasureArray()[tNeighbor];
}

/** returns newly cloned CPurelySpatialCluster */
CPurelySpatialCluster * CPurelySpatialCluster::Clone() const {
  return new CPurelySpatialCluster(*this);;
}

/** compares this cluster definition to passed cluster definition */
void CPurelySpatialCluster::CompareTopCluster(CPurelySpatialCluster & TopShapeCluster, const CSaTScanData * pData) {
  size_t        t;
  CModel      & Probability = pData->GetProbabilityModel();

  m_nRatio = 0;
  for (t=0; t < gStreamData.size(); ++t) {
     ClusterStreamData & StreamData = gStreamData[t];
     if (RateIsOfInterest(StreamData.gCases, StreamData.gMeasure, StreamData.gTotalCases, StreamData.gTotalMeasure))
       m_nRatio += Probability.CalcLogLikelihoodRatio(StreamData.gCases, StreamData.gMeasure, StreamData.gTotalCases, StreamData.gTotalMeasure, m_DuczmalCorrection);
  }

  // NOTE: because of data streams - when only one stream, this comparison is now called regardless
  if (m_nRatio && m_nRatio > TopShapeCluster.m_nRatio)
    TopShapeCluster = *this;
}

/** modifies measure list given this cluster definition */
void CPurelySpatialCluster::ComputeBestMeasures(CMeasureList & MeasureList) {
  ClusterStreamData & StreamData = gStreamData[0];
  MeasureList.AddMeasure(StreamData.gCases, /*gMeasure(*/StreamData.gMeasure/*, gStreamData[0].gSqMeasure)*/);
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
void CPurelySpatialCluster::Initialize(tract_t nCenter=0) {
  StreamDataContainerIterator_t   itr;

  CCluster::Initialize(nCenter);
  m_nSteps     = 1;
  m_nClusterType = PURELYSPATIAL;
  for (itr=gStreamData.begin(); itr != gStreamData.end(); ++itr)
     itr->InitializeData();
}

void CPurelySpatialCluster::SetStartAndEndDates(const Julian* pIntervalStartTimes, int nTimeIntervals) {
  m_nFirstInterval = 0;
  m_nLastInterval  = nTimeIntervals;
  m_nStartDate     = pIntervalStartTimes[m_nFirstInterval];
  m_nEndDate       = pIntervalStartTimes[m_nLastInterval]-1;
}

/** internal setup routine */
void CPurelySpatialCluster::Setup(const CSaTScanData & Data) {
  size_t        t, tSize=Data.GetNumDataStreams();

  try {
    gStreamData.resize(tSize);
    for (t=0; t < tSize; ++t) {
       gStreamData[t].gTotalMeasure = Data.GetTotalDataStreamMeasure(t);
       gStreamData[t].gTotalCases = Data.GetTotalDataStreamCases(t); 
    }
    Initialize(0);
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","CPurelySpatialCluster");
    throw;
  }
}

/** internal setup routine for copy constructor */
void CPurelySpatialCluster::Setup(const CPurelySpatialCluster& rhs) {
  try {
    gStreamData = rhs.gStreamData;
    Initialize(0);
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","CPurelySpatialCluster");
    throw;
  }
}

