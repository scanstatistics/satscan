//******************************************************************************
#ifndef __ClusterData_H
#define __ClusterData_H
//******************************************************************************
#include "AbstractClusterData.h"
#include "MeasureList.h"

/** Class representing accumulated data of spatial clustering. */
class SpatialData : public AbstractSpatialClusterData {
  public:
    SpatialData(const DataStreamInterface& Interface, int iRate);
    SpatialData(const AbtractDataStreamGateway& DataGateway, int iRate);
    virtual ~SpatialData();

    //public data members -- public for speed considerations
    count_t               gtCases;                   /** accumulated cases */
    measure_t             gtMeasure;                 /** accumulated expected cases */
    count_t               gtTotalCases;              /** total cases */
    measure_t             gtTotalMeasure;            /** total expected cases */

    virtual void          Assign(const AbstractSpatialClusterData& rhs);
    virtual SpatialData * Clone() const;
    SpatialData         & operator=(const SpatialData& rhs);

    inline /*virtual*/ void   AddMeasureList(tract_t tCentroidIndex, const DataStreamInterface& Interface,
                                             CMeasureList* pMeasureList, tract_t tNumNeighbors,
                                             unsigned short** ppSorted_UShort_T, tract_t** ppSorted_Tract_T);
    virtual void          AddNeighborData(tract_t tNeighborIndex, const AbtractDataStreamGateway& DataGateway, size_t tSetIndex=0);
    virtual double        CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator);
    virtual count_t       GetCaseCount(unsigned int tSetIndex=0) const;
    virtual measure_t     GetMeasure(unsigned int tSetIndex=0) const;
    inline virtual void   InitializeData();
};

/** adds neighbor data to accumulation and updates measure list */
inline void SpatialData::AddMeasureList(tract_t tCentroidIndex,
                                        const DataStreamInterface& Interface, CMeasureList* pMeasureList,
                                        tract_t tNumNeighbors, unsigned short** ppSorted_UShort_T,
                                        tract_t** ppSorted_Tract_T) {
  tract_t       t, tNeighborIndex;

  gtCases=0;gtMeasure=0; //initialize data
  for (t=0; t < tNumNeighbors; ++t) {
    tNeighborIndex = (ppSorted_UShort_T ? (tract_t)ppSorted_UShort_T[tCentroidIndex][t] : ppSorted_Tract_T[tCentroidIndex][t]);
    gtCases += Interface.gpPSCaseArray[tNeighborIndex];
    gtMeasure += Interface.gpPSMeasureArray[tNeighborIndex];
    pMeasureList->AddMeasure(gtCases, gtMeasure);
  }
}

/** initializes accumulated data to zero */
inline void SpatialData::InitializeData() {gtCases=0;gtMeasure=0;}

/** Class representing accumulated data of temporal clustering
    If instantiated through public constructors, points to already calculated
    purely temporal arrays supplied by DataInterface. The protected constructor
    is intended to permit instantiation through a derived class, where perhaps
    pointers will be allocated and data supplied by some other process. */
class TemporalData : public AbstractTemporalClusterData {
  protected:
    TemporalData();

  public:
    TemporalData(const DataStreamInterface& Interface);
    TemporalData(const AbtractDataStreamGateway& DataGateway);
    virtual ~TemporalData();

    virtual void                Assign(const AbstractTemporalClusterData& rhs);
    virtual TemporalData      * Clone() const;
    TemporalData              & operator=(const TemporalData& rhs);

    count_t                     gtCases;
    measure_t                   gtMeasure;
    count_t                   * gpCases;
    measure_t                 * gpMeasure;
    count_t                     gtTotalCases;
    measure_t                   gtTotalMeasure;

    virtual void                AddNeighborData(tract_t tNeighborIndex, const AbtractDataStreamGateway& DataGateway, size_t tSetIndex=0);
    virtual unsigned int        GetAllocationSize() const;
    virtual count_t             GetCaseCount(unsigned int tSetIndex=0) const;
    virtual measure_t           GetMeasure(unsigned int tSetIndex=0) const;
    virtual void                InitializeData() {gtCases=0;gtMeasure=0;}
};

