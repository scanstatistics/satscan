//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SaTScanData.h"
#include "BatchedClusterData.h"
#include "SSException.h"
#include "BatchedLikelihoodCalculation.h"

//************** class BatchedSpatialData ***************************************

/** class constructor */
BatchedSpatialData::BatchedSpatialData() : SpatialData(), gtMeasureAux(0), gtMeasureAux2(0) {}

/** class constructor */
BatchedSpatialData::BatchedSpatialData(const DataSetInterface& Interface)
    :SpatialData(Interface), gtMeasureAux(0), gtMeasureAux2(0), 
    gPositiveBatches(static_cast<unsigned int>(Interface.GetTotalMeasureCount())),
    gBatches(static_cast<unsigned int>(Interface.GetTotalMeasureCount())) {}

/** class constructor */
BatchedSpatialData::BatchedSpatialData(const AbstractDataSetGateway& DataGateway)
    :SpatialData(DataGateway), gtMeasureAux(0), gtMeasureAux2(0), 
    gPositiveBatches(static_cast<unsigned int>(DataGateway.GetDataSetInterface().GetTotalMeasureCount())),
    gBatches(static_cast<unsigned int>(DataGateway.GetDataSetInterface().GetTotalMeasureCount())) {}

/** Adds neighbor data to accumulation  - caller is responsible for ensuring that
    'tNeighborIndex' and 'tSetIndex' are valid indexes. */
void BatchedSpatialData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex) {
    gtCases += DataGateway.GetDataSetInterface(tSetIndex).GetPSCaseArray()[tNeighborIndex];
    gtMeasure += DataGateway.GetDataSetInterface(tSetIndex).GetPSMeasureArray()[tNeighborIndex];
    gtMeasureAux += DataGateway.GetDataSetInterface(tSetIndex).GetPSMeasureAuxArray()[tNeighborIndex];
    gtMeasureAux2 += DataGateway.GetDataSetInterface(tSetIndex).GetPSMeasureAux2Array()[tNeighborIndex];
    gPositiveBatches |= DataGateway.GetDataSetInterface(tSetIndex).getPsPositiveBatchIndexesArray()[tNeighborIndex];
    gBatches |= DataGateway.GetDataSetInterface(tSetIndex).getPsBatchIndexesArray()[tNeighborIndex];
}

/** Not implemeneted - throws prg_error. */
void BatchedSpatialData::AddMeasureList(const DataSetInterface&, CMeasureList*, const CSaTScanData*) {
    throw prg_error("AddMeasureList(const DataSetInterface&, CMeasureList*, const CSaTScanData*) not implemented.", "BatchedSpatialData");
}

/** Assigns cluster data of passed object to 'this' object. */
void BatchedSpatialData::Assign(const AbstractSpatialClusterData& rhs) {
    *this = (const BatchedSpatialData&)rhs;
}

/** Calculates loglikelihood ratio of data if it fits scanning area of interest (high, low, both).
    Returns zero if rate not of interest else returns loglikelihood ratio as calculated by probability model. */
double BatchedSpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
    BatchedLikelihoodCalculator& batchedCalc = (BatchedLikelihoodCalculator&)Calculator;
    if ((Calculator.*Calculator.gpRateOfInterestBatched)(gtCases, batchedCalc.getExpectedForBatches(gBatches))) {
        ProbabilitiesAOI probabilities;
        batchedCalc.CalculateProbabilities(
            probabilities, gtCases, gtMeasure, gtMeasureAux2, gtMeasureAux, gPositiveBatches
        );
        return batchedCalc.getLoglikelihoodRatio(probabilities);
    } return 0.0;
}

/** Returns newly cloned BatchedSpatialData object. */
BatchedSpatialData* BatchedSpatialData::Clone() const {
    return new BatchedSpatialData(*this);
}

