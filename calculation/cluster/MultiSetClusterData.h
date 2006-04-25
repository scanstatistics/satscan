//******************************************************************************
#ifndef __MultiSetClusterData_H
#define __MultiSetClusterData_H
//******************************************************************************
#include "ClusterData.h"

class ClusterDataFactory; /** forward class declaration */

/** Class representing accumulated data of spatial clustering in multiple data sets. */
class MultiSetSpatialData : public AbstractSpatialClusterData {
  protected:
    ZdPointerVector<SpatialData>           gvSetClusterData;

  public:
    MultiSetSpatialData(const ClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway, int iRate);
    virtual ~MultiSetSpatialData() {}

    virtual void             AddMeasureList(CMeasureList* pMeasureList, tract_t tNeighborIndex, const DataSetInterface& Interface);
    virtual void             AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual void             Assign(const AbstractSpatialClusterData& rhs);
    virtual double           CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator);
    virtual MultiSetSpatialData * Clone() const;
    virtual void             CopyEssentialClassMembers(const AbstractClusterData& rhs);
    virtual count_t          GetCaseCount(unsigned int tSetIndex=0) const;
    virtual void             GetDataSetIndexesComprisedInRatio(double dTargetLoglikelihoodRatio,
                                                               AbstractLikelihoodCalculator& Calculator,
                                                               std::vector<unsigned int>& vDataSetIndexes) const;
    virtual measure_t        GetMeasure(unsigned int tSetIndex=0) const;
    virtual void             InitializeData();
};

/** Abstract class representing accumulated data of temporal clustering in multiple data sets. */
class AbstractMultiSetTemporalData : public AbstractTemporalClusterData {
  public:
    AbstractMultiSetTemporalData() : AbstractTemporalClusterData() {}
    virtual ~AbstractMultiSetTemporalData() {}

    ZdPointerVector<TemporalData> gvSetClusterData;

    virtual void             Assign(const AbstractTemporalClusterData& rhs);
    virtual void             CopyEssentialClassMembers(const AbstractClusterData& rhs);
    virtual count_t          GetCaseCount(unsigned int tSetIndex=0) const;
    virtual void             GetDataSetIndexesComprisedInRatio(double dTargetLoglikelihoodRatio,
                                                               AbstractLikelihoodCalculator& Calculator,
                                                               std::vector<unsigned int>& vDataSetIndexes) const;
    virtual measure_t        GetMeasure(unsigned int tSetIndex=0) const;
};

/** Class representing accumulated data of temporal clustering in multiple data sets. */
class MultiSetTemporalData : public AbstractMultiSetTemporalData {
  public:
    MultiSetTemporalData(const ClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway);
    virtual ~MultiSetTemporalData() {}

    virtual void             AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual MultiSetTemporalData * Clone() const;
    virtual void             InitializeData();
    virtual void             Reassociate(const DataSetInterface& Interface);
    virtual void             Reassociate(const AbstractDataSetGateway& DataGateway);
};

/** Class representing accumulated data of prospective spatial clustering in multiple data sets. */
class MultiSetProspectiveSpatialData : public AbstractMultiSetTemporalData {
  protected:
    EvaluationAssistDataStatus               geEvaluationAssistDataStatus;

  public:
    MultiSetProspectiveSpatialData(const ClusterDataFactory& DataFactory, const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway);
    virtual ~MultiSetProspectiveSpatialData() {}

    virtual void             AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual double           CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator);
    virtual MultiSetProspectiveSpatialData * Clone() const;
    virtual void             DeallocateEvaluationAssistClassMembers();
    virtual void             InitializeData();
    virtual void             Reassociate(const DataSetInterface& Interface) {/*nop*/}
    virtual void             Reassociate(const AbstractDataSetGateway& DataGateway) {/*nop*/}
};

/** Class representing accumulated data of space-time clustering in multiple data set. */
class MultiSetSpaceTimeData : public AbstractMultiSetTemporalData {
  private:
    EvaluationAssistDataStatus      geEvaluationAssistDataStatus;

  public:
    MultiSetSpaceTimeData(const ClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway);
    virtual ~MultiSetSpaceTimeData() {}

    virtual void             AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual MultiSetSpaceTimeData * Clone() const;
    virtual void             DeallocateEvaluationAssistClassMembers();
    virtual void             InitializeData();
    virtual void             Reassociate(const DataSetInterface& Interface) {/*nop*/}
    virtual void             Reassociate(const AbstractDataSetGateway& DataGateway) {/*nop*/}
};
//******************************************************************************
#endif

