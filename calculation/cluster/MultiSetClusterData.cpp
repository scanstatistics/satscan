//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "MultiSetClusterData.h"
#include "ClusterDataFactory.h"
#include "LoglikelihoodRatioUnifier.h"
#include "SSException.h"

//******************* class MultiSetSpatialData ***********************

/** class constructor */
MultiSetSpatialData::MultiSetSpatialData(const ClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway)
                    :AbstractSpatialClusterData() {
  //Allocate SpatialData object for each data set.
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvSetClusterData.push_back(dynamic_cast<SpatialData*>(DataFactory.GetNewSpatialClusterData(DataGateway.GetDataSetInterface(t))));
}

/** Not implemented - throws prg_error. */
void MultiSetSpatialData::AddMeasureList(CMeasureList*, tract_t, const DataSetInterface&) {
  throw prg_error("AddMeasureList(CMeasureList*, tract_t, const DataSetInterface&) not implemented.","MultiSetSpatialData");
}

/** Adds neighbor data to accumulation  - caller is responsible for ensuring that
    'tNeighborIndex' is a valid index. */
void MultiSetSpatialData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway & DataGateway, size_t) {
  unsigned int                           i=0;
  ptr_vector<SpatialData>::iterator itr=gvSetClusterData.begin();
  for (; itr != gvSetClusterData.end(); ++i, ++itr) (*itr)->AddNeighborData(tNeighborIndex, DataGateway, i);
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractSpatialClusterData object
    can be casted to 'MultiSetSpatialData' object. */
void MultiSetSpatialData::Assign(const AbstractSpatialClusterData& rhs) {
  *this = (const MultiSetSpatialData&)rhs;
}

const AbstractLoglikelihoodRatioUnifier & MultiSetSpatialData::getRatioUnified(AbstractLikelihoodCalculator& Calculator) const {
    unsigned int i=0;
    ptr_vector<SpatialData>::const_iterator itr=gvSetClusterData.begin();
    AbstractLoglikelihoodRatioUnifier    & Unifier = Calculator.GetUnifier();

    Unifier.Reset();
    for (; itr != gvSetClusterData.end(); ++itr, ++i)
        Unifier.AdjoinRatio(Calculator, (*itr)->gtCases, (*itr)->gtMeasure, i);
    return Unifier;
}

boost::dynamic_bitset<> MultiSetSpatialData::getRatioSets(AbstractLikelihoodCalculator& Calculator) const {
    return Calculator.GetUnifier().getUnifiedSets();
}

/** Calculates loglikelihood ratio given current accumulated cluster data in
    each data set and adds together. */
double MultiSetSpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
    auto& unifier = getRatioUnified(Calculator);
    AbstractLikelihoodCalculator::SCANRATEMULTISET_FUNCPTR pRateCheck = Calculator._rate_of_interest_multiset;
    if ((Calculator.*pRateCheck)(unifier, false))
        return unifier.GetLoglikelihoodRatio();
    return 0.0;
}

/** Returns newly cloned MultiSetSpatialData object. Caller responsible for deletion of object. */
MultiSetSpatialData * MultiSetSpatialData::Clone() const {
  return new MultiSetSpatialData(*this);
}

/** Copies class data members that reflect the number of cases per ordinal category,
    which is the data we are interested in for possiblely reporting. */
void MultiSetSpatialData::CopyEssentialClassMembers(const AbstractClusterData& rhs) {
  for (size_t t=0; t < ((const MultiSetSpatialData&)rhs).gvSetClusterData.size(); ++t)
    gvSetClusterData[t]->CopyEssentialClassMembers(*((const MultiSetSpatialData&)rhs).gvSetClusterData[t]);
}

/** Returns number of cases in accumulated respective data sets' cluster data.
    Caller responsible for ensuring that 'tSetIndex' is a valid index. */
count_t MultiSetSpatialData::GetCaseCount(unsigned int tSetIndex) const {
  return gvSetClusterData.at(tSetIndex)->GetCaseCount();
}

