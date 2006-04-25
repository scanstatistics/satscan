//******************************************************************************
#ifndef __MultiSetNormalClusterData_H
#define __MultiSetNormalClusterData_H
//******************************************************************************
#include "NormalClusterData.h"

class NormalClusterDataFactory; /* forward class declaration. */
class AbstractLikelihoodCalculator;

/** Class representing accumulated data of a spatial clustering in multiple data
    sets for case data which is partitioned by category. */
class MultiSetNormalSpatialData : public AbstractSpatialClusterData, public AbstractNormalClusterData {
  protected:
    ZdPointerVector<NormalSpatialData>             gvSetClusterData;

  public:
    MultiSetNormalSpatialData(const NormalClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway, int iRate);
    virtual ~MultiSetNormalSpatialData() {}

    virtual void        AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual void        Assign(const AbstractSpatialClusterData& rhs);
    virtual double      CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator);
    virtual MultiSetNormalSpatialData * Clone() const;
    virtual void        CopyEssentialClassMembers(const AbstractClusterData& rhs);
    virtual count_t     GetCaseCount(unsigned int tSetIndex=0) const;
    void                GetDataSetIndexesComprisedInRatio(double dTargetLoglikelihoodRatio,
                                                          AbstractLikelihoodCalculator& Calculator,
                                                          std::vector<unsigned int>& vDataSetIndexes) const;
    virtual measure_t   GetMeasure(unsigned int tSetIndex=0) const;
    virtual measure_t   GetMeasureSq(unsigned int tSetIndex=0) const;
    virtual void        InitializeData();

};

/** Abstract class representing accumulated data of temporal clustering in multiple data sets. */
class AbstractMultiSetNormalTemporalData : public AbstractTemporalClusterData, public AbstractNormalClusterData {
  public:
    AbstractMultiSetNormalTemporalData() : AbstractTemporalClusterData() {}
    virtual ~AbstractMultiSetNormalTemporalData() {}

    ZdPointerVector<NormalTemporalData> gvSetClusterData;

    virtual void             Assign(const AbstractTemporalClusterData& rhs);
    virtual void             CopyEssentialClassMembers(const AbstractClusterData& rhs);
    virtual count_t          GetCaseCount(unsigned int tSetIndex=0) const;
    virtual void             GetDataSetIndexesComprisedInRatio(double dTargetLoglikelihoodRatio,
                                                               AbstractLikelihoodCalculator& Calculator,
                                                               std::vector<unsigned int>& vDataSetIndexes) const;
    virtual measure_t        GetMeasure(unsigned int tSetIndex=0) const;
    virtual measure_t        GetMeasureSq(unsigned int tSetIndex=0) const;
};

/** Class representing accumulated data of temporal clustering in multiple data sets. */
class MultiSetNormalTemporalData : public AbstractMultiSetNormalTemporalData {
  public:
    MultiSetNormalTemporalData(const NormalClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway);
    virtual ~MultiSetNormalTemporalData() {}

    virtual void             AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual MultiSetNormalTemporalData * Clone() const;
    virtual void             InitializeData();
    virtual void             Reassociate(const DataSetInterface& Interface);
    virtual void             Reassociate(const AbstractDataSetGateway& DataGateway);
};

/** Class representing accumulated data of prospective spatial clustering in multiple data sets. */
class MultiSetNormalProspectiveSpatialData : public AbstractMultiSetNormalTemporalData {
  protected:
    EvaluationAssistDataStatus               geEvaluationAssistDataStatus;

  public:
    MultiSetNormalProspectiveSpatialData(const NormalClusterDataFactory& DataFactory, const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway);
    virtual ~MultiSetNormalProspectiveSpatialData() {}

    virtual void             AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual double           CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator);
    virtual MultiSetNormalProspectiveSpatialData * Clone() const;
    virtual void             DeallocateEvaluationAssistClassMembers();
    virtual void             InitializeData();
    virtual void             Reassociate(const DataSetInterface& Interface) {/*nop*/}
    virtual void             Reassociate(const AbstractDataSetGateway& DataGateway) {/*nop*/}
};

/** Class representing accumulated data of space-time clustering in multiple data set. */
class MultiSetNormalSpaceTimeData : public AbstractMultiSetNormalTemporalData {
  private:
    EvaluationAssistDataStatus      geEvaluationAssistDataStatus;

  public:
    MultiSetNormalSpaceTimeData(const NormalClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway);
    virtual ~MultiSetNormalSpaceTimeData() {}

    virtual void             AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual MultiSetNormalSpaceTimeData * Clone() const;
    virtual void             DeallocateEvaluationAssistClassMembers();
    virtual void             InitializeData();
    virtual void             Reassociate(const DataSetInterface& Interface) {/*nop*/}
    virtual void             Reassociate(const AbstractDataSetGateway& DataGateway) {/*nop*/}
};
//******************************************************************************
#endif
