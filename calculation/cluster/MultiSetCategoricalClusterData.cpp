//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "MultiSetCategoricalClusterData.h"
#include "CategoricalClusterDataFactory.h"
#include "LoglikelihoodRatioUnifier.h"
#include "OrdinalLikelihoodCalculation.h"

/** class constructor */
MultiSetCategoricalSpatialData::MultiSetCategoricalSpatialData(const CategoricalClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway, int iRate)
                                  :AbstractSpatialClusterData(0), AbstractCategoricalClusterData() {
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvSetClusterData.push_back(dynamic_cast<CategoricalSpatialData*>(DataFactory.GetNewSpatialClusterData(DataGateway.GetDataSetInterface(t), iRate)));
}

/** class destructor */
MultiSetCategoricalSpatialData::~MultiSetCategoricalSpatialData() {}

/** Returns newly cloned MultiSetCategoricalSpatialData object. Caller is
    responsible for deletion of object. */
MultiSetCategoricalSpatialData * MultiSetCategoricalSpatialData::Clone() const {
  return new MultiSetCategoricalSpatialData(*this);
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractSpatialClusterData object
    can be casted to 'MultiSetCategoricalSpatialData' object. */
void MultiSetCategoricalSpatialData::Assign(const AbstractSpatialClusterData& rhs) {
  const MultiSetCategoricalSpatialData& _rhs = (const MultiSetCategoricalSpatialData&)rhs;
  gvSetClusterData = _rhs.gvSetClusterData;
}

/** Adds neighbor data to accumulation  - caller is responsible for ensuring that
    'tNeighborIndex' is a valid index. */
void MultiSetCategoricalSpatialData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t) {
  unsigned int i;
  for (i=0, gitr=gvSetClusterData.begin(); gitr != gvSetClusterData.end(); ++i, ++gitr)
     (*gitr)->AddNeighborData(tNeighborIndex, DataGateway, i);
}

/** Calculates loglikelihood ratio given current accumulated cluster data in
    each data set and adds together. */
double MultiSetCategoricalSpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
  unsigned int                          i=0;
  AbstractLoglikelihoodRatioUnifier   & Unifier = Calculator.GetUnifier();

 Unifier.Reset();
 for (gitr=gvSetClusterData.begin(); gitr != gvSetClusterData.end(); ++gitr, ++i)
     Unifier.AdjoinRatio(Calculator, (*gitr)->gvCasesPerCategory, i);
  return Unifier.GetLoglikelihoodRatio();
}

/** Not implemented - throws ZdException. */
count_t MultiSetCategoricalSpatialData::GetCaseCount(unsigned int) const {
  ZdGenerateException("GetCaseCount(unsigned int) not implemented.","MultiSetCategoricalSpatialData");
  return 0;
}

/** Returns number of cases in accumulated cluster data for data set and category.
    Caller responsible for ensuring that 'iCategoryIndex' and 'tSetIndex' are valid indexes. */
count_t MultiSetCategoricalSpatialData::GetCategoryCaseCount(unsigned int iCategoryIndex, unsigned int tSetIndex) const {
  return gvSetClusterData[tSetIndex]->GetCategoryCaseCount(iCategoryIndex);
}

/** Given ordinal category cases accumulated in cluster data, re-calculates the log likelihood
    ratio to determine which, if any, categories where combined into one category. */
void MultiSetCategoricalSpatialData::GetOrdinalCombinedCategories(const OrdinalLikelihoodCalculator& Calculator,
                                                                  std::vector<OrdinalCombinedCategory>& vCategoryContainer,
                                                                  unsigned int tSetIndex) const {
  Calculator.CalculateOrdinalCombinedCategories(gvSetClusterData[tSetIndex]->gvCasesPerCategory, vCategoryContainer);
}

/** Not implemented - throws ZdException. */
measure_t MultiSetCategoricalSpatialData::GetMeasure(unsigned int) const {
  ZdGenerateException("GetMeasure(unsigned int) not implemented.","MultiSetCategoricalSpatialData");
  return 0;
}

/** Initializes cluster data in each data set. */
void MultiSetCategoricalSpatialData::InitializeData() {
  for (gitr=gvSetClusterData.begin(); gitr != gvSetClusterData.end(); ++gitr)
     (*gitr)->InitializeData();
}

//******************************************************************************

/** class constructor */
AbstractMultiSetCategoricalTemporalData::AbstractMultiSetCategoricalTemporalData() : AbstractTemporalClusterData() {}

/** class destructor */
AbstractMultiSetCategoricalTemporalData::~AbstractMultiSetCategoricalTemporalData() {}

