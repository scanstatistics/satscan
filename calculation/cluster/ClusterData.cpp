//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "ClusterData.h"
#include "TimeIntervals.h"

/** constructor */
SpatialData::SpatialData(const DataStreamInterface & Interface, int iRate)
            :AbstractSpatialClusterData(iRate),
             gtTotalCases(Interface.GetTotalCasesCount()),
             gtTotalMeasure(Interface.GetTotalMeasureCount()) {
  InitializeData();
}

/** constructor */
SpatialData::SpatialData(const AbtractDataStreamGateway & DataGateway, int iRate)
            :AbstractSpatialClusterData(iRate),
             gtTotalCases(DataGateway.GetDataStreamInterface(0).GetTotalCasesCount()),
             gtTotalMeasure(DataGateway.GetDataStreamInterface(0).GetTotalMeasureCount()) {
  InitializeData();
}

/** destructor */
SpatialData::~SpatialData() {}

/** returns newly cloned SpatialData object */
SpatialData * SpatialData::Clone() const {
  return new SpatialData(*this);
}

/** assigns cluster data of passed object to *this* object
    NOTE: Caller of function is responsible for ensuring that passed object
          is of same class type as *this* object. */
void SpatialData::Assign(const AbstractSpatialClusterData& rhs) {
  //cast to SpatialData type
  const SpatialData& _rhs = (const SpatialData&)rhs;
  //copy data members
  gtTotalCases = _rhs.gtTotalCases;
  gtTotalMeasure = _rhs.gtTotalMeasure;
  gtCases = _rhs.gtCases;
  gtMeasure = _rhs.gtMeasure;
  gfRateOfInterest = _rhs.gfRateOfInterest;
}

/** overloaded assignment operator */
SpatialData & SpatialData::operator=(const SpatialData& rhs) {
  gtTotalCases = rhs.gtTotalCases;
  gtTotalMeasure = rhs.gtTotalMeasure;
  gtCases = rhs.gtCases;
  gtMeasure = rhs.gtMeasure;
  gfRateOfInterest = rhs.gfRateOfInterest;
  return *this;
}

/** adds neighbor data to accumulation - takes only first DataInteface of DataGateway */
void SpatialData::AddNeighborData(tract_t tNeighbor, const AbtractDataStreamGateway & DataGateway, size_t tStream) {
  gtCases += DataGateway.GetDataStreamInterface(tStream).GetPSCaseArray()[tNeighbor];
  gtMeasure += DataGateway.GetDataStreamInterface(tStream).GetPSMeasureArray()[tNeighbor];
}

/** Calculates loglikelihood ratio given current accumulated cluster data if
    it is determined that data fits scanning area of interest (high, low, both).
    Returns zero if rate not of interest else returns loglikelihood ratio as
    calculated by probability model. */
double SpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator & Calculator) {
  if (gfRateOfInterest(gtCases, gtMeasure, gtTotalCases, gtTotalMeasure))
    return Calculator.CalcLogLikelihoodRatio(gtCases, gtMeasure, gtTotalCases, gtTotalMeasure);
  return 0;  
}

/** returns number of cases accumulated in cluster data */
count_t SpatialData::GetCaseCount(unsigned int) const {
  return gtCases;
}

/** returns number of expected cases accumulated in cluster data */
measure_t SpatialData::GetMeasure(unsigned int) const {
  return gtMeasure;
}


/** protected constructor - accessible by derived  classes */
TemporalData::TemporalData()
             :AbstractTemporalClusterData(), gpCases(0), gpMeasure(0),
              gtTotalCases(0), gtTotalMeasure(0) {
  InitializeData();
}

/** constructor */
TemporalData::TemporalData(const DataStreamInterface & Interface)
             :AbstractTemporalClusterData(),
              gpCases(Interface.GetPTCaseArray()), gpMeasure(Interface.GetPTMeasureArray()),
              gtTotalCases(Interface.GetTotalCasesCount()), gtTotalMeasure(Interface.GetTotalMeasureCount()) {
  InitializeData();
}

