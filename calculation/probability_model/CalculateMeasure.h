//******************************************************************************
#ifndef _CALCULATEMEASURE_H
#define _CALCULATEMEASURE_H
//******************************************************************************
#include "SaTScan.h"
#include "Tracts.h"
#include "JulianDates.h"
#include "DataSet.h"

// The functions: CalcRisk(...), Calcm(...), and CalcMeasure(...) are functions
// designed and utilized for the process of calculating the expected number of
// cases with the Poisson probability model.

std::vector<double>& CalcRisk(RealDataSet& DataSet, std::vector<double>& vRisk, Julian StudyStartDate, Julian StudyEndDate);

void Calcm(RealDataSet& thisSet, Julian StudyStartDate, Julian StudyEndDate);

measure_t CalcMeasure(RealDataSet& DataSet, TwoDimMeasureArray_t& NonCumulativeMeasureHandler,
                      const std::vector<Julian>& vIntervalStartDates, Julian StartDate, Julian EndDate);

// The following functions are either unfinished design thoughts or deprecated.

int AdjustForDiscreteTimeTrend(measure_t*** pMeasure,
                               count_t      *Cases[],
                               tract_t      nTracts,
                               int          nTimeIntervals,
                               count_t*     pTotalCases,
                               measure_t*   pTotalMeasure);

void AdjustForPercentageTimeTrend(double       nTimeAdjPercent,
                                  int          nTimeIntervals,
                                  DatePrecisionType eIntervalUnits,
                                  long         nIntervalLength,
                                  tract_t      nTracts,
                                  measure_t*   pTotalMeasure,
                                  measure_t*** pMeasure,
                                  BasePrint *pPrintDirection);

void DisplayInitialData(Julian  StartDate,
                        Julian  EndDate,
                        Julian* pIntvDates,
                        int     nTimeIntervals,
                        double* pAlpha,
                        int     nPops);

bool ValidateMeasures(const TractHandler *pTInfo,
                      measure_t** Measures,
		      measure_t   nTotalMeasure,
		      measure_t   nMaxCircleSize,
		      tract_t     nNumTracts,
		      int         nTimeIntervals,
		      int         nGeoSize,
                      BasePrint *pPrintDirection);

/*void AssignTemporalMeasuresAndCases(int         nTimeIntervals,
                                    tract_t     nTracts,
                                    count_t**   Cases,
                                    measure_t** Measure,
                                    count_t**   pPTCases,
                                    measure_t** pPTMeasure);

void AssignTemporalCases(int       nTimeIntervals,
                         tract_t   nTracts,
                         count_t** Cases,
                         count_t*  pPTCases);
*/
bool ValidateAllCountsArePossitive(tract_t   nTracts,
                                   int       nTimeIntervals,
                                   count_t** Counts,
                                   count_t   nTotalCount,
                                   BasePrint *pPrintDirection);

bool ValidateAllPTCountsArePossitive(tract_t  nTracts,
                                   int      nTimeIntervals,
                                   count_t* Counts,
                                   count_t  nTotalCount,
                                   BasePrint *pPrintDirection);

//******************************************************************************
#endif
