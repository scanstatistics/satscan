//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "MultiSetBatchedClusterData.h"
#include "BatchedClusterDataFactory.h"
#include "LoglikelihoodRatioUnifier.h"
#include "SSException.h"

//***************** class MultiSetBatchedSpatialData ****************************

/** class constructor */
MultiSetBatchedSpatialData::MultiSetBatchedSpatialData(const BatchedClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway)
    :AbstractSpatialClusterData(), AbstractBatchedClusterData() {
    for (size_t t = 0; t < DataGateway.GetNumInterfaces(); ++t)
        gvSetClusterData.push_back(dynamic_cast<BatchedSpatialData*>(DataFactory.GetNewSpatialClusterData(DataGateway.GetDataSetInterface(t))));
}

/** Adds neighbor data to accumulation  - caller is responsible for ensuring that 'tNeighborIndex' is a valid index. */
void MultiSetBatchedSpatialData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t) {
    unsigned int i = 0;
    for (auto set : gvSetClusterData) {
        set->AddNeighborData(tNeighborIndex, DataGateway, i);
        ++i;
    }
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractSpatialClusterData object
    can be casted to 'MultiSetCategoricalSpatialData' object. */
void MultiSetBatchedSpatialData::Assign(const AbstractSpatialClusterData& rhs) {
    *this = (const MultiSetBatchedSpatialData&)rhs;
}

const AbstractLoglikelihoodRatioUnifier& MultiSetBatchedSpatialData::getRatioUnified(AbstractLikelihoodCalculator& Calculator) const {
    AbstractLoglikelihoodRatioUnifier& Unifier = Calculator.GetUnifier();
    Unifier.Reset();
    size_t t = 0;
    for (auto dset : gvSetClusterData) {
        Unifier.AdjoinRatio(
            Calculator, dset->gtCases, dset->gtMeasure, dset->gtMeasureAux, 
            dset->gtMeasureAux2, dset->gPositiveBatches, dset->gBatches, t
        );
        ++t;
    }
    return Unifier;
}

boost::dynamic_bitset<> MultiSetBatchedSpatialData::getRatioSets(AbstractLikelihoodCalculator& Calculator) const {
    return Calculator.GetUnifier().getUnifiedSets();
}

/** Calculates accumulated loglikelihood ratio. */
double MultiSetBatchedSpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
    auto& unifier = getRatioUnified(Calculator);
    AbstractLikelihoodCalculator::SCANRATEMULTISET_FUNCPTR pRateCheck = Calculator._rate_of_interest_multiset;
    if ((Calculator.*pRateCheck)(unifier, false))
        return unifier.GetLoglikelihoodRatio();
    return 0.0;
}

/** Returns newly cloned MultiSetCategoricalSpatialData object. */
MultiSetBatchedSpatialData* MultiSetBatchedSpatialData::Clone() const {
    return new MultiSetBatchedSpatialData(*this);
}

/** Copies class data members that we are interested in for possiblely reporting. */
void MultiSetBatchedSpatialData::CopyEssentialClassMembers(const AbstractClusterData& rhs) {
    for (size_t t = 0; t < ((const MultiSetBatchedSpatialData&)rhs).gvSetClusterData.size(); ++t)
        gvSetClusterData[t]->CopyEssentialClassMembers(*((const MultiSetBatchedSpatialData&)rhs).gvSetClusterData[t]);
}

/** Returns the number of positive batches in data set 'tSetIndex'. */
count_t MultiSetBatchedSpatialData::GetCaseCount(unsigned int tSetIndex) const {
    return gvSetClusterData.at(tSetIndex)->GetCaseCount();
}

/** Sets the number of positive batches in data set 'tSetIndex'. */
void MultiSetBatchedSpatialData::setCaseCount(count_t t, unsigned int tSetIndex) {
    gvSetClusterData.at(tSetIndex)->setCaseCount(t);
}

