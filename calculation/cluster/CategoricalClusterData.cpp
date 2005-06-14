//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "CategoricalClusterData.h"
#include "TimeIntervals.h"

/** class constructor */
CategoricalSpatialData::CategoricalSpatialData(const DataSetInterface& Interface)
                       :AbstractSpatialClusterData(0) {

  gvCasesPerCategory.resize(Interface.GetNumOrdinalCategories(), 0);
  InitializeData();
}

/** class constructor */
CategoricalSpatialData::CategoricalSpatialData(const AbtractDataSetGateway& DataGateway)
                       :AbstractSpatialClusterData(0) {
                        
  gvCasesPerCategory.resize(DataGateway.GetDataSetInterface().GetNumOrdinalCategories(), 0);
  InitializeData();
}

/** destructor */
CategoricalSpatialData::~CategoricalSpatialData() {}

/** Returns newly cloned CategoricalSpatialData object. Caller is responsible for
    deletion of object. */
CategoricalSpatialData * CategoricalSpatialData::Clone() const {
  return new CategoricalSpatialData(*this);
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractSpatialClusterData object
    can be casted to 'CategoricalSpatialData' object. */
void CategoricalSpatialData::Assign(const AbstractSpatialClusterData& rhs) {
  //cast to SpatialData type
  const CategoricalSpatialData& _rhs = (const CategoricalSpatialData&)rhs;
  //copy data members
  gvCasesPerCategory = _rhs.gvCasesPerCategory;
}

/** overloaded assignment operator */
CategoricalSpatialData & CategoricalSpatialData::operator=(const CategoricalSpatialData& rhs) {
  gvCasesPerCategory = rhs.gvCasesPerCategory;
  return *this;
}

/** adds neighbor data to accumulation  - caller is responsible for ensuring that
    'tNeighborIndex' and 'tSetIndex' are valid indexes. */
void CategoricalSpatialData::AddNeighborData(tract_t tNeighborIndex, const AbtractDataSetGateway& DataGateway, size_t tSetIndex) {
  for (size_t t=0; t < gvCasesPerCategory.size(); ++t)
    gvCasesPerCategory[t] += DataGateway.GetDataSetInterface(tSetIndex).GetCategoryCaseArrays()[t][0][tNeighborIndex];
}

/** Calculates loglikelihood ratio, given current accumulated cluster data, if
    it is determined that data fits scanning area of interest (high, low, both).
    Returns zero if rate not of interest else returns loglikelihood ratio as
    calculated by probability model. */
double CategoricalSpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
  return Calculator.CalcLogLikelihoodRatioOrdinal(gvCasesPerCategory);
}

/** No implemented - throws ZdException. */
count_t CategoricalSpatialData::GetCaseCount(unsigned int) const {
  ZdGenerateException("'GetCaseCount(unsigned int)' not implemeneted.","CategoricalSpatialData");
  return 0;
}

/** Returns number of accumulated cases for category index. */
count_t CategoricalSpatialData::GetCategoryCaseCount(unsigned int iCategoryIndex, unsigned int) const {
  return gvCasesPerCategory[iCategoryIndex];
}

/** No implemented - throws ZdException. */
measure_t CategoricalSpatialData::GetMeasure(unsigned int) const {
  ZdGenerateException("'GetMeasure(unsigned int)' not implemeneted.","CategoricalSpatialData");
  return 0;
}

//***************** class CategoricalTemporalData ******************************

/** class constructor */
CategoricalTemporalData::CategoricalTemporalData(const DataSetInterface& Interface)
                        :AbstractTemporalClusterData(),
                         gppCategoryCases(Interface.GetPTCategoryCaseArray()) {

  gvCasesPerCategory.resize(Interface.GetNumOrdinalCategories(), 0);
  InitializeData();
}

/** class constructor */
CategoricalTemporalData::CategoricalTemporalData(const AbtractDataSetGateway& DataGateway)
                        :AbstractTemporalClusterData(),
                         gppCategoryCases(DataGateway.GetDataSetInterface().GetPTCategoryCaseArray()) {

  gvCasesPerCategory.resize(DataGateway.GetDataSetInterface().GetNumOrdinalCategories(), 0);
  InitializeData();
}

/** destructor */
CategoricalTemporalData::~CategoricalTemporalData() {}

/** Returns newly cloned CategoricalTemporalData object. Caller responsible for
    deletion of object. */
