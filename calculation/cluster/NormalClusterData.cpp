//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "NormalClusterData.h"


/** constructor */
NormalSpatialData::NormalSpatialData(const AbtractDataStreamGateway & DataGateway, int iRate)
                  :SpatialData(DataGateway, iRate), gtSqMeasure(0) {}

/** destrcutor */
NormalSpatialData::~NormalSpatialData() {}

/** returns newly cloned NormalSpatialData object */
NormalSpatialData * NormalSpatialData::Clone() const {
  return new NormalSpatialData(*this);
}

/** assigns cluster data of passed object to *this* object
    NOTE: Caller of function is responsible for ensuring that passed object
          is of same class type as *this* object. */
void NormalSpatialData::Assign(const AbstractSpatialClusterData& rhs) {
  const NormalSpatialData& _rhs = (const NormalSpatialData&)rhs;
  gtTotalCases = _rhs.gtTotalCases;
  gtTotalMeasure = _rhs.gtTotalMeasure;
  gtCases = _rhs.gtCases;
  gtMeasure = _rhs.gtMeasure;
  gtSqMeasure = _rhs.gtSqMeasure;
  gfRateOfInterest = _rhs.gfRateOfInterest;
}

/** not implemeneted - throw exception */
void NormalSpatialData::AddMeasureList(const DataStreamInterface & Interface, CMeasureList * pMeasureList, const CSaTScanData * pData) {
   ZdGenerateException("AddMeasureList() not implemented.","NormalSpatialData");
}

/** adds neighbor data to accumulation - only first DataInterface is accessed */
void NormalSpatialData::AddNeighborData(tract_t tNeighbor, const AbtractDataStreamGateway & DataGateway, size_t tStream) {
  gtCases += DataGateway.GetDataStreamInterface(tStream).GetPSCaseArray()[tNeighbor];
  gtMeasure += DataGateway.GetDataStreamInterface(tStream).GetPSMeasureArray()[tNeighbor];
  gtSqMeasure += DataGateway.GetDataStreamInterface(tStream).GetPSSqMeasureArray()[tNeighbor];
}

/** Calculates loglikelihood ratio given current accumulated cluster data if
    it is determined that data fits scanning area of interest (high, low, both).
    Returns zero if rate not of interest else returns loglikelihood ratio as
    calculated by probability model. */
double NormalSpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator & Calculator) {
  if (gfRateOfInterest(gtCases, gtMeasure, gtTotalCases, gtTotalMeasure))
    return Calculator.CalcLogLikelihoodRatioEx(gtCases, gtMeasure, gtSqMeasure, gtTotalCases, gtTotalMeasure);
  return 0;  
}



/** constructor */
NormalTemporalData::NormalTemporalData() : TemporalData(), gtSqMeasure(0), gpSqMeasure(0) {}

/** constructor */
NormalTemporalData::NormalTemporalData(const AbtractDataStreamGateway & DataGateway)
                   :TemporalData(DataGateway), gtSqMeasure(0),
                    gpSqMeasure(DataGateway.GetDataStreamInterface(0).GetPTSqMeasureArray()) {}

/** destructor */
NormalTemporalData::~NormalTemporalData() {}

/** returns newly cloned NormalTemporalData object */
NormalTemporalData * NormalTemporalData::Clone() const {
  return new NormalTemporalData(*this);
}

/** assigns cluster data of passed object to *this* object
    NOTE: Caller of function is responsible for ensuring that passed object
          is of same class type as *this* object. */
void NormalTemporalData::Assign(const AbstractTemporalClusterData& rhs) {
  const NormalTemporalData& _rhs = (const NormalTemporalData&)rhs;
  gtTotalCases = _rhs.gtTotalCases;
  gtTotalMeasure = _rhs.gtTotalMeasure;
  gtCases = _rhs.gtCases;
  gtMeasure = _rhs.gtMeasure;
  gtSqMeasure - _rhs.gtSqMeasure;
  gpCases = _rhs.gpCases;
  gpMeasure = _rhs.gpMeasure;
  gpSqMeasure = _rhs.gpSqMeasure;
}



/** constructor */
NormalProspectiveSpatialData::NormalProspectiveSpatialData(const CSaTScanData & Data, const DataStreamInterface & Interface)
                             :NormalTemporalData() {
  try {
    Init();
    Setup(Data, Interface);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","NormalProspectiveSpatialData");
    throw;
  }
}

