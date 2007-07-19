//******************************************************************************
#ifndef __ClusterData_H
#define __ClusterData_H
//******************************************************************************
#include "AbstractClusterData.h"
#include "MeasureList.h"
#include "Toolkit.h"

/** Class representing accumulated data of spatial clustering. */
class SpatialData : public AbstractSpatialClusterData {
  public:
    SpatialData(const DataSetInterface& Interface);
    SpatialData(const AbstractDataSetGateway& DataGateway);
    virtual ~SpatialData() {}

    //public data members -- public for speed considerations
    count_t               gtCases;                   /** accumulated cases */
    measure_t             gtMeasure;                 /** accumulated expected cases */

    inline void           AddMeasureList(const CentroidNeighbors& CentroidDef, const DataSetInterface& Interface, CMeasureList* pMeasureList);
    virtual void          AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual void          Assign(const AbstractSpatialClusterData& rhs);
    virtual double        CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator);
    virtual SpatialData * Clone() const;
    virtual void          CopyEssentialClassMembers(const AbstractClusterData& rhs);
    virtual count_t       GetCaseCount(unsigned int tSetIndex=0) const;
    virtual double        GetMaximizingValue(AbstractLikelihoodCalculator& Calculator);
    virtual measure_t     GetMeasure(unsigned int tSetIndex=0) const;
    SpatialData         & operator=(const SpatialData& rhs);
    virtual void          InitializeData() {gtCases=0;gtMeasure=0;}
};

/** adds neighbor data to accumulation and updates measure list */
inline void SpatialData::AddMeasureList(const CentroidNeighbors& CentroidDef, const DataSetInterface& Interface, CMeasureList* pMeasureList) {
  macroRunTimeStartFocused(FocusRunTimeComponent::MeasureListScanningAdding);
  tract_t               t, tNeighborIndex, tNumNeighbors=CentroidDef.GetNumNeighbors(),
                      * pIntegerArray = CentroidDef.GetRawIntegerArray();
  unsigned short      * pUnsignedShortArray = CentroidDef.GetRawUnsignedShortArray();

  gtCases=0;gtMeasure=0; //initialize data
  for (t=0; t < tNumNeighbors; ++t) {
    tNeighborIndex = (pUnsignedShortArray ? (tract_t)pUnsignedShortArray[t] : pIntegerArray[t]);
    gtCases += Interface.gpPSCaseArray[tNeighborIndex];
    gtMeasure += Interface.gpPSMeasureArray[tNeighborIndex];
    pMeasureList->AddMeasure(gtCases, gtMeasure);
  }
  macroRunTimeStopFocused(FocusRunTimeComponent::MeasureListScanningAdding);
}

/** Class representing accumulated data of spatial monotone clustering. */
class SpatialMonotoneData : public AbstractClusterData {
  private:
    RATE_FUNCPTRTYPE       m_pfRateOfInterest;   /** scanning rate function pointer */
    tract_t                m_NeighborIteration;  /** total number of tracts added */
    count_t                gtCases;              /** accumulated cases */
    measure_t              gtMeasure;            /** accumulated expected cases */

  public:
    SpatialMonotoneData(const DataSetInterface& Interface);
    SpatialMonotoneData(const AbstractDataSetGateway& DataGateway);
    virtual ~SpatialMonotoneData() {}
    virtual SpatialMonotoneData * Clone() const;
    SpatialMonotoneData & operator=(const SpatialMonotoneData& rhs);

    //public data members -- public for speed considerations
    std::vector<count_t>   gvCasesList;          /** Number of cases in each circle */
    std::vector<measure_t> gvMeasureList;        /** Expected count for each circle */
    std::vector<tract_t>   gvFirstNeighborList;  /** First neighbor in circle */
    std::vector<tract_t>   gvLastNeighborList;   /** Last neighbor in circle */
    tract_t                m_nSteps;             /** Number of concentric steps in cluster */

    virtual void           AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    void                   AddRemainder(const CSaTScanData& Data);
    void                   AllocateForMaxCircles(tract_t nCircles);
    virtual void           Assign(const AbstractClusterData& rhs);
    virtual double         CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator);
    void                   CheckCircle(tract_t n);
    void                   ConcatLastCircles();
    virtual void           CopyEssentialClassMembers(const AbstractClusterData& rhs);
    virtual count_t        GetCaseCount(unsigned int tSetIndex=0) const;
    virtual double         GetMaximizingValue(AbstractLikelihoodCalculator& Calculator);
    virtual measure_t      GetMeasure(unsigned int tSetIndex=0) const;
    tract_t                GetLastCircleIndex() const {return m_nSteps-1;}
    virtual void           InitializeData();
    void                   RemoveRemainder(const CSaTScanData& Data);
    void                   SetCasesAndMeasures();
    void                   SetRate(AreaRateType eRate);
};

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
    virtual ~TemporalData() {}

    count_t                     gtCases;
    measure_t                   gtMeasure;
    count_t                   * gpCases;
    measure_t                 * gpMeasure;

    virtual void                AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual void                Assign(const AbstractTemporalClusterData& rhs);
    virtual unsigned int        GetAllocationSize() const;
    virtual count_t             GetCaseCount(unsigned int tSetIndex=0) const;
    virtual measure_t           GetMeasure(unsigned int tSetIndex=0) const;
    virtual TemporalData      * Clone() const;
    virtual void                CopyEssentialClassMembers(const AbstractClusterData& rhs);
    virtual void                InitializeData() {gtCases=0;gtMeasure=0;}
    TemporalData              & operator=(const TemporalData& rhs);
    virtual void                Reassociate(const DataSetInterface& Interface);
    virtual void                Reassociate(const AbstractDataSetGateway& DataGateway);
};

