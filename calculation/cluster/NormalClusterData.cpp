//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "NormalClusterData.h"

//************** class NormalSpatialData ***************************************

/** class constructor */
NormalSpatialData::NormalSpatialData(const DataSetInterface& Interface, int iRate)
                  :SpatialData(Interface, iRate), gtSqMeasure(0),
                   gtTotalMeasureSq(Interface.GetTotalMeasureSqCount()) {}

/** class constructor */
NormalSpatialData::NormalSpatialData(const AbstractDataSetGateway& DataGateway, int iRate)
                  :SpatialData(DataGateway, iRate), gtSqMeasure(0),
                   gtTotalMeasureSq(DataGateway.GetDataSetInterface().GetTotalMeasureSqCount()) {}

/** Adds neighbor data to accumulation  - caller is responsible for ensuring that
    'tNeighborIndex' and 'tSetIndex' are valid indexes. */
void NormalSpatialData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex) {
  gtCases += DataGateway.GetDataSetInterface(tSetIndex).GetPSCaseArray()[tNeighborIndex];
  gtMeasure += DataGateway.GetDataSetInterface(tSetIndex).GetPSMeasureArray()[tNeighborIndex];
  gtSqMeasure += DataGateway.GetDataSetInterface(tSetIndex).GetPSSqMeasureArray()[tNeighborIndex];
}

/** Not implemeneted - throws ZdException. */
void NormalSpatialData::AddMeasureList(const DataSetInterface&, CMeasureList*, const CSaTScanData*) {
   ZdGenerateException("AddMeasureList(const DataSetInterface&, CMeasureList*, const CSaTScanData*) not implemented.","NormalSpatialData");
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractSpatialClusterData object
    can be casted to 'NormalSpatialData' object. */
void NormalSpatialData::Assign(const AbstractSpatialClusterData& rhs) {
  *this = (const NormalSpatialData&)rhs;
}

/** Calculates loglikelihood ratio, given current accumulated cluster data, if
    it is determined that data fits scanning area of interest (high, low, both).
    Returns zero if rate not of interest else returns loglikelihood ratio as
    calculated by probability model. */
double NormalSpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
  if (gfRateOfInterest(gtCases, gtMeasure, gtTotalCases, gtTotalMeasure))
    return Calculator.CalcLogLikelihoodRatioNormal(gtCases, gtMeasure, gtSqMeasure, gtTotalCases, gtTotalMeasure, gtTotalMeasureSq);
  return 0;
}

/** Returns newly cloned NormalSpatialData object. Caller is responsible for deletion of object. */
NormalSpatialData * NormalSpatialData::Clone() const {
  return new NormalSpatialData(*this);
}

/** Copies class data members that reflect the number of cases, expected, and expected
    squared values, which is the data we are interested in for possiblely reporting. */
void NormalSpatialData::CopyEssentialClassMembers(const AbstractClusterData& rhs) {
  gtCases = ((const NormalSpatialData&)rhs).gtCases;
  gtMeasure = ((const NormalSpatialData&)rhs).gtMeasure;
  gtSqMeasure = ((const NormalSpatialData&)rhs).gtSqMeasure;
}
//************** class NormalTemporalData **************************************

/** class constructor */
NormalTemporalData::NormalTemporalData() : TemporalData(), gtSqMeasure(0), gpSqMeasure(0), gtTotalMeasureSq(0) {}

/** class constructor */
NormalTemporalData::NormalTemporalData(const DataSetInterface& Interface)
                   :TemporalData(Interface), gtSqMeasure(0),
                    gpSqMeasure(Interface.GetPTSqMeasureArray()),
                    gtTotalMeasureSq(Interface.GetTotalMeasureSqCount()) {}

/** class constructor */
NormalTemporalData::NormalTemporalData(const AbstractDataSetGateway& DataGateway)
                   :TemporalData(DataGateway.GetDataSetInterface()), gtSqMeasure(0),
                    gpSqMeasure(DataGateway.GetDataSetInterface().GetPTSqMeasureArray()),
                    gtTotalMeasureSq(DataGateway.GetDataSetInterface().GetTotalMeasureSqCount()) {}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractTemporalClusterData object
    can be casted to 'NormalTemporalData' object. */
void NormalTemporalData::Assign(const AbstractTemporalClusterData& rhs) {
  *this = (const NormalTemporalData&)rhs;
}

/** Returns newly cloned NormalTemporalData object. Caller responsible for deletion of object. */
NormalTemporalData * NormalTemporalData::Clone() const {
  return new NormalTemporalData(*this);
}