/** Copies class data members that are needed for reporting. */
void BatchedSpatialData::CopyEssentialClassMembers(const AbstractClusterData& rhs) {
    gtCases = ((const BatchedSpatialData&)rhs).gtCases;
    gtMeasure = ((const BatchedSpatialData&)rhs).gtMeasure;
    gtMeasureAux = ((const BatchedSpatialData&)rhs).gtMeasureAux;
    gtMeasureAux2 = ((const BatchedSpatialData&)rhs).gtMeasureAux2;
    gPositiveBatches = ((const BatchedSpatialData&)rhs).gPositiveBatches;
    gBatches = ((const BatchedSpatialData&)rhs).gBatches;
}

/** Calculates maximizing value given accumulated cluster data. If data
    is not significant given scanning rate, negation of maximum double returned. */
double BatchedSpatialData::GetMaximizingValue(AbstractLikelihoodCalculator& Calculator) {
    BatchedLikelihoodCalculator& batchedCalc = (BatchedLikelihoodCalculator&)Calculator;
    if ((Calculator.*Calculator.gpRateOfInterestBatched)(gtCases, batchedCalc.getExpectedForBatches(gBatches))) {
        ProbabilitiesAOI probabilities;
        ((BatchedLikelihoodCalculator&)Calculator).CalculateProbabilitiesForSimulation(
            probabilities, gtCases, gtMeasure, gtMeasureAux2, gtMeasureAux, gPositiveBatches
        );
        return batchedCalc.getMaximizingValue(probabilities);
    }
    return -std::numeric_limits<double>::max();
}

//************** class BatchedTemporalData **************************************

/** class constructor */
BatchedTemporalData::BatchedTemporalData(): TemporalData(), 
gtMeasureAux(0), gpMeasureAux(0), gtMeasureAux2(0), gpMeasureAux2(0), gpPositiveBatches(0), gpBatches(0) {}

/** class constructor */
BatchedTemporalData::BatchedTemporalData(const DataSetInterface& Interface): TemporalData(Interface), 
gtMeasureAux(0), gpMeasureAux(Interface.GetPTMeasureAuxArray()), gtMeasureAux2(0), gpMeasureAux2(Interface.GetPTMeasureAux2Array()),
gpPositiveBatches(Interface.getPtPositiveBatchIndexesArray()), gpBatches(Interface.getPtBatchIndexesArray()) {}

/** class constructor */
BatchedTemporalData::BatchedTemporalData(const AbstractDataSetGateway& DataGateway):TemporalData(DataGateway.GetDataSetInterface()), 
gtMeasureAux(0), gpMeasureAux(DataGateway.GetDataSetInterface().GetPTMeasureAuxArray()), 
gtMeasureAux2(0), gpMeasureAux2(DataGateway.GetDataSetInterface().GetPTMeasureAux2Array()),
gpPositiveBatches(DataGateway.GetDataSetInterface().getPtPositiveBatchIndexesArray()),
gpBatches(DataGateway.GetDataSetInterface().getPtBatchIndexesArray()) {}

/** Assigns cluster data of passed object to 'this' object. */
void BatchedTemporalData::Assign(const AbstractTemporalClusterData& rhs) {
    *this = (const BatchedTemporalData&)rhs;
}

/** Returns newly cloned BatchedTemporalData object. */
BatchedTemporalData* BatchedTemporalData::Clone() const {
    return new BatchedTemporalData(*this);
}

/** Copies class data members that are needed for reporting. */
void BatchedTemporalData::CopyEssentialClassMembers(const AbstractClusterData& rhs) {
    gtCases = ((const BatchedTemporalData&)rhs).gtCases;
    gtMeasure = ((const BatchedTemporalData&)rhs).gtMeasure;
    gtMeasureAux = ((const BatchedTemporalData&)rhs).gtMeasureAux;
    gtMeasureAux2 = ((const BatchedTemporalData&)rhs).gtMeasureAux2;
    gPositiveBatches = ((const BatchedTemporalData&)rhs).gPositiveBatches;
    gBatches = ((const BatchedTemporalData&)rhs).gBatches;
}

