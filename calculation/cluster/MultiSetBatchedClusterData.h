//******************************************************************************
#ifndef __MultiSetBatchedClusterData_H
#define __MultiSetBatchedClusterData_H
//******************************************************************************
#include "BatchedClusterData.h"
#include "ptr_vector.h"

class AbstractLikelihoodCalculator;
class BatchedClusterDataFactory; /* forward class declaration. */

/** Class representing accumulated data of a spatial clustering in multiple data sets for batched model. */
class MultiSetBatchedSpatialData : public AbstractSpatialClusterData, public AbstractBatchedClusterData {
protected:
    ptr_vector<BatchedSpatialData> gvSetClusterData;

public:
    MultiSetBatchedSpatialData(const BatchedClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway);
    virtual ~MultiSetBatchedSpatialData() {}
    virtual MultiSetBatchedSpatialData* Clone() const;

    virtual measure_t   GetMeasureAux2(unsigned int tSetIndex = 0) const;
    virtual const BatchIndexes_t& GetPositiveBatches(unsigned int tSetIndex = 0) const;

    virtual void        AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex = 0);
    virtual void        Assign(const AbstractSpatialClusterData& rhs);
    virtual double      CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator);
    virtual void        CopyEssentialClassMembers(const AbstractClusterData& rhs);
    virtual count_t     GetCaseCount(unsigned int tSetIndex = 0) const;
    virtual void        setCaseCount(count_t t, unsigned int tSetIndex = 0);
    virtual double      GetMaximizingValue(AbstractLikelihoodCalculator& Calculator);
    virtual measure_t   GetMeasure(unsigned int tSetIndex = 0) const;
    virtual void        setMeasure(measure_t m, unsigned int tSetIndex = 0);
    virtual measure_t   GetMeasureAux(unsigned int tSetIndex = 0) const;
    virtual void        InitializeData();
    virtual size_t      getNumSets() const { return gvSetClusterData.size(); }
    virtual const AbstractLoglikelihoodRatioUnifier& getRatioUnified(AbstractLikelihoodCalculator& Calculator) const;
    virtual boost::dynamic_bitset<> getRatioSets(AbstractLikelihoodCalculator& Calculator) const;
};

/** Abstract class representing accumulated data of temporal clustering in multiple data sets for batched model. */
class AbstractMultiSetBatchedTemporalData : public AbstractTemporalClusterData, public AbstractBatchedClusterData {
public:
    AbstractMultiSetBatchedTemporalData() : AbstractTemporalClusterData() {}
    virtual ~AbstractMultiSetBatchedTemporalData() {}

    ptr_vector<BatchedTemporalData> gvSetClusterData;

    virtual measure_t        GetMeasureAux2(unsigned int tSetIndex = 0) const;
    virtual const BatchIndexes_t& GetPositiveBatches(unsigned int tSetIndex = 0) const;

    virtual void             Assign(const AbstractTemporalClusterData& rhs);
    virtual void             CopyEssentialClassMembers(const AbstractClusterData& rhs);
    virtual count_t          GetCaseCount(unsigned int tSetIndex = 0) const;
    virtual void             setCaseCount(count_t t, unsigned int tSetIndex = 0);
    virtual measure_t        GetMeasure(unsigned int tSetIndex = 0) const;
    virtual void             setMeasure(measure_t m, unsigned int tSetIndex = 0);
    virtual measure_t        GetMeasureAux(unsigned int tSetIndex = 0) const;
    virtual size_t           getNumSets() const { return gvSetClusterData.size(); }
    virtual const AbstractLoglikelihoodRatioUnifier& getRatioUnified(AbstractLikelihoodCalculator& Calculator) const;
};

/** Class representing accumulated data of temporal clustering in multiple data sets for batched model. */
class MultiSetBatchedTemporalData : public AbstractMultiSetBatchedTemporalData {
public:
    MultiSetBatchedTemporalData(const BatchedClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway);
    virtual ~MultiSetBatchedTemporalData() {}

    virtual void             AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex = 0);
    virtual MultiSetBatchedTemporalData* Clone() const;
    virtual void             InitializeData();
    virtual void             Reassociate(const DataSetInterface& Interface);
    virtual void             Reassociate(const AbstractDataSetGateway& DataGateway);
};

/** Class representing accumulated data of prospective spatial clustering in multiple data sets for batched model. */
class MultiSetBatchedProspectiveSpatialData : public AbstractMultiSetBatchedTemporalData, public AbstractProspectiveSpatialClusterData {
protected:
    EvaluationAssistDataStatus geEvaluationAssistDataStatus;

public:
    MultiSetBatchedProspectiveSpatialData(const BatchedClusterDataFactory& DataFactory, const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway);
    virtual ~MultiSetBatchedProspectiveSpatialData() {}
    virtual MultiSetBatchedProspectiveSpatialData* Clone() const;

    virtual void             AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex = 0);
    virtual double           CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator);
    virtual void             DeallocateEvaluationAssistClassMembers();
    virtual double           GetMaximizingValue(AbstractLikelihoodCalculator& Calculator);
    virtual void             InitializeData();
    virtual void             Reassociate(const DataSetInterface& Interface) { /*nop*/ }
    virtual void             Reassociate(const AbstractDataSetGateway& DataGateway) { /*nop*/ }
    virtual const AbstractLoglikelihoodRatioUnifier& getRatioUnified(AbstractLikelihoodCalculator& Calculator) const;
};

/** Class representing accumulated data of space-time clustering in multiple data set for batched model. */
class MultiSetBatchedSpaceTimeData : public AbstractMultiSetBatchedTemporalData {
private:
    EvaluationAssistDataStatus geEvaluationAssistDataStatus;

public:
    MultiSetBatchedSpaceTimeData(const BatchedClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway);
    virtual ~MultiSetBatchedSpaceTimeData() {}
    virtual MultiSetBatchedSpaceTimeData* Clone() const;

    virtual void             AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex = 0);
    virtual void             DeallocateEvaluationAssistClassMembers();
    virtual void             InitializeData();
    virtual void             Reassociate(const DataSetInterface& Interface) { /*nop*/ }
    virtual void             Reassociate(const AbstractDataSetGateway& DataGateway) { /*nop*/ }
};

//******************************************************************************
#endif