/** Not implemented - throws ZdException. */
count_t AbstractMultiSetCategoricalTemporalData::GetCaseCount(unsigned int) const {
  ZdGenerateException("GetCaseCount(unsigned int) not implemented.","AbstractMultiSetCategoricalTemporalData");
  return 0;
}

/** Returns number of cases in accumulated cluster data for data set and category.
    Caller responsible for ensuring that 'iCategoryIndex' and 'tSetIndex' are valid indexes. */
count_t AbstractMultiSetCategoricalTemporalData::GetCategoryCaseCount(unsigned int iCategoryIndex, unsigned int tSetIndex) const {
  return gvSetClusterData[tSetIndex]->GetCategoryCaseCount(iCategoryIndex);
}

/** Given ordinal category cases accumulated in cluster data, re-calculates the log likelihood
    ratio to determine which, if any, categories where combined into one category. */
void AbstractMultiSetCategoricalTemporalData::GetOrdinalCombinedCategories(const OrdinalLikelihoodCalculator& Calculator,
                                                                           std::vector<OrdinalCombinedCategory>& vCategoryContainer,
                                                                           unsigned int tSetIndex) const {
  Calculator.CalculateOrdinalCombinedCategories(gvSetClusterData[tSetIndex]->gvCasesPerCategory, vCategoryContainer);
}

/** Not implemented - throws ZdException. */
measure_t AbstractMultiSetCategoricalTemporalData::GetMeasure(unsigned int) const {
  ZdGenerateException("GetCaseCount(unsigned int) not implemented.","AbstractMultiSetCategoricalTemporalData");
  return 0;
}

/** Initializes cluster data in each data set. */
void AbstractMultiSetCategoricalTemporalData::InitializeData() {
  for (gitr=gvSetClusterData.begin(); gitr != gvSetClusterData.end(); ++gitr)
     (*gitr)->InitializeData();
}

//******************************************************************************

/** class constructor */
MultiSetCategoricalTemporalData::MultiSetCategoricalTemporalData(const CategoricalClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway)
                                :AbstractMultiSetCategoricalTemporalData() {
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvSetClusterData.push_back(dynamic_cast<CategoricalTemporalData*>(DataFactory.GetNewTemporalClusterData(DataGateway.GetDataSetInterface(t))));
}

/** class destructor */
MultiSetCategoricalTemporalData::~MultiSetCategoricalTemporalData() {}

/** Return newly cloned MultiSetCategoricalTemporalData object.
    Caller is responsible for deletion of object. */