/** constructor */
NormalProspectiveSpatialData::NormalProspectiveSpatialData(const CSaTScanData & Data, const AbtractDataStreamGateway & DataGateway)
                             :NormalTemporalData() {
  try {
    Init();
    Setup(Data, DataGateway.GetDataStreamInterface(0));
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","NormalProspectiveSpatialData");
    throw;
  }
}

/** copy constructor */
NormalProspectiveSpatialData::NormalProspectiveSpatialData(const NormalProspectiveSpatialData& rhs)
                             :NormalTemporalData(), giAllocationSize(rhs.giAllocationSize) {
  try {
    Init();
    gpCases = new count_t[rhs.giAllocationSize];
    gpMeasure = new measure_t[rhs.giAllocationSize];
    gpSqMeasure = new measure_t[rhs.giAllocationSize];
    *this = rhs;
  }
  catch (ZdException &x) {
    delete[] gpCases;
    delete[] gpMeasure;
    delete[] gpSqMeasure;
    x.AddCallpath("copy constructor()","NormalProspectiveSpatialData");
    throw;
  }
}

/** destructor */
NormalProspectiveSpatialData::~NormalProspectiveSpatialData() {
  try {
    delete[] gpCases;
    delete[] gpMeasure;
    delete[] gpSqMeasure;
  }
  catch (...){}
}

/** returns newly cloned NormalProspectiveSpatialData object */
NormalProspectiveSpatialData * NormalProspectiveSpatialData::Clone() const {
   return new NormalProspectiveSpatialData(*this);
}

/** assigns cluster data of passed object to *this* object
    NOTE: Caller of function is responsible for ensuring that passed object
          is of same class type as *this* object. */
void NormalProspectiveSpatialData::Assign(const AbstractTemporalClusterData& rhs) {
  const NormalProspectiveSpatialData& _rhs = (const NormalProspectiveSpatialData&)rhs;
  gtCases = _rhs.gtCases;
  gtMeasure = _rhs.gtMeasure;
  gtTotalCases = _rhs.gtTotalCases;
  gtTotalMeasure = _rhs.gtTotalMeasure;
  giAllocationSize = _rhs.giAllocationSize;
  giNumTimeIntervals = _rhs.giNumTimeIntervals;
  giProspectiveStart = _rhs.giProspectiveStart;
  memcpy(gpCases, _rhs.gpCases, giAllocationSize * sizeof(count_t));
  memcpy(gpMeasure, _rhs.gpMeasure, giAllocationSize * sizeof(measure_t));
  memcpy(gpSqMeasure, _rhs.gpSqMeasure, giAllocationSize * sizeof(measure_t));
}

/** overloaded assignement operator */
NormalProspectiveSpatialData & NormalProspectiveSpatialData::operator=(const NormalProspectiveSpatialData& rhs) {
   gtCases = rhs.gtCases;
   gtMeasure = rhs.gtMeasure;
   gtTotalCases = rhs.gtTotalCases;
   gtTotalMeasure = rhs.gtTotalMeasure;
   giAllocationSize = rhs.giAllocationSize;
   giNumTimeIntervals = rhs.giNumTimeIntervals;
   giProspectiveStart = rhs.giProspectiveStart;
   memcpy(gpCases, rhs.gpCases, giAllocationSize * sizeof(count_t));
   memcpy(gpMeasure, rhs.gpMeasure, giAllocationSize * sizeof(measure_t));
   memcpy(gpSqMeasure, rhs.gpSqMeasure, giAllocationSize * sizeof(measure_t));
   return *this;
}

/** adds neighbor data to accumulation - only first DataInterface is accessed */
void NormalProspectiveSpatialData::AddNeighborData(tract_t tNeighbor, const AbtractDataStreamGateway& DataGateway, size_t tStream) {
  unsigned int           i, j;
  count_t             ** ppCases = DataGateway.GetDataStreamInterface(tStream).GetCaseArray();
  measure_t           ** ppMeasure = DataGateway.GetDataStreamInterface(tStream).GetMeasureArray();
  measure_t           ** ppSqMeasure = DataGateway.GetDataStreamInterface(tStream).GetSqMeasureArray();

  //set cases for entire period added by this neighbor
  gpCases[0]   += ppCases[0][tNeighbor];
  gpMeasure[0] += ppMeasure[0][tNeighbor];
  gpSqMeasure[0] += ppSqMeasure[0][tNeighbor];

  for (j=1, i=giProspectiveStart; i < giNumTimeIntervals; ++j, ++i) {
     gpCases[j] += ppCases[i][tNeighbor];
     gpMeasure[j] += ppMeasure[i][tNeighbor];
     gpSqMeasure[j] += ppSqMeasure[i][tNeighbor];
  }
}

