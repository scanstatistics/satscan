//---------------------------------------------------------------------------
#ifndef NormalClusterDataH
#define NormalClusterDataH
//---------------------------------------------------------------------------
#include "ClusterData.h"

/** class representing accumulated data of spatial clustering of a normal probability model */
class NormalSpatialData : public SpatialData {
  public:
    NormalSpatialData(const AbtractDataStreamGateway & DataGateway, int iRate);
    virtual ~NormalSpatialData();

    //public data memebers
    measure_t                   gtSqMeasure;      /** expected number of cases - squared measure */

    virtual void                Assign(const AbstractSpatialClusterData& rhs);
    virtual NormalSpatialData * Clone() const;

    virtual void                AddMeasureList(const DataStreamInterface & Interface, CMeasureList * pMeasureList, const CSaTScanData * pData);
    virtual void                AddNeighborData(tract_t tNeighbor, const AbtractDataStreamGateway & DataGateway, size_t tStream=0);
    virtual double              CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator & Calculator);
    virtual void                InitializeData() {gtCases=0;gtMeasure=0;gtSqMeasure=0;}
};

/** class representing accumulated data of temporal clustering for normal probability model
    If instantiated through public constructors, points to already calculated
    purely temporal arrays supplied by DataInterface. The protected constructor
    is intended to permit instantiation through a derived class, where perhaps
    pointers will be allocated and data supplied by some other process. */
class NormalTemporalData : public TemporalData {
  protected:
    NormalTemporalData();

  public:
    NormalTemporalData(const DataStreamInterface & Interface);
    NormalTemporalData(const AbtractDataStreamGateway & DataGateway);
    virtual ~NormalTemporalData();

    virtual void                 Assign(const AbstractTemporalClusterData& rhs);
    virtual NormalTemporalData * Clone() const;

    measure_t                    gtSqMeasure;
    measure_t                  * gpSqMeasure;

    virtual void                 InitializeData() {gtCases=0;gtMeasure=0;gtSqMeasure=0;}
};

/** class representing accumulated data of prospective spatial clustering  for normal probability model
    For spatial cluster data, in a prospective analysis, the supposed study
    period does not necessarily remain fixed but changes with the prospective
    end date. This class represents that 'spatial' data clustering. */
class NormalProspectiveSpatialData : public NormalTemporalData {
  private:
     void                                  Init() {gpCases=0;gpMeasure=0;gpSqMeasure=0;}
     void                                  Setup(const CSaTScanData & Data, const DataStreamInterface & Interface);

  protected:
     unsigned int                          giAllocationSize;    /** size of allocated arrays */
     unsigned int                          giNumTimeIntervals;  /** number of time intervals in study period */
     unsigned int                          giProspectiveStart;  /** index of prospective start date in DataInterface case array */
     RATE_FUNCPTRTYPE                      gfRateOfInterest;    /** function pointer to 'rate of interest' function */

  public:
    NormalProspectiveSpatialData(const CSaTScanData & Data, const DataStreamInterface & Interface);
    NormalProspectiveSpatialData(const CSaTScanData & Data, const AbtractDataStreamGateway & DataGateway);
    NormalProspectiveSpatialData(const NormalProspectiveSpatialData& rhs);
    virtual ~NormalProspectiveSpatialData();

    virtual void                           Assign(const AbstractTemporalClusterData& rhs);
    virtual NormalProspectiveSpatialData * Clone() const;
    NormalProspectiveSpatialData         & operator=(const NormalProspectiveSpatialData& rhs);

    virtual void                           AddNeighborData(tract_t tNeighbor, const AbtractDataStreamGateway & DataGateway, size_t tStream=0);
    virtual double                         CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator & Calculator);
    virtual void                           InitializeData();
};

/** class representing accumulated data of space-time clustering for normal probability model */
class NormalSpaceTimeData : public NormalTemporalData {
  private:
     void                         Init() {gpCases=0;gpMeasure=0;gpSqMeasure=0;}
     void                         Setup(const DataStreamInterface & Interface);

  protected:
     unsigned int                 giAllocationSize;  /** size of allocated arrays */

  public:
    NormalSpaceTimeData(const DataStreamInterface & Interface);
    NormalSpaceTimeData(const AbtractDataStreamGateway & DataGateway);
    NormalSpaceTimeData(const NormalSpaceTimeData& rhs);
    virtual ~NormalSpaceTimeData();

    virtual void                  Assign(const AbstractTemporalClusterData& rhs);
    virtual NormalSpaceTimeData * Clone() const;
    NormalSpaceTimeData         & operator=(const NormalSpaceTimeData& rhs);

    virtual void                  AddNeighborData(tract_t tNeighbor, const AbtractDataStreamGateway & DataGateway, size_t tStream=0);
    virtual void                  InitializeData();
};
//---------------------------------------------------------------------------
#endif