/** constructor */
TemporalData::TemporalData(const AbtractDataStreamGateway & DataGateway)
             :AbstractTemporalClusterData(),
              gpCases(DataGateway.GetDataStreamInterface(0).GetPTCaseArray()),
              gpMeasure(DataGateway.GetDataStreamInterface(0).GetPTMeasureArray()),
              gtTotalCases(DataGateway.GetDataStreamInterface(0).GetTotalCasesCount()),
              gtTotalMeasure(DataGateway.GetDataStreamInterface(0).GetTotalMeasureCount()) {
  InitializeData();
}

/** destructor */
TemporalData::~TemporalData() {}

/** returns newly cloned TemporalData object */
TemporalData * TemporalData::Clone() const {
  return new TemporalData(*this);
}

/** assigns cluster data of passed object to *this* object
    NOTE: Caller of function is responsible for ensuring that passed object
          is of same class type *this* object. */
void TemporalData::Assign(const AbstractTemporalClusterData& rhs) {
  const TemporalData& _rhs = (const TemporalData&)rhs;
  gtTotalCases = _rhs.gtTotalCases;
  gtTotalMeasure = _rhs.gtTotalMeasure;
  gtCases = _rhs.gtCases;
  gtMeasure = _rhs.gtMeasure;
  gpCases = _rhs.gpCases;
  gpMeasure = _rhs.gpMeasure;
}

/** overloaded assignment operator */
TemporalData & TemporalData::operator=(const TemporalData& rhs) {
  gtTotalCases = rhs.gtTotalCases;
  gtTotalMeasure = rhs.gtTotalMeasure;
  gtCases = rhs.gtCases;
  gtMeasure = rhs.gtMeasure;
  gpCases = rhs.gpCases;
  gpMeasure = rhs.gpMeasure;
  return *this;
}

/** not implemented - throws exception */
void TemporalData::AddNeighborData(tract_t, const AbtractDataStreamGateway&, size_t) {
  ZdGenerateException("AddNeighborData(tract_t,const AbtractDataStreamGateway&, size_t) not implemeneted.","TemporalData");
}

/** not implemented - throws exception  -- returns size of allocated arrays */
unsigned int TemporalData::GetAllocationSize() const {
  ZdGenerateException("GetAllocationSize() not implemeneted.","TemporalData");
  return 0;
}

/** returns number of cases in accumulated cluster data */
count_t TemporalData::GetCaseCount(unsigned int) const {
  return gtCases;
}

/** returns number of expected cases in accumulated cluster data */
measure_t TemporalData::GetMeasure(unsigned int) const {
  return gtMeasure;
}




/** constructor */
ProspectiveSpatialData::ProspectiveSpatialData(const CSaTScanData & Data, const DataStreamInterface & Interface)
                       :TemporalData() {
  try {
    Init();
    Setup(Data, Interface);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","ProspectiveSpatialData");
    throw;
  }
}

/** constructor */
ProspectiveSpatialData::ProspectiveSpatialData(const CSaTScanData & Data, const AbtractDataStreamGateway & DataGateway)
                       :TemporalData() {
  try {
    Init();
    Setup(Data, DataGateway.GetDataStreamInterface(0));
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","ProspectiveSpatialData");
    throw;
  }
}

/** copy constructor */
ProspectiveSpatialData::ProspectiveSpatialData(const ProspectiveSpatialData& rhs)
                       :TemporalData(), giAllocationSize(rhs.giAllocationSize) {
  try {
    Init();
    gpCases = new count_t[rhs.giAllocationSize];
    gpMeasure = new measure_t[rhs.giAllocationSize];
    *this = rhs;
  }
  catch (ZdException &x) {
    delete[] gpCases;
    delete[] gpMeasure;
    x.AddCallpath("copy constructor()","ProspectiveSpatialData");
    throw;
  }
}

/** destructor */
ProspectiveSpatialData::~ProspectiveSpatialData() {
  try {
    delete[] gpCases;
    delete[] gpMeasure;
  }
  catch (...){}
}

/** returns newly cloned SpaceTimeData object */
ProspectiveSpatialData * ProspectiveSpatialData::Clone() const {
   return new ProspectiveSpatialData(*this);
}