void MultiSetSpatialData::setCaseCount(count_t t, unsigned int tSetIndex) {
  return gvSetClusterData.at(tSetIndex)->setCaseCount(t);
}

/** Implements interface that calculates maximizing value - for multiple data sets, maximizing
    value is the full test statistic/ratio. */
double MultiSetSpatialData::GetMaximizingValue(AbstractLikelihoodCalculator& Calculator) {
  return CalculateLoglikelihoodRatio(Calculator);
}

/** Returns expected number of cases in accumulated respective data sets' cluster data.
    Caller responsible for ensuring that 'tSetIndex' is a valid index. */
measure_t MultiSetSpatialData::GetMeasure(unsigned int tSetIndex) const {
  return gvSetClusterData.at(tSetIndex)->GetMeasure();
}

void MultiSetSpatialData::setMeasure(measure_t t, unsigned int tSetIndex) {
  return gvSetClusterData.at(tSetIndex)->setMeasure(t);
}

/** Initializes cluster data in each data set. */
void MultiSetSpatialData::InitializeData() {
  ptr_vector<SpatialData>::iterator itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++itr) (*itr)->InitializeData();
}

//******************* class AbstractMultiSetTemporalData ***********************

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractTemporalClusterData object
    can be casted to 'MultiSetTemporalData' object. */
void AbstractMultiSetTemporalData::Assign(const AbstractTemporalClusterData& rhs) {
  *this = (const AbstractMultiSetTemporalData&)rhs;
}

/** Copies class data members that reflect the number of cases per ordinal category,
    which is the data we are interested in for possiblely reporting. */
void AbstractMultiSetTemporalData::CopyEssentialClassMembers(const AbstractClusterData& rhs) {
  for (size_t t=0; t < ((const AbstractMultiSetTemporalData&)rhs).gvSetClusterData.size(); ++t)
    gvSetClusterData[t]->CopyEssentialClassMembers(*((const AbstractMultiSetTemporalData&)rhs).gvSetClusterData[t]);
}

/** Returns number of cases in accumulated respective data sets' cluster data.
    Caller is responsible for ensuring that 'tSetIndex' is a valid index. */
count_t AbstractMultiSetTemporalData::GetCaseCount(unsigned int tSetIndex) const {
  return gvSetClusterData.at(tSetIndex)->gtCases;
}

void AbstractMultiSetTemporalData::setCaseCount(count_t t, unsigned int tSetIndex) {
  return gvSetClusterData.at(tSetIndex)->setCaseCount(t);
}

const AbstractLoglikelihoodRatioUnifier & AbstractMultiSetTemporalData::getRatioUnified(AbstractLikelihoodCalculator& Calculator) const {
    AbstractLoglikelihoodRatioUnifier & Unifier = Calculator.GetUnifier();

    Unifier.Reset();
    for (size_t t=0; t < gvSetClusterData.size(); ++t) {
        Unifier.AdjoinRatio(Calculator, gvSetClusterData[t]->gtCases, gvSetClusterData[t]->gtMeasure, t);
    }
    return Unifier;
}

/** Returns expected number of cases in accumulated respective data sets' cluster data.
    Caller is responsible for ensuring that 'tSetIndex' is a valid index. */
measure_t AbstractMultiSetTemporalData::GetMeasure(unsigned int tSetIndex) const {
  return gvSetClusterData.at(tSetIndex)->gtMeasure;
}

void AbstractMultiSetTemporalData::setMeasure(measure_t t, unsigned int tSetIndex) {
  return gvSetClusterData.at(tSetIndex)->setMeasure(t);
}

//********************** class MultiSetTemporalData ****************************

/** class constructor */
MultiSetTemporalData::MultiSetTemporalData(const ClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway)
                     :AbstractMultiSetTemporalData() {
  //Allocate TemporalData object for each data set.
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvSetClusterData.push_back(dynamic_cast<TemporalData*>(DataFactory.GetNewTemporalClusterData(DataGateway.GetDataSetInterface(t))));
}

