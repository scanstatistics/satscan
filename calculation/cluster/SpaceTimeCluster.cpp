#include "SaTScan.h"
#pragma hdrstop
#include "SpaceTimeCluster.h"

SpaceTimeClusterStreamData::SpaceTimeClusterStreamData(int unsigned iAllocationSize)
                           :AbstractTemporalClusterStreamData(), giAllocationSize(iAllocationSize) {
  try {
    Init();
    Setup();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","SpaceTimeClusterStreamData");
    throw;
  }
}

SpaceTimeClusterStreamData::SpaceTimeClusterStreamData(const SpaceTimeClusterStreamData& rhs) {
  try {
    Init();
    gpCases = new count_t[rhs.giAllocationSize];
    gpMeasure = new measure_t[rhs.giAllocationSize];
    //gpSqMeasure = new measure_t[rhs.giAllocationSize];
    *this = rhs;
  }
  catch (ZdException &x) {
    delete[] gpCases; delete[] gpMeasure; //delete[] gpSqMeasure;
    x.AddCallpath("constructor()","SpaceTimeClusterStreamData");
    throw;
  }
}

SpaceTimeClusterStreamData::~SpaceTimeClusterStreamData() {
  try {
    delete[] gpCases; delete[] gpMeasure; //delete[] gpSqMeasure;
  }
  catch(...){}
}

SpaceTimeClusterStreamData & SpaceTimeClusterStreamData::operator=(const SpaceTimeClusterStreamData& rhs) {
   gCases = rhs.gCases;
   gMeasure = rhs.gMeasure;
   gSqMeasure = rhs.gSqMeasure;
   giAllocationSize = rhs.giAllocationSize;
   memcpy(gpCases, rhs.gpCases, giAllocationSize * sizeof(count_t));
   memcpy(gpMeasure, rhs.gpMeasure, giAllocationSize * sizeof(measure_t));
   //memcpy(gpSqMeasure, rhs.gpSqMeasure, giAllocationSize * sizeof(measure_t));
   return *this;
}

SpaceTimeClusterStreamData * SpaceTimeClusterStreamData::Clone() const {
  return new SpaceTimeClusterStreamData(*this);
}

void SpaceTimeClusterStreamData::InitializeData() {
  gCases=0;
  gMeasure=gSqMeasure=0;
  memset(gpCases, 0, sizeof(count_t) * giAllocationSize);
  memset(gpMeasure, 0, sizeof(measure_t) * giAllocationSize);
  //memset(gpSqMeasure, 0, sizeof(measure_t) * giAllocationSize);
}

void SpaceTimeClusterStreamData::Setup() {
  try {
    gpCases = new count_t[giAllocationSize];
    gpMeasure = new measure_t[giAllocationSize];
    //gpSqMeasure = new measure_t[giAllocationSize];
    InitializeData();
  }
  catch (ZdException &x) {
    delete[] gpCases; delete[] gpMeasure; //delete[] gpSqMeasure;
    x.AddCallpath("Setup()","SpaceTimeClusterStreamData");
    throw;
  }
}

SpaceTimeClusterStreamDataEx::SpaceTimeClusterStreamDataEx(int unsigned iAllocationSize)
                             :SpaceTimeClusterStreamData(iAllocationSize) {
  try {
    Setup();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","SpaceTimeClusterStreamDataEx");
    throw;
  }
}

SpaceTimeClusterStreamDataEx::SpaceTimeClusterStreamDataEx(const SpaceTimeClusterStreamDataEx& rhs)
                             :SpaceTimeClusterStreamData(rhs) {
  try {
    gpSqMeasure = new measure_t[rhs.giAllocationSize];
    memcpy(gpSqMeasure, rhs.gpSqMeasure, rhs.giAllocationSize * sizeof(measure_t));
    *this = rhs;
  }
  catch (ZdException &x) {
    delete[] gpSqMeasure;
    x.AddCallpath("constructor()","SpaceTimeClusterStreamDataEx");
    throw;
  }
}

SpaceTimeClusterStreamDataEx::~SpaceTimeClusterStreamDataEx() {
  try {
    delete[] gpSqMeasure;
  }
  catch(...){}
}

SpaceTimeClusterStreamDataEx & SpaceTimeClusterStreamDataEx::operator=(const SpaceTimeClusterStreamDataEx& rhs) {
   gCases = rhs.gCases;
   gMeasure = rhs.gMeasure;
   gSqMeasure = rhs.gSqMeasure;
   giAllocationSize = rhs.giAllocationSize;
   memcpy(gpCases, rhs.gpCases, giAllocationSize * sizeof(count_t));
   memcpy(gpMeasure, rhs.gpMeasure, giAllocationSize * sizeof(measure_t));
   memcpy(gpSqMeasure, rhs.gpSqMeasure, giAllocationSize * sizeof(measure_t));
   return *this;
}

