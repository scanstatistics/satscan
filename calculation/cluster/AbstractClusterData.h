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

    virtual void        AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0) = 0;
    virtual double      CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator);
    virtual count_t     GetCaseCount(unsigned int tSetIndex=0) const = 0;

    virtual count_t     GetCategoryCaseCount(unsigned int iCategoryIndex, unsigned int tSetIndex=0) const {return 0;}// make virtual later = 0;

    virtual measure_t   GetMeasure(unsigned int tSetIndex=0) const = 0;
    virtual void        InitializeData() = 0;
};

/** Abstract class representing accumulated data of spatial clustering. */
class AbstractSpatialClusterData : public AbstractClusterData {
  protected:
    RATE_FUNCPTRTYPE            gfRateOfInterest;

  public:
    AbstractSpatialClusterData(int iRate);
    virtual ~AbstractSpatialClusterData();

    virtual void                         Assign(const AbstractSpatialClusterData& rhs) = 0;
    virtual AbstractSpatialClusterData * Clone() const = 0;
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

