//******************************************************************************
#ifndef __MultiSetNormalClusterData_H
#define __MultiSetNormalClusterData_H
//******************************************************************************
#include "NormalClusterData.h"
#include "ptr_vector.h"

class NormalClusterDataFactory; /* forward class declaration. */
class AbstractLikelihoodCalculator;

/** Class representing accumulated data of a spatial clustering in multiple data
    sets for case data which is partitioned by category. */
class MultiSetNormalSpatialData : public AbstractSpatialClusterData, public AbstractNormalClusterData {
  protected:
    ptr_vector<NormalSpatialData>             gvSetClusterData;

  public:
    MultiSetNormalSpatialData(const NormalClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway);
    virtual ~MultiSetNormalSpatialData() {}

    virtual void        AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual void        Assign(const AbstractSpatialClusterData& rhs);
    virtual double      CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator);
    virtual MultiSetNormalSpatialData * Clone() const;
    virtual void        CopyEssentialClassMembers(const AbstractClusterData& rhs);
    virtual count_t     GetCaseCount(unsigned int tSetIndex=0) const;
    virtual void        setCaseCount(count_t t, unsigned int tSetIndex=0);
    void                GetDataSetIndexesComprisedInRatio(double dTargetLoglikelihoodRatio,
                                                          AbstractLikelihoodCalculator& Calculator,
                                                          std::vector<unsigned int>& vDataSetIndexes) const;
    virtual double      GetMaximizingValue(AbstractLikelihoodCalculator& Calculator);
    virtual measure_t   GetMeasure(unsigned int tSetIndex=0) const;
    virtual void        setMeasure(measure_t m, unsigned int tSetIndex=0);
    virtual measure_t   GetMeasureAux(unsigned int tSetIndex=0) const;
    virtual void        InitializeData();
    virtual size_t      getNumSets() const {return gvSetClusterData.size();}
};

/** Abstract class representing accumulated data of temporal clustering in multiple data sets. */
class AbstractMultiSetNormalTemporalData : public AbstractTemporalClusterData, public AbstractNormalClusterData {
  public:
    AbstractMultiSetNormalTemporalData() : AbstractTemporalClusterData() {}
    virtual ~AbstractMultiSetNormalTemporalData() {}

    ptr_vector<NormalTemporalData> gvSetClusterData;

    virtual void             Assign(const AbstractTemporalClusterData& rhs);
    virtual void             CopyEssentialClassMembers(const AbstractClusterData& rhs);
    virtual count_t          GetCaseCount(unsigned int tSetIndex=0) const;
    virtual void             setCaseCount(count_t t, unsigned int tSetIndex=0);
    virtual void             GetDataSetIndexesComprisedInRatio(double dTargetLoglikelihoodRatio,
                                                               AbstractLikelihoodCalculator& Calculator,
                                                               std::vector<unsigned int>& vDataSetIndexes) const;
    virtual measure_t        GetMeasure(unsigned int tSetIndex=0) const;
    virtual void             setMeasure(measure_t m, unsigned int tSetIndex=0);
    virtual measure_t        GetMeasureAux(unsigned int tSetIndex=0) const;
    virtual size_t           getNumSets() const {return gvSetClusterData.size();}
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
class MultiSetNormalProspectiveSpatialData : public AbstractMultiSetNormalTemporalData, public AbstractProspectiveSpatialClusterData {
  protected:
    EvaluationAssistDataStatus               geEvaluationAssistDataStatus;

  public:
    MultiSetNormalProspectiveSpatialData(const NormalClusterDataFactory& DataFactory, const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway);
    virtual ~MultiSetNormalProspectiveSpatialData() {}

    virtual void             AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual double           CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator);
    virtual MultiSetNormalProspectiveSpatialData * Clone() const;
    virtual void             DeallocateEvaluationAssistClassMembers();
    virtual double           GetMaximizingValue(AbstractLikelihoodCalculator& Calculator);
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