/** Not implemented - throws prg_error. */
void MultiSetTemporalData::AddNeighborData(tract_t, const AbstractDataSetGateway&, size_t) {
  throw prg_error("AddNeighbor(tract_t, const AbstractDataSetGateway&, size_t) not implemeneted.","MultiSetTemporalData");
}

/** Returns newly cloned MultiSetTemporalData object. Caller responsible for
    deletion of object. */
MultiSetTemporalData * MultiSetTemporalData::Clone() const {
  return new MultiSetTemporalData(*this);
}

/** Initializes cluster data in each data set. */
void MultiSetTemporalData::InitializeData() {
  ptr_vector<TemporalData>::iterator itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++itr) (*itr)->InitializeData();
}

/** Reassociates internal data with passed DataSetInterface pointers. */
void MultiSetTemporalData::Reassociate(const DataSetInterface& Interface) {
  ptr_vector<TemporalData>::iterator itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++itr) (*itr)->Reassociate(Interface);
}

/** Reassociates internal data with passed DataSetInterface pointers of DataGateway. */
void MultiSetTemporalData::Reassociate(const AbstractDataSetGateway& DataGateway) {
  ptr_vector<TemporalData>::iterator itr=gvSetClusterData.begin();
  for (; itr != gvSetClusterData.end(); ++itr) (*itr)->Reassociate(DataGateway);
}

//*************** class MultiSetProspectiveSpatialData *************************

/** class constructor */
MultiSetProspectiveSpatialData::MultiSetProspectiveSpatialData(const ClusterDataFactory& DataFactory, const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway)
                               :AbstractMultiSetTemporalData(), geEvaluationAssistDataStatus(Allocated){
  //Allocate ProspectiveSpatialData object for each data set.
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvSetClusterData.push_back(dynamic_cast<TemporalData*>(DataFactory.GetNewProspectiveSpatialClusterData(Data, DataGateway.GetDataSetInterface(t))));
}

/** Adds neighbor data to accumulated cluster data  - caller is responsible for
    ensuring that 'tNeighborIndex' is a valid index. */
void MultiSetProspectiveSpatialData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t) {
  assert(geEvaluationAssistDataStatus == Allocated);
  unsigned int                            i=0;
  ptr_vector<TemporalData>::iterator itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++i, ++itr) (*itr)->AddNeighborData(tNeighborIndex, DataGateway, i);
}

const AbstractLoglikelihoodRatioUnifier & MultiSetProspectiveSpatialData::getRatioUnified(AbstractLikelihoodCalculator& Calculator) const {
    assert(geEvaluationAssistDataStatus == Allocated);
    unsigned int t, iWindowEnd, iAllocationSize;
    double dMaxLoglikelihoodRatio=0;
    AbstractLoglikelihoodRatioUnifier & Unifier = Calculator.GetUnifier();
    ptr_vector<TemporalData>::const_iterator itr=gvSetClusterData.begin();

    Unifier.Reset();
    iAllocationSize = (*gvSetClusterData.begin())->GetAllocationSize();
    for (t=0;itr != gvSetClusterData.end(); ++itr, ++t)
        Unifier.AdjoinRatio(Calculator, (*itr)->gpCases[0], (*itr)->gpMeasure[0], t);
    dMaxLoglikelihoodRatio = Unifier.GetLoglikelihoodRatio();

    std::auto_ptr<AbstractLoglikelihoodRatioUnifier> prospectiveUnifier(Unifier.Clone());
    for (iWindowEnd=1; iWindowEnd < iAllocationSize; ++iWindowEnd) {
        prospectiveUnifier->Reset();
        for (t=0, itr=gvSetClusterData.begin(); itr != gvSetClusterData.end(); ++itr, ++t) {
            (*itr)->gtCases = (*itr)->gpCases[0] - (*itr)->gpCases[iWindowEnd];
            (*itr)->gtMeasure =  (*itr)->gpMeasure[0] - (*itr)->gpMeasure[iWindowEnd];
            prospectiveUnifier->AdjoinRatio(Calculator, (*itr)->gtCases, (*itr)->gtMeasure, t);
        }
        dMaxLoglikelihoodRatio = std::max(dMaxLoglikelihoodRatio, prospectiveUnifier->GetLoglikelihoodRatio());
    }
    // reset calculators unifier if largest LLR came from prospective process.
    if (dMaxLoglikelihoodRatio != Unifier.GetLoglikelihoodRatio()) {
        Unifier = *prospectiveUnifier;
    }
    return Unifier;
}