/** assigns cluster data of passed object to *this* object
    NOTE: Caller of function is responsible for ensuring that passed object
          is of same class type *this* object. */
void ProspectiveSpatialData::Assign(const AbstractTemporalClusterData& rhs) {
  const ProspectiveSpatialData& _rhs = (const ProspectiveSpatialData&)rhs;
  gtCases = _rhs.gtCases;
  gtMeasure = _rhs.gtMeasure;
  gtTotalCases = _rhs.gtTotalCases;
  gtTotalMeasure = _rhs.gtTotalMeasure;
  giAllocationSize = _rhs.giAllocationSize;
  giNumTimeIntervals = _rhs.giNumTimeIntervals;
  giProspectiveStart = _rhs.giProspectiveStart;
  memcpy(gpCases, _rhs.gpCases, giAllocationSize * sizeof(count_t));
  memcpy(gpMeasure, _rhs.gpMeasure, giAllocationSize * sizeof(measure_t));
}

/** overloaded assignement operator */
ProspectiveSpatialData & ProspectiveSpatialData::operator=(const ProspectiveSpatialData& rhs) {
  gtCases = rhs.gtCases;
  gtMeasure = rhs.gtMeasure;
  gtTotalCases = rhs.gtTotalCases;
  gtTotalMeasure = rhs.gtTotalMeasure;
  giAllocationSize = rhs.giAllocationSize;
  giNumTimeIntervals = rhs.giNumTimeIntervals;
  giProspectiveStart = rhs.giProspectiveStart;
  memcpy(gpCases, rhs.gpCases, giAllocationSize * sizeof(count_t));
  memcpy(gpMeasure, rhs.gpMeasure, giAllocationSize * sizeof(measure_t));
  return *this;
}

/** adds neigbor data to accumulation and updates measure list */
void ProspectiveSpatialData::AddMeasureList(tract_t tEllipseOffset, tract_t tCentroid, const DataStreamInterface & Interface, CMeasureList * pMeasureList, const CSaTScanData * pData) {
  unsigned int           i, j, iWindowEnd;
  count_t             ** ppCases = Interface.GetCaseArray();
  measure_t           ** ppMeasure = Interface.GetMeasureArray();
  tract_t                t, tNeighbor, tNumNeighbors = pData->GetNeighborCountArray()[tEllipseOffset][tCentroid];

  InitializeData(); //replace with direct code ?
  for (t=1; t <= tNumNeighbors; ++t) {
    tNeighbor = pData->GetNeighbor(tEllipseOffset, tCentroid, t);
    //update accumulated data
    gpCases[0]   += ppCases[0][tNeighbor]; //set cases for entire period added by this neighbor
    gpMeasure[0] += ppMeasure[0][tNeighbor];
    //set cases for prospective study periods
    for (j=1, i=giProspectiveStart; i < giNumTimeIntervals; ++j, ++i) {
       gpCases[j] += ppCases[i][tNeighbor];
       gpMeasure[j] += ppMeasure[i][tNeighbor];
    }
    //update measure list
    for (iWindowEnd=1; iWindowEnd < giAllocationSize; ++iWindowEnd)
       pMeasureList->AddMeasure(gpCases[0] - gpCases[iWindowEnd], gpMeasure[0] - gpMeasure[iWindowEnd]);
    pMeasureList->AddMeasure(gpCases[0], gpMeasure[0]);
  }
}

/** Adds neighbor data to accumulation - only first DataInterface is accessed. */
void ProspectiveSpatialData::AddNeighborData(tract_t tNeighbor, const AbtractDataStreamGateway& DataGateway, size_t tStream) {
  unsigned int           i, j;
  count_t             ** ppCases = DataGateway.GetDataStreamInterface(tStream).GetCaseArray();
  measure_t           ** ppMeasure = DataGateway.GetDataStreamInterface(tStream).GetMeasureArray();

  //set cases for entire period added by this neighbor
  gpCases[0]   += ppCases[0][tNeighbor];
  gpMeasure[0] += ppMeasure[0][tNeighbor];
  //set cases for prospective study periods
  for (j=1, i=giProspectiveStart; i < giNumTimeIntervals; ++j, ++i) {
     gpCases[j] += ppCases[i][tNeighbor];
     gpMeasure[j] += ppMeasure[i][tNeighbor];
  }  
}

