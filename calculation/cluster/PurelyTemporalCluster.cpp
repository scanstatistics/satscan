#include "SaTScan.h"
#pragma hdrstop
#include "PurelyTemporalCluster.h"

/** constructor */
TemporalClusterStreamData::TemporalClusterStreamData(count_t * pCases, measure_t * pMeasure, measure_t * pSqMeasure)
                          :AbstractTemporalClusterStreamData() {
  gpCases = pCases;
  gpMeasure = pMeasure;
  gpSqMeasure = pSqMeasure;
}

/** destructor */
TemporalClusterStreamData::~TemporalClusterStreamData() {}

/** constructor */
CPurelyTemporalCluster::CPurelyTemporalCluster(const DataStreamGateway & DataGateway,
                                               IncludeClustersType eIncludeClustersType,
                                               const CSaTScanData & Data,
                                               BasePrint & PrintDirection)
                       :CCluster(&PrintDirection) {
  try {
    Init();
    Setup(DataGateway, eIncludeClustersType, Data);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CPurelyTemporalCluster");
    throw;
  }
}

/** constructor */
CPurelyTemporalCluster::CPurelyTemporalCluster(const DataStreamInterface & Interface,
                                               IncludeClustersType eIncludeClustersType,
                                               const CSaTScanData & Data,
                                               BasePrint & PrintDirection)
                       :CCluster(&PrintDirection) {
  try {
    Init();
    Setup(Interface, eIncludeClustersType, Data);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CPurelyTemporalCluster");
    throw;
  }
}

/** copy constructor */
CPurelyTemporalCluster::CPurelyTemporalCluster(const CPurelyTemporalCluster& rhs)
                       : CCluster(rhs.gpPrintDirection) {
  try {
    Init();
    Setup(rhs);
    *this = rhs;
  }
  catch (ZdException &x) {
    x.AddCallpath("copy constructor()","CPurelyTemporalCluster");
    throw;
  }
}

/** desctructor */
CPurelyTemporalCluster::~CPurelyTemporalCluster() {
  try {
    delete m_TI;
  }
  catch(...){}
}

/** overloaded assignment operator */
CPurelyTemporalCluster& CPurelyTemporalCluster::operator=(const CPurelyTemporalCluster& rhs) {
  m_Center              = rhs.m_Center;
  m_nTracts             = rhs.m_nTracts;
  m_nRatio              = rhs.m_nRatio;
  m_nLogLikelihood      = rhs.m_nLogLikelihood;
  m_nRank               = rhs.m_nRank;
  m_nFirstInterval      = rhs.m_nFirstInterval;
  m_nLastInterval       = rhs.m_nLastInterval;
  m_nStartDate          = rhs.m_nStartDate;
  m_nEndDate            = rhs.m_nEndDate;
  m_nTotalIntervals     = rhs.m_nTotalIntervals;
  m_nIntervalCut        = rhs.m_nIntervalCut;
  m_nTIType             = rhs.m_nTIType;
  m_nSteps              = rhs.m_nSteps;
  m_bClusterDefined     = rhs.m_bClusterDefined;
  m_nClusterType        = rhs.m_nClusterType;
  *m_TI                 = *(rhs.m_TI);
  gStreamData           = rhs.gStreamData;
  return *this;
}

/** returns newly cloned CPurelyTemporalCluster */
CPurelyTemporalCluster * CPurelyTemporalCluster::Clone() const {
  return new CPurelyTemporalCluster(*this);
}

/** compares this cluster definition to passed cluster definition */
void CPurelyTemporalCluster::CompareTopCluster(CPurelyTemporalCluster & TopShapeCluster, const CSaTScanData & Data) {
  m_bClusterDefined = true;

  if (Data.GetNumDataStreams() > 1)
     m_TI->CompareDataStreamClusters(*this, TopShapeCluster, gStreamData);
  else
     m_TI->CompareClusters(*this, TopShapeCluster, gStreamData[0]->gpCases, gStreamData[0]->gpMeasure, gStreamData[0]->gpSqMeasure);
}

/** modifies measure list given this cluster definition */
void CPurelyTemporalCluster::ComputeBestMeasures(const CSaTScanData& Data, CMeasureList & MeasureList) {
  m_TI->ComputeBestMeasures(gStreamData[0]->gpCases, gStreamData[0]->gpMeasure, gStreamData[0]->gpSqMeasure, MeasureList);
}

void CPurelyTemporalCluster::DisplayCensusTracts(FILE* fp, const CSaTScanData& Data, int nCluster,
                                                 measure_t nMinMeasure, int iNumSimulations,
                                                 long lReportHistoryRunNumber, bool bIncludeRelRisk,
                                                 bool bIncludePVal, int nLeftMargin, int nRightMargin,
                                                 char cDeliminator, char* szSpacesOnLeft, bool bFormat) {
  //const char * szTID;

  try {
    if (nLeftMargin > 0)
      fprintf(fp, "included.: All\n");
    // Code to print tracts for purely temporal clusters
    //else {
    //  for (int i=0; i<Data.m_nTracts; i++) {
    //       szTID = tiGetTid(i);
    //       fprintf(fp, "%i         ", nCluster);
    //       fprintf(fp, "%s\n", szTID);
    //  }
    //}
  }
  catch (ZdException &x) {
    x.AddCallpath("DisplayCensusTracts()","CPurelyTemporalCluster");
    throw;
  }
}

