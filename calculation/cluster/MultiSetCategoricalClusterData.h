//******************************************************************************
#ifndef __MultiSetCategoricalClusterData_H
#define __MultiSetCategoricalClusterData_H
//******************************************************************************
#include "CategoricalClusterData.h"
#include "ptr_vector.h"

class CategoricalClusterDataFactory; /* forward class declaration. */
class AbstractLikelihoodCalculator;

/** Class representing accumulated data of a spatial clustering in multiple data
    sets for case data which is partitioned by category. */
class MultiSetCategoricalSpatialData : public AbstractSpatialClusterData, public AbstractCategoricalClusterData {
  protected:
    ptr_vector<CategoricalSpatialData>             gvSetClusterData;

  public:
    MultiSetCategoricalSpatialData(const CategoricalClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway);
    virtual ~MultiSetCategoricalSpatialData() {}

    virtual void        AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual void        Assign(const AbstractSpatialClusterData& rhs);
    virtual double      CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator);
    virtual MultiSetCategoricalSpatialData            * Clone() const;
    virtual void        CopyEssentialClassMembers(const AbstractClusterData& rhs);
    virtual count_t     GetCaseCount(unsigned int tSetIndex=0) const;
    virtual count_t     GetCategoryCaseCount(unsigned int iCategoryIndex, unsigned int tSetIndex=0) const;
    void                GetDataSetIndexesComprisedInRatio(double dTargetLoglikelihoodRatio,
                                                          AbstractLikelihoodCalculator& Calculator,
                                                          std::vector<unsigned int>& vDataSetIndexes) const;
    virtual double      GetMaximizingValue(AbstractLikelihoodCalculator& Calculator);
    virtual measure_t   GetMeasure(unsigned int tSetIndex=0) const;
    virtual void        GetOrdinalCombinedCategories(const OrdinalLikelihoodCalculator& Calculator,
                                                     std::vector<OrdinalCombinedCategory>& vCategoryContainer,
                                                     unsigned int tSetIndex=0) const;
    virtual void        InitializeData();

};

/** Abstract class representing accumulated data of a temporal clustering in multiple
    data sets for case data which is partitioned by category. */
class AbstractMultiSetCategoricalTemporalData : public AbstractTemporalClusterData, public AbstractCategoricalClusterData {
  public:
    AbstractMultiSetCategoricalTemporalData() {}
    virtual ~AbstractMultiSetCategoricalTemporalData() {}

    ptr_vector<CategoricalTemporalData>   gvSetClusterData; /* Note 'Public' member for speed reasons.*/

    virtual void        CopyEssentialClassMembers(const AbstractClusterData& rhs);
    virtual count_t     GetCaseCount(unsigned int tSetIndex=0) const;
    virtual count_t     GetCategoryCaseCount(unsigned int iCategoryIndex, unsigned int tSetIndex=0) const;
    void                GetDataSetIndexesComprisedInRatio(double dTargetLoglikelihoodRatio,
                                                          AbstractLikelihoodCalculator& Calculator,
                                                          std::vector<unsigned int>& vDataSetIndexes) const;
    virtual measure_t   GetMeasure(unsigned int tSetIndex=0) const;
    virtual void        GetOrdinalCombinedCategories(const OrdinalLikelihoodCalculator& Calculator,
                                                     std::vector<OrdinalCombinedCategory>& vCategoryContainer,
                                                     unsigned int tSetIndex=0) const;
    virtual void        InitializeData();
};

/** Class representing accumulated data of a temporal clustering in multiple data
    sets for case data which is partitioned by category. */
class MultiSetCategoricalTemporalData : public AbstractMultiSetCategoricalTemporalData {
  public:
    MultiSetCategoricalTemporalData(const CategoricalClusterDataFactory& DataFactory, const AbstractDataSetGateway & DataGateway);
    virtual ~MultiSetCategoricalTemporalData() {}

    virtual void        AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual void        Assign(const AbstractTemporalClusterData& rhs);
    virtual MultiSetCategoricalTemporalData * Clone() const;
    virtual void        Reassociate(const DataSetInterface& Interface);
    virtual void        Reassociate(const AbstractDataSetGateway& DataGateway);
};

/** Class representing accumulated data of a prospective spatial clustering in
    multiple data sets for case data which is partitioned by category. */
class MultiSetCategoricalProspectiveSpatialData : public AbstractMultiSetCategoricalTemporalData, public AbstractProspectiveSpatialClusterData {
  protected:
     EvaluationAssistDataStatus          geEvaluationAssistDataStatus;
  
  public:
    MultiSetCategoricalProspectiveSpatialData(const CategoricalClusterDataFactory& DataFactory, const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway);
    virtual ~MultiSetCategoricalProspectiveSpatialData() {}

    virtual void        AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual void        Assign(const AbstractTemporalClusterData& rhs);
    virtual double      CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator);
    virtual MultiSetCategoricalProspectiveSpatialData * Clone() const;
    virtual void        DeallocateEvaluationAssistClassMembers();
    virtual double      GetMaximizingValue(AbstractLikelihoodCalculator& Calculator);
    virtual void        Reassociate(const DataSetInterface& Interface) {/*nop*/}
    virtual void        Reassociate(const AbstractDataSetGateway& DataGateway) {/*nop*/}
};

/** Class representing accumulated data of a space-time clustering in multiple
    data sets for case data which is partitioned by category. */
class MultiSetCategoricalSpaceTimeData : public AbstractMultiSetCategoricalTemporalData {
  protected:
     EvaluationAssistDataStatus          geEvaluationAssistDataStatus;

  public:
    MultiSetCategoricalSpaceTimeData(const CategoricalClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway);
    virtual ~MultiSetCategoricalSpaceTimeData() {}

    virtual void        AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual void        Assign(const AbstractTemporalClusterData& rhs);
    virtual MultiSetCategoricalSpaceTimeData * Clone() const;
    virtual void        DeallocateEvaluationAssistClassMembers();
    virtual void        Reassociate(const DataSetInterface& Interface) {/*nop*/}
    virtual void        Reassociate(const AbstractDataSetGateway& DataGateway) {/*nop*/}
};
//******************************************************************************
#endif