/** Not implemented - throws exception */
void BatchedTemporalData::Reassociate(const DataSetInterface& Interface) {
    throw prg_error("Reassociate(const DataSetInterface&) not implemented.", "BatchedTemporalData");
}

/** Reassociates internal data with passed DataSetInterface pointers of DataGateway. */
void BatchedTemporalData::Reassociate(const AbstractDataSetGateway& DataGateway) {
    TemporalData::Reassociate(DataGateway.GetDataSetInterface());
    gpMeasureAux = DataGateway.GetDataSetInterface().GetPTMeasureAuxArray();
    gpMeasureAux2 = DataGateway.GetDataSetInterface().GetPTMeasureAux2Array();
    gpPositiveBatches = DataGateway.GetDataSetInterface().getPtPositiveBatchIndexesArray();
}
//**************** class BatchedProspectiveSpatialData **************************

/** class constructor */
BatchedProspectiveSpatialData::BatchedProspectiveSpatialData(const CSaTScanData& Data, const DataSetInterface& Interface)
    :BatchedTemporalData(), geEvaluationAssistDataStatus(Allocated) {
    try {
        Init();
        Setup(Data, Interface);
    } catch (prg_exception& x) {
        x.addTrace("constructor()", "BatchedProspectiveSpatialData");
        throw;
    }
}

/** class constructor */
BatchedProspectiveSpatialData::BatchedProspectiveSpatialData(const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway)
    :BatchedTemporalData(), geEvaluationAssistDataStatus(Allocated) {
    try {
        Init();
        Setup(Data, DataGateway.GetDataSetInterface());
    } catch (prg_exception& x) {
        x.addTrace("constructor()", "BatchedProspectiveSpatialData");
        throw;
    }
}

/** class copy constructor */
BatchedProspectiveSpatialData::BatchedProspectiveSpatialData(const BatchedProspectiveSpatialData& rhs)
    :BatchedTemporalData() {
    try {
        Init();
        *this = rhs;
    } catch (prg_exception& x) {
        x.addTrace("copy constructor()", "BatchedProspectiveSpatialData");
        throw;
    }
}

/** class destructor */
BatchedProspectiveSpatialData::~BatchedProspectiveSpatialData() {
    try {
        delete[] gpCases;
        delete[] gpMeasure;
        delete[] gpMeasureAux;
        delete[] gpMeasureAux2;
        delete[] gpPositiveBatches;
        delete[] gpBatches;
    } catch (...) {}
}

/** Adds neighbor data to accumulation - caller is responsible for ensuring that
    'tNeighborIndex' and 'tSetIndex' are valid indexes. */
void BatchedProspectiveSpatialData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex) {
    assert(geEvaluationAssistDataStatus == Allocated);
    unsigned int i, j;
    count_t** ppCases = DataGateway.GetDataSetInterface(tSetIndex).GetCaseArray();
    measure_t** ppMeasure = DataGateway.GetDataSetInterface(tSetIndex).GetMeasureArray();
    measure_t** ppMeasureAux = DataGateway.GetDataSetInterface(tSetIndex).GetMeasureAuxArray();
    measure_t** ppMeasureAux2 = DataGateway.GetDataSetInterface(tSetIndex).GetMeasureAux2Array();
    BatchIndexes_t** ppPositiveBatch = DataGateway.GetDataSetInterface(tSetIndex).getPositiveBatchIndexesArray();
    BatchIndexes_t** ppBatch = DataGateway.GetDataSetInterface(tSetIndex).getBatchIndexesArray();

    //set cases for entire period added by this neighbor
    gpCases[0] += ppCases[0][tNeighborIndex];
    gpMeasure[0] += ppMeasure[0][tNeighborIndex];
    gpMeasureAux[0] += ppMeasureAux[0][tNeighborIndex];
    gpMeasureAux2[0] += ppMeasureAux2[0][tNeighborIndex];
    gpPositiveBatches[0] |= ppPositiveBatch[0][tNeighborIndex];
    gpBatches[0] |= ppBatch[0][tNeighborIndex];
    for (j = 1, i = giProspectiveStart; i < giNumTimeIntervals; ++j, ++i) {
        gpCases[j] += ppCases[i][tNeighborIndex];
        gpMeasure[j] += ppMeasure[i][tNeighborIndex];
        gpMeasureAux[j] += ppMeasureAux[i][tNeighborIndex];
        gpMeasureAux2[j] += ppMeasureAux2[i][tNeighborIndex];
        gpPositiveBatches[j] |= ppPositiveBatch[i][tNeighborIndex];
        gpBatches[j] |= ppBatch[i][tNeighborIndex];
    }
}

