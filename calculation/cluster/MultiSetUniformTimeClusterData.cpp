//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "MultiSetUniformTimeClusterData.h"
#include "UniformTimeClusterDataFactory.h"
#include "LoglikelihoodRatioUnifier.h"
#include "cluster.h"
#include "SSException.h"


//********************** class MultiSetUniformTimeTemporalData ****************************

/** class constructor */
MultiSetUniformTimeTemporalData::MultiSetUniformTimeTemporalData(const UniformTimeClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway)
                           :AbstractMultiSetUniformTimeTemporalData() {
  //Allocate TemporalData object for each data set.
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvSetClusterData.push_back(dynamic_cast<UniformTimeTemporalData*>(DataFactory.GetNewTemporalClusterData(DataGateway.GetDataSetInterface(t))));
}

MultiSetUniformTimeTemporalData::MultiSetUniformTimeTemporalData(const MultiSetUniformTimeTemporalData& rhs) : AbstractMultiSetUniformTimeTemporalData() {
    gvSetClusterData.killAll();
    for (size_t t = 0; t < rhs.gvSetClusterData.size(); ++t)
        gvSetClusterData.push_back(rhs.gvSetClusterData[t]->Clone());
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
is responsible for ensuring that passed AbstractTemporalClusterData object
can be casted to 'MultiSetTemporalData' object. */
void MultiSetUniformTimeTemporalData::Assign(const AbstractTemporalClusterData& rhs) {
    gvSetClusterData.killAll();
    MultiSetUniformTimeTemporalData& rhsm = (MultiSetUniformTimeTemporalData&)(rhs);
    for (size_t t = 0; t < rhsm.gvSetClusterData.size(); ++t)
        gvSetClusterData.push_back(rhsm.gvSetClusterData[t]->Clone());
}

/** Not implemented - throws prg_error. */
void MultiSetUniformTimeTemporalData::AddNeighborData(tract_t, const AbstractDataSetGateway&, size_t) {
  throw prg_error("AddNeighbor(tract_t, const AbstractDataSetGateway&, size_t) not implemeneted.","MultiSetUniformTimeTemporalData");
}

/** Returns newly cloned MultiSetTemporalData object. Caller responsible for deletion of object. */
MultiSetUniformTimeTemporalData * MultiSetUniformTimeTemporalData::Clone() const {
  return new MultiSetUniformTimeTemporalData(*this);
}

UniformTimeClusterDataInterface& MultiSetUniformTimeTemporalData::getUniformTimeClusterDataInterface(unsigned int tSetIndex) {
    UniformTimeClusterDataInterface * cluster_data;
    if ((cluster_data = dynamic_cast<UniformTimeClusterDataInterface*>(gvSetClusterData[tSetIndex])) == 0)
        throw prg_error("Unable to dynamically cast to type.", "getUniformTimeClusterDataInterface()");
    return *cluster_data;
}

const AbstractLoglikelihoodRatioUnifier & MultiSetUniformTimeTemporalData::getRatioUnified(AbstractLikelihoodCalculator& Calculator) const {
    AbstractLoglikelihoodRatioUnifier & Unifier = Calculator.GetUnifier();

    Unifier.Reset();
    for (size_t t=0; t < gvSetClusterData.size(); ++t) {
        Unifier.AdjoinRatio(Calculator, gvSetClusterData[t]->gtCases, gvSetClusterData[t]->gtMeasure, gvSetClusterData[t]->gtCasesInPeriod, gvSetClusterData[t]->gtMeasureInPeriod, t);
    }
    return Unifier;
}

/** Copies class data members that reflect the number of cases per ordinal category,
which is the data we are interested in for possiblely reporting. */
void MultiSetUniformTimeTemporalData::CopyEssentialClassMembers(const AbstractClusterData& rhs) {
    const MultiSetUniformTimeTemporalData * rhs_p;
    if ((rhs_p = dynamic_cast<const MultiSetUniformTimeTemporalData*>(&rhs)) == 0)
        throw prg_error("Unable to dynamically cast to type.", "GetClusterDataAsType()");
    for (size_t t = 0; t < rhs_p->gvSetClusterData.size(); ++t) {
        gvSetClusterData[t]->CopyEssentialClassMembers(*(rhs_p->gvSetClusterData[t]));
    }
}

/** Initializes cluster data in each data set. */
void MultiSetUniformTimeTemporalData::InitializeData() {
  ptr_vector<UniformTimeTemporalData>::iterator itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++itr) (*itr)->InitializeData();
}

/** Reassociates internal data with passed DataSetInterface pointers. */
void MultiSetUniformTimeTemporalData::Reassociate(const DataSetInterface& Interface) {
  ptr_vector<UniformTimeTemporalData>::iterator itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++itr) (*itr)->Reassociate(Interface);
}

/** Reassociates internal data with passed DataSetInterface pointers of DataGateway. */
void MultiSetUniformTimeTemporalData::Reassociate(const AbstractDataSetGateway& DataGateway) {
  ptr_vector<UniformTimeTemporalData>::iterator itr=gvSetClusterData.begin();
  for (; itr != gvSetClusterData.end(); ++itr) (*itr)->Reassociate(DataGateway);
}

