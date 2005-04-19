//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "NormalClusterData.h"

/** class constructor */
NormalSpatialData::NormalSpatialData(const AbtractDataSetGateway& DataGateway, int iRate)
                  :SpatialData(DataGateway, iRate), gtSqMeasure(0) {}

/** class destrcutor */
NormalSpatialData::~NormalSpatialData() {}

/** Returns newly cloned NormalSpatialData object. Caller is responsible for deletion of object. */
NormalSpatialData * NormalSpatialData::Clone() const {
  return new NormalSpatialData(*this);
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractSpatialClusterData object
    can be casted to 'NormalSpatialData' object. */
void NormalSpatialData::Assign(const AbstractSpatialClusterData& rhs) {
  const NormalSpatialData& _rhs = (const NormalSpatialData&)rhs;
  gtTotalCases = _rhs.gtTotalCases;
  gtTotalMeasure = _rhs.gtTotalMeasure;
  gtCases = _rhs.gtCases;
  gtMeasure = _rhs.gtMeasure;
  gtSqMeasure = _rhs.gtSqMeasure;
  gfRateOfInterest = _rhs.gfRateOfInterest;
}

/** Not implemeneted - throws ZdException. */
void NormalSpatialData::AddMeasureList(const DataSetInterface&, CMeasureList*, const CSaTScanData*) {
   ZdGenerateException("AddMeasureList(const DataSetInterface&, CMeasureList*, const CSaTScanData*) not implemented.","NormalSpatialData");
}

/** Adds neighbor data to accumulation  - caller is responsible for ensuring that
    'tNeighborIndex' and 'tSetIndex' are valid indexes. */
void NormalSpatialData::AddNeighborData(tract_t tNeighborIndex, const AbtractDataSetGateway& DataGateway, size_t tSetIndex) {
  gtCases += DataGateway.GetDataSetInterface(tSetIndex).GetPSCaseArray()[tNeighborIndex];
  gtMeasure += DataGateway.GetDataSetInterface(tSetIndex).GetPSMeasureArray()[tNeighborIndex];
  gtSqMeasure += DataGateway.GetDataSetInterface(tSetIndex).GetPSSqMeasureArray()[tNeighborIndex];
}

/** Calculates loglikelihood ratio, given current accumulated cluster data, if
    it is determined that data fits scanning area of interest (high, low, both).
    Returns zero if rate not of interest else returns loglikelihood ratio as
    calculated by probability model. */
double NormalSpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
  if (gfRateOfInterest(gtCases, gtMeasure, gtTotalCases, gtTotalMeasure))
    return Calculator.CalcLogLikelihoodRatioNormal(gtCases, gtMeasure, gtSqMeasure, gtTotalCases, gtTotalMeasure);
  return 0;  
}

//******************************************************************************

/** class constructor */
NormalTemporalData::NormalTemporalData() : TemporalData(), gtSqMeasure(0), gpSqMeasure(0) {}

/** class constructor */
NormalTemporalData::NormalTemporalData(const AbtractDataSetGateway& DataGateway)
                   :TemporalData(DataGateway), gtSqMeasure(0),
                    gpSqMeasure(DataGateway.GetDataSetInterface(0).GetPTSqMeasureArray()) {}

/** class destructor */
NormalTemporalData::~NormalTemporalData() {}

