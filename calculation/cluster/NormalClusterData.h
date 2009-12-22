//******************************************************************************
#ifndef __NormalClusterData_H
#define __NormalClusterData_H
//******************************************************************************
#include "ClusterData.h"

class ColumnVector;
class Matrix;

/**Abstract base class for all normal cluster data. */
class AbstractNormalClusterData {
  public:
    AbstractNormalClusterData() {}
    virtual ~AbstractNormalClusterData() {}

    virtual measure_t        GetMeasureAux(unsigned int tSetIndex) const = 0;
};
/** Class representing accumulated data of spatial clustering of a normal probability model. */
class NormalSpatialData : public SpatialData, public AbstractNormalClusterData {
  protected:
    NormalSpatialData();

  public:
    NormalSpatialData(const DataSetInterface& Interface);
    NormalSpatialData(const AbstractDataSetGateway& DataGateway);
    virtual ~NormalSpatialData() {}

    //public data memebers
    measure_t                gtMeasureAux;      /** expected number of cases - squared measure */

    virtual void             AddMeasureList(const DataSetInterface& Interface, CMeasureList* pMeasureList, const CSaTScanData* pData);
    virtual void             AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual void             Assign(const AbstractSpatialClusterData& rhs);
    virtual double           CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator);
    virtual NormalSpatialData * Clone() const;
    virtual void             CopyEssentialClassMembers(const AbstractClusterData& rhs);
    virtual double           GetMaximizingValue(AbstractLikelihoodCalculator& Calculator);
    virtual measure_t        GetMeasureAux(unsigned int tSetIndex=0) const {return gtMeasureAux;}
    virtual void             InitializeData() {gtCases=0;gtMeasure=0;gtMeasureAux=0;}
};

/** Class representing accumulated data of temporal clustering for normal probability model.
    If instantiated through public constructors, points to already calculated
    purely temporal arrays supplied by DataInterface. The protected constructor
    is intended to permit instantiation through a derived class, where perhaps
    pointers will be allocated and data supplied by some other process. */
class NormalTemporalData : public TemporalData, public AbstractNormalClusterData {
  protected:
    NormalTemporalData();

  public:
    NormalTemporalData(const DataSetInterface& DataGateway);
    NormalTemporalData(const AbstractDataSetGateway& DataGateway);
    virtual ~NormalTemporalData() {}

    measure_t                    gtMeasureAux;
    measure_t                  * gpMeasureAux;

    virtual void             Assign(const AbstractTemporalClusterData& rhs);
    virtual void             CopyEssentialClassMembers(const AbstractClusterData& rhs);
    virtual NormalTemporalData * Clone() const;
    virtual void             InitializeData() {gtCases=0;gtMeasure=0;gtMeasureAux=0;}
    virtual measure_t        GetMeasureAux(unsigned int tSetIndex=0) const {return gtMeasureAux;}
    virtual void             Reassociate(const DataSetInterface& Interface);
    virtual void             Reassociate(const AbstractDataSetGateway& DataGateway);
};

/** class representing accumulated data of prospective spatial clustering  for normal probability model
    For spatial cluster data, in a prospective analysis, the supposed study
    period does not necessarily remain fixed but changes with the prospective
    end date. This class represents that 'spatial' data clustering. */
class NormalProspectiveSpatialData : public NormalTemporalData, public AbstractProspectiveSpatialClusterData {
  private:
     void                                  Init() {gpCases=0;gpMeasure=0;gpMeasureAux=0;}
     void                                  Setup(const CSaTScanData& Data, const DataSetInterface& Interface);

  protected:
     EvaluationAssistDataStatus            geEvaluationAssistDataStatus;
     unsigned int                          giAllocationSize;    /** size of allocated arrays */
     unsigned int                          giNumTimeIntervals;  /** number of time intervals in study period */
     unsigned int                          giProspectiveStart;  /** index of prospective start date in DataInterface case array */

  public:
    NormalProspectiveSpatialData(const CSaTScanData& Data, const DataSetInterface& Interface);
    NormalProspectiveSpatialData(const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway);
    NormalProspectiveSpatialData(const NormalProspectiveSpatialData& rhs);
    virtual ~NormalProspectiveSpatialData();

    virtual void             AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual void             Assign(const AbstractTemporalClusterData& rhs);
    virtual double           CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator);
    virtual NormalProspectiveSpatialData * Clone() const;
    virtual void             DeallocateEvaluationAssistClassMembers();
    virtual double           GetMaximizingValue(AbstractLikelihoodCalculator& Calculator);
    virtual void             InitializeData();
    NormalProspectiveSpatialData & operator=(const NormalProspectiveSpatialData& rhs);
    virtual void             Reassociate(const DataSetInterface& Interface) {/*nop*/}
    virtual void             Reassociate(const AbstractDataSetGateway& DataGateway) {/*nop*/}
};

/** Class representing accumulated data of space-time clustering for normal probability model. */
class NormalSpaceTimeData : public NormalTemporalData {
  private:
     void                         Setup(const DataSetInterface & Interface);

  protected:
     EvaluationAssistDataStatus   geEvaluationAssistDataStatus;
     unsigned int                 giAllocationSize;  /** size of allocated arrays */

  public:
    NormalSpaceTimeData(const DataSetInterface& Interface);
    NormalSpaceTimeData(const AbstractDataSetGateway& DataGateway);
    NormalSpaceTimeData(const NormalSpaceTimeData& rhs);
    virtual ~NormalSpaceTimeData();

    virtual void             AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual void             Assign(const AbstractTemporalClusterData& rhs);
    virtual NormalSpaceTimeData * Clone() const;
    virtual void             DeallocateEvaluationAssistClassMembers();
    virtual void             InitializeData();
    NormalSpaceTimeData    & operator=(const NormalSpaceTimeData& rhs);
    virtual void             Reassociate(const DataSetInterface& Interface) {/*nop*/}
    virtual void             Reassociate(const AbstractDataSetGateway& DataGateway) {/*nop*/}
};

/** Class representing accumulated data of spatial clustering of a normal probability model
    with data that specifies weight and covariates of observations. */
class NormalCovariateSpatialData : public NormalSpatialData {
  private:
     void                Setup(const DataSetInterface& Interface);

public:
    NormalCovariateSpatialData(const DataSetInterface& Interface);
    NormalCovariateSpatialData(const AbstractDataSetGateway& DataGateway);
    NormalCovariateSpatialData(const NormalCovariateSpatialData& rhs);
    virtual ~NormalCovariateSpatialData();

    //public data memebers
    Matrix       * _xg;
    Matrix       * _tobeinversed;
    Matrix       * _xgsigmaw;
    ColumnVector * _deltag;
    ColumnVector * _w_div_delta;
    EvaluationAssistDataStatus   geEvaluationAssistDataStatus;

    virtual void             AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual void             Assign(const AbstractSpatialClusterData& rhs);
    virtual double           CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator);
    virtual NormalCovariateSpatialData * Clone() const;
    virtual void             CopyEssentialClassMembers(const AbstractClusterData& rhs);
    virtual void             DeallocateEvaluationAssistClassMembers();
    virtual double           GetMaximizingValue(AbstractLikelihoodCalculator& Calculator);
    virtual void             InitializeData();
    virtual void             InitializeData(const AbstractDataSetGateway& DataGateway);
    NormalCovariateSpatialData    & operator=(const NormalCovariateSpatialData& rhs);
};

//******************************************************************************
#endif

