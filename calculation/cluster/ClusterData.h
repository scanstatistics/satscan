//******************************************************************************
#ifndef __ClusterData_H
#define __ClusterData_H
//******************************************************************************
#include "AbstractClusterData.h"
#include "MeasureList.h"

/** Class representing accumulated data of spatial clustering. */
class SpatialData : public AbstractSpatialClusterData {
  public:
    SpatialData(const DataSetInterface& Interface, int iRate);
    SpatialData(const AbstractDataSetGateway& DataGateway, int iRate);
    virtual ~SpatialData();

    //public data members -- public for speed considerations
    count_t               gtCases;                   /** accumulated cases */
    measure_t             gtMeasure;                 /** accumulated expected cases */
    count_t               gtTotalCases;              /** total cases */
    measure_t             gtTotalMeasure;            /** total expected cases */

    virtual void          Assign(const AbstractSpatialClusterData& rhs);
    virtual SpatialData * Clone() const;
    SpatialData         & operator=(const SpatialData& rhs);

    inline /*virtual*/ void   AddMeasureList(const CentroidNeighbors& CentroidDef, const DataSetInterface& Interface, CMeasureList* pMeasureList);
    virtual void          AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual double        CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator);
    virtual count_t       GetCaseCount(unsigned int tSetIndex=0) const;
    virtual measure_t     GetMeasure(unsigned int tSetIndex=0) const;
    inline virtual void   InitializeData();
};

/** adds neighbor data to accumulation and updates measure list */
inline void SpatialData::AddMeasureList(const CentroidNeighbors& CentroidDef, const DataSetInterface& Interface, CMeasureList* pMeasureList) {
  macroRunTimeStartFocused(FocusRunTimeComponent::AddingMeasureList);

  tract_t       t, tNeighborIndex;

  gtCases=0;gtMeasure=0; //initialize data
  for (t=0; t < CentroidDef.GetNumNeighbors(); ++t) {
    tNeighborIndex = CentroidDef.GetNeighborTractIndex(t);
    gtCases += Interface.gpPSCaseArray[tNeighborIndex];
    gtMeasure += Interface.gpPSMeasureArray[tNeighborIndex];
    pMeasureList->AddMeasure(gtCases, gtMeasure);
  }

  macroRunTimeStopFocused(FocusRunTimeComponent::AddingMeasureList);  
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
    TemporalData(const DataSetInterface& Interface);
    TemporalData(const AbstractDataSetGateway& DataGateway);
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

    virtual void                AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual unsigned int        GetAllocationSize() const;
    virtual count_t             GetCaseCount(unsigned int tSetIndex=0) const;
    virtual measure_t           GetMeasure(unsigned int tSetIndex=0) const;
    virtual void                InitializeData() {gtCases=0;gtMeasure=0;}
    virtual void                Reassociate(const DataSetInterface& Interface);
    virtual void                Reassociate(const AbstractDataSetGateway& DataGateway);
};

/** Class representing accumulated data of prospective spatial clustering
    For spatial cluster data, in a prospective analysis, the supposed study
    period does not necessarily remain fixed but changes with the prospective
    end date. This class represents that 'spatial' data clustering. */
class ProspectiveSpatialData : public TemporalData {
  private:
     void                            Init() {gpCases=0;gpMeasure=0;}
     void                            Setup(const CSaTScanData & Data, const DataSetInterface& Interface);

  protected:
     unsigned int                    giAllocationSize;         /** size of allocated arrays */
     unsigned int                    giNumTimeIntervals;       /** number of time intervals in study period */
     unsigned int                    giProspectiveStart;       /** index of prospective start date in DataInterface case array */
     RATE_FUNCPTRTYPE                gfRateOfInterest;         /** function pointer to 'rate of interest' function */

  public:
    ProspectiveSpatialData(const CSaTScanData& Data, const DataSetInterface& Interface);
    ProspectiveSpatialData(const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway);
    ProspectiveSpatialData(const ProspectiveSpatialData& rhs);
    virtual ~ProspectiveSpatialData();

    virtual void                     Assign(const AbstractTemporalClusterData& rhs);
    virtual ProspectiveSpatialData * Clone() const;
    ProspectiveSpatialData         & operator=(const ProspectiveSpatialData& rhs);

    /*virtual*/ void                 AddMeasureList(const CentroidNeighbors& CentroidDef, const DataSetInterface& Interface, CMeasureList* pMeasureList);
    virtual void                     AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual double                   CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator);
    virtual unsigned int             GetAllocationSize() const {return giAllocationSize;}
    inline virtual void              InitializeData();
    virtual void                     Reassociate(const DataSetInterface& Interface) {/*nop*/}
    virtual void                     Reassociate(const AbstractDataSetGateway& DataGateway) {/*nop*/}
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
     void                       Setup(const DataSetInterface& Interface);

  protected:
     unsigned int               giAllocationSize;

  public:
    SpaceTimeData(const DataSetInterface& Interface);
    SpaceTimeData(const AbstractDataSetGateway& DataGateway);
    SpaceTimeData(const SpaceTimeData& rhs);
    virtual ~SpaceTimeData();

    virtual void                Assign(const AbstractTemporalClusterData& rhs);
    virtual SpaceTimeData     * Clone() const;
    SpaceTimeData             & operator=(const SpaceTimeData& rhs);

    /*virtual*/ void            AddNeighborDataAndCompare(const CentroidNeighbors& CentroidDef,
                                                          const DataSetInterface& Interface,
                                                          CTimeIntervals& TimeIntervals,
                                                          CMeasureList& MeasureList);
    virtual void                AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway & DataGateway, size_t tSetIndex=0);
    inline virtual void         InitializeData();
    virtual void                Reassociate(const DataSetInterface& Interface) {/*nop*/}
    virtual void                Reassociate(const AbstractDataSetGateway& DataGateway) {/*nop*/}
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

