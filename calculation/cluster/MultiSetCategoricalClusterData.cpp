//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "CategoricalClusterDataFactory.h"
#include "LoglikelihoodRatioUnifier.h"
#include "OrdinalLikelihoodCalculation.h"
#include "MultiSetCategoricalClusterData.h"
#include "SSException.h"

//***************** class MultiSetCategoricalSpatialData ***********************

/** class constructor */
MultiSetCategoricalSpatialData::MultiSetCategoricalSpatialData(const CategoricalClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway)
                                  :AbstractSpatialClusterData(), AbstractCategoricalClusterData() {
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvSetClusterData.push_back(dynamic_cast<CategoricalSpatialData*>(DataFactory.GetNewSpatialClusterData(DataGateway.GetDataSetInterface(t))));
}

/** Adds neighbor data to accumulation  - caller is responsible for ensuring that
    'tNeighborIndex' is a valid index. */
void MultiSetCategoricalSpatialData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t) {
  unsigned int                                        i=0;
  ptr_vector<CategoricalSpatialData>::iterator   itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++i, ++itr) (*itr)->AddNeighborData(tNeighborIndex, DataGateway, i);
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractSpatialClusterData object
    can be casted to 'MultiSetCategoricalSpatialData' object. */
void MultiSetCategoricalSpatialData::Assign(const AbstractSpatialClusterData& rhs) {
  *this = (const MultiSetCategoricalSpatialData&)rhs;
}

const AbstractLoglikelihoodRatioUnifier & MultiSetCategoricalSpatialData::getRatioUnified(AbstractLikelihoodCalculator& Calculator) const {
    unsigned int i=0;
    ptr_vector<CategoricalSpatialData>::const_iterator itr=gvSetClusterData.begin();
    AbstractLoglikelihoodRatioUnifier & Unifier = Calculator.GetUnifier();

    Unifier.Reset();
    for (;itr != gvSetClusterData.end(); ++itr, ++i) Unifier.AdjoinRatio(Calculator, (*itr)->gvCasesPerCategory, i);
    return Unifier;
}

/** Calculates loglikelihood ratio given current accumulated cluster data in
    each data set and adds together. */
double MultiSetCategoricalSpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
    return getRatioUnified(Calculator).GetLoglikelihoodRatio();
}

/** Returns newly cloned MultiSetCategoricalSpatialData object. Caller is
    responsible for deletion of object. */
MultiSetCategoricalSpatialData * MultiSetCategoricalSpatialData::Clone() const {
  return new MultiSetCategoricalSpatialData(*this);
}

/** Copies class data members that reflect the number of cases per ordinal category,
    which is the data we are interested in for possiblely reporting. */
void MultiSetCategoricalSpatialData::CopyEssentialClassMembers(const AbstractClusterData& rhs) {
  for (size_t t=0; t < ((const MultiSetCategoricalSpatialData&)rhs).gvSetClusterData.size(); ++t)
    gvSetClusterData[t]->CopyEssentialClassMembers(*((const MultiSetCategoricalSpatialData&)rhs).gvSetClusterData[t]);
}

/** Not implemented - throws prg_error. */
count_t MultiSetCategoricalSpatialData::GetCaseCount(unsigned int) const {
  throw prg_error("GetCaseCount(unsigned int) not implemented.","MultiSetCategoricalSpatialData");
}

/** No implemented - throws prg_error. */
void MultiSetCategoricalSpatialData::setCaseCount(count_t t, unsigned int tSetIndex) {
  throw prg_error("'setCaseCount(count_t,unsigned int)' not implemeneted.","MultiSetCategoricalSpatialData");
}

/** Returns number of cases in accumulated cluster data for data set and category.
    Caller responsible for ensuring that 'iCategoryIndex' and 'tSetIndex' are valid indexes. */
count_t MultiSetCategoricalSpatialData::GetCategoryCaseCount(unsigned int iCategoryIndex, unsigned int tSetIndex) const {
  return gvSetClusterData.at(tSetIndex)->GetCategoryCaseCount(iCategoryIndex);
}

/** Implements interface that calculates maximizing value - for multiple data set, maximizing
    value is the full test statistic/ratio. */
double MultiSetCategoricalSpatialData::GetMaximizingValue(AbstractLikelihoodCalculator& Calculator) {
  return CalculateLoglikelihoodRatio(Calculator);
}

/** Not implemented - throws prg_error. */
measure_t MultiSetCategoricalSpatialData::GetMeasure(unsigned int) const {
  throw prg_error("GetMeasure(unsigned int) not implemented.","MultiSetCategoricalSpatialData");
}