CategoricalTemporalData * CategoricalTemporalData::Clone() const {
  return new CategoricalTemporalData(*this);
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractTemporalClusterData object
    can be casted to 'CategoricalTemporalData' object. */
void CategoricalTemporalData::Assign(const AbstractTemporalClusterData& rhs) {
  const CategoricalTemporalData& _rhs = (const CategoricalTemporalData&)rhs;
  gppCategoryCases = _rhs.gppCategoryCases;
  gvCasesPerCategory = _rhs.gvCasesPerCategory;
}

/** overloaded assignment operator */
CategoricalTemporalData & CategoricalTemporalData::operator=(const CategoricalTemporalData& rhs) {
  gppCategoryCases = rhs.gppCategoryCases;
  gvCasesPerCategory = rhs.gvCasesPerCategory;
  return *this;
}

/** Not implemented - throws ZdException. */
void CategoricalTemporalData::AddNeighborData(tract_t, const AbtractDataSetGateway&, size_t) {
  ZdGenerateException("AddNeighborData(tract_t,const AbtractDataSetGateway&, size_t) not implemeneted.","CategoricalTemporalData");
}

/** Not implemented - throws ZdException. */
unsigned int CategoricalTemporalData::GetAllocationSize() const {
  ZdGenerateException("GetAllocationSize() not implemeneted.","CategoricalTemporalData");
  return 0;
}

/** Not implemented - throws ZdException. */
count_t CategoricalTemporalData::GetCaseCount(unsigned int) const {
  ZdGenerateException("GetCaseCount(unsigned int) not implemeneted.","CategoricalTemporalData");
  return 0;
}

/** Returns number of accumulated cases for category. */
count_t CategoricalTemporalData::GetCategoryCaseCount(unsigned int iCategoryIndex, unsigned int) const {
  return gvCasesPerCategory[iCategoryIndex];
}

/** Not implemented - throws ZdException. */
measure_t CategoricalTemporalData::GetMeasure(unsigned int) const {
  ZdGenerateException("GetMeasure(unsigned int) not implemeneted.","CategoricalTemporalData");
  return 0;
}

/** Reassociates internal data with passed DataSetInterface pointers. */
void CategoricalTemporalData::Reassociate(const DataSetInterface& Interface) {
  gppCategoryCases = Interface.GetPTCategoryCaseArray();
}

/** Reassociates internal data with passed DataSetInterface pointers of DataGateway. */
void CategoricalTemporalData::Reassociate(const AbtractDataSetGateway& DataGateway) {
  gppCategoryCases = DataGateway.GetDataSetInterface().GetPTCategoryCaseArray();
}
//************** class CategoricalProspectiveSpatialData ***********************

/** class constructor */
CategoricalProspectiveSpatialData::CategoricalProspectiveSpatialData(const CSaTScanData& Data, const DataSetInterface& Interface)
                                  :CategoricalTemporalData(Interface) {
  try {
    Init();
    Setup(Data, Interface);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor(const CSaTScanData&, const DataSetInterface&)","CategoricalProspectiveSpatialData");
    throw;
  }
}

/** class constructor */
CategoricalProspectiveSpatialData::CategoricalProspectiveSpatialData(const CSaTScanData& Data, const AbtractDataSetGateway& DataGateway)
                                  :CategoricalTemporalData(DataGateway) {
  try {
    Init();
    Setup(Data, DataGateway.GetDataSetInterface());
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor(const CSaTScanData&, const AbtractDataSetGateway&)","CategoricalProspectiveSpatialData");
    throw;
  }
}

/** class copy constructor */
CategoricalProspectiveSpatialData::CategoricalProspectiveSpatialData(const CategoricalProspectiveSpatialData& rhs)
                                  :CategoricalTemporalData(rhs) {
  try {
    Init();
    gpCategoryCasesHandler = new TwoDimensionArrayHandler<count_t>(rhs.gpCategoryCasesHandler->Get1stDimension(), rhs.gpCategoryCasesHandler->Get2ndDimension(), 0);
    gppCategoryCases = gpCategoryCasesHandler->GetArray();
    *this = rhs;
  }
  catch (ZdException &x) {
    delete gpCategoryCasesHandler;
    x.AddCallpath("constructor(const CategoricalProspectiveSpatialData&)","CategoricalProspectiveSpatialData");
    throw;
  }
}

/** class destructor */
CategoricalProspectiveSpatialData::~CategoricalProspectiveSpatialData() {
  try {
    delete gpCategoryCasesHandler;
  }
  catch (...){}
}

/** Returns newly cloned CategoricalProspectiveSpatialData object. Caller
    responsible for deletion of object. */
CategoricalProspectiveSpatialData * CategoricalProspectiveSpatialData::Clone() const {
   return new CategoricalProspectiveSpatialData(*this);
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractTemporalClusterData object
    can be casted to 'CategoricalProspectiveSpatialData' object. */
void CategoricalProspectiveSpatialData::Assign(const AbstractTemporalClusterData& rhs) {
  const CategoricalProspectiveSpatialData& _rhs = (const CategoricalProspectiveSpatialData&)rhs;
  *gpCategoryCasesHandler = *(_rhs.gpCategoryCasesHandler);
  giNumTimeIntervals = _rhs.giNumTimeIntervals;
  giProspectiveStart = _rhs.giProspectiveStart;
  gvCasesPerCategory = _rhs.gvCasesPerCategory;
}

/** overloaded assignement operator */
CategoricalProspectiveSpatialData & CategoricalProspectiveSpatialData::operator=(const CategoricalProspectiveSpatialData& rhs) {
  *gpCategoryCasesHandler = *(rhs.gpCategoryCasesHandler);
  giNumTimeIntervals = rhs.giNumTimeIntervals;
  giProspectiveStart = rhs.giProspectiveStart;
  gvCasesPerCategory = rhs.gvCasesPerCategory;
  return *this;
}

/** Adds neighbor data to accumulation  - caller is responsible for ensuring that
    'tNeighborIndex' and 'tSetIndex' are valid indexes. */
void CategoricalProspectiveSpatialData::AddNeighborData(tract_t tNeighborIndex, const AbtractDataSetGateway& DataGateway, size_t tSetIndex) {
  unsigned int           i, j;
  count_t             ** ppCases = 0;

  //set cases for entire period added by this neighbor
  for (size_t t=0; t < gvCasesPerCategory.size(); ++t) {
     ppCases = DataGateway.GetDataSetInterface(tSetIndex).GetCategoryCaseArrays()[t];
     gppCategoryCases[t][0] += ppCases[0][tNeighborIndex];

    //set cases for prospective study periods
    for (j=1, i=giProspectiveStart; i < giNumTimeIntervals; ++j, ++i)
       gppCategoryCases[t][j] += ppCases[i][tNeighborIndex];
  }
}

/** Calculates loglikelihood ratio given current accumulated cluster data if
    it is determined that data fits scanning area of interest (high, low, both).
    Returns zero if all windows rates not of interest else returns greatest
    loglikelihood ratio as calculated by probability model. */
double CategoricalProspectiveSpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
  unsigned int  iWindowEnd;
  double        dMaxLoglikelihoodRatio=0;
  unsigned int  iMaxWindow = gpCategoryCasesHandler->Get2ndDimension() - 1;

  for (size_t t=0; t < gvCasesPerCategory.size(); ++t)
    gvCasesPerCategory[t] = gppCategoryCases[t][0];
  dMaxLoglikelihoodRatio = Calculator.CalcLogLikelihoodRatioOrdinal(gvCasesPerCategory);

  for (iWindowEnd=1; iWindowEnd < iMaxWindow; ++iWindowEnd) {
    for (size_t t=0; t < gvCasesPerCategory.size(); ++t)
       gvCasesPerCategory[t] = gppCategoryCases[t][0] - gppCategoryCases[t][iWindowEnd];
     dMaxLoglikelihoodRatio = std::max(dMaxLoglikelihoodRatio, Calculator.CalcLogLikelihoodRatioOrdinal(gvCasesPerCategory));
  }
  return dMaxLoglikelihoodRatio;
}

/** internal setup function */
void CategoricalProspectiveSpatialData::Setup(const CSaTScanData& Data, const DataSetInterface& Interface) {
  try {
    giNumTimeIntervals = Data.m_nTimeIntervals;
    giProspectiveStart = Data.GetProspectiveStartIndex();
    gpCategoryCasesHandler = new TwoDimensionArrayHandler<count_t>(Interface.GetNumOrdinalCategories(), 1 + giNumTimeIntervals - giProspectiveStart, 0);
    gppCategoryCases = gpCategoryCasesHandler->GetArray();
    gvCasesPerCategory.resize(Interface.GetNumOrdinalCategories(), 0);
  }
  catch (ZdException &x) {
    delete gpCategoryCasesHandler;
    x.AddCallpath("Setup(const CSaTScanData&, const DataSetInterface&)","CategoricalProspectiveSpatialData");
    throw;
  }
}

//****************** class  CategoricalSpaceTimeData ***************************

/** class constructor */
CategoricalSpaceTimeData::CategoricalSpaceTimeData(const DataSetInterface& Interface)
                         :CategoricalTemporalData(Interface) {
  try {
    Init();
    Setup(Interface);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor(const DataSetInterface&)","CategoricalSpaceTimeData");
    throw;
  }
}

/** class constructor */
CategoricalSpaceTimeData::CategoricalSpaceTimeData(const AbtractDataSetGateway& DataGateway)
                         :CategoricalTemporalData(DataGateway) {
  try {
    Init();
    Setup(DataGateway.GetDataSetInterface());
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor(const AbtractDataSetGateway&)","CategoricalSpaceTimeData");
    throw;
  }
}

/** class copy constructor */
CategoricalSpaceTimeData::CategoricalSpaceTimeData(const CategoricalSpaceTimeData& rhs)
                         :CategoricalTemporalData(rhs) {
  try {
    Init();
    gpCategoryCasesHandler = new TwoDimensionArrayHandler<count_t>(rhs.gpCategoryCasesHandler->Get1stDimension(), rhs.gpCategoryCasesHandler->Get2ndDimension(), 0);
    gppCategoryCases = gpCategoryCasesHandler->GetArray();
    *this = rhs;
  }
  catch (ZdException &x) {
    delete gpCategoryCasesHandler;
    x.AddCallpath("constructor(const CategoricalSpaceTimeData&)","CategoricalSpaceTimeData");
    throw;
  }
}

/** class destructor */
CategoricalSpaceTimeData::~CategoricalSpaceTimeData() {
  try {
    delete gpCategoryCasesHandler;
  }
  catch (...){}
}

/** Returns newly cloned CategoricalSpaceTimeData object. Caller responsible for
    deletion of object. */
CategoricalSpaceTimeData * CategoricalSpaceTimeData::Clone() const {
   return new CategoricalSpaceTimeData(*this);
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractTemporalClusterData object
    can be casted to 'CategoricalSpaceTimeData' object. */
void CategoricalSpaceTimeData::Assign(const AbstractTemporalClusterData& rhs) {
  const CategoricalSpaceTimeData& _rhs = (const CategoricalSpaceTimeData&)rhs;
  *gpCategoryCasesHandler = *(_rhs.gpCategoryCasesHandler);
  gvCasesPerCategory = _rhs.gvCasesPerCategory;
}

/** overloaded assignement operator */
CategoricalSpaceTimeData & CategoricalSpaceTimeData::operator=(const CategoricalSpaceTimeData& rhs) {
  *gpCategoryCasesHandler = *(rhs.gpCategoryCasesHandler);
  gvCasesPerCategory = rhs.gvCasesPerCategory;
  return *this;
}

/** Adds neighbor data to accumulation  - caller is responsible for ensuring that
    'tNeighborIndex' and 'tSetIndex' are valid indexes. */
void CategoricalSpaceTimeData::AddNeighborData(tract_t tNeighborIndex, const AbtractDataSetGateway& DataGateway, size_t tSetIndex) {
  count_t            ** ppCases=0;
  unsigned int          i, iMaxWindow = gpCategoryCasesHandler->Get2ndDimension() - 1;

  for (size_t t=0; t < gvCasesPerCategory.size(); ++t) {
    ppCases = DataGateway.GetDataSetInterface(tSetIndex).GetCategoryCaseArrays()[t];
    for (i=0; i < iMaxWindow; ++i)
       gppCategoryCases[t][i] += ppCases[i][tNeighborIndex];
  }        
}

/** internal setup function */
void CategoricalSpaceTimeData::Setup(const DataSetInterface& Interface) {
  try {
    //Note that second dimension is number of time intervals plus one - this permits
    //us to evaluate last time intervals data with same code as other time intervals
    //in CTimeIntervals object.
    gpCategoryCasesHandler = new TwoDimensionArrayHandler<count_t>(Interface.GetNumOrdinalCategories(), Interface.GetNumTimeIntervals() + 1, 0);
    gppCategoryCases = gpCategoryCasesHandler->GetArray();
  }
  catch (ZdException &x) {
    delete gpCategoryCasesHandler;
    x.AddCallpath("Setup(const DataSetInterface&)","CategoricalSpaceTimeData");
    throw;
  }
}

