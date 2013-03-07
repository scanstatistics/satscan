//******************************************************************************
#ifndef __AbstractClusterData_H
#define __AbstractClusterData_H
//******************************************************************************
#include "IncidentRate.h"
#include "DataSetGateway.h"
#include "LikelihoodCalculation.h"


/** Abstract class representing accumulated cluster data. */
class AbstractClusterData {
  public:
    AbstractClusterData();
    virtual ~AbstractClusterData();

    virtual AbstractClusterData * Clone() const = 0;

    //Some derived classes contain data members which are allocated solely to aid in the evaluation process.
    //Ideally it would be better to isolate these members into a separate class but execution speed and not
    //wanting to refactor classes of AbstractClusterData hierachy at this time bring purpose to this solution.
    //In code where these evaluation assisting data must be alllocated, we'll utilize 'assert()' functionality
    //to prevent accessing of invalid memory. Note that typically this functionality will be used in prospective
    //purely spatial and space-time clusters.
    enum                EvaluationAssistDataStatus {Allocated=0, Deallocated};

    virtual void        AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0) = 0;
    virtual double      CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator);
    virtual void        CopyEssentialClassMembers(const AbstractClusterData& rhs) = 0;
    virtual void        DeallocateEvaluationAssistClassMembers() {/*nop*/}
    virtual count_t     GetCaseCount(unsigned int tSetIndex=0) const = 0;
    virtual void        setCaseCount(count_t t, unsigned int tSetIndex=0) = 0;
    virtual count_t     GetCategoryCaseCount(unsigned int iCategoryIndex, unsigned int tSetIndex=0) const;
    virtual void        GetDataSetIndexesComprisedInRatio(double dTargetLoglikelihoodRatio,
                                                          AbstractLikelihoodCalculator& Calculator,
                                                          std::vector<unsigned int>& vDataSetIndexes) const;
    virtual measure_t   GetMeasure(unsigned int tSetIndex=0) const = 0;
    virtual void        setMeasure(measure_t m, unsigned int tSetIndex=0) = 0;
    virtual void        InitializeData() = 0;
    virtual void        InitializeData(const AbstractDataSetGateway& DataGateway) {InitializeData();}
    virtual size_t      getNumSets() const {return 1;}
    virtual const AbstractLoglikelihoodRatioUnifier & getRatioUnified(AbstractLikelihoodCalculator& Calculator) const;
};

/** Abstract class representing accumulated data of spatial clustering. */
class AbstractSpatialClusterData : public AbstractClusterData {
  public:
    AbstractSpatialClusterData();
    virtual ~AbstractSpatialClusterData();

    virtual void                         Assign(const AbstractSpatialClusterData& rhs) = 0;
    virtual AbstractSpatialClusterData * Clone() const = 0;

    virtual double      GetMaximizingValue(AbstractLikelihoodCalculator& Calculator) = 0;
};

/**Abstract base class for all categorical cluster data. */
class AbstractProspectiveSpatialClusterData {
  public:
    AbstractProspectiveSpatialClusterData() {}
    virtual ~AbstractProspectiveSpatialClusterData() {}

    virtual double      GetMaximizingValue(AbstractLikelihoodCalculator& Calculator) = 0;
};

/** Abstract class representing accumulated data of temporal clustering. */
class AbstractTemporalClusterData : public AbstractClusterData {
  public:
    AbstractTemporalClusterData();
    virtual ~AbstractTemporalClusterData();

    virtual void                          Assign(const AbstractTemporalClusterData& rhs) = 0;
    virtual AbstractTemporalClusterData * Clone() const = 0;
    virtual void                          Reassociate(const DataSetInterface& Interface);
    virtual void                          Reassociate(const AbstractDataSetGateway& DataGateway);
};
//******************************************************************************
#endif