void MultiSetCategoricalSpatialData::setMeasure(measure_t m, unsigned int tSetIndex) {
  throw prg_error("'setMeasure(measure,unsigned int)' not implemeneted.","MultiSetCategoricalSpatialData");
}

/** Given ordinal category cases accumulated in cluster data, re-calculates the log likelihood
    ratio to determine which, if any, categories where combined into one category. */
void MultiSetCategoricalSpatialData::GetOrdinalCombinedCategories(const OrdinalLikelihoodCalculator& Calculator,
                                                                  std::vector<OrdinalCombinedCategory>& vCategoryContainer,
                                                                  unsigned int tSetIndex) const {
  Calculator.CalculateOrdinalCombinedCategories(gvSetClusterData[tSetIndex]->gvCasesPerCategory, vCategoryContainer, tSetIndex);
}

/** Initializes cluster data in each data set. */
void MultiSetCategoricalSpatialData::InitializeData() {
  ptr_vector<CategoricalSpatialData>::iterator   itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++itr) (*itr)->InitializeData();
}

//***************** class AbstractMultiSetCategoricalTemporalData **************

void AbstractMultiSetCategoricalTemporalData::CopyEssentialClassMembers(const AbstractClusterData& rhs) {
  for (size_t t=0; t < ((const AbstractMultiSetCategoricalTemporalData&)rhs).gvSetClusterData.size(); ++t)
    gvSetClusterData[t]->CopyEssentialClassMembers(*((const AbstractMultiSetCategoricalTemporalData&)rhs).gvSetClusterData[t]);
}

/** Not implemented - throws prg_error. */
count_t AbstractMultiSetCategoricalTemporalData::GetCaseCount(unsigned int) const {
  throw prg_error("GetCaseCount(unsigned int) not implemented.","AbstractMultiSetCategoricalTemporalData");
}

/** No implemented - throws prg_error. */
void AbstractMultiSetCategoricalTemporalData::setCaseCount(count_t t, unsigned int tSetIndex) {
  throw prg_error("'setCaseCount(count_t,unsigned int)' not implemeneted.","AbstractMultiSetCategoricalTemporalData");
}

const AbstractLoglikelihoodRatioUnifier & AbstractMultiSetCategoricalTemporalData::getRatioUnified(AbstractLikelihoodCalculator& Calculator) const {
    AbstractLoglikelihoodRatioUnifier & Unifier = Calculator.GetUnifier();

    Unifier.Reset();
    for (size_t t=0; t < gvSetClusterData.size(); ++t) {
        Unifier.AdjoinRatio(Calculator, gvSetClusterData[t]->gvCasesPerCategory, t);
    }
    return Unifier;
}

/** Returns number of cases in accumulated cluster data for data set and category.
    Caller responsible for ensuring that 'iCategoryIndex' and 'tSetIndex' are valid indexes. */
count_t AbstractMultiSetCategoricalTemporalData::GetCategoryCaseCount(unsigned int iCategoryIndex, unsigned int tSetIndex) const {
  return gvSetClusterData.at(tSetIndex)->GetCategoryCaseCount(iCategoryIndex);
}

/** Not implemented - throws prg_error. */
measure_t AbstractMultiSetCategoricalTemporalData::GetMeasure(unsigned int) const {
  throw prg_error("GetMeasure(unsigned int) not implemented.","AbstractMultiSetCategoricalTemporalData");
}

void AbstractMultiSetCategoricalTemporalData::setMeasure(measure_t m, unsigned int tSetIndex) {
  throw prg_error("'setMeasure(measure,unsigned int)' not implemeneted.","AbstractMultiSetCategoricalTemporalData");
}

/** Given ordinal category cases accumulated in cluster data, re-calculates the log likelihood
    ratio to determine which, if any, categories where combined into one category. */
void AbstractMultiSetCategoricalTemporalData::GetOrdinalCombinedCategories(const OrdinalLikelihoodCalculator& Calculator,
                                                                           std::vector<OrdinalCombinedCategory>& vCategoryContainer,
                                                                           unsigned int tSetIndex) const {
  Calculator.CalculateOrdinalCombinedCategories(gvSetClusterData.at(tSetIndex)->gvCasesPerCategory, vCategoryContainer, tSetIndex);
}

/** Initializes cluster data in each data set. */
void AbstractMultiSetCategoricalTemporalData::InitializeData() {
  ptr_vector<CategoricalTemporalData>::iterator   itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++itr) (*itr)->InitializeData();
}

//************** class MultiSetCategoricalTemporalData *************************

