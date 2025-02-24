//******************************************************************************
#ifndef __BatchedClusterData_H
#define __BatchedClusterData_H
//******************************************************************************
#include "ClusterData.h"

/**Abstract base class for all batched cluster data. */
class AbstractBatchedClusterData {
public:
    AbstractBatchedClusterData() {}
    virtual ~AbstractBatchedClusterData() {}

    virtual measure_t GetMeasureAux(unsigned int tSetIndex) const = 0; // represents number of negative batches
    virtual measure_t GetMeasureAux2(unsigned int tSetIndex) const = 0; // represents number of positive batches
    virtual const BatchIndexes_t& GetPositiveBatches(unsigned int tSetIndex=0) const = 0;
    virtual const BatchIndexes_t& GetBatches(unsigned int tSetIndex = 0) const = 0;
};

/** Class representing accumulated data of spatial clustering of a batched probability model. */
class BatchedSpatialData : public SpatialData, public AbstractBatchedClusterData {
protected:
    BatchedSpatialData();

public:
    BatchedSpatialData(const DataSetInterface& Interface);
    BatchedSpatialData(const AbstractDataSetGateway& DataGateway);
    virtual ~BatchedSpatialData() {}
    virtual BatchedSpatialData* Clone() const;

    measure_t gtMeasureAux; // represents number of negative batches
    measure_t gtMeasureAux2; // represents number of positive batches
    BatchIndexes_t gPositiveBatches; // all positive batches in this clustering
    BatchIndexes_t gBatches; // all batches in this clustering

    virtual void             AddMeasureList(const DataSetInterface& Interface, CMeasureList* pMeasureList, const CSaTScanData* pData);
    virtual void             AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex = 0);
    virtual void             Assign(const AbstractSpatialClusterData& rhs);
    virtual double           CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator);
    virtual void             CopyEssentialClassMembers(const AbstractClusterData& rhs);
    virtual double           GetMaximizingValue(AbstractLikelihoodCalculator& Calculator);
    virtual measure_t        GetMeasureAux(unsigned int tSetIndex = 0) const { return gtMeasureAux; }
    virtual measure_t        GetMeasureAux2(unsigned int tSetIndex = 0) const { return gtMeasureAux2; }
    virtual const BatchIndexes_t& GetPositiveBatches(unsigned int tSetIndex = 0) const { return gPositiveBatches; }
    virtual const BatchIndexes_t& GetBatches(unsigned int tSetIndex = 0) const { return gBatches; }
    virtual void             InitializeData() { gtCases = 0; gtMeasure = 0; gtMeasureAux = 0; gtMeasureAux2 = 0; gPositiveBatches.reset(); gBatches.reset(); }
};

/** Class representing accumulated data of temporal clustering for batched probability model.
    If instantiated through public constructors, points to already calculated purely temporal
    arrays supplied by DataInterface. The protected constructor is intended to permit 
    instantiation through a derived class, where perhaps pointers will be allocated and data
    supplied by some other process. */
class BatchedTemporalData : public TemporalData, public AbstractBatchedClusterData {
protected:
    BatchedTemporalData();

public:
    BatchedTemporalData(const DataSetInterface& DataGateway);
    BatchedTemporalData(const AbstractDataSetGateway& DataGateway);
    virtual ~BatchedTemporalData() {}
    virtual BatchedTemporalData * Clone() const;

    measure_t        gtMeasureAux; // represents number of negative batches
    measure_t      * gpMeasureAux; // represents number of negative batches by time interval
    measure_t        gtMeasureAux2; // represents number of positive batches
    measure_t      * gpMeasureAux2; // represents number of positive batches by time interval
    BatchIndexes_t   gPositiveBatches; // all positive batches in this clustering
    BatchIndexes_t * gpPositiveBatches; // all positive batches in this clustering by time interval, using by scanning
    BatchIndexes_t   gBatches; // all batches in this clustering
    BatchIndexes_t * gpBatches; // all batches in this clustering by time interval, using by scanning