/** Assigns cluster data of passed object to 'this' object. */
void BatchedProspectiveSpatialData::Assign(const AbstractTemporalClusterData& rhs) {
    *this = (const BatchedProspectiveSpatialData&)rhs;
}

/** Calculates loglikelihood ratio of data if it fits scanning area of interest (high, low, both).
    Returns zero if rate not of interest else returns loglikelihood ratio as calculated by probability model. */
double BatchedProspectiveSpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
    assert(geEvaluationAssistDataStatus == Allocated);
    BatchedLikelihoodCalculator& batchedCalc = (BatchedLikelihoodCalculator&)Calculator;
    ProbabilitiesAOI probabilities;

    gtCases = gpCases[0];
    gtMeasure = gpMeasure[0];
    gtMeasureAux = gpMeasureAux[0];
    gtMeasureAux2 = gpMeasureAux2[0];
    gPositiveBatches = gpPositiveBatches[0];
    gBatches = gpBatches[0];
    if ((Calculator.*Calculator.gpRateOfInterestBatched)(gtCases, batchedCalc.getExpectedForBatches(gBatches))) {
        ProbabilitiesAOI probabilities;
        batchedCalc.CalculateProbabilities(
            probabilities, gtCases, gtMeasure, gtMeasureAux2, gtMeasureAux, gPositiveBatches
        );
        return batchedCalc.getLoglikelihoodRatio(probabilities);
    } return 0.0;
}

/** Calculates and returns maximizing value given accumulated cluster data. If data
    is not significant given scanning rate, negation of maximum double returned. */
double BatchedProspectiveSpatialData::GetMaximizingValue(AbstractLikelihoodCalculator& Calculator) {
    assert(geEvaluationAssistDataStatus == Allocated);
    double dMaxValue(-std::numeric_limits<double>::max());
    BatchedLikelihoodCalculator& batchedCalc = (BatchedLikelihoodCalculator&)Calculator;
    ProbabilitiesAOI probabilities;

    gtCases = gpCases[0];
    gtMeasure = gpMeasure[0];
    gtMeasureAux = gpMeasureAux[0];
    gtMeasureAux2 = gpMeasureAux2[0];
    gPositiveBatches = gpPositiveBatches[0];
    gBatches = gpBatches[0];
    if ((Calculator.*Calculator.gpRateOfInterestBatched)(gtCases, batchedCalc.getExpectedForBatches(gBatches))) {
        ProbabilitiesAOI probabilities;
        ((BatchedLikelihoodCalculator&)Calculator).CalculateProbabilitiesForSimulation(
            probabilities, gtCases, gtMeasure, gtMeasureAux2, gtMeasureAux, gPositiveBatches
        );
        return batchedCalc.getMaximizingValue(probabilities);
    }
    return dMaxValue;
}

/** Returns newly cloned BatchedProspectiveSpatialData object. */
BatchedProspectiveSpatialData* BatchedProspectiveSpatialData::Clone() const {
    return new BatchedProspectiveSpatialData(*this);
}