/** Copies class data members that reflect the number of cases, expected, and expected
    squared values, which is the data we are interested in for possiblely reporting. */
void NormalTemporalData::CopyEssentialClassMembers(const AbstractClusterData& rhs) {
  gtCases = ((const NormalTemporalData&)rhs).gtCases;
  gtMeasure = ((const NormalTemporalData&)rhs).gtMeasure;
  gtSqMeasure = ((const NormalTemporalData&)rhs).gtSqMeasure;
}

/** Reassociates internal data with passed DataSetInterface pointers.
    Not implemented - throws exception */
void NormalTemporalData::Reassociate(const DataSetInterface& Interface) {
  ZdGenerateException("Reassociate(const DataSetInterface&) not implemented.","NormalTemporalData");
}

/** Reassociates internal data with passed DataSetInterface pointers of DataGateway. */
void NormalTemporalData::Reassociate(const AbstractDataSetGateway& DataGateway) {
  TemporalData::Reassociate(DataGateway.GetDataSetInterface());
  gpSqMeasure = DataGateway.GetDataSetInterface().GetPTSqMeasureArray();
}
//**************** class NormalProspectiveSpatialData **************************

/** class constructor */
NormalProspectiveSpatialData::NormalProspectiveSpatialData(const CSaTScanData& Data, const DataSetInterface& Interface)
                             :NormalTemporalData(), geEvaluationAssistDataStatus(Allocated) {
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
NormalProspectiveSpatialData::NormalProspectiveSpatialData(const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway)
                             :NormalTemporalData(), geEvaluationAssistDataStatus(Allocated) {
  try {
    Init();
    Setup(Data, DataGateway.GetDataSetInterface());
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","NormalProspectiveSpatialData");
    throw;
  }
}

/** class copy constructor */
NormalProspectiveSpatialData::NormalProspectiveSpatialData(const NormalProspectiveSpatialData& rhs)
                             :NormalTemporalData() {
  try {
    Init();
    *this = rhs;
  }
  catch (ZdException &x) {
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

/** Adds neighbor data to accumulation - caller is responsible for ensuring that
    'tNeighborIndex' and 'tSetIndex' are valid indexes. */
void NormalProspectiveSpatialData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex) {
  assert(geEvaluationAssistDataStatus == Allocated);
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

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractTemporalClusterData object
    can be casted to 'NormalProspectiveSpatialData' object. */
void NormalProspectiveSpatialData::Assign(const AbstractTemporalClusterData& rhs) {
  *this = (const NormalProspectiveSpatialData&)rhs;
}

/** Calculates loglikelihood ratio, given current accumulated cluster data, if
    it is determined that data fits scanning area of interest (high, low, both).
    Returns zero if all windows rates not of interest else returns greatest
    loglikelihood ratio as calculated by probability model. */
double NormalProspectiveSpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
  assert(geEvaluationAssistDataStatus == Allocated);
  unsigned int  iWindowEnd;
  double        dMaxLoglikelihoodRatio=0;

  gtCases = gpCases[0];
  gtMeasure = gpMeasure[0];
  gtSqMeasure = gpSqMeasure[0];
  if (gfRateOfInterest(gtCases, gtMeasure, gtTotalCases, gtTotalMeasure))
    dMaxLoglikelihoodRatio = Calculator.CalcLogLikelihoodRatioNormal(gtCases, gtMeasure, gtSqMeasure, gtTotalCases, gtTotalMeasure, gtTotalMeasureSq);

  for (iWindowEnd=1; iWindowEnd < giAllocationSize - 1; ++iWindowEnd) {
    gtCases = gpCases[0] - gpCases[iWindowEnd];
    gtMeasure = gpMeasure[0] - gpMeasure[iWindowEnd];
    gtSqMeasure = gpSqMeasure[0] - gpSqMeasure[iWindowEnd];
    if (gfRateOfInterest(gtCases, gtMeasure, gtTotalCases, gtTotalMeasure))
      dMaxLoglikelihoodRatio = std::max(dMaxLoglikelihoodRatio,
                                        Calculator.CalcLogLikelihoodRatioNormal(gtCases, gtMeasure, gtSqMeasure, gtTotalCases, gtTotalMeasure, gtTotalMeasureSq));
  }
  return dMaxLoglikelihoodRatio;
}

/** Returns newly cloned NormalProspectiveSpatialData object. Caller responsible
    for deletion of object. */
NormalProspectiveSpatialData * NormalProspectiveSpatialData::Clone() const {
   return new NormalProspectiveSpatialData(*this);
}

/** Deallocates data members that assist with evaluation of temporal data.
    Once this function is called various class member functions become invalid
    and an assertion will fail if called. */
void NormalProspectiveSpatialData::DeallocateEvaluationAssistClassMembers() {
  try {
    delete[] gpCases; gpCases=0;
    delete[] gpMeasure; gpMeasure=0;
    delete[] gpSqMeasure; gpSqMeasure=0;
    giAllocationSize=0;
    geEvaluationAssistDataStatus = Deallocated;
  }
  catch (...){}
}

/** re-initialize data*/
void NormalProspectiveSpatialData::InitializeData() {
  assert(geEvaluationAssistDataStatus == Allocated);
  gtCases=0;
  gtMeasure=0;
  gtSqMeasure=0;
  memset(gpCases, 0, sizeof(count_t) * giAllocationSize);
  memset(gpMeasure, 0, sizeof(measure_t) * giAllocationSize);
  memset(gpSqMeasure, 0, sizeof(measure_t) * giAllocationSize);
}

/** overloaded assignement operator */
NormalProspectiveSpatialData & NormalProspectiveSpatialData::operator=(const NormalProspectiveSpatialData& rhs) {
   gtCases = rhs.gtCases;
   gtMeasure = rhs.gtMeasure;
   gtTotalCases = rhs.gtTotalCases;
   gtTotalMeasure = rhs.gtTotalMeasure;
   gtTotalMeasureSq = rhs.gtTotalMeasureSq;
   giAllocationSize = rhs.giAllocationSize;
   giNumTimeIntervals = rhs.giNumTimeIntervals;
   giProspectiveStart = rhs.giProspectiveStart;
   if (rhs.geEvaluationAssistDataStatus == Allocated) {
     if (!gpCases) gpCases = new count_t[rhs.giAllocationSize];
     if (!gpMeasure) gpMeasure = new measure_t[rhs.giAllocationSize];
     if (!gpSqMeasure) gpSqMeasure = new measure_t[rhs.giAllocationSize];
     memcpy(gpCases, rhs.gpCases, giAllocationSize * sizeof(count_t));
     memcpy(gpMeasure, rhs.gpMeasure, giAllocationSize * sizeof(measure_t));
     memcpy(gpSqMeasure, rhs.gpSqMeasure, giAllocationSize * sizeof(measure_t));
   }
   else {
    delete[] gpCases; gpCases=0;
    delete[] gpMeasure; gpMeasure=0;
    delete[] gpSqMeasure; gpSqMeasure=0;
   }
   geEvaluationAssistDataStatus = rhs.geEvaluationAssistDataStatus;
   return *this;
}

/** internal setup function */
void NormalProspectiveSpatialData::Setup(const CSaTScanData& Data, const DataSetInterface& Interface) {
  try {
    giAllocationSize = 1 + Data.m_nTimeIntervals - Data.GetProspectiveStartIndex();
    giNumTimeIntervals = Data.m_nTimeIntervals;
    giProspectiveStart = Data.GetProspectiveStartIndex();
    gtTotalCases = Interface.GetTotalCasesCount();
    gtTotalMeasure = Interface.GetTotalMeasureCount();
    gtTotalMeasureSq = Interface.GetTotalMeasureSqCount();

    gpCases = new count_t[giAllocationSize];
    memset(gpCases, 0, sizeof(count_t) * giAllocationSize);
    gpMeasure = new measure_t[giAllocationSize];
    memset(gpMeasure, 0, sizeof(measure_t) * giAllocationSize);
    gpSqMeasure = new measure_t[giAllocationSize];
    memset(gpSqMeasure, 0, sizeof(measure_t) * giAllocationSize);
    switch (Data.GetParameters().GetExecuteScanRateType()) {
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

//*************** class NormalSpaceTimeData ************************************

/** class constructor */
NormalSpaceTimeData::NormalSpaceTimeData(const DataSetInterface& Interface)
                    :NormalTemporalData(), geEvaluationAssistDataStatus(Allocated) {
  try {
    Setup(Interface);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor(const DataSetInterface&)","NormalSpaceTimeData");
    throw;
  }
}

/** constructor */
NormalSpaceTimeData::NormalSpaceTimeData(const AbstractDataSetGateway& DataGateway)
                    :NormalTemporalData(), geEvaluationAssistDataStatus(Allocated) {
  try {
    Setup(DataGateway.GetDataSetInterface());
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor(const AbstractDataSetGateway&)","NormalSpaceTimeData");
    throw;
  }
}

/** class copy constructor */
NormalSpaceTimeData::NormalSpaceTimeData(const NormalSpaceTimeData& rhs)
                    :NormalTemporalData() {
  try {
    *this = rhs;
  }
  catch (ZdException &x) {
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

/** Adds neighbor data to accumulation - caller is responsible for ensuring that
    'tNeighborIndex' and 'tSetIndex' are valid indexes. */
void NormalSpaceTimeData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex) {
  assert(geEvaluationAssistDataStatus == Allocated);
  count_t    ** ppCases = DataGateway.GetDataSetInterface(tSetIndex).GetCaseArray();
  measure_t  ** ppMeasure = DataGateway.GetDataSetInterface(tSetIndex).GetMeasureArray();
  measure_t  ** ppSqMeasure = DataGateway.GetDataSetInterface(tSetIndex).GetSqMeasureArray();

  for (unsigned int i=0; i < giAllocationSize - 1; ++i) {
     gpCases[i] += ppCases[i][tNeighborIndex];
     gpMeasure[i] += ppMeasure[i][tNeighborIndex];
     gpSqMeasure[i] += ppSqMeasure[i][tNeighborIndex];
  }
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractTemporalClusterData object
    can be casted to 'NormalSpaceTimeData' object. */
void NormalSpaceTimeData::Assign(const AbstractTemporalClusterData& rhs) {
  *this = (const NormalSpaceTimeData&)rhs;
}

/** Returns newly cloned NormalSpaceTimeData object. Caller responsible for deletion
    of object. */
NormalSpaceTimeData * NormalSpaceTimeData::Clone() const {
   return new NormalSpaceTimeData(*this);
}

/** Deallocates data members that assist with evaluation of temporal data.
    Once this function is called various class member functions become invalid
    and an assertion will fail if called. */
void NormalSpaceTimeData::DeallocateEvaluationAssistClassMembers() {
  try {
    delete[] gpCases; gpCases=0;
    delete[] gpMeasure; gpMeasure=0;
    delete[] gpSqMeasure; gpSqMeasure=0;
    giAllocationSize=0;
    geEvaluationAssistDataStatus = Deallocated;
  }
  catch (...){}
}

/** re-initialize data */
void NormalSpaceTimeData::InitializeData() {
  assert(geEvaluationAssistDataStatus == Allocated);
  gtCases=0;
  gtMeasure=0;
  gtSqMeasure=0;
  memset(gpCases, 0, sizeof(count_t) * giAllocationSize);
  memset(gpMeasure, 0, sizeof(measure_t) * giAllocationSize);
  memset(gpSqMeasure, 0, sizeof(measure_t) * giAllocationSize);
}

/** overloaded assignement operator */
NormalSpaceTimeData & NormalSpaceTimeData::operator=(const NormalSpaceTimeData& rhs) {
  gtCases = rhs.gtCases;
  gtMeasure = rhs.gtMeasure;
  gtTotalCases = rhs.gtTotalCases;
  gtTotalMeasure = rhs.gtTotalMeasure;
  gtTotalMeasureSq = rhs.gtTotalMeasureSq;
  giAllocationSize = rhs.giAllocationSize;
  if (rhs.geEvaluationAssistDataStatus == Allocated) {
    if (!gpCases) gpCases = new count_t[rhs.giAllocationSize];
    if (!gpMeasure) gpMeasure = new measure_t[rhs.giAllocationSize];
    if (!gpSqMeasure) gpSqMeasure = new measure_t[rhs.giAllocationSize];
    memcpy(gpCases, rhs.gpCases, giAllocationSize * sizeof(count_t));
    memcpy(gpMeasure, rhs.gpMeasure, giAllocationSize * sizeof(measure_t));
    memcpy(gpSqMeasure, rhs.gpSqMeasure, giAllocationSize * sizeof(measure_t));
  }
  else {
    delete[] gpCases; gpCases=0;
    delete[] gpMeasure; gpMeasure=0;
    delete[] gpSqMeasure; gpSqMeasure=0;
  }
  geEvaluationAssistDataStatus = rhs.geEvaluationAssistDataStatus;
  return *this;
}

/** internal setup function */
void NormalSpaceTimeData::Setup(const DataSetInterface& Interface) {
  try {
    //Note that giAllocationSize is number of time intervals plus one - this permits
    //us to evaluate last time intervals data with same code as other time intervals
    //in CTimeIntervals object.
    giAllocationSize = Interface.GetNumTimeIntervals() + 1;
    gtTotalCases = Interface.GetTotalCasesCount();
    gtTotalMeasure = Interface.GetTotalMeasureCount();
    gtTotalMeasureSq = Interface.GetTotalMeasureSqCount();
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