SpaceTimeClusterStreamDataEx * SpaceTimeClusterStreamDataEx::Clone() const {
  return new SpaceTimeClusterStreamDataEx(*this);
}

void SpaceTimeClusterStreamDataEx::InitializeData() {
  gCases=0;
  gMeasure=gSqMeasure=0;
  memset(gpCases, 0, sizeof(count_t) * giAllocationSize);
  memset(gpMeasure, 0, sizeof(measure_t) * giAllocationSize);
  memset(gpSqMeasure, 0, sizeof(measure_t) * giAllocationSize);
}

void SpaceTimeClusterStreamDataEx::Setup() {
  try {
    gpSqMeasure = new measure_t[giAllocationSize];
    memset(gpSqMeasure, 0, sizeof(measure_t) * giAllocationSize);
  }
  catch (ZdException &x) {
    delete[] gpSqMeasure;
    x.AddCallpath("Setup()","SpaceTimeClusterStreamDataEx");
    throw;
  }
}

/** constructor */
CSpaceTimeCluster::CSpaceTimeCluster(IncludeClustersType eIncludeClustersType, const CSaTScanData & Data, BasePrint & PrintDirection)
                  :CCluster(&PrintDirection) {
  try {
    Init();
    Setup(eIncludeClustersType, Data);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CSpaceTimeCluster");
    throw;
  }
}

/** copy constructor */
CSpaceTimeCluster::CSpaceTimeCluster(const CSpaceTimeCluster& rhs)
                  : CCluster(rhs.gpPrintDirection) {
  try {
    Init();
    Setup(rhs);
    *this = rhs;
  }
  catch (ZdException &x) {
    x.AddCallpath("copy constructor()","CSpaceTimeCluster");
    throw;
  }
}

/** destructor */
CSpaceTimeCluster::~CSpaceTimeCluster() {
  try {
    delete TI;
  }
  catch(...){}
}

/** overloaded assignment operator */
CSpaceTimeCluster& CSpaceTimeCluster::operator =(const CSpaceTimeCluster& rhs) {
  m_Center                      = rhs.m_Center;
  m_nTracts                     = rhs.m_nTracts;
  m_nRatio                      = rhs.m_nRatio;
  m_nLogLikelihood              = rhs.m_nLogLikelihood;
  m_nRank                       = rhs.m_nRank;
  m_DuczmalCorrection           = rhs.m_DuczmalCorrection;
  m_nFirstInterval              = rhs.m_nFirstInterval;
  m_nLastInterval               = rhs.m_nLastInterval;
  m_nStartDate                  = rhs.m_nStartDate;
  m_nEndDate                    = rhs.m_nEndDate;
  m_nTotalIntervals             = rhs.m_nTotalIntervals;
  m_nIntervalCut                = rhs.m_nIntervalCut;
  m_nTIType                     = rhs.m_nTIType;
  m_bClusterDefined             = rhs.m_bClusterDefined;
  m_iEllipseOffset              = rhs.m_iEllipseOffset;
  *TI                           = *(rhs.TI);
  gStreamData                   = rhs.gStreamData;
  return *this;
}

/** add neighbor tract data from DataGateway */
void CSpaceTimeCluster::AddNeighbor(tract_t tNeighbor, const DataStreamGateway & DataGateway) {
  ++m_nTracts;

  for (size_t tStream=0; tStream < DataGateway.GetNumInterfaces(); ++tStream)
    (this->*fAddNeighborData)(tNeighbor, DataGateway.GetDataStreamInterface(tStream), tStream);
}

/** add neighbor tract data from DataStreamInterface */
void CSpaceTimeCluster::AddNeighborData(tract_t tNeighbor, const DataStreamInterface & Interface, size_t tStream) {
  int           i;
  count_t    ** ppCases = Interface.GetCaseArray(), * pStreamCases = gStreamData[tStream]->gpCases;
  measure_t  ** ppMeasure = Interface.GetMeasureArray(), * pStreamMeasure = gStreamData[tStream]->gpMeasure;

  AbstractTemporalClusterStreamData * pStreamData = gStreamData[tStream];
  for (i=0; i < m_nTotalIntervals; ++i) {
     pStreamCases[i]   += ppCases[i][tNeighbor];
     pStreamMeasure[i] += ppMeasure[i][tNeighbor];
  }
}