/** class constructor */
MultiSetCategoricalTemporalData::MultiSetCategoricalTemporalData(const CategoricalClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway)
                                :AbstractMultiSetCategoricalTemporalData() {
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvSetClusterData.push_back(dynamic_cast<CategoricalTemporalData*>(DataFactory.GetNewTemporalClusterData(DataGateway.GetDataSetInterface(t))));
}

/** Not implemented - throws prg_error. */
void MultiSetCategoricalTemporalData::AddNeighborData(tract_t, const AbstractDataSetGateway&, size_t) {
  throw prg_error("AddNeighbor(tract_t, const AbstractDataSetGateway&, size_t) not implemeneted.","MultiSetCategoricalTemporalData");
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractTemporalClusterData object
    can be casted to 'MultiSetCategoricalTemporalData' object. */
void MultiSetCategoricalTemporalData::Assign(const AbstractTemporalClusterData& rhs) {
  *this = (const MultiSetCategoricalTemporalData&)rhs;
}

/** Return newly cloned MultiSetCategoricalTemporalData object.
    Caller is responsible for deletion of object. */
MultiSetCategoricalTemporalData * MultiSetCategoricalTemporalData::Clone() const {
  return new MultiSetCategoricalTemporalData(*this);
}

/** Reassociates internal data with passed DataSetInterface pointers.
    Not implemented - throws exception */
void MultiSetCategoricalTemporalData::Reassociate(const DataSetInterface& Interface) {
  throw prg_error("Reassociate(const DataSetInterface&) not implemented.","MultiSetCategoricalTemporalData");
}

/** Reassociates internal data with passed DataSetInterface pointers of DataGateway. */
void MultiSetCategoricalTemporalData::Reassociate(const AbstractDataSetGateway& DataGateway) {
  ptr_vector<CategoricalTemporalData>::iterator   itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++itr) (*itr)->Reassociate(DataGateway);
}
//************* class MultiSetCategoricalProspectiveSpatialData ****************

/** class constructor */
MultiSetCategoricalProspectiveSpatialData::MultiSetCategoricalProspectiveSpatialData(const CategoricalClusterDataFactory& DataFactory, const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway)
                                          :AbstractMultiSetCategoricalTemporalData(), geEvaluationAssistDataStatus(Allocated) {
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvSetClusterData.push_back(dynamic_cast<CategoricalTemporalData*>(DataFactory.GetNewProspectiveSpatialClusterData(Data, DataGateway.GetDataSetInterface(t))));
}

/** Adds neighbor data to accumulated cluster data  - caller is responsible for
    ensuring that 'tNeighborIndex' is a valid index. */
void MultiSetCategoricalProspectiveSpatialData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t) {
  assert(geEvaluationAssistDataStatus == Allocated);
  unsigned int                                         i=0;
  ptr_vector<CategoricalTemporalData>::iterator   itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++i, ++itr) (*itr)->AddNeighborData(tNeighborIndex, DataGateway, i);
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractTemporalClusterData object
    can be casted to 'MultiSetCategoricalProspectiveSpatialData' object. */
void MultiSetCategoricalProspectiveSpatialData::Assign(const AbstractTemporalClusterData& rhs) {
  *this = (const MultiSetCategoricalProspectiveSpatialData&)rhs;
}

/** Return newly cloned MultiSetCategoricalProspectiveSpatialData object.
    Caller is responsible for deletion of object. */
MultiSetCategoricalProspectiveSpatialData * MultiSetCategoricalProspectiveSpatialData::Clone() const {
  return new MultiSetCategoricalProspectiveSpatialData(*this);
}