/** Class representing accumulated data of prospective spatial clustering
    For spatial cluster data, in a prospective analysis, the supposed study
    period does not necessarily remain fixed but changes with the prospective
    end date. This class represents that 'spatial' data clustering. */
class ProspectiveSpatialData : public TemporalData {
  private:
     void                            Init() {gpCases=0;gpMeasure=0;}
     void                            Setup(const CSaTScanData & Data, const DataStreamInterface& Interface);

  protected:
     unsigned int                    giAllocationSize;         /** size of allocated arrays */
     unsigned int                    giNumTimeIntervals;       /** number of time intervals in study period */
     unsigned int                    giProspectiveStart;       /** index of prospective start date in DataInterface case array */
     RATE_FUNCPTRTYPE                gfRateOfInterest;         /** function pointer to 'rate of interest' function */

  public:
    ProspectiveSpatialData(const CSaTScanData& Data, const DataStreamInterface& Interface);
    ProspectiveSpatialData(const CSaTScanData& Data, const AbtractDataStreamGateway& DataGateway);
    ProspectiveSpatialData(const ProspectiveSpatialData& rhs);
    virtual ~ProspectiveSpatialData();

    virtual void                     Assign(const AbstractTemporalClusterData& rhs);
    virtual ProspectiveSpatialData * Clone() const;
    ProspectiveSpatialData         & operator=(const ProspectiveSpatialData& rhs);

    /*virtual*/ void                 AddMeasureList(tract_t tCentroidIndex, const DataStreamInterface& Interface,
                                                    CMeasureList* pMeasureList, tract_t tNumNeighbors,
                                                    unsigned short** ppSorted_UShort_T, tract_t** ppSorted_Tract_T);
    virtual void                     AddNeighborData(tract_t tNeighborIndex, const AbtractDataStreamGateway& DataGateway, size_t tSetIndex=0);
    virtual double                   CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator);
    virtual unsigned int             GetAllocationSize() const {return giAllocationSize;}
    inline virtual void              InitializeData();
};

/** re-initialize data */
inline void ProspectiveSpatialData::InitializeData() {
  gtCases=0;
  gtMeasure=0;
  memset(gpCases, 0, sizeof(count_t) * giAllocationSize);
  memset(gpMeasure, 0, sizeof(measure_t) * giAllocationSize);
}

class CTimeIntervals; /** forward class declaration */
/** class representing accumulated data of space-time clustering */
class SpaceTimeData : public TemporalData {
  private:
     void                       Init() {gpCases=0;gpMeasure=0;}
     void                       Setup(const DataStreamInterface& Interface);

  protected:
     unsigned int               giAllocationSize;

  public:
    SpaceTimeData(const DataStreamInterface& Interface);
    SpaceTimeData(const AbtractDataStreamGateway& DataGateway);
    SpaceTimeData(const SpaceTimeData& rhs);
    virtual ~SpaceTimeData();

    virtual void                Assign(const AbstractTemporalClusterData& rhs);
    virtual SpaceTimeData     * Clone() const;
    SpaceTimeData             & operator=(const SpaceTimeData& rhs);

    /*virtual*/ void            AddNeighborDataAndCompare(tract_t tCentroidIndex,
                                                          const DataStreamInterface& Interface,
                                                          tract_t tNumNeighbors,
                                                          unsigned short** ppSorted_UShort_T,
                                                          tract_t** ppSorted_Tract_T,
                                                          CTimeIntervals& TimeIntervals,
                                                          CMeasureList& MeasureList);
    virtual void                AddNeighborData(tract_t tNeighborIndex, const AbtractDataStreamGateway & DataGateway, size_t tSetIndex=0);
    inline virtual void         InitializeData();
};

/** re-initialize data*/
inline void SpaceTimeData::InitializeData() {
  gtCases=0;
  gtMeasure=0;
  memset(gpCases, 0, sizeof(count_t) * giAllocationSize);
  memset(gpMeasure, 0, sizeof(measure_t) * giAllocationSize);
}
//******************************************************************************
#endif