/** Deallocates data members that assist with evaluation of temporal data.
    Once this function is called various class member functions become invalid and an assertion will fail if called. */
void BatchedProspectiveSpatialData::DeallocateEvaluationAssistClassMembers() {
    try {
        delete[] gpCases; gpCases = 0;
        delete[] gpMeasure; gpMeasure = 0;
        delete[] gpMeasureAux; gpMeasureAux = 0;
        delete[] gpMeasureAux2; gpMeasureAux2 = 0;
        delete[] gpPositiveBatches; gpPositiveBatches = 0;
        delete[] gpBatches; gpBatches = 0;
        giAllocationSize = 0;
        geEvaluationAssistDataStatus = Deallocated;
    } catch (...) {}
}

/** Re-initializes data for scanning. */
void BatchedProspectiveSpatialData::InitializeData() {
    assert(geEvaluationAssistDataStatus == Allocated);
    gtCases = 0;
    gtMeasure = 0;
    gtMeasureAux = 0;
    gtMeasureAux2 = 0;
    gPositiveBatches.reset();
    gBatches.reset();
    memset(gpCases, 0, sizeof(count_t) * giAllocationSize);
    memset(gpMeasure, 0, sizeof(measure_t) * giAllocationSize);
    memset(gpMeasureAux, 0, sizeof(measure_t) * giAllocationSize);
    memset(gpMeasureAux2, 0, sizeof(measure_t) * giAllocationSize);
    for (size_t t = 0; t < giAllocationSize; ++t) {
        gpPositiveBatches[t].reset();
        gpBatches[t].reset();
    }
}

/** overloaded assignement operator */
BatchedProspectiveSpatialData& BatchedProspectiveSpatialData::operator=(const BatchedProspectiveSpatialData& rhs) {
    gtCases = rhs.gtCases;
    gtMeasure = rhs.gtMeasure;
    gtMeasureAux = rhs.gtMeasureAux;
    gtMeasureAux2 = rhs.gtMeasureAux2;
    gPositiveBatches = rhs.gPositiveBatches;
    gBatches = rhs.gBatches;

    giAllocationSize = rhs.giAllocationSize;
    giNumTimeIntervals = rhs.giNumTimeIntervals;
    giProspectiveStart = rhs.giProspectiveStart;
    if (rhs.geEvaluationAssistDataStatus == Allocated) {
        if (!gpCases) gpCases = new count_t[rhs.giAllocationSize];
        if (!gpMeasure) gpMeasure = new measure_t[rhs.giAllocationSize];
        if (!gpMeasureAux) gpMeasureAux = new measure_t[rhs.giAllocationSize];
        if (!gpMeasureAux2) gpMeasureAux2 = new measure_t[rhs.giAllocationSize];
        if (!gpPositiveBatches) gpPositiveBatches = new BatchIndexes_t[rhs.giAllocationSize];
        if (!gpBatches) gpBatches = new BatchIndexes_t[rhs.giAllocationSize];
        memcpy(gpCases, rhs.gpCases, giAllocationSize * sizeof(count_t));
        memcpy(gpMeasure, rhs.gpMeasure, giAllocationSize * sizeof(measure_t));
        memcpy(gpMeasureAux, rhs.gpMeasureAux, giAllocationSize * sizeof(measure_t));
        memcpy(gpMeasureAux2, rhs.gpMeasureAux2, giAllocationSize * sizeof(measure_t));
        for (size_t t = 0; t < giAllocationSize; ++t) {
            gpPositiveBatches[t] = rhs.gpPositiveBatches[t];
            gpBatches[t] = rhs.gpBatches[t];
        }
    } else {
        delete[] gpCases; gpCases = 0;
        delete[] gpMeasure; gpMeasure = 0;
        delete[] gpMeasureAux; gpMeasureAux = 0;
        delete[] gpMeasureAux2; gpMeasureAux2 = 0;
        delete[] gpPositiveBatches; gpPositiveBatches = 0;
        delete[] gpBatches; gpBatches = 0;
    }
    geEvaluationAssistDataStatus = rhs.geEvaluationAssistDataStatus;
    return *this;
}