/** Calculates loglikelihood ratio given current accumulated cluster data if
    it is determined that data fits scanning area of interest (high, low, both).
    Returns zero if all windows rates not of interest else returns greatest
    loglikelihood ratio as calculated by probability model. */
double NormalProspectiveSpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator & Calculator) {
  unsigned int  iWindowEnd;
  double        dLoglikelihood, dMaxLoglikelihood=0;

  gtCases = gpCases[0];
  gtMeasure =  gpMeasure[0];
  gtSqMeasure  =  gpSqMeasure[0];
  if (gfRateOfInterest(gtCases, gtMeasure, gtTotalCases, gtTotalMeasure)) {
    dLoglikelihood = Calculator.CalcLogLikelihoodRatioEx(gtCases, gtMeasure, gtSqMeasure, gtTotalCases, gtTotalMeasure);
    dMaxLoglikelihood = std::max(dMaxLoglikelihood, dLoglikelihood);
  }  

  for (iWindowEnd=1; iWindowEnd < giAllocationSize; ++iWindowEnd) {
    gtCases = gpCases[0] - gpCases[iWindowEnd];
    gtMeasure =  gpMeasure[0] - gpMeasure[iWindowEnd];
    gtSqMeasure =  gpSqMeasure[0] - gpSqMeasure[iWindowEnd];
    if (gfRateOfInterest(gtCases, gtMeasure, gtTotalCases, gtTotalMeasure)) {
      dLoglikelihood = Calculator.CalcLogLikelihoodRatioEx(gtCases, gtMeasure, gtSqMeasure, gtTotalCases, gtTotalMeasure);
      dMaxLoglikelihood = std::max(dMaxLoglikelihood, dLoglikelihood);
    }
  }
  return dMaxLoglikelihood;
}

/** re-initialize data*/
void NormalProspectiveSpatialData::InitializeData() {
  gtCases=0;
  gtMeasure=0;
  gtSqMeasure=0;
  memset(gpCases, 0, sizeof(count_t) * giAllocationSize);
  memset(gpMeasure, 0, sizeof(measure_t) * giAllocationSize);
  memset(gpSqMeasure, 0, sizeof(measure_t) * giAllocationSize);
}

/** internal setup function */
void NormalProspectiveSpatialData::Setup(const CSaTScanData & Data, const DataStreamInterface & Interface) {
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
    gpSqMeasure = new measure_t[giAllocationSize];
    memset(gpSqMeasure, 0, sizeof(measure_t) * giAllocationSize);
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
    delete[] gpSqMeasure;
    x.AddCallpath("Setup()","NormalProspectiveSpatialData");
    throw;
  }
}


/** constructor */
NormalSpaceTimeData::NormalSpaceTimeData(const DataStreamInterface & Interface)
                    :NormalTemporalData() {
  try {
    Init();
    Setup(Interface);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","NormalSpaceTimeData");
    throw;
  }
}

/** constructor */
NormalSpaceTimeData::NormalSpaceTimeData(const AbtractDataStreamGateway & DataGateway)
                    :NormalTemporalData() {
  try {
    Init();
    Setup(DataGateway.GetDataStreamInterface(0));
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","NormalSpaceTimeData");
    throw;
  }
}

/** copy constructor */
NormalSpaceTimeData::NormalSpaceTimeData(const NormalSpaceTimeData& rhs)
                    :NormalTemporalData(), giAllocationSize(rhs.giAllocationSize) {
  try {
    Init();
    gpCases = new count_t[giAllocationSize];
    gpMeasure = new measure_t[giAllocationSize];
    gpSqMeasure = new measure_t[giAllocationSize];
    *this = rhs;
  }
  catch (ZdException &x) {
    delete[] gpCases;
    delete[] gpMeasure;
    delete[] gpSqMeasure;
    x.AddCallpath("copy constructor()","NormalSpaceTimeData");
    throw;
  }
}