/** Calculates the maximizing value for multiple data set in the context of a simulation. */
double MultiSetBatchedSpatialData::GetMaximizingValue(AbstractLikelihoodCalculator& Calculator) {
    AbstractLikelihoodCalculator::SCANRATEMULTISET_FUNCPTR pRateCheck = Calculator._rate_of_interest_multiset;
    AbstractLoglikelihoodRatioUnifier& Unifier = Calculator.GetUnifier();
    Unifier.Reset();
    size_t t = 0;
    for (auto& set : gvSetClusterData) { // Adjoin this data set in the context of a simulation
        Unifier.AdjoinRatioSimulation(
            Calculator, set->gtCases, set->gtMeasure, set->gtMeasureAux, 
            set->gtMeasureAux2, set->gPositiveBatches, set->gBatches, t
        );
        ++t;
    }
    if ((Calculator.*pRateCheck)(Unifier, false))
        return Unifier.GetLoglikelihoodRatio(); // maximizing value is the log-likelihood ratio here
    return 0.0;
}

/** Returns the number of batches in data set 'tSetIndex'. */
measure_t MultiSetBatchedSpatialData::GetMeasure(unsigned int tSetIndex) const {
    return gvSetClusterData[tSetIndex]->GetMeasure();
}

/** Sets the number of batches in data set 'tSetIndex'. */
void MultiSetBatchedSpatialData::setMeasure(measure_t m, unsigned int tSetIndex) {
    gvSetClusterData.at(tSetIndex)->setMeasure(m);
}

/** Returns the sum of the negative batch sizes in data set 'tSetIndex'. */
measure_t MultiSetBatchedSpatialData::GetMeasureAux(unsigned int tSetIndex) const {
    return gvSetClusterData[tSetIndex]->GetMeasureAux();
}

/** Returns the sum of the postive batch sizes in data set 'tSetIndex'. */
measure_t MultiSetBatchedSpatialData::GetMeasureAux2(unsigned int tSetIndex) const {
    return gvSetClusterData[tSetIndex]->GetMeasureAux2();
}

/** Returns a bitset which indicates which batches are positive in data set. */
const BatchIndexes_t& MultiSetBatchedSpatialData::GetPositiveBatches(unsigned int tSetIndex) const {
    return gvSetClusterData[tSetIndex]->GetPositiveBatches();
}

/** Returns a bitset which indicates which batches are in data set. */
const BatchIndexes_t& MultiSetBatchedSpatialData::GetBatches(unsigned int tSetIndex) const {
    return gvSetClusterData[tSetIndex]->GetBatches();
}

/** Initializes cluster data in each data set. */
void MultiSetBatchedSpatialData::InitializeData() {
    for (auto& set: gvSetClusterData) set->InitializeData();
}

//******************* class AbstractMultiSetBatchedTemporalData *****************

/** Assigns cluster data of passed object to 'this' object. 
    Caller of function is responsible for ensuring that passed AbstractTemporalClusterData
    object can be casted to 'AbstractMultiSetBatchedTemporalData' object. */
void AbstractMultiSetBatchedTemporalData::Assign(const AbstractTemporalClusterData& rhs) {
    *this = (const AbstractMultiSetBatchedTemporalData&)rhs;
}

/** Copies class data members that we are interested in for possiblely reporting. 
    Caller of function is responsible for ensuring that passed AbstractTemporalClusterData
    object can be casted to 'AbstractMultiSetBatchedTemporalData' object.
*/
void AbstractMultiSetBatchedTemporalData::CopyEssentialClassMembers(const AbstractClusterData& rhs) {
    for (size_t t = 0; t < ((const AbstractMultiSetBatchedTemporalData&)rhs).gvSetClusterData.size(); ++t)
        gvSetClusterData[t]->CopyEssentialClassMembers(*((const AbstractMultiSetBatchedTemporalData&)rhs).gvSetClusterData[t]);
}

/** Returns the number of positive batches in data set 'tSetIndex'. */
count_t AbstractMultiSetBatchedTemporalData::GetCaseCount(unsigned int tSetIndex) const {
    return gvSetClusterData.at(tSetIndex)->gtCases;
}

/** Sets the number of positive batches in data set 'tSetIndex'. */
void AbstractMultiSetBatchedTemporalData::setCaseCount(count_t t, unsigned int tSetIndex) {
    gvSetClusterData.at(tSetIndex)->setCaseCount(t);
}

/** Combines the data in sets into the unifier. */
const AbstractLoglikelihoodRatioUnifier& AbstractMultiSetBatchedTemporalData::getRatioUnified(AbstractLikelihoodCalculator& Calculator) const {
    AbstractLoglikelihoodRatioUnifier& Unifier = Calculator.GetUnifier();
    Unifier.Reset();
    size_t t = 0;
    for (auto set : gvSetClusterData) {
        Unifier.AdjoinRatio(
            Calculator, set->gtCases, set->gtMeasure, set->gtMeasureAux, 
            set->gtMeasureAux2, set->gPositiveBatches, set->gBatches, t);
        ++t;
    }
    return Unifier;
}