/** Calculates loglikelihood ratio given current accumulated cluster data if
    it is determined that data fits scanning area of interest (high, low, both).
    Returns zero if all windows rates not of interest else returns greatest
    loglikelihood ratio as calculated by probability model. */
double ProspectiveSpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator & Calculator) {
  unsigned int  iWindowEnd;
  double        dMaxLoglikelihoodRatio=0;

  gtCases = gpCases[0];
  gtMeasure =  gpMeasure[0];
  if (gfRateOfInterest(gtCases, gtMeasure, gtTotalCases, gtTotalMeasure))
    dMaxLoglikelihoodRatio = Calculator.CalcLogLikelihoodRatio(gtCases, gtMeasure, gtTotalCases, gtTotalMeasure);

  for (iWindowEnd=1; iWindowEnd < giAllocationSize; ++iWindowEnd) {
    gtCases = gpCases[0] - gpCases[iWindowEnd];
    gtMeasure =  gpMeasure[0] - gpMeasure[iWindowEnd];
    if (gfRateOfInterest(gtCases, gtMeasure, gtTotalCases, gtTotalMeasure))
      dMaxLoglikelihoodRatio = std::max(dMaxLoglikelihoodRatio,
                                        Calculator.CalcLogLikelihoodRatio(gtCases, gtMeasure, gtTotalCases, gtTotalMeasure));
  }
  return dMaxLoglikelihoodRatio;
}

/** internal setup function */
void ProspectiveSpatialData::Setup(const CSaTScanData & Data, const DataStreamInterface & Interface) {
  try {
    giAllocationSize = 1 + Data.m_nTimeIntervals - Data.m_nProspectiveIntervalStart;
    giNumTimeIntervals = Data.m_nTimeIntervals;
    giProspectiveStart = Data.m_nProspectiveIntervalStart;
    gtTotalCases = Interface.GetTotalCasesCount();
    gtTotalMeasure = Interface.GetTotalMeasureCount();

    gpCases = new count_t[giAllocationSize];
    memset(gpCases, 0, sizeof(count_t) * giAllocationSize);
    gpMeasure = new measure_t[giAllocationSize];
    memset(gpMeasure, 0, sizeof(measure_t) * giAllocationSize);
    switch (Data.GetParameters().GetAreaScanRateType()) {
      case LOW        : gfRateOfInterest = LowRate;       break;
      case HIGHANDLOW : gfRateOfInterest = HighOrLowRate; break;
      case HIGH       :
      default         : gfRateOfInterest = HighRate;
    };
  }
  catch (ZdException &x) {
    delete[] gpCases;
    delete[] gpMeasure;
    x.AddCallpath("Setup()","ProspectiveSpatialData");
    throw;
  }
}


/** constructor */
SpaceTimeData::SpaceTimeData(const DataStreamInterface & Interface)
              :TemporalData() {
  try {
    Init();
    Setup(Interface);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","SpaceTimeData");
    throw;
  }
}

/** constructor */
SpaceTimeData::SpaceTimeData(const AbtractDataStreamGateway & DataGateway)
              :TemporalData() {
  try {
    Init();
    Setup(DataGateway.GetDataStreamInterface(0));
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","SpaceTimeData");
    throw;
  }
}

/** copy constructor */
SpaceTimeData::SpaceTimeData(const SpaceTimeData& rhs)
              :TemporalData(), giAllocationSize(rhs.giAllocationSize) {
  try {
    Init();
    gpCases = new count_t[rhs.giAllocationSize];
    gpMeasure = new measure_t[rhs.giAllocationSize];
    *this = rhs;
  }
  catch (ZdException &x) {
    delete[] gpCases;
    delete[] gpMeasure;
    x.AddCallpath("copy constructor()","SpaceTimeData");
    throw;
  }
}

/** destructor */
SpaceTimeData::~SpaceTimeData() {
  try {
    delete[] gpCases;
    delete[] gpMeasure;
  }
  catch (...){}
}

