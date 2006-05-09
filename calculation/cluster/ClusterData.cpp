//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ClusterData.h"
#include "TimeIntervals.h"

//**************** class SpatialData *******************************************

/** class constructor */
SpatialData::SpatialData(const DataSetInterface& Interface) : AbstractSpatialClusterData() {
  InitializeData();
}

/** class constructor */
SpatialData::SpatialData(const AbstractDataSetGateway& DataGateway) : AbstractSpatialClusterData() {
  InitializeData();
}

/** Adds neighbor data to accumulation - caller is responsible for ensuring that
    'tNeighborIndex' and 'tSetIndex' are valid indexes. */
void SpatialData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex) {
  gtCases += DataGateway.GetDataSetInterface(tSetIndex).GetPSCaseArray()[tNeighborIndex];
  gtMeasure += DataGateway.GetDataSetInterface(tSetIndex).GetPSMeasureArray()[tNeighborIndex];
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractSpatialClusterData object
    can be casted to 'SpatialData' object. */
void SpatialData::Assign(const AbstractSpatialClusterData& rhs) {
  *this = (const SpatialData&)rhs;
}

/** Calculates loglikelihood ratio, given current accumulated cluster data, if
    it is determined that data fits scanning area of interest (high, low or both).
    Returns zero if rate not of interest else returns loglikelihood ratio as
    calculated by probability model. */
double SpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
  if ((Calculator.*Calculator.gpRateOfInterest)(gtCases, gtMeasure, 0))
    return Calculator.CalcLogLikelihoodRatio(gtCases, gtMeasure);
  return 0;
}

/** Calculates and returns maximizing value given accumulated data. If accumulated data
    is not significant, the negation of the maximum double is returned. */
double SpatialData::GetMaximizingValue(AbstractLikelihoodCalculator& Calculator) {
  if ((Calculator.*Calculator.gpRateOfInterest)(gtCases, gtMeasure, 0))
    return Calculator.CalculateMaximizingValue(gtCases, gtMeasure);
  return -std::numeric_limits<double>::max();
}

/** Returns newly cloned SpatialData object. Caller resonsible for deletion of object. */
SpatialData * SpatialData::Clone() const {
  return new SpatialData(*this);
}

void SpatialData::CopyEssentialClassMembers(const AbstractClusterData& rhs) {
  gtCases = ((const SpatialData&)rhs).gtCases;
  gtMeasure = ((const SpatialData&)rhs).gtMeasure;
}

/** Returns number of cases accumulated in cluster data. */
count_t SpatialData::GetCaseCount(unsigned int) const {
  return gtCases;
}

/** Returns number of expected cases accumulated in cluster data. */
measure_t SpatialData::GetMeasure(unsigned int) const {
  return gtMeasure;
}

/** Overloaded assignment operator. */
SpatialData & SpatialData::operator=(const SpatialData& rhs) {
  gtCases = rhs.gtCases;
  gtMeasure = rhs.gtMeasure;
  return *this;
}

//**************** class TemporalData ******************************************

/** Protected class constructor - accessible by derived classes only. */
TemporalData::TemporalData() : AbstractTemporalClusterData(), gpCases(0), gpMeasure(0) {
  InitializeData();
}

/** class constructor */
TemporalData::TemporalData(const DataSetInterface& Interface) : AbstractTemporalClusterData(),
              gpCases(Interface.GetPTCaseArray()), gpMeasure(Interface.GetPTMeasureArray()) {
  InitializeData();
}

/** class constructor */
TemporalData::TemporalData(const AbstractDataSetGateway& DataGateway): AbstractTemporalClusterData(),
              gpCases(DataGateway.GetDataSetInterface().GetPTCaseArray()),
              gpMeasure(DataGateway.GetDataSetInterface().GetPTMeasureArray()) {              
  InitializeData();
}

