//---------------------------------------------------------------------------
#ifndef MultipleStreamClusterDataH
#define MultipleStreamClusterDataH
//---------------------------------------------------------------------------
#include "ClusterData.h"

class ClusterDataFactory; /** forward class declaration */

/** class representing accumulated data of spatial clustering in multiple data streams */
class MutlipleStreamSpatialData : public AbstractSpatialClusterData {
  protected:
    ZdPointerVector<AbstractSpatialClusterData>             gvStreamData;
    ZdPointerVector<AbstractSpatialClusterData>::iterator   gitr;

  public:
    MutlipleStreamSpatialData(const ClusterDataFactory& DataFactory, const AbtractDataStreamGateway & DataGateway, int iRate);
    virtual ~MutlipleStreamSpatialData();

    virtual void                                Assign(const AbstractSpatialClusterData& rhs);
    virtual MutlipleStreamSpatialData         * Clone() const;

    virtual void                                AddMeasureList(CMeasureList * pMeasureList, tract_t tNeighbor, const DataStreamInterface & Interface);
    virtual void                                AddNeighborData(tract_t tNeighbor, const AbtractDataStreamGateway & DataGateway, size_t tStream=0);
    virtual double                              CalculateLoglikelihoodRatio(CModel & Model);
    virtual count_t                             GetCaseCount(unsigned int iStream=0) const;
    virtual measure_t                           GetMeasure(unsigned int iStream=0) const;
    virtual void                                InitializeData();
};

/** abstract class representing accumulated data of temporal clustering in multiple data streams */
class AbstractMutlipleStreamTemporalData : public AbstractTemporalClusterData {
  protected:
   ZdPointerVector<TemporalData>::iterator     gitr;

  public:
    AbstractMutlipleStreamTemporalData() : AbstractTemporalClusterData() {}
    virtual ~AbstractMutlipleStreamTemporalData() {}

   ZdPointerVector<TemporalData>               gvStreamData;
};

/** class representing accumulated data of temporal clustering in multiple data streams */
class MutlipleStreamTemporalData : public AbstractMutlipleStreamTemporalData {
  public:
    MutlipleStreamTemporalData(const ClusterDataFactory& DataFactory, const AbtractDataStreamGateway & DataGateway);
    virtual ~MutlipleStreamTemporalData();

    virtual void                         Assign(const AbstractTemporalClusterData& rhs);
    virtual MutlipleStreamTemporalData * Clone() const;

    virtual void                         AddNeighborData(tract_t tNeighbor, const AbtractDataStreamGateway & DataGateway, size_t tStream=0);
    virtual count_t                      GetCaseCount(unsigned int iStream=0) const;
    virtual measure_t                    GetMeasure(unsigned int iStream=0) const;
    virtual void                         InitializeData();
    virtual void                         SetCaseCount(count_t tCases, unsigned int iStream=0) {gvStreamData[iStream]->gtCases = tCases;}
    virtual void                         SetMeasure(measure_t tMeasure, unsigned int iStream=0) {gvStreamData[iStream]->gtMeasure = tMeasure;}
};

/** class representing accumulated data of prospective spatial clustering
    in multiple data streams                                                  */
class MutlipleStreamProspectiveSpatialData : public AbstractMutlipleStreamTemporalData {
  protected:
//     std::vector<ProspectiveSpatialData>           gvStreamData;
//     std::vector<ProspectiveSpatialData>::iterator gitr;
     RATE_FUNCPTRTYPE                              gfRateOfInterest;

  public:
    MutlipleStreamProspectiveSpatialData(const ClusterDataFactory& DataFactory, const CSaTScanData & Data, const AbtractDataStreamGateway & DataGateway);
    virtual ~MutlipleStreamProspectiveSpatialData();

    virtual void                                   Assign(const AbstractTemporalClusterData& rhs);
    virtual MutlipleStreamProspectiveSpatialData * Clone() const;

    virtual void                                   AddNeighborData(tract_t tNeighbor, const AbtractDataStreamGateway & DataGateway, size_t tStream=0);
    virtual double                                 CalculateLoglikelihoodRatio(CModel & Model);
    virtual count_t                                GetCaseCount(unsigned int iStream=0) const;
    virtual measure_t                              GetMeasure(unsigned int iStream=0) const;
    virtual void                                   InitializeData();
    virtual void                                   SetCaseCount(count_t tCases, unsigned int iStream=0) {gvStreamData[iStream]->gtCases = tCases;}
    virtual void                                   SetMeasure(measure_t tMeasure, unsigned int iStream=0) {gvStreamData[iStream]->gtMeasure = tMeasure;}
};

/** class representing accumulated data of space-time clustering in multiple data streams */
class MutlipleStreamSpaceTimeData : public AbstractMutlipleStreamTemporalData {
  public:
    MutlipleStreamSpaceTimeData(const ClusterDataFactory& DataFactory, const AbtractDataStreamGateway & DataGateway);
    virtual ~MutlipleStreamSpaceTimeData();

    virtual void                          Assign(const AbstractTemporalClusterData& rhs);
    virtual MutlipleStreamSpaceTimeData * Clone() const;

    virtual void        AddNeighborData(tract_t tNeighbor, const AbtractDataStreamGateway & DataGateway, size_t tStream=0);
    virtual count_t     GetCaseCount(unsigned int iStream=0) const;
    virtual measure_t   GetMeasure(unsigned int iStream=0) const;
    virtual void        InitializeData();
    virtual void        SetCaseCount(count_t tCases, unsigned int iStream=0) {gvStreamData[iStream]->gtCases = tCases;}
    virtual void        SetMeasure(measure_t tMeasure, unsigned int iStream=0) {gvStreamData[iStream]->gtMeasure = tMeasure;}
};
//---------------------------------------------------------------------------
#endif