/** Class representing accumulated data of prospective spatial clustering
    For spatial cluster data, in a prospective analysis, the supposed study
    period does not necessarily remain fixed but changes with the prospective
    end date. This class represents that 'spatial' data clustering. */
class ProspectiveSpatialData : public TemporalData, public AbstractProspectiveSpatialClusterData {
  private:
     void                            Init() {gpCases=0;gpMeasure=0;}
     void                            Setup(const CSaTScanData & Data, const DataSetInterface& Interface);

  protected:
     EvaluationAssistDataStatus      geEvaluationAssistDataStatus;
     unsigned int                    giAllocationSize;         /** size of allocated arrays */
     unsigned int                    giNumTimeIntervals;       /** number of time intervals in study period */
     unsigned int                    giProspectiveStart;       /** index of prospective start date in DataInterface case array */

  public:
    ProspectiveSpatialData(const CSaTScanData& Data, const DataSetInterface& Interface);
    ProspectiveSpatialData(const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway);
    ProspectiveSpatialData(const ProspectiveSpatialData& rhs);
    virtual ~ProspectiveSpatialData();

    void                             AddMeasureList(const CentroidNeighbors& CentroidDef, const DataSetInterface& Interface, CMeasureList* pMeasureList);
    virtual void                     AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual void                     Assign(const AbstractTemporalClusterData& rhs);
    virtual double                   CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator);
    virtual ProspectiveSpatialData * Clone() const;
    virtual void                     DeallocateEvaluationAssistClassMembers();
    virtual unsigned int             GetAllocationSize() const {return giAllocationSize;}
    virtual double                   GetMaximizingValue(AbstractLikelihoodCalculator& Calculator);
    inline virtual void              InitializeData();
    ProspectiveSpatialData         & operator=(const ProspectiveSpatialData& rhs);
    virtual void                     Reassociate(const DataSetInterface& Interface) {/*nop*/}
    virtual void                     Reassociate(const AbstractDataSetGateway& DataGateway) {/*nop*/}
};

/** re-initialize data */
inline void ProspectiveSpatialData::InitializeData() {
  assert(geEvaluationAssistDataStatus == Allocated);
  gtCases=0;
  gtMeasure=0;
  memset(gpCases, 0, sizeof(count_t) * giAllocationSize);
  memset(gpMeasure, 0, sizeof(measure_t) * giAllocationSize);
}

class CTimeIntervals; /** forward class declaration */
/** class representing accumulated data of space-time clustering */
class SpaceTimeData : public TemporalData {
  private:
     EvaluationAssistDataStatus geEvaluationAssistDataStatus;
     
     void                       Init() {gpCases=0;gpMeasure=0;}
     void                       Setup(const DataSetInterface& Interface);

  protected:
     unsigned int               giAllocationSize;

  public:
    SpaceTimeData(const DataSetInterface& Interface);
    SpaceTimeData(const AbstractDataSetGateway& DataGateway);
    SpaceTimeData(const SpaceTimeData& rhs);
    virtual ~SpaceTimeData();

    virtual void                AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway & DataGateway, size_t tSetIndex=0);
    /*virtual*/ void            AddNeighborDataAndCompare(const CentroidNeighbors& CentroidDef,
                                                          const DataSetInterface& Interface,
                                                          CTimeIntervals& TimeIntervals,
                                                          CMeasureList& MeasureList);
    virtual void                Assign(const AbstractTemporalClusterData& rhs);
    virtual SpaceTimeData     * Clone() const;
    virtual void                DeallocateEvaluationAssistClassMembers();
    inline virtual void         InitializeData();
    SpaceTimeData             & operator=(const SpaceTimeData& rhs);
    virtual void                Reassociate(const DataSetInterface& Interface) {/*nop*/}
    virtual void                Reassociate(const AbstractDataSetGateway& DataGateway) {/*nop*/}
};

/** re-initialize data*/
inline void SpaceTimeData::InitializeData() {
  assert(geEvaluationAssistDataStatus == Allocated);
  gtCases=0;
  gtMeasure=0;
  memset(gpCases, 0, sizeof(count_t) * giAllocationSize);
  memset(gpMeasure, 0, sizeof(measure_t) * giAllocationSize);
}
//******************************************************************************
#endif