/** Not implemented - throws exception. */
void TemporalData::AddNeighborData(tract_t, const AbstractDataSetGateway&, size_t) {
  ZdGenerateException("AddNeighborData(tract_t, const AbstractDataSetGateway&, size_t) not implemeneted.","TemporalData");
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractTemporalClusterData object
    can be casted to 'TemporalData' object. */
void TemporalData::Assign(const AbstractTemporalClusterData& rhs) {
  *this = (const TemporalData&)rhs;
}

/** Returns newly cloned TemporalData object. Caller is responsible for deletion of object. */
TemporalData * TemporalData::Clone() const {
  return new TemporalData(*this);
}

/** Copies class data members that reflect the number of cases and expected values,
    which is the data we are interested in for possiblely reporting. */
void TemporalData::CopyEssentialClassMembers(const AbstractClusterData& rhs) {
  gtCases = ((const TemporalData&)rhs).gtCases;
  gtMeasure = ((const TemporalData&)rhs).gtMeasure;
}

/** Not implemented - throws exception. */
unsigned int TemporalData::GetAllocationSize() const {
  ZdGenerateException("GetAllocationSize() not implemeneted.","TemporalData");
  return 0;
}

/** Returns number of cases in accumulated cluster data. */
count_t TemporalData::GetCaseCount(unsigned int) const {
  return gtCases;
}

/** Returns number of expected cases in accumulated cluster data. */
measure_t TemporalData::GetMeasure(unsigned int) const {
  return gtMeasure;
}

/** Overloaded assignment operator. */
TemporalData & TemporalData::operator=(const TemporalData& rhs) {
  gtCases = rhs.gtCases;
  gtMeasure = rhs.gtMeasure;
  gpCases = rhs.gpCases;
  gpMeasure = rhs.gpMeasure;
  return *this;
}

/** Reassociates internal data with passed DataSetInterface pointers. */
void TemporalData::Reassociate(const DataSetInterface& Interface) {
  gpCases = Interface.GetPTCaseArray();
  gpMeasure = Interface.GetPTMeasureArray();
}

/** Reassociates internal data with passed DataSetInterface pointers of DataGateway. */
void TemporalData::Reassociate(const AbstractDataSetGateway& DataGateway) {
   gpCases = DataGateway.GetDataSetInterface().GetPTCaseArray();
   gpMeasure = DataGateway.GetDataSetInterface().GetPTMeasureArray();
}

//***************** class ProspectiveSpatialData *******************************

/** class constructor */
ProspectiveSpatialData::ProspectiveSpatialData(const CSaTScanData& Data, const DataSetInterface& Interface)
                       :TemporalData(), geEvaluationAssistDataStatus(Allocated) {
  try {
    Init();
    Setup(Data, Interface);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor(const CSaTScanData&a, const DataSetInterface&)","ProspectiveSpatialData");
    throw;
  }
}

/** class constructor */
ProspectiveSpatialData::ProspectiveSpatialData(const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway)
                       :TemporalData(), geEvaluationAssistDataStatus(Allocated) {
  try {
    Init();
    Setup(Data, DataGateway.GetDataSetInterface());
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor(const CSaTScanData&, const AbstractDataSetGateway&)","ProspectiveSpatialData");
    throw;
  }
}

/** class copy constructor */
ProspectiveSpatialData::ProspectiveSpatialData(const ProspectiveSpatialData& rhs)
                       :TemporalData() {
  try {
    Init();
    *this = rhs;
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor(const ProspectiveSpatialData&)","ProspectiveSpatialData");
    throw;
  }
}

/** class destructor */
ProspectiveSpatialData::~ProspectiveSpatialData() {
  try {
    delete[] gpCases;
    delete[] gpMeasure;
  }
  catch (...){}
}

/** Adds neigbor data to accumulation and updates measure list object. Caller is
    responsible for ensuring that 'tCentroidIndex' and 'tNumNeighbors' are valid
    indexes; as well as 'pMeasureList' and either 'ppSorted_UShort_T' or
    'ppSorted_Tract_T' point to valid data structures. */
void ProspectiveSpatialData::AddMeasureList(const CentroidNeighbors& CentroidDef, const DataSetInterface& Interface, CMeasureList* pMeasureList) {
  assert(geEvaluationAssistDataStatus == Allocated);
  macroRunTimeStartFocused(FocusRunTimeComponent::MeasureListScanningAdding);
  unsigned int           i, j, iWindowEnd;
  count_t             ** ppCases = Interface.GetCaseArray();
  measure_t           ** ppMeasure = Interface.GetMeasureArray();
  tract_t                t, tNeighborIndex, tNumNeighbors=CentroidDef.GetNumNeighbors(),
                       * pIntegerArray = CentroidDef.GetRawIntegerArray();
  unsigned short       * pUnsignedShortArray = CentroidDef.GetRawUnsignedShortArray();

  // reset accumulated case/measure data
  memset(gpCases, 0, sizeof(count_t) * giAllocationSize);
  memset(gpMeasure, 0, sizeof(measure_t) * giAllocationSize);
  for (t=0; t < tNumNeighbors; ++t) {
    tNeighborIndex = (pUnsignedShortArray ? (tract_t)pUnsignedShortArray[t] : pIntegerArray[t]);
    //update accumulated data
    gpCases[0]   += ppCases[0][tNeighborIndex]; //set cases for entire period added by this neighbor
    gpMeasure[0] += ppMeasure[0][tNeighborIndex];
    //set cases for prospective study periods
    for (j=1, i=giProspectiveStart; i < giNumTimeIntervals; ++j, ++i) {
       gpCases[j] += ppCases[i][tNeighborIndex];
       gpMeasure[j] += ppMeasure[i][tNeighborIndex];
    }
    //update measure list
    for (iWindowEnd=1; iWindowEnd < giAllocationSize; ++iWindowEnd)
       pMeasureList->AddMeasure(gpCases[0] - gpCases[iWindowEnd], gpMeasure[0] - gpMeasure[iWindowEnd]);
    pMeasureList->AddMeasure(gpCases[0], gpMeasure[0]);
  }
  macroRunTimeStopFocused(FocusRunTimeComponent::MeasureListScanningAdding);
}

/** Adds neighbor data to accumulation  - caller is responsible for ensuring that
    'tNeighborIndex' and 'tSetIndex' are valid indexes. */
void ProspectiveSpatialData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex) {
  assert(geEvaluationAssistDataStatus == Allocated);
  unsigned int           i, j;
  count_t             ** ppCases = DataGateway.GetDataSetInterface(tSetIndex).GetCaseArray();
  measure_t           ** ppMeasure = DataGateway.GetDataSetInterface(tSetIndex).GetMeasureArray();

  //set cases for entire period added by this neighbor
  gpCases[0]   += ppCases[0][tNeighborIndex];
  gpMeasure[0] += ppMeasure[0][tNeighborIndex];
  //set cases for prospective study periods
  for (j=1, i=giProspectiveStart; i < giNumTimeIntervals; ++j, ++i) {
     gpCases[j] += ppCases[i][tNeighborIndex];
     gpMeasure[j] += ppMeasure[i][tNeighborIndex];
  }  
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractSpatialClusterData object
    can be casted to 'ProspectiveSpatialData' object. */
void ProspectiveSpatialData::Assign(const AbstractTemporalClusterData& rhs) {
  *this = (const ProspectiveSpatialData&)rhs;
}

/** Calculates loglikelihood ratio, given current accumulated cluster data, if
    it is determined that data fits scanning area of interest (high, low, both).
    Returns zero if all windows rates not of interest else returns greatest
    loglikelihood ratio as calculated by probability model. */
double ProspectiveSpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
  assert(geEvaluationAssistDataStatus == Allocated);
  unsigned int  iWindowEnd;
  double        dMaxLoglikelihoodRatio=0;

  gtCases = gpCases[0];
  gtMeasure =  gpMeasure[0];
  if ((Calculator.*Calculator.gpRateOfInterest)(gtCases, gtMeasure, 0))
    dMaxLoglikelihoodRatio = Calculator.CalcLogLikelihoodRatio(gtCases, gtMeasure);

  for (iWindowEnd=1; iWindowEnd < giAllocationSize; ++iWindowEnd) {
    gtCases = gpCases[0] - gpCases[iWindowEnd];
    gtMeasure =  gpMeasure[0] - gpMeasure[iWindowEnd];
    if ((Calculator.*Calculator.gpRateOfInterest)(gtCases, gtMeasure, 0))
      dMaxLoglikelihoodRatio = std::max(dMaxLoglikelihoodRatio, Calculator.CalcLogLikelihoodRatio(gtCases, gtMeasure));
  }
  return dMaxLoglikelihoodRatio;
}