/** returns newly cloned SpaceTimeData object */
SpaceTimeData * SpaceTimeData::Clone() const {
   return new SpaceTimeData(*this);
}

/** assigns cluster data of passed object to *this* object
    NOTE: Caller of function is responsible for ensuring that passed object
          is of same class type as *this* object. */
void SpaceTimeData::Assign(const AbstractTemporalClusterData& rhs) {
  const SpaceTimeData& _rhs = (const SpaceTimeData&)rhs;
  gtCases = _rhs.gtCases;
  gtMeasure = _rhs.gtMeasure;
  gtTotalCases = _rhs.gtTotalCases;
  gtTotalMeasure = _rhs.gtTotalMeasure;
  giAllocationSize = _rhs.giAllocationSize;
  memcpy(gpCases, _rhs.gpCases, giAllocationSize * sizeof(count_t));
  memcpy(gpMeasure, _rhs.gpMeasure, giAllocationSize * sizeof(measure_t));
}

/** overloaded assignement operator */
SpaceTimeData & SpaceTimeData::operator=(const SpaceTimeData& rhs) {
  gtCases = rhs.gtCases;
  gtMeasure = rhs.gtMeasure;
  gtTotalCases = rhs.gtTotalCases;
  gtTotalMeasure = rhs.gtTotalMeasure;
  giAllocationSize = rhs.giAllocationSize;
  memcpy(gpCases, rhs.gpCases, giAllocationSize * sizeof(count_t));
  memcpy(gpMeasure, rhs.gpMeasure, giAllocationSize * sizeof(measure_t));
  return *this;
}

/** adds neighbor data to accumulation */
void SpaceTimeData::AddNeighborDataAndCompare(tract_t tEllipseOffset,
                                              tract_t tCentroid,
                                              const DataStreamInterface & Interface,
                                              const CSaTScanData * pData,
                                              CTimeIntervals * pTimeIntervals,
                                              CMeasureList * pMeasureList) {

  unsigned int  i, iIntervals = giAllocationSize - 1;
  count_t    ** ppCases = Interface.GetCaseArray();
  measure_t  ** ppMeasure = Interface.GetMeasureArray();
  tract_t       t, tNeighbor, tNumNeighbors = pData->GetNeighborCountArray()[tEllipseOffset][tCentroid];

  InitializeData(); //replace with direct code ?
  for (t=1; t <= tNumNeighbors; t++) {
     tNeighbor = pData->GetNeighbor(tEllipseOffset, tCentroid, t);
     for (i=0; i < iIntervals; i++) {
       gpCases[i] += ppCases[i][tNeighbor];
       gpMeasure[i] += ppMeasure[i][tNeighbor];
     }
     pTimeIntervals->CompareMeasures(this, pMeasureList);
  }
}

/** adds neighbor data to accumulation - only first DataInterface accessed */
void SpaceTimeData::AddNeighborData(tract_t tNeighbor, const AbtractDataStreamGateway& DataGateway, size_t tStream) {
  count_t    ** ppCases = DataGateway.GetDataStreamInterface(tStream).GetCaseArray();
  measure_t  ** ppMeasure = DataGateway.GetDataStreamInterface(tStream).GetMeasureArray();

  for (unsigned int i=0; i < giAllocationSize - 1; ++i) {
     gpCases[i] += ppCases[i][tNeighbor];
     gpMeasure[i] += ppMeasure[i][tNeighbor];
  }
}

/** internal setup function */
void SpaceTimeData::Setup(const DataStreamInterface & Interface) {
  try {
    giAllocationSize = Interface.GetNumTimIntervals() + 1; 
    gtTotalCases = Interface.GetTotalCasesCount();
    gtTotalMeasure = Interface.GetTotalMeasureCount();
    gpCases = new count_t[giAllocationSize];
    memset(gpCases, 0, sizeof(count_t) * giAllocationSize);
    gpMeasure = new measure_t[giAllocationSize];
    memset(gpMeasure, 0, sizeof(measure_t) * giAllocationSize);
  }
  catch (ZdException &x) {
    delete[] gpCases;
    delete[] gpMeasure;
    x.AddCallpath("Setup()","SpaceTimeData");
    throw;
  }
}