/** add neighbor tract data from DataStreamInterface */
void CSpaceTimeCluster::AddNeighborDataEx(tract_t tNeighbor, const DataStreamInterface & Interface, size_t tStream) {
  int           i;
  count_t    ** ppCases = Interface.GetCaseArray();
  measure_t  ** ppMeasure = Interface.GetMeasureArray(),
             ** ppMeasureSquared = Interface.GetSqMeasureArray();

  AbstractTemporalClusterStreamData * pStreamData = gStreamData[tStream];
  for (i=0; i < m_nTotalIntervals; ++i) {
     pStreamData->gpCases[i]   += ppCases[i][tNeighbor];
     pStreamData->gpMeasure[i] += ppMeasure[i][tNeighbor];
     pStreamData->gpSqMeasure[i] += ppMeasureSquared[i][tNeighbor];
  }
}

/** returns newly cloned CSpaceTimeCluster */
CSpaceTimeCluster * CSpaceTimeCluster::Clone() const {
  return new CSpaceTimeCluster(*this);
}

/** compares this cluster definition to passed cluster definition */
void CSpaceTimeCluster::CompareTopCluster(CSpaceTimeCluster & TopShapeCluster, const CSaTScanData & Data) {
  m_bClusterDefined = true;
  (TI->*fCompareClusters)(*this, TopShapeCluster, gStreamData);
}

/** modifies measure list given this cluster definition */
void CSpaceTimeCluster::ComputeBestMeasures(CMeasureList & MeasureList) {
  TI->ComputeBestMeasures(gStreamData[0], MeasureList);
}

/** returns the number of cases for tract as defined by cluster
    NOTE: Hard coded to return the number of cases from first data stream.
          This will need modification when the reporting aspect of multiple
          data streams is hashed out.                                        */
count_t CSpaceTimeCluster::GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data) const {
  return TI->GetCaseCountForTract(*this, tTract, Data.GetDataStreamHandler().GetStream(0/*for now*/).GetCaseArray());
}

/** Returns the measure for tract as defined by cluster.
    NOTE: Hard coded to return the measure from first data stream.
          This will need modification when the reporting aspect of multiple
          data streams is hashed out.                                       */
measure_t CSpaceTimeCluster::GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const {
  return Data.GetMeasureAdjustment() *
         TI->GetMeasureForTract(*this, tTract, Data.GetDataStreamHandler().GetStream(0/*for now*/).GetMeasureArray());
}

/** internal class initialization */
void CSpaceTimeCluster::Init() {
  TI=0;
  m_nTotalIntervals=0;
  m_nIntervalCut=0;
}

/** re-initializes cluster data */
void CSpaceTimeCluster::Initialize(tract_t nCenter = 0) {
  m_Center = nCenter;
  m_nTracts = 0;
  m_bClusterDefined = false;
  m_nRatio = 0;
  m_nLogLikelihood = 0; //may go away
  for (gitr=gStreamData.begin(); gitr != gStreamData.end(); ++gitr)
     (*gitr)->InitializeData();
}

/** internal setup function */
void CSpaceTimeCluster::Setup(IncludeClustersType eIncludeClustersType, const CSaTScanData & Data) {
  AbstractTemporalClusterStreamData * pStreamData;

  try {
    //set AddNeighbor function pointer - for Normal model we will set to AddNeighborDataEx
    fAddNeighborData = &CSpaceTimeCluster::AddNeighborData;
    //set CTimeIntervals function pointers - for Normal model we will set to SomeFuncEx
    fCompareClusters = (Data.GetNumDataStreams() == 1 ? &CTimeIntervals::CompareClusters : &CTimeIntervals::CompareDataStreamClusters);

    m_nTotalIntervals = Data.m_nTimeIntervals;
    m_nIntervalCut = Data.m_nIntervalCut;
    m_nTIType = eIncludeClustersType;

    switch (m_nTIType) {
      case ALLCLUSTERS     :
      case CLUSTERSINRANGE : TI = new TimeIntervalRange(Data); break;
      case ALIVECLUSTERS   : TI = new CTIAlive(Data); break;
      default : ZdGenerateException("Unknown clusters type: '%d'.","Setup()", m_nTIType);
    }
    gStreamData.resize(Data.GetNumDataStreams(), 0);
    for (size_t t=0; t < Data.GetNumDataStreams(); ++t) {
       pStreamData = gStreamData[t] = new SpaceTimeClusterStreamData(Data.m_nTimeIntervals + 1);
       pStreamData->gTotalMeasure = Data.GetTotalDataStreamMeasure(t);
       pStreamData->gTotalCases = Data.GetTotalDataStreamCases(t);
    }
    Initialize(0);
  }
  catch (ZdException &x) {
    delete TI;
    x.AddCallpath("Setup()","CSpaceTimeCluster");
    throw;
  }
}

/** internal setup function */
void CSpaceTimeCluster::Setup(const CSpaceTimeCluster& rhs) {
  try {
    TI = rhs.TI->Clone();
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","CSpaceTimeCluster");
    throw;
  }
}
