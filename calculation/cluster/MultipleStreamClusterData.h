//---------------------------------------------------------------------------
#ifndef MultipleStreamClusterDataH
#define MultipleStreamClusterDataH
//---------------------------------------------------------------------------
#include "ClusterData.h"

class ClusterDataFactory; /** forward class declaration */

/** class representing accumulated data of spatial clustering in multiple data streams */
class MultipleStreamSpatialData : public AbstractSpatialClusterData {
  protected:
    ZdPointerVector<AbstractSpatialClusterData>             gvStreamData;
    ZdPointerVector<AbstractSpatialClusterData>::iterator   gitr;

  public:
    MultipleStreamSpatialData(const ClusterDataFactory& DataFactory, const AbtractDataStreamGateway & DataGateway, int iRate);
    virtual ~MultipleStreamSpatialData();

    virtual void                                Assign(const AbstractSpatialClusterData& rhs);
    virtual MultipleStreamSpatialData         * Clone() const;

    virtual void                                AddMeasureList(CMeasureList * pMeasureList, tract_t tNeighbor, const DataStreamInterface & Interface);
    virtual void                                AddNeighborData(tract_t tNeighbor, const AbtractDataStreamGateway & DataGateway, size_t tStream=0);
    virtual double                              CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator & Calculator);
    virtual count_t                             GetCaseCount(unsigned int iStream=0) const;
    virtual measure_t                           GetMeasure(unsigned int iStream=0) const;
    virtual void                                InitializeData();
};

/** abstract class representing accumulated data of temporal clustering in multiple data streams */
class AbstractMultipleStreamTemporalData : public AbstractTemporalClusterData {
  protected:
   ZdPointerVector<TemporalData>::iterator     gitr;

  public:
    AbstractMultipleStreamTemporalData() : AbstractTemporalClusterData() {}
    virtual ~AbstractMultipleStreamTemporalData() {}

   ZdPointerVector<TemporalData>               gvStreamData;
};

/** class representing accumulated data of temporal clustering in multiple data streams */
class MultipleStreamTemporalData : public AbstractMultipleStreamTemporalData {
  public:
    MultipleStreamTemporalData(const ClusterDataFactory& DataFactory, const AbtractDataStreamGateway & DataGateway);
    virtual ~MultipleStreamTemporalData();

    virtual void                         Assign(const AbstractTemporalClusterData& rhs);
    virtual MultipleStreamTemporalData * Clone() const;

    virtual void                         AddNeighborData(tract_t tNeighbor, const AbtractDataStreamGateway & DataGateway, size_t tStream=0);
    virtual count_t                      GetCaseCount(unsigned int iStream=0) const;
    virtual measure_t                    GetMeasure(unsigned int iStream=0) const;
    virtual void                         InitializeData();
};

/** class representing accumulated data of prospective spatial clustering
    in multiple data streams                                                  */
class MultipleStreamProspectiveSpatialData : public AbstractMultipleStreamTemporalData {
  protected:
//     std::vector<ProspectiveSpatialData>           gvStreamData;
//     std::vector<ProspectiveSpatialData>::iterator gitr;
     RATE_FUNCPTRTYPE                              gfRateOfInterest;

  public:
    MultipleStreamProspectiveSpatialData(const ClusterDataFactory& DataFactory, const CSaTScanData & Data, const AbtractDataStreamGateway & DataGateway);
    virtual ~MultipleStreamProspectiveSpatialData();

    virtual void                                   Assign(const AbstractTemporalClusterData& rhs);
    virtual MultipleStreamProspectiveSpatialData * Clone() const;

    virtual void                                   AddNeighborData(tract_t tNeighbor, const AbtractDataStreamGateway & DataGateway, size_t tStream=0);
    virtual double                                 CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator & Calculator);
    virtual count_t                                GetCaseCount(unsigned int iStream=0) const;
    virtual measure_t                              GetMeasure(unsigned int iStream=0) const;
    virtual void                                   InitializeData();
};

/** class representing accumulated data of space-time clustering in multiple data streams */
class MultipleStreamSpaceTimeData : public AbstractMultipleStreamTemporalData {
  public:
    MultipleStreamSpaceTimeData(const ClusterDataFactory& DataFactory, const AbtractDataStreamGateway & DataGateway);
    virtual ~MultipleStreamSpaceTimeData();

    virtual void                          Assign(const AbstractTemporalClusterData& rhs);
    virtual MultipleStreamSpaceTimeData * Clone() const;

    virtual void        AddNeighborData(tract_t tNeighbor, const AbtractDataStreamGateway & DataGateway, size_t tStream=0);
    virtual count_t     GetCaseCount(unsigned int iStream=0) const;
    virtual measure_t   GetMeasure(unsigned int iStream=0) const;
    virtual void        InitializeData();
};
//---------------------------------------------------------------------------
#endif