/** Returns newly cloned SpaceTimeData object. Caller is responsible for deletion of object. */
ProspectiveSpatialData * ProspectiveSpatialData::Clone() const {
   return new ProspectiveSpatialData(*this);
}

/** Deallocates data members that assist with evaluation of temporal data.
    Once this function is called various class member functions become invalid
    and an assertion will fail if called. */
void ProspectiveSpatialData::DeallocateEvaluationAssistClassMembers() {
  try {
    delete[] gpCases; gpCases=0;
    delete[] gpMeasure; gpMeasure=0;
    giAllocationSize=0;
    geEvaluationAssistDataStatus = Deallocated;
  }
  catch (...){}
}

/** Calculates and returns maximizing value given accumulated data. If accumulated data
    is not significant, the negation of the maximum double is returned. */
double ProspectiveSpatialData::GetMaximizingValue(AbstractLikelihoodCalculator& Calculator) {
  assert(geEvaluationAssistDataStatus == Allocated);
  unsigned int  iWindowEnd;
  double        dMaxValue(-std::numeric_limits<double>::max());

  gtCases = gpCases[0];
  gtMeasure =  gpMeasure[0];
  if ((Calculator.*Calculator.gpRateOfInterest)(gtCases, gtMeasure, 0))
    dMaxValue = Calculator.CalculateMaximizingValue(gtCases, gtMeasure, 0);

  for (iWindowEnd=1; iWindowEnd < giAllocationSize; ++iWindowEnd) {
    gtCases = gpCases[0] - gpCases[iWindowEnd];
    gtMeasure =  gpMeasure[0] - gpMeasure[iWindowEnd];
    if ((Calculator.*Calculator.gpRateOfInterest)(gtCases, gtMeasure, 0))
      dMaxValue = std::max(dMaxValue, Calculator.CalculateMaximizingValue(gtCases, gtMeasure, 0));
  }
  return dMaxValue;
}

