//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ClusterData.h"
#include "TimeIntervals.h"
#include "SSException.h"

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
  if ((Calculator.*Calculator.gpRateOfInterest)(gtCases, gtMeasure))
    return Calculator.CalcLogLikelihoodRatio(gtCases, gtMeasure);
  return 0;
}

/** Calculates and returns maximizing value given accumulated data. If accumulated data
    is not significant, the negation of the maximum double is returned. */
double SpatialData::GetMaximizingValue(AbstractLikelihoodCalculator& Calculator) {
  if ((Calculator.*Calculator.gpRateOfInterest)(gtCases, gtMeasure))
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

//**************** class SpatialHomogenenousData ******************************

/** class constructor */
SpatialHomogeneousData::SpatialHomogeneousData(const DataSetInterface& Interface) : AbstractSpatialClusterData() {
  InitializeData();
}

/** class constructor */
SpatialHomogeneousData::SpatialHomogeneousData(const AbstractDataSetGateway& DataGateway) : AbstractSpatialClusterData() {
  InitializeData();
}

/** Adds neighbor data to accumulation - caller is responsible for ensuring that
    'tNeighborIndex' and 'tSetIndex' are valid indexes. */
void SpatialHomogeneousData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex) {
  // gtCases ??
  // gtMeasure ??
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractSpatialClusterData object
    can be casted to 'SpatialData' object. */
void SpatialHomogeneousData::Assign(const AbstractSpatialClusterData& rhs) {
  *this = (const SpatialHomogeneousData&)rhs;
}

/** Calculates loglikelihood ratio, given current accumulated cluster data, if
    it is determined that data fits scanning area of interest (high, low or both).
    Returns zero if rate not of interest else returns loglikelihood ratio as
    calculated by probability model. */
double SpatialHomogeneousData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
  if ((Calculator.*Calculator.gpRateOfInterest)(gtCases, gtMeasure))
    return Calculator.CalcLogLikelihoodRatio(gtCases, gtMeasure);
  return 0;
}

/** Calculates and returns maximizing value given accumulated data. If accumulated data
    is not significant, the negation of the maximum double is returned. */
double SpatialHomogeneousData::GetMaximizingValue(AbstractLikelihoodCalculator& Calculator) {
  if ((Calculator.*Calculator.gpRateOfInterest)(gtCases, gtMeasure))
    return Calculator.CalculateMaximizingValue(gtCases, gtMeasure);
  return -std::numeric_limits<double>::max();
}

/** Returns newly cloned SpatialHomogeneousData object. Caller resonsible for deletion of object. */
SpatialHomogeneousData * SpatialHomogeneousData::Clone() const {
  return new SpatialHomogeneousData(*this);
}

void SpatialHomogeneousData::CopyEssentialClassMembers(const AbstractClusterData& rhs) {
  gtCases = ((const SpatialHomogeneousData&)rhs).gtCases;
  gtMeasure = ((const SpatialHomogeneousData&)rhs).gtMeasure;
}

/** Returns number of cases accumulated in cluster data. */
count_t SpatialHomogeneousData::GetCaseCount(unsigned int) const {
  return gtCases;
}

/** Returns number of expected cases accumulated in cluster data. */
measure_t SpatialHomogeneousData::GetMeasure(unsigned int) const {
  return gtMeasure;
}

/** Overloaded assignment operator. */
SpatialHomogeneousData & SpatialHomogeneousData::operator=(const SpatialHomogeneousData& rhs) {
  gtCases = rhs.gtCases;
  gtMeasure = rhs.gtMeasure;
  return *this;
}

//**************** class SpatialMonotoneData************************************

/** class constructor */
SpatialMonotoneData::SpatialMonotoneData(const DataSetInterface& Interface) : AbstractClusterData() {
  InitializeData();
}

/** class constructor */
SpatialMonotoneData::SpatialMonotoneData(const AbstractDataSetGateway& DataGateway) : AbstractClusterData() {
  InitializeData();
}

/** Adds neighbor data to accumulation - caller is responsible for ensuring that
    'tNeighborIndex' and 'tSetIndex' are valid indexes. */
void SpatialMonotoneData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex) {
  ++m_nSteps;
  ++m_NeighborIteration;
  gtCases += DataGateway.GetDataSetInterface(tSetIndex).GetPSCaseArray()[tNeighborIndex];
  gtMeasure += DataGateway.GetDataSetInterface(tSetIndex).GetPSMeasureArray()[tNeighborIndex];
  gvCasesList[m_nSteps-1] = DataGateway.GetDataSetInterface(tSetIndex).GetPSCaseArray()[tNeighborIndex];
  gvMeasureList[m_nSteps-1] = DataGateway.GetDataSetInterface(tSetIndex).GetPSMeasureArray()[tNeighborIndex];
  gvFirstNeighborList[m_nSteps-1] = m_NeighborIteration;
  gvLastNeighborList[m_nSteps-1] = m_NeighborIteration;
}

