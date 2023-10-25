//*****************************************************************************
#ifndef __ADJUSTMENTHANDLER_H
#define __ADJUSTMENTHANDLER_H
//*****************************************************************************
#include "SaTScan.h"
#include "JulianDates.h"
#include "Tracts.h"
#include "DataSet.h"
#include <deque>

class RelativeRiskAdjustment {
  private:
    measure_t gdRelativeRisk;
    Julian    gStartDate;
    Julian    gEndDate;

  public:
    RelativeRiskAdjustment(measure_t dRelativeRisk, Julian StartDate, Julian EndDate);

    Julian      GetEndDate() const {return gEndDate;}
    measure_t   GetRelativeRisk() const {return gdRelativeRisk;}
    Julian      GetStartDate() const {return gStartDate;}
    void        MultiplyRisk(measure_t dRisk);
    void        SetEndDate(Julian Date) {gEndDate = Date;}
    void        SetRelativeRisk(double dRelativeRisk) {gdRelativeRisk = dRelativeRisk;}
    void        SetStartDate(Julian Date) {gStartDate = Date;}
};

typedef std::deque<RelativeRiskAdjustment>      TractContainer_t;
typedef TractContainer_t::iterator              TractContainerIterator_t;
typedef TractContainer_t::const_iterator        TractContainerIteratorConst_t;
typedef std::map<tract_t, TractContainer_t >    AdjustmentsContainer_t;
typedef AdjustmentsContainer_t::const_iterator  AdjustmentsIterator_t;

class CSaTScanData;

class RelativeRiskAdjustmentHandler {
  private:
    const CSaTScanData & _dataHub;
    AdjustmentsContainer_t gTractAdjustments;

    bool                      AdjustMeasure(const TwoDimMeasureArray_t& PopMeasure, const PopulationData & Population, tract_t Tract, double dRelativeRisk, Julian StartDate, Julian EndDate, TwoDimMeasureArray_t& adjustMeasure, const TwoDimCountArray_t * pCases=0) const;
    measure_t                 CalcMeasureForTimeInterval(const PopulationData & Population, measure_t ** ppPopulationMeasure, tract_t Tract, Julian StartDate, Julian NextStartDate) const;
    measure_t                 DateMeasure(const PopulationData & Population, measure_t ** ppPopulationMeasure, Julian Date, tract_t Tract) const;
    count_t                   getCaseCount(count_t ** ppCumulativeCases, int iInterval, tract_t tTract) const;
    TractContainerIterator_t  GetMaxPeriodIndex(TractContainer_t & Container, Julian Date);

  public:
      RelativeRiskAdjustmentHandler(const CSaTScanData& dataHub) : _dataHub(dataHub) {}

    void                            add(tract_t tTractIndex, measure_t dRelativeRisk, Julian StartDate, Julian EndDate);
    void                            apply(const TwoDimMeasureArray_t& Measure, const PopulationData & Population, measure_t totalMeasure, TwoDimMeasureArray_t& adjustMeasure, const TwoDimCountArray_t * pCases=0) const;
    void                            empty() {gTractAdjustments.clear();}
    const AdjustmentsContainer_t &  get() const {return gTractAdjustments;}
    void                            print(IdentifiersManager & identifierMgr);
};
#endif
