//******************************************************************************
#ifndef __MultiSetCategoricalClusterData_H
#define __MultiSetCategoricalClusterData_H
//******************************************************************************
#include "CategoricalClusterData.h"

class CategoricalClusterDataFactory; /* forward class declaration. */

/** Class representing accumulated data of a spatial clustering in multiple data
    sets for case data which is partitioned by category. */
class MultiSetCategoricalSpatialData : public AbstractSpatialClusterData {
  protected:
    ZdPointerVector<CategoricalSpatialData>             gvSetClusterData;
    ZdPointerVector<CategoricalSpatialData>::iterator   gitr;

  public:
    MultiSetCategoricalSpatialData(const CategoricalClusterDataFactory& DataFactory, const AbtractDataSetGateway& DataGateway, int iRate);
    virtual ~MultiSetCategoricalSpatialData();

    virtual void                                        Assign(const AbstractSpatialClusterData& rhs);
    virtual MultiSetCategoricalSpatialData            * Clone() const;

    virtual void        AddNeighborData(tract_t tNeighborIndex, const AbtractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual double      CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator);
    virtual count_t     GetCaseCount(unsigned int tSetIndex=0) const;
    virtual count_t     GetCategoryCaseCount(unsigned int iCategoryIndex, unsigned int tSetIndex=0) const;
    virtual measure_t   GetMeasure(unsigned int tSetIndex=0) const;
    virtual void        InitializeData();
};

/** Abstract class representing accumulated data of a temporal clustering in multiple
    data sets for case data which is partitioned by category. */
class AbstractMultiSetCategoricalTemporalData : public AbstractTemporalClusterData {
  protected:
    ZdPointerVector<CategoricalTemporalData>::iterator   gitr;

  public:
    AbstractMultiSetCategoricalTemporalData();
    virtual ~AbstractMultiSetCategoricalTemporalData();

   ZdPointerVector<CategoricalTemporalData>   gvSetClusterData; /* Note 'Public' member for speed reasons.*/

    virtual count_t     GetCaseCount(unsigned int tSetIndex=0) const;
    virtual count_t     GetCategoryCaseCount(unsigned int iCategoryIndex, unsigned int tSetIndex=0) const;
    virtual measure_t   GetMeasure(unsigned int tSetIndex=0) const;
    virtual void        InitializeData();
};

/** Class representing accumulated data of a temporal clustering in multiple data
    sets for case data which is partitioned by category. */
class MultiSetCategoricalTemporalData : public AbstractMultiSetCategoricalTemporalData {
  public:
    MultiSetCategoricalTemporalData(const CategoricalClusterDataFactory& DataFactory, const AbtractDataSetGateway & DataGateway);
    virtual ~MultiSetCategoricalTemporalData();

    virtual void                              Assign(const AbstractTemporalClusterData& rhs);
    virtual MultiSetCategoricalTemporalData * Clone() const;

    virtual void        AddNeighborData(tract_t tNeighborIndex, const AbtractDataSetGateway& DataGateway, size_t tSetIndex=0);
};

/** Class representing accumulated data of a prospective spatial clustering in
    multiple data sets for case data which is partitioned by category. */
class MultiSetCategoricalProspectiveSpatialData : public AbstractMultiSetCategoricalTemporalData {
  public:
    MultiSetCategoricalProspectiveSpatialData(const CategoricalClusterDataFactory& DataFactory, const CSaTScanData& Data, const AbtractDataSetGateway& DataGateway);
    virtual ~MultiSetCategoricalProspectiveSpatialData();

    virtual void                                        Assign(const AbstractTemporalClusterData& rhs);
    virtual MultiSetCategoricalProspectiveSpatialData * Clone() const;

    virtual void        AddNeighborData(tract_t tNeighborIndex, const AbtractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual double      CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator);
};

/** Class representing accumulated data of a space-time clustering in multiple
    data sets for case data which is partitioned by category. */
class MultiSetCategoricalSpaceTimeData : public AbstractMultiSetCategoricalTemporalData {
  public:
    MultiSetCategoricalSpaceTimeData(const CategoricalClusterDataFactory& DataFactory, const AbtractDataSetGateway& DataGateway);
    virtual ~MultiSetCategoricalSpaceTimeData();

    virtual void                               Assign(const AbstractTemporalClusterData& rhs);
    virtual MultiSetCategoricalSpaceTimeData * Clone() const;

    virtual void        AddNeighborData(tract_t tNeighborIndex, const AbtractDataSetGateway& DataGateway, size_t tSetIndex=0);
};
//******************************************************************************
#endif

