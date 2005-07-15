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
    ZdPointerVector<SpatialData>::iterator gitr;

  public:
    MultiSetSpatialData(const ClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway, int iRate);
    virtual ~MultiSetSpatialData();

    virtual void                  Assign(const AbstractSpatialClusterData& rhs);
    virtual MultiSetSpatialData * Clone() const;

    virtual void                  AddMeasureList(CMeasureList* pMeasureList, tract_t tNeighborIndex, const DataSetInterface& Interface);
    virtual void                  AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual double                CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator);
    virtual count_t               GetCaseCount(unsigned int tSetIndex=0) const;
    virtual measure_t             GetMeasure(unsigned int tSetIndex=0) const;
    virtual void                  InitializeData();
};

/** Abstract class representing accumulated data of temporal clustering in multiple data sets. */
class AbstractMultiSetTemporalData : public AbstractTemporalClusterData {
  protected:
   ZdPointerVector<TemporalData>::iterator gitr;

  public:
    AbstractMultiSetTemporalData() : AbstractTemporalClusterData() {}
    virtual ~AbstractMultiSetTemporalData() {}

   ZdPointerVector<TemporalData>           gvSetClusterData;
};

/** Class representing accumulated data of temporal clustering in multiple data sets. */
class MultiSetTemporalData : public AbstractMultiSetTemporalData {
  public:
    MultiSetTemporalData(const ClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway);
    virtual ~MultiSetTemporalData();

    virtual void                   Assign(const AbstractTemporalClusterData& rhs);
    virtual MultiSetTemporalData * Clone() const;

    virtual void                   AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual count_t                GetCaseCount(unsigned int tSetIndex=0) const;
    virtual measure_t              GetMeasure(unsigned int tSetIndex=0) const;
    virtual void                   InitializeData();
    virtual void                   Reassociate(const DataSetInterface& Interface);
    virtual void                   Reassociate(const AbstractDataSetGateway& DataGateway);
};

/** Class representing accumulated data of prospective spatial clustering in
    multiple data sets. */
class MultiSetProspectiveSpatialData : public AbstractMultiSetTemporalData {
  protected:
     RATE_FUNCPTRTYPE                              gfRateOfInterest;

  public:
    MultiSetProspectiveSpatialData(const ClusterDataFactory& DataFactory, const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway);
    virtual ~MultiSetProspectiveSpatialData();

    virtual void                             Assign(const AbstractTemporalClusterData& rhs);
    virtual MultiSetProspectiveSpatialData * Clone() const;

    virtual void                             AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual double                           CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator);
    virtual count_t                          GetCaseCount(unsigned int tSetIndex=0) const;
    virtual measure_t                        GetMeasure(unsigned int tSetIndex=0) const;
    virtual void                             InitializeData();
    virtual void                             Reassociate(const DataSetInterface& Interface) {/*nop*/}
    virtual void                             Reassociate(const AbstractDataSetGateway& DataGateway) {/*nop*/}
};

/** Class representing accumulated data of space-time clustering in multiple data set. */
class MultiSetSpaceTimeData : public AbstractMultiSetTemporalData {
  public:
    MultiSetSpaceTimeData(const ClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway);
    virtual ~MultiSetSpaceTimeData();

    virtual void                    Assign(const AbstractTemporalClusterData& rhs);
    virtual MultiSetSpaceTimeData * Clone() const;

    virtual void                    AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual count_t                 GetCaseCount(unsigned int tSetIndex=0) const;
    virtual measure_t               GetMeasure(unsigned int tSetIndex=0) const;
    virtual void                    InitializeData();
    virtual void                    Reassociate(const DataSetInterface& Interface) {/*nop*/}
    virtual void                    Reassociate(const AbstractDataSetGateway& DataGateway) {/*nop*/}
};
//******************************************************************************
#endif