/** Adds another step and assigns from remainder of cases not already accounted for in existing steps. */
void SpatialMonotoneData::AddRemainder(const CSaTScanData& Data) {
  if (Data.GetTotalCases() != gtCases) {
    ++m_nSteps;
    gvCasesList[m_nSteps-1] = Data.GetTotalCases() - gtCases;
    gvMeasureList[m_nSteps-1] = Data.GetTotalMeasure() - gtMeasure;
    CheckCircle(GetLastCircleIndex());
  }
}

/** Allocates internal structures for maximum number of circles in spatial cluster data. */
void SpatialMonotoneData::AllocateForMaxCircles(tract_t nCircles) {
  gvCasesList.resize(nCircles);
  gvMeasureList.resize(nCircles);
  gvFirstNeighborList.resize(nCircles);
  gvLastNeighborList.resize(nCircles);
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractSpatialClusterData object
    can be casted to 'SpatialData' object. */
void SpatialMonotoneData::Assign(const AbstractClusterData& rhs) {
  *this = (const SpatialMonotoneData&)rhs;
}

/** Calculates loglikelihood ratio, given current accumulated cluster data. */
double SpatialMonotoneData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
  return Calculator.CalcMonotoneLogLikelihood(m_nSteps, gvCasesList, gvMeasureList) - Calculator.GetLogLikelihoodForTotal();
}

/** Recursively collapses outer most circles upon inner circles. */
void SpatialMonotoneData::CheckCircle(tract_t n) {
  if (n != 0)
    if (!m_pfRateOfInterest(gvCasesList[n-1], gvMeasureList[n-1], gvCasesList[n], gvMeasureList[n])) {
      ConcatLastCircles();
      CheckCircle(n-1);
    }
}

/** Returns newly cloned SpatialData object. Caller resonsible for deletion of object. */
SpatialMonotoneData * SpatialMonotoneData::Clone() const {
  return new SpatialMonotoneData(*this);
}

/** Collapses outer circle upon next inner circle. */
void SpatialMonotoneData::ConcatLastCircles() {
  --m_nSteps;
  gvCasesList[m_nSteps-1] += gvCasesList[m_nSteps];
  gvMeasureList[m_nSteps-1] += gvMeasureList[m_nSteps];
  gvLastNeighborList[m_nSteps-1] = gvLastNeighborList[m_nSteps];

  gvCasesList[m_nSteps] = 0;
  gvMeasureList[m_nSteps] = 0;
  gvFirstNeighborList[m_nSteps] = 0;
  gvLastNeighborList[m_nSteps] = 0;
}

/** Restricted version of assignment operator - only copies data needed during reporting of cluster. */
void SpatialMonotoneData::CopyEssentialClassMembers(const AbstractClusterData& rhs) {
  gtCases = ((const SpatialMonotoneData&)rhs).gtCases;
  gtMeasure = ((const SpatialMonotoneData&)rhs).gtMeasure;
  m_nSteps = ((const SpatialMonotoneData&)rhs).m_nSteps;
  gvCasesList = ((const SpatialMonotoneData&)rhs).gvCasesList;
  gvMeasureList = ((const SpatialMonotoneData&)rhs).gvMeasureList;
  gvFirstNeighborList = ((const SpatialMonotoneData&)rhs).gvFirstNeighborList;
  gvLastNeighborList = ((const SpatialMonotoneData&)rhs).gvLastNeighborList;
}

/** Calculates and returns maximizing value given accumulated data. */
double SpatialMonotoneData::GetMaximizingValue(AbstractLikelihoodCalculator& Calculator) {
  return Calculator.CalcMonotoneLogLikelihood(m_nSteps, gvCasesList, gvMeasureList);
}

/** Returns number of cases accumulated in cluster data. */
count_t SpatialMonotoneData::GetCaseCount(unsigned int) const {
  return gtCases;
}

/** Returns number of expected cases accumulated in cluster data. */
measure_t SpatialMonotoneData::GetMeasure(unsigned int) const {
  return gtMeasure;
}

/** Initializes cluster data. */
void SpatialMonotoneData::InitializeData() {
  gtCases=0;
  gtMeasure=0;
  m_nSteps = 0;
  m_NeighborIteration = 0;
  std::fill(gvCasesList.begin(), gvCasesList.end(), 0);
  std::fill(gvMeasureList.begin(), gvMeasureList.end(), 0);
  std::fill(gvFirstNeighborList.begin(), gvFirstNeighborList.end(), 0);
  std::fill(gvLastNeighborList.begin(), gvLastNeighborList.end(), 0);
}

/** Overloaded assignment operator. */
SpatialMonotoneData & SpatialMonotoneData::operator=(const SpatialMonotoneData& rhs) {
  gtCases             = rhs.gtCases;
  gtMeasure           = rhs.gtMeasure;
  m_nSteps            = rhs.m_nSteps;
  m_NeighborIteration = rhs.m_NeighborIteration;
  gvCasesList         = rhs.gvCasesList;
  gvMeasureList       = rhs.gvMeasureList;
  gvFirstNeighborList = rhs.gvFirstNeighborList;
  gvLastNeighborList  = rhs.gvLastNeighborList;
  m_pfRateOfInterest  = rhs.m_pfRateOfInterest;
  return *this;
}