/** Overloaded assignement operator. */
ProspectiveSpatialData & ProspectiveSpatialData::operator=(const ProspectiveSpatialData& rhs) {
  gtCases = rhs.gtCases;
  gtMeasure = rhs.gtMeasure;
  giAllocationSize = rhs.giAllocationSize;
  giNumTimeIntervals = rhs.giNumTimeIntervals;
  giProspectiveStart = rhs.giProspectiveStart;
  if (rhs.geEvaluationAssistDataStatus == Allocated) {
    if (!gpCases) gpCases = new count_t[giAllocationSize];
    if (!gpMeasure) gpMeasure = new measure_t[giAllocationSize];
    memcpy(gpCases, rhs.gpCases, giAllocationSize * sizeof(count_t));
    memcpy(gpMeasure, rhs.gpMeasure, giAllocationSize * sizeof(measure_t));
  }
  else {
    delete[] gpCases; gpCases=0;
    delete[] gpMeasure; gpMeasure=0;
  }
  geEvaluationAssistDataStatus = rhs.geEvaluationAssistDataStatus;
  return *this;
}

/** internal setup function */
void ProspectiveSpatialData::Setup(const CSaTScanData& Data, const DataSetInterface& Interface) {
  try {
    giAllocationSize = 1 + Data.m_nTimeIntervals - Data.GetProspectiveStartIndex();
    giNumTimeIntervals = Data.m_nTimeIntervals;
    giProspectiveStart = Data.GetProspectiveStartIndex();
    gpCases = new count_t[giAllocationSize];
    memset(gpCases, 0, sizeof(count_t) * giAllocationSize);
    gpMeasure = new measure_t[giAllocationSize];
    memset(gpMeasure, 0, sizeof(measure_t) * giAllocationSize);
  }
  catch (ZdException &x) {
    delete[] gpCases;
    delete[] gpMeasure;
    x.AddCallpath("Setup(const CSaTScanData&, const DataSetInterface&)","ProspectiveSpatialData");
    throw;
  }
}

//****************** class SpaceTimeData ***************************************

/** class constructor */
SpaceTimeData::SpaceTimeData(const DataSetInterface& Interface)
              :TemporalData(), geEvaluationAssistDataStatus(Allocated) {
  try {
    Init();
    Setup(Interface);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor(const DataSetInterface&)","SpaceTimeData");
    throw;
  }
}

/** class constructor */
SpaceTimeData::SpaceTimeData(const AbstractDataSetGateway& DataGateway)
              :TemporalData(), geEvaluationAssistDataStatus(Allocated) {
  try {
    Init();
    Setup(DataGateway.GetDataSetInterface());
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor(const AbstractDataSetGateway&)","SpaceTimeData");
    throw;
  }
}

/** class copy constructor */
SpaceTimeData::SpaceTimeData(const SpaceTimeData& rhs)
              :TemporalData() {
  try {
    Init();
    *this = rhs;
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor(const SpaceTimeData&)","SpaceTimeData");
    throw;
  }
}

/** class destructor */
SpaceTimeData::~SpaceTimeData() {
  try {
    delete[] gpCases;
    delete[] gpMeasure;
  }
  catch (...){}
}