/** Returns newly cloned NormalTemporalData object. Caller responsible for deletion of object. */
NormalTemporalData * NormalTemporalData::Clone() const {
  return new NormalTemporalData(*this);
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractTemporalClusterData object
    can be casted to 'NormalTemporalData' object. */
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

//******************************************************************************

/** class constructor */
NormalProspectiveSpatialData::NormalProspectiveSpatialData(const CSaTScanData& Data, const DataSetInterface& Interface)
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

/** class constructor */
NormalProspectiveSpatialData::NormalProspectiveSpatialData(const CSaTScanData& Data, const AbtractDataSetGateway& DataGateway)
                             :NormalTemporalData() {
  try {
    Init();
    Setup(Data, DataGateway.GetDataSetInterface(0));
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","NormalProspectiveSpatialData");
    throw;
  }
}

/** class copy constructor */
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

/** class destructor */
NormalProspectiveSpatialData::~NormalProspectiveSpatialData() {
  try {
    delete[] gpCases;
    delete[] gpMeasure;
    delete[] gpSqMeasure;
  }
  catch (...){}
}

/** Returns newly cloned NormalProspectiveSpatialData object. Caller responsible
    for deletion of object. */
NormalProspectiveSpatialData * NormalProspectiveSpatialData::Clone() const {
   return new NormalProspectiveSpatialData(*this);
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractTemporalClusterData object
    can be casted to 'NormalProspectiveSpatialData' object. */
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

/** Adds neighbor data to accumulation - caller is responsible for ensuring that
    'tNeighborIndex' and 'tSetIndex' are valid indexes. */
void NormalProspectiveSpatialData::AddNeighborData(tract_t tNeighborIndex, const AbtractDataSetGateway& DataGateway, size_t tSetIndex) {
  unsigned int           i, j;
  count_t             ** ppCases = DataGateway.GetDataSetInterface(tSetIndex).GetCaseArray();
  measure_t           ** ppMeasure = DataGateway.GetDataSetInterface(tSetIndex).GetMeasureArray();
  measure_t           ** ppSqMeasure = DataGateway.GetDataSetInterface(tSetIndex).GetSqMeasureArray();

  //set cases for entire period added by this neighbor
  gpCases[0]   += ppCases[0][tNeighborIndex];
  gpMeasure[0] += ppMeasure[0][tNeighborIndex];
  gpSqMeasure[0] += ppSqMeasure[0][tNeighborIndex];

  for (j=1, i=giProspectiveStart; i < giNumTimeIntervals; ++j, ++i) {
     gpCases[j] += ppCases[i][tNeighborIndex];
     gpMeasure[j] += ppMeasure[i][tNeighborIndex];
     gpSqMeasure[j] += ppSqMeasure[i][tNeighborIndex];
  }
}

/** Calculates loglikelihood ratio, given current accumulated cluster data, if
    it is determined that data fits scanning area of interest (high, low, both).
    Returns zero if all windows rates not of interest else returns greatest
    loglikelihood ratio as calculated by probability model. */
double NormalProspectiveSpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
  unsigned int  iWindowEnd;
  double        dMaxLoglikelihoodRatio=0;

  gtCases = gpCases[0];
  gtMeasure = gpMeasure[0];
  gtSqMeasure = gpSqMeasure[0];
  if (gfRateOfInterest(gtCases, gtMeasure, gtTotalCases, gtTotalMeasure))
    dMaxLoglikelihoodRatio = Calculator.CalcLogLikelihoodRatioNormal(gtCases, gtMeasure, gtSqMeasure, gtTotalCases, gtTotalMeasure);

  for (iWindowEnd=1; iWindowEnd < giAllocationSize; ++iWindowEnd) {
    gtCases = gpCases[0] - gpCases[iWindowEnd];
    gtMeasure = gpMeasure[0] - gpMeasure[iWindowEnd];
    gtSqMeasure = gpSqMeasure[0] - gpSqMeasure[iWindowEnd];
    if (gfRateOfInterest(gtCases, gtMeasure, gtTotalCases, gtTotalMeasure))
      dMaxLoglikelihoodRatio = std::max(dMaxLoglikelihoodRatio,
                                        Calculator.CalcLogLikelihoodRatioNormal(gtCases, gtMeasure, gtSqMeasure, gtTotalCases, gtTotalMeasure));
  }
  return dMaxLoglikelihoodRatio;
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
void NormalProspectiveSpatialData::Setup(const CSaTScanData& Data, const DataSetInterface& Interface) {
  try {
    giAllocationSize = 1 + Data.m_nTimeIntervals - Data.GetProspectiveStartIndex();
    giNumTimeIntervals = Data.m_nTimeIntervals;
    giProspectiveStart = Data.GetProspectiveStartIndex();
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
    x.AddCallpath("Setup(const CSaTScanData&, const DataSetInterface&)","NormalProspectiveSpatialData");
    throw;
  }
}

//******************************************************************************

/** class constructor */
NormalSpaceTimeData::NormalSpaceTimeData(const DataSetInterface& Interface)
                    :NormalTemporalData() {
  try {
    Init();
    Setup(Interface);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor(const DataSetInterface&)","NormalSpaceTimeData");
    throw;
  }
}

/** constructor */
NormalSpaceTimeData::NormalSpaceTimeData(const AbtractDataSetGateway& DataGateway)
                    :NormalTemporalData() {
  try {
    Init();
    Setup(DataGateway.GetDataSetInterface(0));
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor(const AbtractDataSetGateway&)","NormalSpaceTimeData");
    throw;
  }
}

/** class copy constructor */
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
    x.AddCallpath("constructor(const NormalSpaceTimeData&)","NormalSpaceTimeData");
    throw;
  }
}

/** class destructor */
NormalSpaceTimeData::~NormalSpaceTimeData() {
  try {
    delete[] gpCases;
    delete[] gpMeasure;
    delete[] gpSqMeasure;
  }
  catch (...){}
}

/** Returns newly cloned NormalSpaceTimeData object. Caller responsible for deletion
    of object. */
NormalSpaceTimeData * NormalSpaceTimeData::Clone() const {
   return new NormalSpaceTimeData(*this);
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractTemporalClusterData object
    can be casted to 'NormalSpaceTimeData' object. */
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

/** Adds neighbor data to accumulation - caller is responsible for ensuring that
    'tNeighborIndex' and 'tSetIndex' are valid indexes. */
void NormalSpaceTimeData::AddNeighborData(tract_t tNeighborIndex, const AbtractDataSetGateway& DataGateway, size_t tSetIndex) {
  count_t    ** ppCases = DataGateway.GetDataSetInterface(tSetIndex).GetCaseArray();
  measure_t  ** ppMeasure = DataGateway.GetDataSetInterface(tSetIndex).GetMeasureArray();
  measure_t  ** ppSqMeasure = DataGateway.GetDataSetInterface(tSetIndex).GetSqMeasureArray();

  for (unsigned int i=0; i < giAllocationSize; ++i) {
     gpCases[i] += ppCases[i][tNeighborIndex];
     gpMeasure[i] += ppMeasure[i][tNeighborIndex];
     gpSqMeasure[i] += ppSqMeasure[i][tNeighborIndex];
  }
}

/** re-initialize data */
void NormalSpaceTimeData::InitializeData() {
  gtCases=0;
  gtMeasure=0;
  gtSqMeasure=0;
  memset(gpCases, 0, sizeof(count_t) * giAllocationSize);
  memset(gpMeasure, 0, sizeof(measure_t) * giAllocationSize);
  memset(gpSqMeasure, 0, sizeof(measure_t) * giAllocationSize);
}

/** internal setup function */
void NormalSpaceTimeData::Setup(const DataSetInterface& Interface) {
  try {
    giAllocationSize = Interface.GetNumTimeIntervals() + 1; 
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
    x.AddCallpath("Setup(const DataSetInterface&)","NormalSpaceTimeData");
    throw;
  }
}
