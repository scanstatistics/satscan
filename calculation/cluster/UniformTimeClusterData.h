//******************************************************************************
#ifndef __UniformTimeClusterData_H
#define __UniformTimeClusterData_H
//******************************************************************************
#include "ClusterData.h"

/** Abstract base class for all uniform time cluster data. */
class AbstractUniformTimeClusterData {
  public:
    AbstractUniformTimeClusterData(): gtCasesInPeriod(0), gtMeasureInPeriod(0) {}
    virtual ~AbstractUniformTimeClusterData() {}

    count_t                  gtCasesInPeriod;
    measure_t                gtMeasureInPeriod;

    virtual count_t          GetCasesInPeriod(unsigned int tSetIndex) const { return gtCasesInPeriod; };
    virtual measure_t        GetMeasureInPeriod(unsigned int tSetIndex = 0) const { return gtMeasureInPeriod; }
};

class UniformTimeClusterDataInterface: public AbstractUniformTimeClusterData {
public:
    UniformTimeClusterDataInterface() : AbstractUniformTimeClusterData(){}
    virtual ~UniformTimeClusterDataInterface() {}

    virtual count_t        * getCasesArray() const;
    virtual measure_t      * getMeasureArray() const;

    virtual count_t          getCases() const;
    virtual void             setCases(count_t c);
    virtual measure_t        getMeasure() const;
    virtual void             setMeasure(measure_t m);
};

/** Class representing accumulated data of temporal clustering for uniform time probability model.
    If instantiated through public constructors, points to already calculated
    purely temporal arrays supplied by DataInterface. The protected constructor
    is intended to permit instantiation through a derived class, where perhaps
    pointers will be allocated and data supplied by some other process. */
class UniformTimeTemporalData : public TemporalData, public UniformTimeClusterDataInterface {
  protected:
    UniformTimeTemporalData();

  public:
    UniformTimeTemporalData(const DataSetInterface& DataGateway);
    UniformTimeTemporalData(const AbstractDataSetGateway& DataGateway);
    virtual ~UniformTimeTemporalData() {}

    virtual count_t        * getCasesArray() const { return gpCases; }
    virtual measure_t      * getMeasureArray() const { return gpMeasure; }

    virtual count_t          getCases() const { return gtCases;  }
    virtual void             setCases(count_t c) { gtCases = c;  }
    virtual measure_t        getMeasure() const { return gtMeasure; }
    virtual void             setMeasure(measure_t m) { gtMeasure = m; }

    virtual void             Assign(const AbstractTemporalClusterData& rhs);
    virtual void             CopyEssentialClassMembers(const AbstractClusterData& rhs);
    virtual UniformTimeTemporalData * Clone() const;
    virtual void             InitializeData() {gtCases=0;gtMeasure=0;gtCasesInPeriod=0;gtMeasureInPeriod=0;}
    virtual void             Reassociate(const DataSetInterface& Interface);
    virtual void             Reassociate(const AbstractDataSetGateway& DataGateway);
};

/** Class representing accumulated data of space-time clustering for uniform time probability model. */
class UniformTimeSpaceTimeData : public SpaceTimeData, public UniformTimeClusterDataInterface {
  public:
    UniformTimeSpaceTimeData(const DataSetInterface& Interface);
    UniformTimeSpaceTimeData(const AbstractDataSetGateway& DataGateway);
    UniformTimeSpaceTimeData(const UniformTimeSpaceTimeData& rhs);
    virtual ~UniformTimeSpaceTimeData();

    virtual count_t        * getCasesArray() const { return gpCases; }
    virtual measure_t      * getMeasureArray() const { return gpMeasure; }

    virtual count_t          getCases() const { return gtCases; }
    virtual void             setCases(count_t c) { gtCases = c; }
    virtual measure_t        getMeasure() const { return gtMeasure; }
    virtual void             setMeasure(measure_t m) { gtMeasure = m; }

    virtual void             Assign(const AbstractTemporalClusterData& rhs);
    virtual void             CopyEssentialClassMembers(const AbstractClusterData& rhs);
    virtual UniformTimeSpaceTimeData * Clone() const;
    virtual void             InitializeData();
    UniformTimeSpaceTimeData    & operator=(const UniformTimeSpaceTimeData& rhs);
};

//******************************************************************************
#endif