/** internal setup function */
void BatchedProspectiveSpatialData::Setup(const CSaTScanData& Data, const DataSetInterface& Interface) {
    try {
        giAllocationSize = 1 + Data.GetNumTimeIntervals() - Data.GetProspectiveStartIndex();
        // The allocation size used to be more than one when we had the 'adjustment for earlier analyses'
        // with prospective scans. This feature was removed many versions ago, so this cluster object
        // should only ever have an allocation size of one. Just hedge on the possiblity that that feature
        // comes back, all the *ProspectiveSpatialData classes are left being derived from a *TemporalData class.
        assert(giAllocationSize == 1);
        giNumTimeIntervals = Data.GetNumTimeIntervals();
        giProspectiveStart = Data.GetProspectiveStartIndex();
        gpCases = new count_t[giAllocationSize];
        memset(gpCases, 0, sizeof(count_t) * giAllocationSize);
        gpMeasure = new measure_t[giAllocationSize];
        memset(gpMeasure, 0, sizeof(measure_t) * giAllocationSize);
        gpMeasureAux = new measure_t[giAllocationSize];
        memset(gpMeasureAux, 0, sizeof(measure_t) * giAllocationSize);
        gpMeasureAux2 = new measure_t[giAllocationSize];
        memset(gpMeasureAux2, 0, sizeof(measure_t) * giAllocationSize);
        boost::dynamic_bitset<> bset(static_cast<unsigned int>(Interface.GetTotalMeasureCount()));
        gpPositiveBatches = new BatchIndexes_t[giAllocationSize];
        gpBatches = new BatchIndexes_t[giAllocationSize];
        for (size_t t = 0; t < giAllocationSize; ++t) {
            gpPositiveBatches[t] = bset;
            gpBatches[t] = bset;
        }
    } catch (prg_exception& x) {
        delete[] gpCases;
        delete[] gpMeasure;
        delete[] gpMeasureAux;
        delete[] gpMeasureAux2;
        delete[] gpPositiveBatches;
        delete[] gpBatches;
        x.addTrace("Setup(const CSaTScanData&, const DataSetInterface&)", "BatchedProspectiveSpatialData");
        throw;
    }
}

//*************** class BatchedSpaceTimeData ************************************

/** class constructor */
BatchedSpaceTimeData::BatchedSpaceTimeData(const DataSetInterface& Interface)
    :BatchedTemporalData(), geEvaluationAssistDataStatus(Allocated){
    try {
        Setup(Interface);
    } catch (prg_exception& x) {
        x.addTrace("constructor(const DataSetInterface&)", "BatchedSpaceTimeData");
        throw;
    }
}

/** constructor */
BatchedSpaceTimeData::BatchedSpaceTimeData(const AbstractDataSetGateway& DataGateway): 
    BatchedTemporalData(), geEvaluationAssistDataStatus(Allocated), _start_index(0) {
    try {
        Setup(DataGateway.GetDataSetInterface());
    } catch (prg_exception& x) {
        x.addTrace("constructor(const AbstractDataSetGateway&)", "BatchedSpaceTimeData");
        throw;
    }
}

/** class copy constructor */
BatchedSpaceTimeData::BatchedSpaceTimeData(const BatchedSpaceTimeData& rhs): 
    BatchedTemporalData(), _start_index(0) {
    try {
        *this = rhs;
    } catch (prg_exception& x) {
        x.addTrace("constructor(const NormalSpaceTimeData&)", "BatchedSpaceTimeData");
        throw;
    }
}

/** class destructor */
BatchedSpaceTimeData::~BatchedSpaceTimeData() {
    try {
        delete[] gpCases;
        delete[] gpMeasure;
        delete[] gpMeasureAux;
        delete[] gpMeasureAux2;
        delete[] gpPositiveBatches;
        delete[] gpBatches;
    } catch (...) {}
}