/** Returns the number of batches in data set 'tSetIndex'. */
measure_t AbstractMultiSetBatchedTemporalData::GetMeasure(unsigned int tSetIndex) const {
    return gvSetClusterData.at(tSetIndex)->gtMeasure;
}

/** Sets the number of batches in data set 'tSetIndex'. */
void AbstractMultiSetBatchedTemporalData::setMeasure(measure_t m, unsigned int tSetIndex) {
    gvSetClusterData.at(tSetIndex)->setMeasure(m);
}

/** Returns the sum of the negative batch sizes in data set 'tSetIndex'. */
measure_t AbstractMultiSetBatchedTemporalData::GetMeasureAux(unsigned int tSetIndex) const {
    return gvSetClusterData.at(tSetIndex)->gtMeasureAux;
}

/** Returns the sum of the positive batch sizes in data set 'tSetIndex'. */
measure_t AbstractMultiSetBatchedTemporalData::GetMeasureAux2(unsigned int tSetIndex) const {
    return gvSetClusterData.at(tSetIndex)->gtMeasureAux2;
}

/** Returns a bitset which indicates which positive batches in data set 'tSetIndex'. */
const BatchIndexes_t& AbstractMultiSetBatchedTemporalData::GetPositiveBatches(unsigned int tSetIndex) const {
    return gvSetClusterData.at(tSetIndex)->gPositiveBatches;
}

/** Returns the batches in cluster data of specific data stream. */
const BatchIndexes_t& AbstractMultiSetBatchedTemporalData::GetBatches(unsigned int tSetIndex) const {
    return gvSetClusterData.at(tSetIndex)->GetBatches();
}

//********************** class MultiSetBatchedTemporalData ****************************

/** class constructor */
MultiSetBatchedTemporalData::MultiSetBatchedTemporalData(const BatchedClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway):AbstractMultiSetBatchedTemporalData() {
    for (size_t t = 0; t < DataGateway.GetNumInterfaces(); ++t)
        gvSetClusterData.push_back(
            dynamic_cast<BatchedTemporalData*>(DataFactory.GetNewTemporalClusterData(DataGateway.GetDataSetInterface(t)))
        );
}

/** Not implemented, throws prg_error. */
void MultiSetBatchedTemporalData::AddNeighborData(tract_t, const AbstractDataSetGateway&, size_t) {
    throw prg_error("AddNeighbor(tract_t, const AbstractDataSetGateway&, size_t) not implemeneted.", "MultiSetBatchedTemporalData");
}

/** Returns newly cloned MultiSetTemporalData object. */
MultiSetBatchedTemporalData* MultiSetBatchedTemporalData::Clone() const {
    return new MultiSetBatchedTemporalData(*this);
}

/** Initializes cluster data in each data set. */
void MultiSetBatchedTemporalData::InitializeData() {
    for (auto set: gvSetClusterData) set->InitializeData();
}

/** Reassociates internal data with passed DataSetInterface pointers. */
void MultiSetBatchedTemporalData::Reassociate(const DataSetInterface& Interface) {
    for (auto set : gvSetClusterData) set->Reassociate(Interface);
}

/** Reassociates internal data with passed DataSetInterface pointers of DataGateway. */
void MultiSetBatchedTemporalData::Reassociate(const AbstractDataSetGateway& DataGateway) {
    for (auto set: gvSetClusterData) set->Reassociate(DataGateway);
}

//*************** class MultiSetBatchedProspectiveSpatialData *******************

/** class constructor */
MultiSetBatchedProspectiveSpatialData::MultiSetBatchedProspectiveSpatialData(const BatchedClusterDataFactory& DataFactory, const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway)
    :AbstractMultiSetBatchedTemporalData(), geEvaluationAssistDataStatus(Allocated) {
    for (size_t t = 0; t < DataGateway.GetNumInterfaces(); ++t)
        gvSetClusterData.push_back(dynamic_cast<BatchedTemporalData*>(DataFactory.GetNewProspectiveSpatialClusterData(Data, DataGateway.GetDataSetInterface(t))));
}

