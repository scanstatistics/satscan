//*****************************************************************************
#ifndef __ADJUSTMENTHANDLER_H
#define __ADJUSTMENTHANDLER_H
//*****************************************************************************
#include "SaTScan.h"
#include "JulianDates.h"
#include "Tracts.h"
#include "deque.h"

class RelativeRiskAdjustment {
  private:
    measure_t				gdRelativeRisk;
    Julian				gStartDate;
    Julian				gEndDate;

  public:
    RelativeRiskAdjustment(measure_t dRelativeRisk, Julian StartDate, Julian EndDate);
    ~RelativeRiskAdjustment();

    Julian				GetEndDate() const {return gEndDate;}
    measure_t				GetRelativeRisk() const {return gdRelativeRisk;}
    Julian				GetStartDate() const {return gStartDate;}
    void                                MultiplyRisk(measure_t dRisk);
    void				SetEndDate(Julian Date) {gEndDate = Date;}
    void				SetRelativeRisk(double dRelativeRisk) {gdRelativeRisk = dRelativeRisk;}
    void				SetStartDate(Julian Date) {gStartDate = Date;}
};

typedef std::deque<RelativeRiskAdjustment>      TractContainer_t;
typedef TractContainer_t::iterator              TractContainerIterator_t;
typedef TractContainer_t::const_iterator        TractContainerIteratorConst_t;
typedef std::map<tract_t, TractContainer_t >    AdjustmentsContainer_t;
typedef AdjustmentsContainer_t::const_iterator  AdjustmentsIterator_t;

class RelativeRiskAdjustmentHandler {
  private:
    AdjustmentsContainer_t              gTractAdjustments;

  public:
    RelativeRiskAdjustmentHandler();
    ~RelativeRiskAdjustmentHandler();
    
    void				AddAdjustmentData(tract_t tTractIndex, measure_t dRelativeRisk, Julian StartDate, Julian EndDate);
    const AdjustmentsContainer_t      & GetAdjustments() const {return gTractAdjustments;}
    TractContainerIterator_t            GetMaxPeriodIndex(TractContainer_t & Container, Julian Date);
    unsigned int			GetNumAdjustments() const {return gTractAdjustments.size();}

    void                                PrintAdjustments(TractHandler & tHandler);
};
#endif