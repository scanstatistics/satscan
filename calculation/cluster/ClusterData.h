//---------------------------------------------------------------------------
#ifndef ClusterDataH
#define ClusterDataH
//---------------------------------------------------------------------------
#include "AbstractClusterData.h"
#include "MeasureList.h"

/** class representing accumulated data of spatial clustering */
class SpatialData : public AbstractSpatialClusterData {
  public:
    SpatialData(const DataStreamInterface & Interface, int iRate);
    SpatialData(const AbtractDataStreamGateway & DataGateway, int iRate);
    virtual ~SpatialData();

    //public data members -- public for speed considerations
    count_t               gtCases;                   /** accumulated cases */
    measure_t             gtMeasure;                 /** accumulated expected cases */
    count_t               gtTotalCases;              /** total cases */
    measure_t             gtTotalMeasure;            /** total expected cases */

    virtual void          Assign(const AbstractSpatialClusterData& rhs);
    virtual SpatialData * Clone() const;
    SpatialData         & operator=(const SpatialData& rhs);

    inline virtual void   AddMeasureList(const DataStreamInterface & Interface, CMeasureList * pMeasureList, const CSaTScanData * pData);
    virtual void          AddNeighborData(tract_t tNeighbor, const AbtractDataStreamGateway & DataGateway, size_t tStream=0);
    virtual double        CalculateLoglikelihoodRatio(CModel & Model);
    virtual count_t       GetCaseCount(unsigned int iStream=0) const;
    virtual measure_t     GetMeasure(unsigned int iStream=0) const;
    inline virtual void   InitializeData();
};

/** adds neighbor data to accumulation and updates measure list */
inline void SpatialData::AddMeasureList(const DataStreamInterface & Interface, CMeasureList * pMeasureList, const CSaTScanData * pData) {
  tract_t       t, tNeighbor, tNumNeighbors = pData->GetImpliedNeighborCount();

  InitializeData(); //replace with direct code ?
  for (t=1; t <= tNumNeighbors; ++t) {
    tNeighbor = pData->GetNeighborTractIndex(t);
    gtCases += Interface.gpPSCaseArray[tNeighbor];
    gtMeasure += Interface.gpPSMeasureArray[tNeighbor];
    pMeasureList->AddMeasure(gtCases, gtMeasure);
  }
}

/** initializes accumulated data to zero */
inline void SpatialData::InitializeData() {gtCases=0;gtMeasure=0;}

/** class representing accumulated data of temporal clustering
    If instantiated through public constructors, points to already calculated
    purely temporal arrays supplied by DataInterface. The protected constructor
    is intended to permit instantiation through a derived class, where perhaps
    pointers will be allocated and data supplied by some other process. */
class TemporalData : public AbstractTemporalClusterData {
  protected:
    TemporalData();

  public:
    TemporalData(const DataStreamInterface & Interface);
    TemporalData(const AbtractDataStreamGateway & DataGateway);
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

    virtual void                AddNeighborData(tract_t tNeighbor, const AbtractDataStreamGateway & DataGateway, size_t tStream=0);
    virtual unsigned int        GetAllocationSize() const;
    virtual count_t             GetCaseCount(unsigned int iStream=0) const;
    virtual measure_t           GetMeasure(unsigned int iStream=0) const;
    virtual void                InitializeData() {gtCases=0;gtMeasure=0;}
    virtual void                SetCaseCount(count_t tCases, unsigned int iStream=0) {gtCases = tCases;}
    virtual void                SetMeasure(measure_t tMeasure, unsigned int iStream=0) {gtMeasure = tMeasure;}
};

/** class representing accumulated data of prospective spatial clustering
    For spatial cluster data, in a prospective analysis, the supposed study
    period does not necessarily remain fixed but changes with the prospective
    end date. This class represents that 'spatial' data clustering. */
class ProspectiveSpatialData : public TemporalData {
  private:
     void                            Init() {gpCases=0;gpMeasure=0;}
     void                            Setup(const CSaTScanData & Data, const DataStreamInterface & Interface);

  protected:
     unsigned int                    giAllocationSize;         /** size of allocated arrays */
     unsigned int                    giNumTimeIntervals;       /** number of time intervals in study period */
     unsigned int                    giProspectiveStart;       /** index of prospective start date in DataInterface case array */
     RATE_FUNCPTRTYPE                gfRateOfInterest;         /** function pointer to 'rate of interest' function */

  public:
    ProspectiveSpatialData(const CSaTScanData & Data, const DataStreamInterface & Interface);
    ProspectiveSpatialData(const CSaTScanData & Data, const AbtractDataStreamGateway & DataGateway);
    ProspectiveSpatialData(const ProspectiveSpatialData& rhs);
    virtual ~ProspectiveSpatialData();

    virtual void                     Assign(const AbstractTemporalClusterData& rhs);
    virtual ProspectiveSpatialData * Clone() const;
    ProspectiveSpatialData         & operator=(const ProspectiveSpatialData& rhs);

    virtual void                     AddMeasureList(const DataStreamInterface & Interface, CMeasureList * pMeasureList, const CSaTScanData * pData);
    virtual void                     AddNeighborData(tract_t tNeighbor, const AbtractDataStreamGateway & DataGateway, size_t tStream=0);
    virtual double                   CalculateLoglikelihoodRatio(CModel & Model);
    virtual unsigned int             GetAllocationSize() const {return giAllocationSize;}
    virtual void                     InitializeData();
};

class CTimeIntervals; /** forward class declaration */
/** class representing accumulated data of space-time clustering */
class SpaceTimeData : public TemporalData {
  private:
     void                       Init() {gpCases=0;gpMeasure=0;}
     void                       Setup(const DataStreamInterface & Interface);

  protected:
     unsigned int               giAllocationSize;

  public:
    SpaceTimeData(const DataStreamInterface & Interface);
    SpaceTimeData(const AbtractDataStreamGateway & DataGateway);
    SpaceTimeData(const SpaceTimeData& rhs);
    virtual ~SpaceTimeData();

    virtual void                Assign(const AbstractTemporalClusterData& rhs);
    virtual SpaceTimeData     * Clone() const;
    SpaceTimeData             & operator=(const SpaceTimeData& rhs);

    virtual void                AddNeighborDataAndCompare(const DataStreamInterface & Interface,
                                                          const CSaTScanData * pData,
                                                          CTimeIntervals * pTimeIntervals,
                                                          CMeasureList * pMeasureList);
    virtual void                AddNeighborData(tract_t tNeighbor, const AbtractDataStreamGateway & DataGateway, size_t tStream=0);
    virtual void                InitializeData();
};
//---------------------------------------------------------------------------
#endif