    virtual void             Assign(const AbstractTemporalClusterData& rhs);
    virtual void             CopyEssentialClassMembers(const AbstractClusterData& rhs);
    virtual void             InitializeData() { gtCases = 0; gtMeasure = 0; gtMeasureAux = 0; gtMeasureAux2 = 0; gPositiveBatches.reset(); gBatches.reset(); }
    virtual measure_t        GetMeasureAux(unsigned int tSetIndex = 0) const { return gtMeasureAux; }
    virtual measure_t        GetMeasureAux2(unsigned int tSetIndex = 0) const { return gtMeasureAux2; }
    virtual const BatchIndexes_t& GetPositiveBatches(unsigned int tSetIndex = 0) const { return gPositiveBatches; }
    virtual const BatchIndexes_t& GetBatches(unsigned int tSetIndex = 0) const { return gBatches; }
    virtual void             Reassociate(const DataSetInterface& Interface);
    virtual void             Reassociate(const AbstractDataSetGateway& DataGateway);
};

/** Class representing accumulated data of prospective spatial clustering  for batched probability model.
    The ProspectiveSpatialData classes used to have a special purpose but this feature has been deprecated. This class
    keeps the pattern, otherwise we'd need to refactor many other places. */
class BatchedProspectiveSpatialData : public BatchedTemporalData, public AbstractProspectiveSpatialClusterData {
private:
    void                                  Init() { gpCases = 0; gpMeasure = 0; gpMeasureAux = 0; gpMeasureAux2 = 0; 
                                                   gPositiveBatches.reset(); gBatches.reset(); }
    void                                  Setup(const CSaTScanData& Data, const DataSetInterface& Interface);

protected:
    EvaluationAssistDataStatus            geEvaluationAssistDataStatus;
    unsigned int                          giAllocationSize;    /** size of allocated arrays */
    unsigned int                          giNumTimeIntervals;  /** number of time intervals in study period */
    unsigned int                          giProspectiveStart;  /** index of prospective start date in DataInterface case array */

public:
    BatchedProspectiveSpatialData(const CSaTScanData& Data, const DataSetInterface& Interface);
    BatchedProspectiveSpatialData(const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway);
    BatchedProspectiveSpatialData(const BatchedProspectiveSpatialData& rhs);
    virtual ~BatchedProspectiveSpatialData();
    virtual BatchedProspectiveSpatialData* Clone() const;
    BatchedProspectiveSpatialData& operator=(const BatchedProspectiveSpatialData& rhs);

    virtual void             AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex = 0);
    virtual void             Assign(const AbstractTemporalClusterData& rhs);
    virtual double           CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator);
    virtual void             DeallocateEvaluationAssistClassMembers();
    virtual double           GetMaximizingValue(AbstractLikelihoodCalculator& Calculator);
    virtual void             InitializeData();
    virtual void             Reassociate(const DataSetInterface& Interface) {/*nop*/ }
    virtual void             Reassociate(const AbstractDataSetGateway& DataGateway) {/*nop*/ }
    virtual unsigned int     GetAllocationSize() const { return giAllocationSize; }
};

/** Class representing accumulated data of space-time clustering for batched probability model. */
class BatchedSpaceTimeData : public BatchedTemporalData {
private:
    void                         Setup(const DataSetInterface& Interface);

protected:
    EvaluationAssistDataStatus   geEvaluationAssistDataStatus;
    unsigned int                 giAllocationSize;  /** size of allocated arrays */
    unsigned int                 _start_index;

public:
    BatchedSpaceTimeData(const DataSetInterface& Interface);
    BatchedSpaceTimeData(const AbstractDataSetGateway& DataGateway);
    BatchedSpaceTimeData(const BatchedSpaceTimeData& rhs);
    virtual ~BatchedSpaceTimeData();
    virtual BatchedSpaceTimeData* Clone() const;
    BatchedSpaceTimeData   & operator=(const BatchedSpaceTimeData& rhs);

    virtual void             AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex = 0);
    virtual void             Assign(const AbstractTemporalClusterData& rhs);
    virtual void             DeallocateEvaluationAssistClassMembers();
    virtual void             InitializeData();
    virtual void             Reassociate(const DataSetInterface& Interface) {/*nop*/ }
    virtual void             Reassociate(const AbstractDataSetGateway& DataGateway) {/*nop*/ }
};

//******************************************************************************
#endif