/** Adds neighbor data to accumulated cluster data  - caller is responsible for
    ensuring that 'tNeighborIndex' is a valid index. */
void MultiSetBatchedProspectiveSpatialData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t) {
    assert(geEvaluationAssistDataStatus == Allocated);
    unsigned int i = 0;
    for (auto set : gvSetClusterData) {
        set->AddNeighborData(tNeighborIndex, DataGateway, i);
        ++i;
    }
}

/** Combines the data in sets into the unifier. */
const AbstractLoglikelihoodRatioUnifier& MultiSetBatchedProspectiveSpatialData::getRatioUnified(AbstractLikelihoodCalculator& Calculator) const {
    assert(geEvaluationAssistDataStatus == Allocated);
    unsigned int t=0, iAllocationSize((*gvSetClusterData.begin())->GetAllocationSize());
    double dMaxLoglikelihoodRatio = 0;
    AbstractLoglikelihoodRatioUnifier& Unifier = Calculator.GetUnifier();

    Unifier.Reset();
    for (auto set : gvSetClusterData) {
        Unifier.AdjoinRatio(
            Calculator, set->gpCases[0], set->gpMeasure[0], set->gpMeasureAux[0], 
            set->gpMeasureAux2[0], set->gpPositiveBatches[0], set->gpBatches[0], t
        );
        ++t;
    }
    dMaxLoglikelihoodRatio = Unifier.GetLoglikelihoodRatio();

    std::auto_ptr<AbstractLoglikelihoodRatioUnifier> prospectiveUnifier(Unifier.Clone());
    for (unsigned int iWindowEnd = 1; iWindowEnd < iAllocationSize; ++iWindowEnd) {
        prospectiveUnifier->Reset();
        t = 0;
        for (auto set : gvSetClusterData) {
            set->gtCases = set->gpCases[0] - set->gpCases[iWindowEnd];
            set->gtMeasure = set->gpMeasure[0] - set->gpMeasure[iWindowEnd];
            set->gtMeasureAux = set->gpMeasureAux[0] - set->gpMeasureAux[iWindowEnd];
            prospectiveUnifier->AdjoinRatio(
                Calculator, set->gtCases, set->gtMeasure, set->gtMeasureAux, 
                set->gtMeasureAux2, set->gPositiveBatches, set->gBatches, t
            );
            ++t;
        }
        dMaxLoglikelihoodRatio = std::max(dMaxLoglikelihoodRatio, prospectiveUnifier->GetLoglikelihoodRatio());
    }
    // reset calculators unifier if largest LLR came from prospective process.
    if (dMaxLoglikelihoodRatio != Unifier.GetLoglikelihoodRatio()) {
        Unifier = *prospectiveUnifier;
    }
    return Unifier;
}

/** Calculates accumulated loglikelihood ratio. */
double MultiSetBatchedProspectiveSpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
    auto& unifier = getRatioUnified(Calculator);
    AbstractLikelihoodCalculator::SCANRATEMULTISET_FUNCPTR pRateCheck = Calculator._rate_of_interest_multiset;
    if ((Calculator.*pRateCheck)(unifier, false))
        return unifier.GetLoglikelihoodRatio();
    return 0.0;
}

/** Returns newly cloned MultiSetProspectiveSpatialData object. */
MultiSetBatchedProspectiveSpatialData* MultiSetBatchedProspectiveSpatialData::Clone() const {
    return new MultiSetBatchedProspectiveSpatialData(*this);
}

/** Deallocates data members that assist with evaluation of temporal data.
    Once this function is called various class member functions become invalid and an assertion will fail if called. */
void MultiSetBatchedProspectiveSpatialData::DeallocateEvaluationAssistClassMembers() {
    for (auto set : gvSetClusterData) set->DeallocateEvaluationAssistClassMembers();
    geEvaluationAssistDataStatus = Deallocated;
}