/** re-initializes cluster data */
void CPurelyTemporalCluster::Initialize(tract_t nCenter) {
  size_t        t;

  CCluster::Initialize(nCenter);
  m_nClusterType = PURELYTEMPORAL;
  for (t=0; t < gStreamData.size(); ++t)
     gStreamData[t]->InitializeData();
}

count_t CPurelyTemporalCluster::GetCaseCount(unsigned int iStream) const {
  return gStreamData[iStream]->gCases;
}

measure_t CPurelyTemporalCluster::GetMeasure(unsigned int iStream) const {
  return gStreamData[iStream]->gMeasure;
}

/** returns the number of cases for tract as defined by cluster
    NOTE: Hard coded to return the number of cases from first data stream.
          This will need modification when the reporting aspect of multiple
          data streams is hashed out.                                        */
count_t CPurelyTemporalCluster::GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data) const {
  return m_TI->GetCaseCountForTract(*this, tTract, Data.GetDataStreamHandler().GetStream(0/*for now*/).GetCaseArray());
}

/** Returns the measure for tract as defined by cluster.
    NOTE: Hard coded to return the measure from first data stream.
          This will need modification when the reporting aspect of multiple
          data streams is hashed out.                                       */
measure_t CPurelyTemporalCluster::GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const {
  return m_TI->GetMeasureForTract(*this, tTract, Data.GetDataStreamHandler().GetStream(0/*for now*/).GetMeasureArray());
}

/** internal setup function */
void CPurelyTemporalCluster::Setup(const DataStreamGateway & DataGateway, IncludeClustersType eIncludeClustersType, const CSaTScanData & Data) {
  try {
    m_nTotalIntervals = Data.m_nTimeIntervals;
    m_nIntervalCut = Data.m_nIntervalCut;
    m_nTIType = eIncludeClustersType;

    switch (m_nTIType) {
      case ALLCLUSTERS     :
      case CLUSTERSINRANGE : m_TI = new TimeIntervalRange(Data); break;
      case ALIVECLUSTERS   : m_TI = new CTIAlive(Data); break;
      default : ZdGenerateException("Unknown clusters type: '%d'.","Setup()", m_nTIType);
    }
    gStreamData.resize(DataGateway.GetNumInterfaces(), 0);
    for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t) {
       gStreamData[t] = new TemporalClusterStreamData(DataGateway.GetDataStreamInterface(t).GetPTCaseArray(),
                                                      DataGateway.GetDataStreamInterface(t).GetPTMeasureArray(), 0);
       gStreamData[t]->gTotalMeasure = Data.GetTotalDataStreamMeasure(t);
       gStreamData[t]->gTotalCases = Data.GetTotalDataStreamCases(t);
    }
    Initialize(0);
  }
  catch (ZdException &x) {
    delete m_TI;
    x.AddCallpath("Setup()","CPurelyTemporalCluster");
    throw;
  }
}

/** internal setup function */
void CPurelyTemporalCluster::Setup(const DataStreamInterface & Interface, IncludeClustersType eIncludeClustersType, const CSaTScanData & Data) {
  try {
    m_nTotalIntervals = Data.m_nTimeIntervals;
    m_nIntervalCut = Data.m_nIntervalCut;
    m_nTIType = eIncludeClustersType;

    switch (m_nTIType) {
      case ALLCLUSTERS     :
      case CLUSTERSINRANGE : m_TI = new TimeIntervalRange(Data); break;
      case ALIVECLUSTERS   : m_TI = new CTIAlive(Data); break;
      default : ZdGenerateException("Unknown clusters type: '%d'.","Setup()", m_nTIType);
    }
    gStreamData.resize(1, 0);
    gStreamData[0] = new TemporalClusterStreamData(Interface.GetPTCaseArray(), Interface.GetPTMeasureArray(), 0);
    gStreamData[0]->gTotalMeasure = Data.GetTotalDataStreamMeasure(0);
    gStreamData[0]->gTotalCases = Data.GetTotalDataStreamCases(0);
    Initialize(0);
  }
  catch (ZdException &x) {
    delete m_TI;
    x.AddCallpath("Setup()","CPurelyTemporalCluster");
    throw;
  }
}

/** internal setup function */
void CPurelyTemporalCluster::Setup(const CPurelyTemporalCluster& rhs) {
  try {
    m_TI = rhs.m_TI->Clone();
  }
  catch (ZdException &x) {
    delete m_TI;
    x.AddCallpath("Setup()","CPurelyTemporalCluster");
    throw;
  }
}


