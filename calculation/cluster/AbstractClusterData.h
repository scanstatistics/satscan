//---------------------------------------------------------------------------
#ifndef AbstractClusterDataH
#define AbstractClusterDataH
//---------------------------------------------------------------------------
#include "IncidentRate.h"
#include "ProbabilityModel.h"
#include "DataStreamGateway.h"
#include "LikelihoodCalculation.h"

class CMeasureList; /** forward class declaration */
class CSaTScanData; /** forward class declaration */

/** abstract class representing accumulated cluster data */
class AbstractClusterData {
  public:
    AbstractClusterData();
    virtual ~AbstractClusterData();

    virtual AbstractClusterData * Clone() const = 0;

    virtual void        AddNeighborData(tract_t tNeighbor, const AbtractDataStreamGateway & DataGateway, size_t tStream=0) = 0;
    virtual double      CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator & Calculator);
    virtual count_t     GetCaseCount(unsigned int iStream=0) const = 0;
    virtual measure_t   GetMeasure(unsigned int iStream=0) const = 0;
    virtual void        InitializeData() = 0;
};

class CSaTScanData; /** forward class declaration */

/** abstract class representing accumulated data of spatial clustering */
class AbstractSpatialClusterData : public AbstractClusterData {
  protected:
    RATE_FUNCPTRTYPE            gfRateOfInterest;

  public:
    AbstractSpatialClusterData(int iRate);
    virtual ~AbstractSpatialClusterData();

    virtual void                         Assign(const AbstractSpatialClusterData& rhs) = 0;
    virtual AbstractSpatialClusterData * Clone() const = 0;
};

/** abstract class representing accumulated data of temporal clustering */
class AbstractTemporalClusterData : public AbstractClusterData {
  public:
    AbstractTemporalClusterData();
    virtual ~AbstractTemporalClusterData();

    virtual void                          Assign(const AbstractTemporalClusterData& rhs) = 0;
    virtual AbstractTemporalClusterData * Clone() const = 0;

    virtual void        SetCaseCount(count_t tCases, unsigned int iStream=0) = 0;
    virtual void        SetMeasure(measure_t tMeasure, unsigned int iStream=0) = 0;
};
//---------------------------------------------------------------------------
#endif