const AbstractLoglikelihoodRatioUnifier & MultiSetCategoricalProspectiveSpatialData::getRatioUnified(AbstractLikelihoodCalculator& Calculator) const {
    assert(geEvaluationAssistDataStatus == Allocated);
    unsigned int                                         i=0, iWindowEnd, iAllocationSize;
    double                                               dMaxLoglikelihoodRatio=0;
    AbstractLoglikelihoodRatioUnifier                  & Unifier = Calculator.GetUnifier();
    ptr_vector<CategoricalTemporalData>::const_iterator   itr=gvSetClusterData.begin();

    Unifier.Reset();
    iAllocationSize = (*gvSetClusterData.begin())->GetAllocationSize();
    for (;itr != gvSetClusterData.end(); ++itr, ++i) {
        for (size_t t=0; t < (*itr)->gvCasesPerCategory.size(); ++t)
            (*itr)->gvCasesPerCategory[t] = (*itr)->gppCategoryCases[t][0];
        Unifier.AdjoinRatio(Calculator, (*itr)->gvCasesPerCategory, i);
    }
    dMaxLoglikelihoodRatio = Unifier.GetLoglikelihoodRatio();

    std::auto_ptr<AbstractLoglikelihoodRatioUnifier> prospectiveUnifier(Unifier.Clone());
    for (iWindowEnd=1; iWindowEnd < iAllocationSize; ++iWindowEnd) {
        prospectiveUnifier->Reset();
        for (i=0, itr=gvSetClusterData.begin(); itr != gvSetClusterData.end(); ++itr, ++i) {
            for (size_t t=0; t < (*itr)->gvCasesPerCategory.size(); ++t)
                (*itr)->gvCasesPerCategory[t] = (*itr)->gppCategoryCases[t][0] - (*itr)->gppCategoryCases[t][iWindowEnd];
            prospectiveUnifier->AdjoinRatio(Calculator, (*itr)->gvCasesPerCategory, i);
        }
        dMaxLoglikelihoodRatio = std::max(dMaxLoglikelihoodRatio, prospectiveUnifier->GetLoglikelihoodRatio());
    }

    // reset calculators unifier if largest LLR came from prospective process.
    if (dMaxLoglikelihoodRatio != Unifier.GetLoglikelihoodRatio()) {
        Unifier = *prospectiveUnifier;
    }
    return Unifier;
}

/** Calculates loglikelihood ratio given current accumulated cluster data in each data set and adds together.*/
double MultiSetCategoricalProspectiveSpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
    return getRatioUnified(Calculator).GetLoglikelihoodRatio();
}

/** Implements interface that calculates maximizing value - for multiple data set, maximizing
    value is the full test statistic/ratio. */
double MultiSetCategoricalProspectiveSpatialData::GetMaximizingValue(AbstractLikelihoodCalculator& Calculator) {
  return CalculateLoglikelihoodRatio(Calculator);
}

/** Deallocates data members that assist with evaluation of temporal data.
    Once this function is called various class member functions become invalid
    and an assertion will fail if called. */
void MultiSetCategoricalProspectiveSpatialData::DeallocateEvaluationAssistClassMembers() {
  unsigned int                                         i=0;
  ptr_vector<CategoricalTemporalData>::iterator   itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++i, ++itr) (*itr)->DeallocateEvaluationAssistClassMembers();
  geEvaluationAssistDataStatus = Deallocated;
}

//************* class MultiSetCategoricalSpaceTimeData *************************

/** class constructor */
MultiSetCategoricalSpaceTimeData::MultiSetCategoricalSpaceTimeData(const CategoricalClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway)
                                 :AbstractMultiSetCategoricalTemporalData(), geEvaluationAssistDataStatus(Allocated) {
  gvSetClusterData.resize(DataGateway.GetNumInterfaces(), 0);
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvSetClusterData[t] = dynamic_cast<CategoricalTemporalData*>(DataFactory.GetNewSpaceTimeClusterData(DataGateway.GetDataSetInterface(t)));
}

/** Add neighbor data to accumulation  - caller is responsible for ensuring that
   'tNeighborIndex' is a valid index. */
void MultiSetCategoricalSpaceTimeData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t) {
  assert(geEvaluationAssistDataStatus == Allocated);
  unsigned int                                         i=0;
  ptr_vector<CategoricalTemporalData>::iterator   itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++i, ++itr) (*itr)->AddNeighborData(tNeighborIndex, DataGateway, i);
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractTemporalClusterData object
    can be casted to 'MultiSetCategoricalSpaceTimeData' object. */
void MultiSetCategoricalSpaceTimeData::Assign(const AbstractTemporalClusterData& rhs) {
  *this = (const MultiSetCategoricalSpaceTimeData&)rhs;
}

/** Return newly cloned MultiSetCategoricalSpaceTimeData object.
    Caller is responsible for deletion of object. */
MultiSetCategoricalSpaceTimeData * MultiSetCategoricalSpaceTimeData::Clone() const {
  return new MultiSetCategoricalSpaceTimeData(*this);
}

/** Deallocates data members that assist with evaluation of temporal data.
    Once this function is called various class member functions become invalid
    and an assertion will fail if called. */
void MultiSetCategoricalSpaceTimeData::DeallocateEvaluationAssistClassMembers() {
  unsigned int                                         i=0;
  ptr_vector<CategoricalTemporalData>::iterator   itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++i, ++itr) (*itr)->DeallocateEvaluationAssistClassMembers();
  geEvaluationAssistDataStatus = Deallocated;
}