//********************** class MultiSetUniformTimeSpaceTimeData *********************

/** class constructor */
MultiSetUniformTimeSpaceTimeData::MultiSetUniformTimeSpaceTimeData(const UniformTimeClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway)
                            :AbstractMultiSetUniformTimeTemporalData(), geEvaluationAssistDataStatus(Allocated) {
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvSetClusterData.push_back(dynamic_cast<UniformTimeSpaceTimeData*>(DataFactory.GetNewSpaceTimeClusterData(DataGateway.GetDataSetInterface(t))));
}

MultiSetUniformTimeSpaceTimeData::MultiSetUniformTimeSpaceTimeData(const MultiSetUniformTimeSpaceTimeData& rhs) 
    : AbstractMultiSetUniformTimeTemporalData(), geEvaluationAssistDataStatus(Allocated) {
    gvSetClusterData.killAll();
    for (size_t t = 0; t < rhs.gvSetClusterData.size(); ++t)
        gvSetClusterData.push_back(rhs.gvSetClusterData[t]->Clone());
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
is responsible for ensuring that passed AbstractTemporalClusterData object
can be casted to 'MultiSetTemporalData' object. */
void MultiSetUniformTimeSpaceTimeData::Assign(const AbstractTemporalClusterData& rhs) {
    gvSetClusterData.killAll();
    MultiSetUniformTimeSpaceTimeData& rhsm = (MultiSetUniformTimeSpaceTimeData&)(rhs);
    for (size_t t = 0; t < rhsm.gvSetClusterData.size(); ++t)
        gvSetClusterData.push_back(rhsm.gvSetClusterData[t]->Clone());
}

/** Add neighbor data to accumulation  - caller is responsible for
    ensuring that 'tNeighborIndex' is a valid index. */
void MultiSetUniformTimeSpaceTimeData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t) {
  assert(geEvaluationAssistDataStatus == Allocated);
  unsigned int                            i=0;
  ptr_vector<UniformTimeSpaceTimeData>::iterator itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++i, ++itr) (*itr)->AddNeighborData(tNeighborIndex, DataGateway, i);
}

UniformTimeClusterDataInterface& MultiSetUniformTimeSpaceTimeData::getUniformTimeClusterDataInterface(unsigned int tSetIndex) {
    UniformTimeClusterDataInterface * cluster_data;
    if ((cluster_data = dynamic_cast<UniformTimeClusterDataInterface*>(gvSetClusterData[tSetIndex])) == 0)
        throw prg_error("Unable to dynamically cast to type.", "getUniformTimeClusterDataInterface()");
    return *cluster_data;
}

/** Copies class data members that reflect the number of cases per ordinal category,
which is the data we are interested in for possiblely reporting. */
void MultiSetUniformTimeSpaceTimeData::CopyEssentialClassMembers(const AbstractClusterData& rhs) {
    const MultiSetUniformTimeSpaceTimeData * rhs_p;
    if ((rhs_p = dynamic_cast<const MultiSetUniformTimeSpaceTimeData*>(&rhs)) == 0)
        throw prg_error("Unable to dynamically cast to type.", "GetClusterDataAsType()");
    for (size_t t = 0; t < rhs_p->gvSetClusterData.size(); ++t) {
        gvSetClusterData[t]->CopyEssentialClassMembers(*(rhs_p->gvSetClusterData[t]));
    }
}

/** Returns newly cloned MultiSetSpaceTimeData object. Caller responsible for deletion
    of object. */
MultiSetUniformTimeSpaceTimeData * MultiSetUniformTimeSpaceTimeData::Clone() const {
  return new MultiSetUniformTimeSpaceTimeData(*this);
}

const AbstractLoglikelihoodRatioUnifier & MultiSetUniformTimeSpaceTimeData::getRatioUnified(AbstractLikelihoodCalculator& Calculator) const {
    AbstractLoglikelihoodRatioUnifier & Unifier = Calculator.GetUnifier();

    Unifier.Reset();
    for (size_t t = 0; t < gvSetClusterData.size(); ++t) {
        Unifier.AdjoinRatio(Calculator, gvSetClusterData[t]->gtCases, gvSetClusterData[t]->gtMeasure, gvSetClusterData[t]->gtCasesInPeriod, gvSetClusterData[t]->gtMeasureInPeriod, t);
    }
    return Unifier;
}

/** Deallocates data members that assist with evaluation of temporal data.
    Once this function is called various class member functions become invalid
    and an assertion will fail if called. */
void MultiSetUniformTimeSpaceTimeData::DeallocateEvaluationAssistClassMembers() {
  unsigned int                            i=0;
  ptr_vector<UniformTimeSpaceTimeData>::iterator itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++i, ++itr)
     (*itr)->DeallocateEvaluationAssistClassMembers();
  geEvaluationAssistDataStatus = Deallocated;
}

/** Initializes cluster data in each data set. */
void MultiSetUniformTimeSpaceTimeData::InitializeData() {
  assert(geEvaluationAssistDataStatus == Allocated);
  ptr_vector<UniformTimeSpaceTimeData>::iterator itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++itr) (*itr)->InitializeData();
}