/** destructor */
NormalSpaceTimeData::~NormalSpaceTimeData() {
  try {
    delete[] gpCases;
    delete[] gpMeasure;
    delete[] gpSqMeasure;
  }
  catch (...){}
}

/** returns newly cloned NormalSpaceTimeData object */
NormalSpaceTimeData * NormalSpaceTimeData::Clone() const {
   return new NormalSpaceTimeData(*this);
}

/** assigns cluster data of passed object to *this* object
    NOTE: Caller of function is responsible for ensuring that passed object
          is of same class type as *this* object. */
void NormalSpaceTimeData::Assign(const AbstractTemporalClusterData& rhs) {
  const NormalSpaceTimeData& _rhs = (const NormalSpaceTimeData&)rhs;
  gtCases = _rhs.gtCases;
  gtMeasure = _rhs.gtMeasure;
  gtTotalCases = _rhs.gtTotalCases;
  gtTotalMeasure = _rhs.gtTotalMeasure;
  giAllocationSize = _rhs.giAllocationSize;
  memcpy(gpCases, _rhs.gpCases, giAllocationSize * sizeof(count_t));
  memcpy(gpMeasure, _rhs.gpMeasure, giAllocationSize * sizeof(measure_t));
  memcpy(gpSqMeasure, _rhs.gpSqMeasure, giAllocationSize * sizeof(measure_t));
}

/** overloaded assignement operator */
NormalSpaceTimeData & NormalSpaceTimeData::operator=(const NormalSpaceTimeData& rhs) {
  gtCases = rhs.gtCases;
  gtMeasure = rhs.gtMeasure;
  gtTotalCases = rhs.gtTotalCases;
  gtTotalMeasure = rhs.gtTotalMeasure;
  giAllocationSize = rhs.giAllocationSize;
  memcpy(gpCases, rhs.gpCases, giAllocationSize * sizeof(count_t));
  memcpy(gpMeasure, rhs.gpMeasure, giAllocationSize * sizeof(measure_t));
  memcpy(gpSqMeasure, rhs.gpSqMeasure, giAllocationSize * sizeof(measure_t));
  return *this;
}

/** adds neighbor data to accumulation - only first DataInterface accessed */
void NormalSpaceTimeData::AddNeighborData(tract_t tNeighbor, const AbtractDataStreamGateway& DataGateway, size_t tStream) {
  count_t    ** ppCases = DataGateway.GetDataStreamInterface(tStream).GetCaseArray();
  measure_t  ** ppMeasure = DataGateway.GetDataStreamInterface(tStream).GetMeasureArray();
  measure_t  ** ppSqMeasure = DataGateway.GetDataStreamInterface(tStream).GetSqMeasureArray();

  for (unsigned int i=0; i < giAllocationSize; ++i) {
     gpCases[i] += ppCases[i][tNeighbor];
     gpMeasure[i] += ppMeasure[i][tNeighbor];
     gpSqMeasure[i] += ppMeasure[i][tNeighbor];
  }
}

/** re-initialize data*/
void NormalSpaceTimeData::InitializeData() {
  gtCases=0;
  gtMeasure=0;
  gtSqMeasure=0;
  memset(gpCases, 0, sizeof(count_t) * giAllocationSize);
  memset(gpMeasure, 0, sizeof(measure_t) * giAllocationSize);
  memset(gpSqMeasure, 0, sizeof(measure_t) * giAllocationSize);
}

/** internal setup funciton */
void NormalSpaceTimeData::Setup(const DataStreamInterface& Interface) {
  try {
    giAllocationSize = Interface.GetNumTimIntervals() + 1; 
    gtTotalCases = Interface.GetTotalCasesCount();
    gtTotalMeasure = Interface.GetTotalMeasureCount();
    gpCases = new count_t[giAllocationSize];
    memset(gpCases, 0, sizeof(count_t) * giAllocationSize);
    gpMeasure = new measure_t[giAllocationSize];
    memset(gpMeasure, 0, sizeof(measure_t) * giAllocationSize);
    gpSqMeasure = new measure_t[giAllocationSize];
    memset(gpSqMeasure, 0, sizeof(measure_t) * giAllocationSize);
  }
  catch (ZdException &x) {
    delete[] gpCases;
    delete[] gpMeasure;
    delete[] gpSqMeasure;
    x.AddCallpath("Setup()","NormalSpaceTimeData");
    throw;
  }
}