/** Adds neighbor data to accumulated data and updates CMeasureList object accordingly. */
void SpaceTimeData::AddNeighborDataAndCompare(const CentroidNeighbors& CentroidDef,
                                              const DataSetInterface& Interface,
                                              CTimeIntervals& TimeIntervals,
                                              CMeasureList& MeasureList) {
  assert(geEvaluationAssistDataStatus == Allocated);
  macroRunTimeStartFocused(FocusRunTimeComponent::MeasureListScanningAdding);

  unsigned int          i, iIntervals = giAllocationSize - 1;
  count_t            ** ppCases = Interface.GetCaseArray();
  measure_t          ** ppMeasure = Interface.GetMeasureArray();
  tract_t               t, tNeighborIndex, tNumNeighbors = CentroidDef.GetNumNeighbors(),
                      * pIntegerArray = CentroidDef.GetRawIntegerArray();
  unsigned short      * pUnsignedShortArray = CentroidDef.GetRawUnsignedShortArray();

  //reset accumulated case/measure data
  memset(gpCases, 0, sizeof(count_t) * giAllocationSize);
  memset(gpMeasure, 0, sizeof(measure_t) * giAllocationSize);
  for (t=0; t < tNumNeighbors; ++t) {
     tNeighborIndex = (pUnsignedShortArray ? (tract_t)pUnsignedShortArray[t] : pIntegerArray[t]);
     for (i=0; i < iIntervals; ++i) {
       gpCases[i] += ppCases[i][tNeighborIndex];
       gpMeasure[i] += ppMeasure[i][tNeighborIndex];
     }
     TimeIntervals.CompareMeasures(*this, MeasureList);
  }
  macroRunTimeStopFocused(FocusRunTimeComponent::MeasureListScanningAdding);
}

/** Adds neighbor data to accumulation - caller is responsible for ensuring that
    'tNeighborIndex' and 'tSetIndex' are valid indexes. */
void SpaceTimeData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex) {
  assert(geEvaluationAssistDataStatus == Allocated);
  count_t    ** ppCases = DataGateway.GetDataSetInterface(tSetIndex).GetCaseArray();
  measure_t  ** ppMeasure = DataGateway.GetDataSetInterface(tSetIndex).GetMeasureArray();

  for (unsigned int i=0; i < giAllocationSize - 1; ++i) {
     gpCases[i] += ppCases[i][tNeighborIndex];
     gpMeasure[i] += ppMeasure[i][tNeighborIndex];
  }
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractSpatialClusterData object
    can be casted to 'SpaceTimeData' object. */
void SpaceTimeData::Assign(const AbstractTemporalClusterData& rhs) {
  *this = (const SpaceTimeData&)rhs;
}

/** Returns newly cloned SpaceTimeData object. Caller resonsible for deletion of object. */
SpaceTimeData * SpaceTimeData::Clone() const {
   return new SpaceTimeData(*this);
}

/** Deallocates data members that assist with evaluation of temporal data.
    Once this function is called various class member functions become invalid
    and an assertion will fail if called. */
void SpaceTimeData::DeallocateEvaluationAssistClassMembers() {
  try {
    delete[] gpCases; gpCases=0;
    delete[] gpMeasure; gpMeasure=0;
    giAllocationSize=0;
    geEvaluationAssistDataStatus = Deallocated;
  }
  catch (...){}
}

/** overloaded assignement operator */
SpaceTimeData & SpaceTimeData::operator=(const SpaceTimeData& rhs) {
  gtCases = rhs.gtCases;
  gtMeasure = rhs.gtMeasure;
  giAllocationSize = rhs.giAllocationSize;
  if (rhs.geEvaluationAssistDataStatus == Allocated) {
    if(!gpCases) gpCases = new count_t[rhs.giAllocationSize];
    if (!gpMeasure) gpMeasure = new measure_t[rhs.giAllocationSize];
    memcpy(gpCases, rhs.gpCases, giAllocationSize * sizeof(count_t));
    memcpy(gpMeasure, rhs.gpMeasure, giAllocationSize * sizeof(measure_t));
  }
  else {
    delete[] gpCases; gpCases=0;
    delete[] gpMeasure; gpMeasure=0;
  }
  geEvaluationAssistDataStatus = rhs.geEvaluationAssistDataStatus;
  return *this;
}

/** internal setup function */
void SpaceTimeData::Setup(const DataSetInterface& Interface) {
  try {
    //Note that giAllocationSize is number of time intervals plus one - this permits
    //us to evaluate last time intervals data with same code as other time intervals
    //in CTimeIntervals object.
    giAllocationSize = Interface.GetNumTimeIntervals() + 1;
    gpCases = new count_t[giAllocationSize];
    memset(gpCases, 0, sizeof(count_t) * giAllocationSize);
    gpMeasure = new measure_t[giAllocationSize];
    memset(gpMeasure, 0, sizeof(measure_t) * giAllocationSize);
  }
  catch (ZdException &x) {
    delete[] gpCases;
    delete[] gpMeasure;
    x.AddCallpath("Setup(const DataSetInterface&)","SpaceTimeData");
    throw;
  }
}