/** Adds neighbor data to accumulation. */
void BatchedSpaceTimeData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex) {
    assert(geEvaluationAssistDataStatus == Allocated);
    count_t** ppCases = DataGateway.GetDataSetInterface(tSetIndex).GetCaseArray();
    measure_t** ppMeasure = DataGateway.GetDataSetInterface(tSetIndex).GetMeasureArray();
    measure_t** ppMeasureAux = DataGateway.GetDataSetInterface(tSetIndex).GetMeasureAuxArray();
    measure_t** ppMeasureAux2 = DataGateway.GetDataSetInterface(tSetIndex).GetMeasureAux2Array();
    BatchIndexes_t** ppBatch = DataGateway.GetDataSetInterface(tSetIndex).getBatchIndexesArray();
    BatchIndexes_t** ppPositiveBatch = DataGateway.GetDataSetInterface(tSetIndex).getPositiveBatchIndexesArray();
    for (unsigned int i = _start_index; i < giAllocationSize - 1; ++i) {
        gpCases[i] += ppCases[i][tNeighborIndex];
        gpMeasure[i] += ppMeasure[i][tNeighborIndex];
        gpMeasureAux[i] += ppMeasureAux[i][tNeighborIndex];
        gpMeasureAux2[i] += ppMeasureAux2[i][tNeighborIndex];
        gpBatches[i] |= ppBatch[i][tNeighborIndex];
        gpPositiveBatches[i] |= ppPositiveBatch[i][tNeighborIndex];
    }
}

/** Assigns cluster data of passed object to 'this' object. */
void BatchedSpaceTimeData::Assign(const AbstractTemporalClusterData& rhs) {
    *this = (const BatchedSpaceTimeData&)rhs;
}

/** Returns newly cloned BatchedSpaceTimeData object. */
BatchedSpaceTimeData* BatchedSpaceTimeData::Clone() const {
    return new BatchedSpaceTimeData(*this);
}

/** Deallocates data members that assist with evaluation of temporal data.
    Once this function is called various class member functions become invalid
    and an assertion will fail if called. */
void BatchedSpaceTimeData::DeallocateEvaluationAssistClassMembers() {
    try {
        delete[] gpCases; gpCases = 0;
        delete[] gpMeasure; gpMeasure = 0;
        delete[] gpMeasureAux; gpMeasureAux = 0;
        delete[] gpMeasureAux2; gpMeasureAux2 = 0;
        delete[] gpBatches; gpBatches = 0;
        delete[] gpPositiveBatches; gpPositiveBatches = 0;
        giAllocationSize = 0;
        geEvaluationAssistDataStatus = Deallocated;
    } catch (...) {}
}

/** Re-initialize data for scanning. */
void BatchedSpaceTimeData::InitializeData() {
    assert(geEvaluationAssistDataStatus == Allocated);
    gtCases = 0;
    gtMeasure = 0;
    gtMeasureAux = 0;
    gtMeasureAux2 = 0;
    gBatches.reset();
    gPositiveBatches.reset();
    memset(gpCases, 0, sizeof(count_t) * giAllocationSize);
    memset(gpMeasure, 0, sizeof(measure_t) * giAllocationSize);
    memset(gpMeasureAux, 0, sizeof(measure_t) * giAllocationSize);
    memset(gpMeasureAux2, 0, sizeof(measure_t) * giAllocationSize);
    for (size_t t = 0; t < giAllocationSize; ++t) {
        gpBatches[t].reset();
        gpPositiveBatches[t].reset();
    }
}

