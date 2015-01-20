//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "CategoricalClusterData.h"
#include "TimeIntervals.h"
#include "OrdinalLikelihoodCalculation.h"
#include "SSException.h"

/** class constructor */
CategoricalSpatialData::CategoricalSpatialData(const DataSetInterface& Interface)
                       :AbstractSpatialClusterData(), AbstractCategoricalClusterData() {

  gvCasesPerCategory.resize(Interface.GetNumOrdinalCategories(), 0);
  InitializeData();
}

/** class constructor */
CategoricalSpatialData::CategoricalSpatialData(const AbstractDataSetGateway& DataGateway)
                       :AbstractSpatialClusterData(), AbstractCategoricalClusterData() {
                        
  gvCasesPerCategory.resize(DataGateway.GetDataSetInterface().GetNumOrdinalCategories(), 0);
  InitializeData();
}

/** destructor */
CategoricalSpatialData::~CategoricalSpatialData() {}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractSpatialClusterData object
    can be casted to 'CategoricalSpatialData' object. */
void CategoricalSpatialData::Assign(const AbstractSpatialClusterData& rhs) {
  *this = (const CategoricalSpatialData&)rhs;
}

/** adds neighbor data to accumulation  - caller is responsible for ensuring that
    'tNeighborIndex' and 'tSetIndex' are valid indexes. */