/** Removes remainder cases added in AddRemainder() function. */
void SpatialMonotoneData::RemoveRemainder(const CSaTScanData& Data) {
  if (Data.GetTotalCases() != gtCases) {
    --m_nSteps;
    gvCasesList[m_nSteps] = 0;
    gvMeasureList[m_nSteps] = 0;
    gvFirstNeighborList[m_nSteps] = 0;
    gvLastNeighborList[m_nSteps] = 0;
    if (m_nSteps==0)
      InitializeData();
  }
}

/** Re-calculates the number of cases and expected cases inside of cluster based upon the number of steps. */
void SpatialMonotoneData::SetCasesAndMeasures() {
  gtCases   = 0;
  gtMeasure = 0;
  for (int i=0; i < m_nSteps; ++i) {
     gtCases   += gvCasesList[i];
     gtMeasure += gvMeasureList[i];
  }
}

/** Sets scanning area rate. */
void SpatialMonotoneData::SetRate(AreaRateType eRate) {
  switch (eRate) {
    case LOW        : m_pfRateOfInterest = LowRate;       break;
    case HIGHANDLOW : m_pfRateOfInterest = HighOrLowRate; break;
    case HIGH       :
    default         : m_pfRateOfInterest = HighRate;
  }
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
  throw prg_error("AddNeighborData(tract_t, const AbstractDataSetGateway&, size_t) not implemeneted.","TemporalData");
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
  throw prg_error("GetAllocationSize() not implemeneted.","TemporalData");
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
  catch (prg_exception& x) {
    x.addTrace("constructor(const CSaTScanData&a, const DataSetInterface&)","ProspectiveSpatialData");
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
  catch (prg_exception& x) {
    x.addTrace("constructor(const CSaTScanData&, const AbstractDataSetGateway&)","ProspectiveSpatialData");
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
  catch (prg_exception& x) {
    x.addTrace("constructor(const ProspectiveSpatialData&)","ProspectiveSpatialData");
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
  if ((Calculator.*Calculator.gpRateOfInterest)(gtCases, gtMeasure))
    dMaxLoglikelihoodRatio = Calculator.CalcLogLikelihoodRatio(gtCases, gtMeasure);

  for (iWindowEnd=1; iWindowEnd < giAllocationSize; ++iWindowEnd) {
    gtCases = gpCases[0] - gpCases[iWindowEnd];
    gtMeasure =  gpMeasure[0] - gpMeasure[iWindowEnd];
    if ((Calculator.*Calculator.gpRateOfInterest)(gtCases, gtMeasure))
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
  if ((Calculator.*Calculator.gpRateOfInterest)(gtCases, gtMeasure))
    dMaxValue = Calculator.CalculateMaximizingValue(gtCases, gtMeasure);

  for (iWindowEnd=1; iWindowEnd < giAllocationSize; ++iWindowEnd) {
    gtCases = gpCases[0] - gpCases[iWindowEnd];
    gtMeasure =  gpMeasure[0] - gpMeasure[iWindowEnd];
    if ((Calculator.*Calculator.gpRateOfInterest)(gtCases, gtMeasure))
      dMaxValue = std::max(dMaxValue, Calculator.CalculateMaximizingValue(gtCases, gtMeasure));
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
    giAllocationSize = 1 + Data.GetNumTimeIntervals() - Data.GetProspectiveStartIndex();
    giNumTimeIntervals = Data.GetNumTimeIntervals();
    giProspectiveStart = Data.GetProspectiveStartIndex();
    gpCases = new count_t[giAllocationSize];
    memset(gpCases, 0, sizeof(count_t) * giAllocationSize);
    gpMeasure = new measure_t[giAllocationSize];
    memset(gpMeasure, 0, sizeof(measure_t) * giAllocationSize);
  }
  catch (prg_exception& x) {
    delete[] gpCases;
    delete[] gpMeasure;
    x.addTrace("Setup(const CSaTScanData&, const DataSetInterface&)","ProspectiveSpatialData");
    throw;
  }
}

//****************** class SpaceTimeData ***************************************

/** Protected class constructor - accessible by derived classes only. */
SpaceTimeData::SpaceTimeData() : TemporalData() {
    InitializeData();
}

/** class constructor */
SpaceTimeData::SpaceTimeData(const DataSetInterface& Interface)
              :TemporalData(), geEvaluationAssistDataStatus(Allocated) {
  try {
    Init();
    Setup(Interface);
  }
  catch (prg_exception& x) {
    x.addTrace("constructor(const DataSetInterface&)","SpaceTimeData");
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
  catch (prg_exception& x) {
    x.addTrace("constructor(const AbstractDataSetGateway&)","SpaceTimeData");
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
  catch (prg_exception& x) {
    x.addTrace("constructor(const SpaceTimeData&)","SpaceTimeData");
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
  catch (prg_exception& x) {
    delete[] gpCases;
    delete[] gpMeasure;
    x.addTrace("Setup(const DataSetInterface&)","SpaceTimeData");
    throw;
  }
}