MultiSetCategoricalTemporalData * MultiSetCategoricalTemporalData::Clone() const {
  return new MultiSetCategoricalTemporalData(*this);
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractTemporalClusterData object
    can be casted to 'MultiSetCategoricalTemporalData' object. */
void MultiSetCategoricalTemporalData::Assign(const AbstractTemporalClusterData& rhs) {
  const MultiSetCategoricalTemporalData& _rhs = (const MultiSetCategoricalTemporalData&)rhs;
  gvSetClusterData = _rhs.gvSetClusterData;
}

/** Not implemented - throws ZdException. */
void MultiSetCategoricalTemporalData::AddNeighborData(tract_t, const AbstractDataSetGateway&, size_t) {
  ZdGenerateException("AddNeighbor(tract_t, const AbstractDataSetGateway&, size_t) not implemeneted.","MultiSetCategoricalTemporalData");
}

/** Reassociates internal data with passed DataSetInterface pointers.
    Not implemented - throws exception */
void MultiSetCategoricalTemporalData::Reassociate(const DataSetInterface& Interface) {
  ZdGenerateException("Reassociate(const DataSetInterface&) not implemented.","MultiSetCategoricalTemporalData");
}

/** Reassociates internal data with passed DataSetInterface pointers of DataGateway. */
void MultiSetCategoricalTemporalData::Reassociate(const AbstractDataSetGateway& DataGateway) {
  for (gitr=gvSetClusterData.begin(); gitr != gvSetClusterData.end(); ++gitr)
     (*gitr)->Reassociate(DataGateway);
}
//******************************************************************************

/** class constructor */
MultiSetCategoricalProspectiveSpatialData::MultiSetCategoricalProspectiveSpatialData(const CategoricalClusterDataFactory& DataFactory, const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway)
                                          :AbstractMultiSetCategoricalTemporalData() {
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvSetClusterData.push_back(dynamic_cast<CategoricalTemporalData*>(DataFactory.GetNewProspectiveSpatialClusterData(Data, DataGateway.GetDataSetInterface(t))));
}

/** class destructor */
MultiSetCategoricalProspectiveSpatialData::~MultiSetCategoricalProspectiveSpatialData() {}

/** Return newly cloned MultiSetCategoricalProspectiveSpatialData object.
    Caller is responsible for deletion of object. */
MultiSetCategoricalProspectiveSpatialData * MultiSetCategoricalProspectiveSpatialData::Clone() const {
  return new MultiSetCategoricalProspectiveSpatialData(*this);
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractTemporalClusterData object
    can be casted to 'MultiSetCategoricalProspectiveSpatialData' object. */
void MultiSetCategoricalProspectiveSpatialData::Assign(const AbstractTemporalClusterData& rhs) {
  const MultiSetCategoricalProspectiveSpatialData& _rhs = (const MultiSetCategoricalProspectiveSpatialData&)rhs;
  gvSetClusterData = _rhs.gvSetClusterData;
}

/** Adds neighbor data to accumulated cluster data  - caller is responsible for
    ensuring that 'tNeighborIndex' is a valid index. */
void MultiSetCategoricalProspectiveSpatialData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t) {
  unsigned int i;

  for (i=0, gitr=gvSetClusterData.begin(); gitr != gvSetClusterData.end(); ++i, ++gitr)
     (*gitr)->AddNeighborData(tNeighborIndex, DataGateway, i);
}

/** Calculates loglikelihood ratio given current accumulated cluster data in
    each data set and adds together.*/
double MultiSetCategoricalProspectiveSpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
  unsigned int                          i, iWindowEnd, iAllocationSize;
  double                                dMaxLoglikelihoodRatio=0;
  AbstractLoglikelihoodRatioUnifier   & Unifier = Calculator.GetUnifier();

  Unifier.Reset();
  iAllocationSize = (*gvSetClusterData.begin())->GetAllocationSize();
  for (i=0, gitr=gvSetClusterData.begin(); gitr != gvSetClusterData.end(); ++gitr, ++i) {
     for (size_t t=0; t < (*gitr)->gvCasesPerCategory.size(); ++t)
        (*gitr)->gvCasesPerCategory[t] = (*gitr)->gppCategoryCases[t][0];
    Unifier.AdjoinRatio(Calculator, (*gitr)->gvCasesPerCategory, i);
  }  
  dMaxLoglikelihoodRatio = Unifier.GetLoglikelihoodRatio();

 for (iWindowEnd=1; iWindowEnd < iAllocationSize; ++iWindowEnd) {
     Unifier.Reset();
     for (i=0, gitr=gvSetClusterData.begin(); gitr != gvSetClusterData.end(); ++gitr, ++i) {
        for (size_t t=0; t < (*gitr)->gvCasesPerCategory.size(); ++t)
           (*gitr)->gvCasesPerCategory[t] = (*gitr)->gppCategoryCases[t][0] - (*gitr)->gppCategoryCases[t][iWindowEnd];
        Unifier.AdjoinRatio(Calculator, (*gitr)->gvCasesPerCategory, i);
     }
     dMaxLoglikelihoodRatio = std::max(dMaxLoglikelihoodRatio, Unifier.GetLoglikelihoodRatio());
  }

  return dMaxLoglikelihoodRatio;
}

//******************************************************************************

/** class constructor */
MultiSetCategoricalSpaceTimeData::MultiSetCategoricalSpaceTimeData(const CategoricalClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway)
                                 :AbstractMultiSetCategoricalTemporalData() {
  gvSetClusterData.resize(DataGateway.GetNumInterfaces(), 0);
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvSetClusterData[t] = dynamic_cast<CategoricalTemporalData*>(DataFactory.GetNewSpaceTimeClusterData(DataGateway.GetDataSetInterface(t)));
}

/** class destructor */
MultiSetCategoricalSpaceTimeData::~MultiSetCategoricalSpaceTimeData() {}

/** Return newly cloned MultiSetCategoricalSpaceTimeData object.
    Caller is responsible for deletion of object. */
MultiSetCategoricalSpaceTimeData * MultiSetCategoricalSpaceTimeData::Clone() const {
  return new MultiSetCategoricalSpaceTimeData(*this);
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractTemporalClusterData object
    can be casted to 'MultiSetCategoricalSpaceTimeData' object. */
void MultiSetCategoricalSpaceTimeData::Assign(const AbstractTemporalClusterData& rhs) {
  const MultiSetCategoricalSpaceTimeData& _rhs = (const MultiSetCategoricalSpaceTimeData&)rhs;
  gvSetClusterData = _rhs.gvSetClusterData;
}

/** Add neighbor data to accumulation  - caller is responsible for ensuring that
   'tNeighborIndex' is a valid index. */
void MultiSetCategoricalSpaceTimeData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t) {
  unsigned int i;
  for (i=0, gitr=gvSetClusterData.begin(); gitr != gvSetClusterData.end(); ++i, ++gitr)
     (*gitr)->AddNeighborData(tNeighborIndex, DataGateway, i);
}