/** Calculates loglikelihood ratio given current accumulated cluster data in
    each data set and adds together.*/
double MultiSetProspectiveSpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
    auto& unifier = getRatioUnified(Calculator);
    AbstractLikelihoodCalculator::SCANRATEMULTISET_FUNCPTR pRateCheck = Calculator._rate_of_interest_multiset;
    if ((Calculator.*pRateCheck)(unifier, false))
        return unifier.GetLoglikelihoodRatio();
    return 0.0;
}

/** Returns newly cloned MultiSetProspectiveSpatialData object. Caller responsible
    for deletion of object. */
MultiSetProspectiveSpatialData * MultiSetProspectiveSpatialData::Clone() const {
  return new MultiSetProspectiveSpatialData(*this);
}

/** Deallocates data members that assist with evaluation of temporal data.
    Once this function is called various class member functions become invalid
    and an assertion will fail if called. */
void MultiSetProspectiveSpatialData::DeallocateEvaluationAssistClassMembers() {
  unsigned int                            i=0;
  ptr_vector<TemporalData>::iterator itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++i, ++itr) (*itr)->DeallocateEvaluationAssistClassMembers();
  geEvaluationAssistDataStatus = Deallocated;
}

/** Initializes cluster data in each data set. */
void MultiSetProspectiveSpatialData::InitializeData() {
  assert(geEvaluationAssistDataStatus == Allocated);
  ptr_vector<TemporalData>::iterator itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++itr) (*itr)->InitializeData();
}

/** Implements interface that calculates maximizing value - for multiple data sets, maximizing
    value is the full test statistic/ratio. */
double MultiSetProspectiveSpatialData::GetMaximizingValue(AbstractLikelihoodCalculator& Calculator) {
  return CalculateLoglikelihoodRatio(Calculator);
}
//********************** class MultiSetSpaceTimeData ***************************

/** class constructor */
MultiSetSpaceTimeData::MultiSetSpaceTimeData(const ClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway)
                      :AbstractMultiSetTemporalData(), geEvaluationAssistDataStatus(Allocated) {
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvSetClusterData.push_back(dynamic_cast<TemporalData*>(DataFactory.GetNewSpaceTimeClusterData(DataGateway.GetDataSetInterface(t))));
}

/** Add neighbor data to accumulation  - caller is responsible for
    ensuring that 'tNeighborIndex' is a valid index. */
void MultiSetSpaceTimeData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t) {
  assert(geEvaluationAssistDataStatus == Allocated);
  unsigned int                            i=0;
  ptr_vector<TemporalData>::iterator itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++i, ++itr) (*itr)->AddNeighborData(tNeighborIndex, DataGateway, i);
}

/** Returns newly cloned MultiSetSpaceTimeData object. Caller responsible for deletion
    of object. */
MultiSetSpaceTimeData * MultiSetSpaceTimeData::Clone() const {
  return new MultiSetSpaceTimeData(*this);
}

/** Deallocates data members that assist with evaluation of temporal data.
    Once this function is called various class member functions become invalid
    and an assertion will fail if called. */
void MultiSetSpaceTimeData::DeallocateEvaluationAssistClassMembers() {
  unsigned int                            i=0;
  ptr_vector<TemporalData>::iterator itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++i, ++itr)
     (*itr)->DeallocateEvaluationAssistClassMembers();
  geEvaluationAssistDataStatus = Deallocated;
}

/** Initializes cluster data in each data set. */
void MultiSetSpaceTimeData::InitializeData() {
  assert(geEvaluationAssistDataStatus == Allocated);
  ptr_vector<TemporalData>::iterator itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++itr) (*itr)->InitializeData();
}