/** overloaded assignement operator */
BatchedSpaceTimeData& BatchedSpaceTimeData::operator=(const BatchedSpaceTimeData& rhs) {
    gtCases = rhs.gtCases;
    gtMeasure = rhs.gtMeasure;
    gtMeasureAux = rhs.gtMeasureAux;
    gtMeasureAux2 = rhs.gtMeasureAux2;
    gBatches = rhs.gBatches;
    gPositiveBatches = rhs.gPositiveBatches;
    giAllocationSize = rhs.giAllocationSize;
    if (rhs.geEvaluationAssistDataStatus == Allocated) {
        if (!gpCases) gpCases = new count_t[rhs.giAllocationSize];
        if (!gpMeasure) gpMeasure = new measure_t[rhs.giAllocationSize];
        if (!gpMeasureAux) gpMeasureAux = new measure_t[rhs.giAllocationSize];
        if (!gpMeasureAux2) gpMeasureAux2 = new measure_t[rhs.giAllocationSize];
        if (!gpBatches) gpBatches = new BatchIndexes_t[giAllocationSize];
        if (!gpPositiveBatches) gpPositiveBatches = new BatchIndexes_t[giAllocationSize];
        memcpy(gpCases, rhs.gpCases, giAllocationSize * sizeof(count_t));
        memcpy(gpMeasure, rhs.gpMeasure, giAllocationSize * sizeof(measure_t));
        memcpy(gpMeasureAux, rhs.gpMeasureAux, giAllocationSize * sizeof(measure_t));
        memcpy(gpMeasureAux2, rhs.gpMeasureAux2, giAllocationSize * sizeof(measure_t));
        for (size_t t = 0; t < giAllocationSize; ++t) {
            gpBatches[t] = rhs.gpBatches[t];
            gpPositiveBatches[t] = rhs.gpPositiveBatches[t];
        }
    } else {
        delete[] gpCases; gpCases = 0;
        delete[] gpMeasure; gpMeasure = 0;
        delete[] gpMeasureAux; gpMeasureAux = 0;
        delete[] gpMeasureAux2; gpMeasureAux2 = 0;
        delete[] gpBatches; gpBatches = 0;
        delete[] gpPositiveBatches; gpPositiveBatches = 0;
    }
    geEvaluationAssistDataStatus = rhs.geEvaluationAssistDataStatus;
    return *this;
}

/** internal setup function */
void BatchedSpaceTimeData::Setup(const DataSetInterface& Interface) {
    try {
        //Note that giAllocationSize is number of time intervals plus one - this permits
        //us to evaluate last time intervals data with same code as other time intervals
        //in CTimeIntervals object.
        giAllocationSize = Interface.GetNumTimeIntervals() + 1;
        gpCases = new count_t[giAllocationSize];
        memset(gpCases, 0, sizeof(count_t) * giAllocationSize);
        gpMeasure = new measure_t[giAllocationSize];
        memset(gpMeasure, 0, sizeof(measure_t) * giAllocationSize);
        gpMeasureAux = new measure_t[giAllocationSize];
        memset(gpMeasureAux, 0, sizeof(measure_t) * giAllocationSize);
        gpMeasureAux2 = new measure_t[giAllocationSize];
        memset(gpMeasureAux2, 0, sizeof(measure_t) * giAllocationSize);
        boost::dynamic_bitset<> bset(static_cast<unsigned int>(Interface.GetTotalMeasureCount()));
        gpBatches = new BatchIndexes_t[giAllocationSize];
        gpPositiveBatches = new BatchIndexes_t[giAllocationSize];
        for (size_t t = 0; t < giAllocationSize; ++t) {
            gpBatches[t] = bset;
            gpPositiveBatches[t] = bset;
        }
        _start_index = Interface.getDataStartIndex();
    } catch (prg_exception& x) {
        delete[] gpCases;
        delete[] gpMeasure;
        delete[] gpMeasureAux;
        delete[] gpMeasureAux2;
        delete[] gpBatches;
        delete[] gpPositiveBatches;
        x.addTrace("Setup(const DataSetInterface&)", "NormalSpaceTimeData");
        throw;
    }
}
