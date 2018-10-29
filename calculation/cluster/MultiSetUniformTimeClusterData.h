//******************************************************************************
#ifndef __MultiSetUniformTimeClusterData_H
#define __MultiSetUniformTimeClusterData_H
//******************************************************************************
#include "UniformTimeClusterData.h"
#include "ptr_vector.h"

class UniformTimeClusterDataFactory; /* forward class declaration. */
class AbstractLikelihoodCalculator;

/** Abstract class representing accumulated data of temporal clustering in multiple data sets. */
class AbstractMultiSetUniformTimeTemporalData : public AbstractTemporalClusterData, public AbstractUniformTimeClusterData {
  public:
    AbstractMultiSetUniformTimeTemporalData() : AbstractTemporalClusterData() {}
    virtual ~AbstractMultiSetUniformTimeTemporalData() {}

    virtual size_t           getNumSets() const = 0;
    virtual UniformTimeClusterDataInterface& getUniformTimeClusterDataInterface(unsigned int tSetIndex) = 0;
};

/** Class representing accumulated data of temporal clustering in multiple data sets. */
class MultiSetUniformTimeTemporalData : public AbstractMultiSetUniformTimeTemporalData {
  public:
    MultiSetUniformTimeTemporalData(const MultiSetUniformTimeTemporalData& rhs);
    MultiSetUniformTimeTemporalData(const UniformTimeClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway);
    virtual ~MultiSetUniformTimeTemporalData() {}

    ptr_vector<UniformTimeTemporalData> gvSetClusterData;

    virtual const AbstractLoglikelihoodRatioUnifier & getRatioUnified(AbstractLikelihoodCalculator& Calculator) const;

    virtual void             Assign(const AbstractTemporalClusterData& rhs);
    virtual UniformTimeClusterDataInterface& getUniformTimeClusterDataInterface(unsigned int tSetIndex);
    virtual void             GetDataSetIndexesComprisedInRatio(double dTargetLoglikelihoodRatio, AbstractLikelihoodCalculator& Calculator, std::vector<unsigned int>& vDataSetIndexes) const;
    virtual count_t          GetCaseCount(unsigned int tSetIndex = 0) const { return gvSetClusterData.at(tSetIndex)->gtCases; }
    virtual void             setCaseCount(count_t t, unsigned int tSetIndex = 0) { gvSetClusterData.at(tSetIndex)->setCaseCount(t); }
    virtual count_t          GetCasesInPeriod(unsigned int tSetIndex) const { return gvSetClusterData.at(tSetIndex)->gtCasesInPeriod; }
    virtual measure_t        GetMeasureInPeriod(unsigned int tSetIndex) const { return gvSetClusterData.at(tSetIndex)->gtMeasureInPeriod; }

    virtual measure_t        GetMeasure(unsigned int tSetIndex = 0) const { return gvSetClusterData.at(tSetIndex)->gtMeasure; }
    virtual void             setMeasure(measure_t m, unsigned int tSetIndex = 0) { gvSetClusterData.at(tSetIndex)->setMeasure(m); }

    virtual void             AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual MultiSetUniformTimeTemporalData * Clone() const;
    virtual void             CopyEssentialClassMembers(const AbstractClusterData& rhs);
    virtual size_t           getNumSets() const { return gvSetClusterData.size(); }
    virtual void             InitializeData();
    virtual void             Reassociate(const DataSetInterface& Interface);
    virtual void             Reassociate(const AbstractDataSetGateway& DataGateway);
};


/** Class representing accumulated data of space-time clustering in multiple data set. */
class MultiSetUniformTimeSpaceTimeData : public AbstractMultiSetUniformTimeTemporalData {
  private:
    EvaluationAssistDataStatus      geEvaluationAssistDataStatus;

  public:
    MultiSetUniformTimeSpaceTimeData(const MultiSetUniformTimeSpaceTimeData& rhs);
    MultiSetUniformTimeSpaceTimeData(const UniformTimeClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway);    
    virtual ~MultiSetUniformTimeSpaceTimeData() {}

    ptr_vector<UniformTimeSpaceTimeData> gvSetClusterData;

    virtual void             Assign(const AbstractTemporalClusterData& rhs);
    virtual const AbstractLoglikelihoodRatioUnifier & getRatioUnified(AbstractLikelihoodCalculator& Calculator) const;

    virtual UniformTimeClusterDataInterface& getUniformTimeClusterDataInterface(unsigned int tSetIndex);
    virtual void             GetDataSetIndexesComprisedInRatio(double dTargetLoglikelihoodRatio, AbstractLikelihoodCalculator& Calculator, std::vector<unsigned int>& vDataSetIndexes) const;
    virtual count_t          GetCaseCount(unsigned int tSetIndex = 0) const { return gvSetClusterData.at(tSetIndex)->gtCases; }
    virtual void             setCaseCount(count_t t, unsigned int tSetIndex = 0) { gvSetClusterData.at(tSetIndex)->setCaseCount(t); }
    virtual count_t          GetCasesInPeriod(unsigned int tSetIndex) const { return gvSetClusterData.at(tSetIndex)->gtCasesInPeriod; }
    virtual measure_t        GetMeasureInPeriod(unsigned int tSetIndex) const { return gvSetClusterData.at(tSetIndex)->gtMeasureInPeriod; }

    virtual measure_t        GetMeasure(unsigned int tSetIndex = 0) const { return gvSetClusterData.at(tSetIndex)->gtMeasure; }
    virtual void             setMeasure(measure_t m, unsigned int tSetIndex = 0) { gvSetClusterData.at(tSetIndex)->setMeasure(m); }

    virtual void             AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual MultiSetUniformTimeSpaceTimeData * Clone() const;
    virtual void             CopyEssentialClassMembers(const AbstractClusterData& rhs);
    virtual void             DeallocateEvaluationAssistClassMembers();
    virtual size_t           getNumSets() const { return gvSetClusterData.size(); }
    virtual void             InitializeData();
    virtual void             Reassociate(const DataSetInterface& Interface) {/*nop*/}
    virtual void             Reassociate(const AbstractDataSetGateway& DataGateway) {/*nop*/}
};
//******************************************************************************
#endif