/** Calculates the maximizing value by combining set data into unifier. */
double MultiSetBatchedProspectiveSpatialData::GetMaximizingValue(AbstractLikelihoodCalculator& Calculator) {
    assert(geEvaluationAssistDataStatus == Allocated);
    unsigned int t = 0, iAllocationSize((*gvSetClusterData.begin())->GetAllocationSize());
    double dMaxLoglikelihoodRatio = 0;
    AbstractLoglikelihoodRatioUnifier& Unifier = Calculator.GetUnifier();
    AbstractLikelihoodCalculator::SCANRATEMULTISET_FUNCPTR pRateCheck = Calculator._rate_of_interest_multiset;

    Unifier.Reset();
    for (auto set : gvSetClusterData) {
        Unifier.AdjoinRatioSimulation(
            Calculator, set->gpCases[0], set->gpMeasure[0], set->gpMeasureAux[0], 
            set->gpMeasureAux2[0], set->gpPositiveBatches[0], set->gpBatches[0], t
        );
        ++t;
    }
    dMaxLoglikelihoodRatio = Unifier.GetLoglikelihoodRatio();

    std::auto_ptr<AbstractLoglikelihoodRatioUnifier> prospectiveUnifier(Unifier.Clone());
    for (unsigned int iWindowEnd = 1; iWindowEnd < iAllocationSize; ++iWindowEnd) {
        prospectiveUnifier->Reset();
        t = 0;
        for (auto set : gvSetClusterData) {
            set->gtCases = set->gpCases[0] - set->gpCases[iWindowEnd];
            set->gtMeasure = set->gpMeasure[0] - set->gpMeasure[iWindowEnd];
            set->gtMeasureAux = set->gpMeasureAux[0] - set->gpMeasureAux[iWindowEnd];
            prospectiveUnifier->AdjoinRatioSimulation(
                Calculator, set->gtCases, set->gtMeasure, set->gtMeasureAux, 
                set->gtMeasureAux2, set->gPositiveBatches, set->gBatches, t
            );
            ++t;
        }
        dMaxLoglikelihoodRatio = std::max(dMaxLoglikelihoodRatio, prospectiveUnifier->GetLoglikelihoodRatio());
    }
    // reset calculators unifier if largest LLR came from prospective process.
    if (dMaxLoglikelihoodRatio != Unifier.GetLoglikelihoodRatio()) {
        Unifier = *prospectiveUnifier;
    }
    if ((Calculator.*pRateCheck)(Unifier, false))
        return Unifier.GetLoglikelihoodRatio(); // maximizing value is the log-likelihood ratio here
    return 0.0;
}

/** Initializes cluster data in each data set. */
void MultiSetBatchedProspectiveSpatialData::InitializeData() {
    assert(geEvaluationAssistDataStatus == Allocated);
    for (auto set : gvSetClusterData) set->InitializeData();
}

//********************** class MultiSetBatchedSpaceTimeData *********************

/** class constructor */
MultiSetBatchedSpaceTimeData::MultiSetBatchedSpaceTimeData(const BatchedClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway)
    :AbstractMultiSetBatchedTemporalData(), geEvaluationAssistDataStatus(Allocated) {
    for (size_t t = 0; t < DataGateway.GetNumInterfaces(); ++t)
        gvSetClusterData.push_back(
            dynamic_cast<BatchedTemporalData*>(DataFactory.GetNewSpaceTimeClusterData(DataGateway.GetDataSetInterface(t)))
        );
}

/** Add neighbor data to accumulation  - caller is responsible for
    ensuring that 'tNeighborIndex' is a valid index. */
void MultiSetBatchedSpaceTimeData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t) {
    assert(geEvaluationAssistDataStatus == Allocated);
    unsigned int i = 0;
    for (auto set : gvSetClusterData) {
        set->AddNeighborData(tNeighborIndex, DataGateway, i);
        ++i;
    }
}

/** Returns newly cloned MultiSetBatchedSpaceTimeData object. */
MultiSetBatchedSpaceTimeData* MultiSetBatchedSpaceTimeData::Clone() const {
    return new MultiSetBatchedSpaceTimeData(*this);
}

/** Deallocates data members that assist with evaluation of temporal data.
    Once this function is called various class member functions become invalid
    and an assertion will fail if called. */
void MultiSetBatchedSpaceTimeData::DeallocateEvaluationAssistClassMembers() {
    for (auto set : gvSetClusterData) set->DeallocateEvaluationAssistClassMembers();
    geEvaluationAssistDataStatus = Deallocated;
}

/** Initializes cluster data in each data set. */
void MultiSetBatchedSpaceTimeData::InitializeData() {
    assert(geEvaluationAssistDataStatus == Allocated);
    for (auto set: gvSetClusterData) set->InitializeData();
}