void CategoricalSpatialData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex) {
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

/** Returns newly cloned CategoricalSpatialData object. Caller is responsible for
    deletion of object. */
CategoricalSpatialData * CategoricalSpatialData::Clone() const {
  return new CategoricalSpatialData(*this);
}

/** Copies class data members that reflect the number of cases per ordinal category,
    which is the data we are interested in for possiblely reporting. */
void CategoricalSpatialData::CopyEssentialClassMembers(const AbstractClusterData& rhs) {
  gvCasesPerCategory = ((const CategoricalSpatialData&)rhs).gvCasesPerCategory;
}

/** No implemented - throws prg_error. */
count_t CategoricalSpatialData::GetCaseCount(unsigned int) const {
  throw prg_error("'GetCaseCount(unsigned int)' not implemeneted.","CategoricalSpatialData");
}

/** No implemented - throws prg_error. */
void CategoricalSpatialData::setCaseCount(count_t t, unsigned int tSetIndex) {
  throw prg_error("'setCaseCount(count_t,unsigned int)' not implemeneted.","CategoricalSpatialData");
}

/** Returns number of accumulated cases for category index. */
count_t CategoricalSpatialData::GetCategoryCaseCount(unsigned int iCategoryIndex, unsigned int) const {
  return gvCasesPerCategory[iCategoryIndex];
}

/** Given ordinal category cases accumulated in cluster data, re-calculates the log likelihood
    ratio to determine which, if any, categories where combined into one category. */
void CategoricalSpatialData::GetOrdinalCombinedCategories(const OrdinalLikelihoodCalculator& Calculator,
                                                          std::vector<OrdinalCombinedCategory>& vCategoryContainer,
                                                          unsigned int) const {
  Calculator.CalculateOrdinalCombinedCategories(gvCasesPerCategory, vCategoryContainer);
}

/** Calculates and returns maximizing value given accumulated data, as calculated by passed AbstractLikelihoodCalculator. */
double CategoricalSpatialData::GetMaximizingValue(AbstractLikelihoodCalculator& Calculator) {
  return Calculator.CalculateMaximizingValueOrdinal(gvCasesPerCategory);
}

/** No implemented - throws prg_error. */
measure_t CategoricalSpatialData::GetMeasure(unsigned int) const {
  throw prg_error("'GetMeasure(unsigned int)' not implemeneted.","CategoricalSpatialData");
}

void CategoricalSpatialData::setMeasure(measure_t m, unsigned int tSetIndex) {
  throw prg_error("'setMeasure(measure,unsigned int)' not implemeneted.","CategoricalSpatialData");
}

//***************** class CategoricalTemporalData ******************************

/** class constructor */
CategoricalTemporalData::CategoricalTemporalData(const DataSetInterface& Interface)
                        :AbstractTemporalClusterData(), AbstractCategoricalClusterData(),
                         gppCategoryCases(Interface.GetPTCategoryCaseArray()) {
  gvCasesPerCategory.resize(Interface.GetNumOrdinalCategories(), 0);
  InitializeData();
}

/** class constructor */
CategoricalTemporalData::CategoricalTemporalData(const AbstractDataSetGateway& DataGateway)
                        :AbstractTemporalClusterData(),
                         gppCategoryCases(DataGateway.GetDataSetInterface().GetPTCategoryCaseArray()) {
  gvCasesPerCategory.resize(DataGateway.GetDataSetInterface().GetNumOrdinalCategories(), 0);
  InitializeData();
}

/** destructor */
CategoricalTemporalData::~CategoricalTemporalData() {}

/** Not implemented - throws prg_error. */
void CategoricalTemporalData::AddNeighborData(tract_t, const AbstractDataSetGateway&, size_t) {
  throw prg_error("AddNeighborData(tract_t,const AbstractDataSetGateway&, size_t) not implemeneted.","CategoricalTemporalData");
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractTemporalClusterData object
    can be casted to 'CategoricalTemporalData' object. */
void CategoricalTemporalData::Assign(const AbstractTemporalClusterData& rhs) {
  *this = (const CategoricalTemporalData&)rhs;
}
void CategoricalTemporalData::CopyEssentialClassMembers(const AbstractClusterData& rhs) {
  gvCasesPerCategory = ((const CategoricalTemporalData&)rhs).gvCasesPerCategory;
}

/** Returns newly cloned CategoricalTemporalData object. Caller responsible for deletion of object. */
CategoricalTemporalData * CategoricalTemporalData::Clone() const {
  return new CategoricalTemporalData(*this);
}

/** Not implemented - throws prg_error. */
unsigned int CategoricalTemporalData::GetAllocationSize() const {
  throw prg_error("GetAllocationSize() not implemeneted.","CategoricalTemporalData");
}

/** Not implemented - throws prg_error. */
count_t CategoricalTemporalData::GetCaseCount(unsigned int) const {
  throw prg_error("GetCaseCount(unsigned int) not implemeneted.","CategoricalTemporalData");
}

/** No implemented - throws prg_error. */
void CategoricalTemporalData::setCaseCount(count_t t, unsigned int tSetIndex) {
  throw prg_error("'setCaseCount(count_t,unsigned int)' not implemeneted.","CategoricalSpatialData");
}

/** Returns number of accumulated cases for category. */
count_t CategoricalTemporalData::GetCategoryCaseCount(unsigned int iCategoryIndex, unsigned int) const {
  return gvCasesPerCategory[iCategoryIndex];
}

/** Given ordinal category cases accumulated in cluster data, re-calculates the log likelihood
    ratio to determine which, if any, categories where combined into one category. */
void CategoricalTemporalData::GetOrdinalCombinedCategories(const OrdinalLikelihoodCalculator& Calculator,
                                                           std::vector<OrdinalCombinedCategory>& vCategoryContainer,
                                                           unsigned int) const {
  Calculator.CalculateOrdinalCombinedCategories(gvCasesPerCategory, vCategoryContainer);
}

/** Not implemented - throws prg_error. */
measure_t CategoricalTemporalData::GetMeasure(unsigned int) const {
  throw prg_error("GetMeasure(unsigned int) not implemeneted.","CategoricalTemporalData");
}

void CategoricalTemporalData::setMeasure(measure_t m, unsigned int tSetIndex) {
  throw prg_error("'setMeasure(measure,unsigned int)' not implemeneted.","CategoricalSpatialData");
}

/** Reassociates internal data with passed DataSetInterface pointers. */
void CategoricalTemporalData::Reassociate(const DataSetInterface& Interface) {
  gppCategoryCases = Interface.GetPTCategoryCaseArray();
}

/** Reassociates internal data with passed DataSetInterface pointers of DataGateway. */
void CategoricalTemporalData::Reassociate(const AbstractDataSetGateway& DataGateway) {
  gppCategoryCases = DataGateway.GetDataSetInterface().GetPTCategoryCaseArray();
}
//************** class CategoricalProspectiveSpatialData ***********************

/** class constructor */
CategoricalProspectiveSpatialData::CategoricalProspectiveSpatialData(const CSaTScanData& Data, const DataSetInterface& Interface)
                                  :CategoricalTemporalData(Interface), geEvaluationAssistDataStatus(Allocated) {
  try {
    Setup(Data, Interface);
  }
  catch (prg_exception& x) {
    x.addTrace("constructor(const CSaTScanData&, const DataSetInterface&)","CategoricalProspectiveSpatialData");
    throw;
  }
}

/** class constructor */
CategoricalProspectiveSpatialData::CategoricalProspectiveSpatialData(const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway)
                                  :CategoricalTemporalData(DataGateway), geEvaluationAssistDataStatus(Allocated) {
  try {
    Setup(Data, DataGateway.GetDataSetInterface());
  }
  catch (prg_exception& x) {
    x.addTrace("constructor(const CSaTScanData&, const AbstractDataSetGateway&)","CategoricalProspectiveSpatialData");
    throw;
  }
}

/** class copy constructor */
CategoricalProspectiveSpatialData::CategoricalProspectiveSpatialData(const CategoricalProspectiveSpatialData& rhs)
                                  :CategoricalTemporalData(rhs) {
  try {
    *this = rhs;
  }
  catch (prg_exception& x) {
    x.addTrace("constructor(const CategoricalProspectiveSpatialData&)","CategoricalProspectiveSpatialData");
    throw;
  }
}

/** Adds neighbor data to accumulation  - caller is responsible for ensuring that
    'tNeighborIndex' and 'tSetIndex' are valid indexes. */
void CategoricalProspectiveSpatialData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex) {
  assert(geEvaluationAssistDataStatus == Allocated);
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

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractTemporalClusterData object
    can be casted to 'CategoricalProspectiveSpatialData' object. */
void CategoricalProspectiveSpatialData::Assign(const AbstractTemporalClusterData& rhs) {
  *this = (const CategoricalProspectiveSpatialData&)rhs;
}

/** Calculates loglikelihood ratio given current accumulated cluster data if
    it is determined that data fits scanning area of interest (high, low, both).
    Returns zero if all windows rates not of interest else returns greatest
    loglikelihood ratio as calculated by probability model. */
double CategoricalProspectiveSpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
  assert(geEvaluationAssistDataStatus == Allocated);
  unsigned int  iWindowEnd;
  double        dMaxLoglikelihoodRatio=0;
  unsigned int  iMaxWindow = gCategoryCasesHandler->Get2ndDimension() - 1;

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

/** Calculates and returns maximizing value given accumulated data, as calculated by passed AbstractLikelihoodCalculator. */
double CategoricalProspectiveSpatialData::GetMaximizingValue(AbstractLikelihoodCalculator& Calculator) {
  assert(geEvaluationAssistDataStatus == Allocated);
  unsigned int  iWindowEnd;
  double        dMaxValue(-std::numeric_limits<double>::max());
  unsigned int  iMaxWindow = gCategoryCasesHandler->Get2ndDimension() - 1;

  for (size_t t=0; t < gvCasesPerCategory.size(); ++t)
    gvCasesPerCategory[t] = gppCategoryCases[t][0];
  dMaxValue = Calculator.CalculateMaximizingValueOrdinal(gvCasesPerCategory, 0);

  for (iWindowEnd=1; iWindowEnd < iMaxWindow; ++iWindowEnd) {
    for (size_t t=0; t < gvCasesPerCategory.size(); ++t)
       gvCasesPerCategory[t] = gppCategoryCases[t][0] - gppCategoryCases[t][iWindowEnd];
     dMaxValue = std::max(dMaxValue, Calculator.CalculateMaximizingValueOrdinal(gvCasesPerCategory, 0));
  }
  return dMaxValue;
}

/** Returns newly cloned CategoricalProspectiveSpatialData object. Caller
    responsible for deletion of object. */
CategoricalProspectiveSpatialData * CategoricalProspectiveSpatialData::Clone() const {
   return new CategoricalProspectiveSpatialData(*this);
}

/** Deallocates data members that assist with evaluation of temporal data.
    Once this function is called various class member functions become invalid
    and an assertion will fail if called. */
void CategoricalProspectiveSpatialData::DeallocateEvaluationAssistClassMembers() {
  gCategoryCasesHandler.reset(0); gppCategoryCases=0;
  geEvaluationAssistDataStatus = Deallocated;
}

/** overloaded assignement operator */
CategoricalProspectiveSpatialData & CategoricalProspectiveSpatialData::operator=(const CategoricalProspectiveSpatialData& rhs) {
  //data member 'gpCategoryCasesHandler' is an evaluation assisting data member that might be deallocated
  if (rhs.geEvaluationAssistDataStatus == Allocated) {
    if (!gCategoryCasesHandler.get())
      gCategoryCasesHandler.reset(new TwoDimensionArrayHandler<count_t>(rhs.gCategoryCasesHandler->Get1stDimension(), rhs.gCategoryCasesHandler->Get2ndDimension(), 0));
    *gCategoryCasesHandler = *(rhs.gCategoryCasesHandler);
    gppCategoryCases = gCategoryCasesHandler->GetArray();
  }
  else {
    gCategoryCasesHandler.reset(0); gppCategoryCases=0;
  }
  geEvaluationAssistDataStatus = rhs.geEvaluationAssistDataStatus;
  giNumTimeIntervals = rhs.giNumTimeIntervals;
  giProspectiveStart = rhs.giProspectiveStart;
  gvCasesPerCategory = rhs.gvCasesPerCategory;
  return *this;
}

/** internal setup function */
void CategoricalProspectiveSpatialData::Setup(const CSaTScanData& Data, const DataSetInterface& Interface) {
  try {
    giNumTimeIntervals = Data.GetNumTimeIntervals();
    giProspectiveStart = Data.GetProspectiveStartIndex();
    gCategoryCasesHandler.reset(new TwoDimensionArrayHandler<count_t>(Interface.GetNumOrdinalCategories(), 1 + giNumTimeIntervals - giProspectiveStart, 0));
    gppCategoryCases = gCategoryCasesHandler->GetArray();
    gvCasesPerCategory.resize(Interface.GetNumOrdinalCategories(), 0);
  }
  catch (prg_exception& x) {
    x.addTrace("Setup(const CSaTScanData&, const DataSetInterface&)","CategoricalProspectiveSpatialData");
    throw;
  }
}

//****************** class  CategoricalSpaceTimeData ***************************

/** class constructor */
CategoricalSpaceTimeData::CategoricalSpaceTimeData(const DataSetInterface& Interface)
                         :CategoricalTemporalData(Interface), geEvaluationAssistDataStatus(Allocated) {
  try {
    Setup(Interface);
  }
  catch (prg_exception& x) {
    x.addTrace("constructor(const DataSetInterface&)","CategoricalSpaceTimeData");
    throw;
  }
}

/** class constructor */
CategoricalSpaceTimeData::CategoricalSpaceTimeData(const AbstractDataSetGateway& DataGateway)
                         :CategoricalTemporalData(DataGateway), geEvaluationAssistDataStatus(Allocated) {
  try {
    Setup(DataGateway.GetDataSetInterface());
  }
  catch (prg_exception& x) {
    x.addTrace("constructor(const AbstractDataSetGateway&)","CategoricalSpaceTimeData");
    throw;
  }
}

/** class copy constructor */
CategoricalSpaceTimeData::CategoricalSpaceTimeData(const CategoricalSpaceTimeData& rhs)
                         :CategoricalTemporalData(rhs) {
  try {
    *this = rhs;
  }
  catch (prg_exception& x) {
    x.addTrace("constructor(const CategoricalSpaceTimeData&)","CategoricalSpaceTimeData");
    throw;
  }
}

/** Adds neighbor data to accumulation  - caller is responsible for ensuring that
    'tNeighborIndex' and 'tSetIndex' are valid indexes. */
void CategoricalSpaceTimeData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex) {
  assert(geEvaluationAssistDataStatus == Allocated);
  count_t            ** ppCases=0;
  unsigned int          i, iMaxWindow = gCategoryCasesHandler->Get2ndDimension() - 1;

  for (size_t t=0; t < gvCasesPerCategory.size(); ++t) {
    ppCases = DataGateway.GetDataSetInterface(tSetIndex).GetCategoryCaseArrays()[t];
    for (i=0; i < iMaxWindow; ++i)
       gppCategoryCases[t][i] += ppCases[i][tNeighborIndex];
  }
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractTemporalClusterData object
    can be casted to 'CategoricalSpaceTimeData' object. */
void CategoricalSpaceTimeData::Assign(const AbstractTemporalClusterData& rhs) {
  *this = (const CategoricalSpaceTimeData&)rhs;
}

/** Returns newly cloned CategoricalSpaceTimeData object. Caller responsible for
    deletion of object. */
CategoricalSpaceTimeData * CategoricalSpaceTimeData::Clone() const {
   return new CategoricalSpaceTimeData(*this);
}

/** Deallocates data members that assist with evaluation of temporal data.
    Once this function is called various class member functions become invalid
    and on assertion will fail if called. */
void CategoricalSpaceTimeData::DeallocateEvaluationAssistClassMembers() {
  gCategoryCasesHandler.reset(0); gppCategoryCases = 0;
}

/** overloaded assignement operator */
CategoricalSpaceTimeData & CategoricalSpaceTimeData::operator=(const CategoricalSpaceTimeData& rhs) {
  if (rhs.geEvaluationAssistDataStatus == Allocated) {
    if (!gCategoryCasesHandler.get())
      gCategoryCasesHandler.reset(new TwoDimensionArrayHandler<count_t>(rhs.gCategoryCasesHandler->Get1stDimension(), rhs.gCategoryCasesHandler->Get2ndDimension(), 0));
    *gCategoryCasesHandler = *(rhs.gCategoryCasesHandler);
    gppCategoryCases = gCategoryCasesHandler->GetArray();
  }
  else {
    gCategoryCasesHandler.reset(0); gppCategoryCases=0;
  }
  gvCasesPerCategory = rhs.gvCasesPerCategory;
  geEvaluationAssistDataStatus = rhs.geEvaluationAssistDataStatus;
  return *this;
}

/** internal setup function */
void CategoricalSpaceTimeData::Setup(const DataSetInterface& Interface) {
  try {
    //Note that second dimension is number of time intervals plus one - this permits
    //us to evaluate last time intervals data with same code as other time intervals
    //in CTimeIntervals object.
    gCategoryCasesHandler.reset(new TwoDimensionArrayHandler<count_t>(Interface.GetNumOrdinalCategories(), Interface.GetNumTimeIntervals() + 1, 0));
    gppCategoryCases = gCategoryCasesHandler->GetArray();
  }
  catch (prg_exception& x) {
    x.addTrace("Setup(const DataSetInterface&)","CategoricalSpaceTimeData");
    throw;
  }
}

