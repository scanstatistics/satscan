/* Calcmsr.h */

#ifndef _CALCMSR_H
#define _CALCMSR_H

#include "basic.h"
#include "spatscan.h"

int AssignMeasure(
            count_t      *Cases[],
            Julian**     Times,
            tract_t      NumTracts,
            Julian       StartDate,
            Julian       EndDate,
            Julian*      pStartDates,
            BOOL         bExactTimes,
            int          nTimeAdjust,
            double       nTimeAdjPercent,
            int          nTimeIntervals,
            int          nIntervalUnits,
            long         nIntervalLength,
            measure_t*** pMeasure,
            count_t*     pTotalCases,
            double*      pTotalPop,
            measure_t*   pTotalMeasure);

int CalcRisk(double** pRisk,
             double*  pAlpha,
             int      nCats,
             tract_t  nTracts,
             int      nPops,
             double*    pTotalPop,
             count_t* pTotalCases);

int Calcm(measure_t*** m,
          double*      pRisk,
          int          nCats,
          tract_t      nTracts,
          int          nPops);


int CalcMeasure(measure_t*** pMeasure,
                measure_t**  m,
                Julian*      pStartDates,
                Julian       StartDate,
                Julian       EndDate,
                int          nCats,
                tract_t      nTracts,
                int          nPops,
                int          nTimeIntervals,
                measure_t*   pTotalMeasure);

int AdjustForDiscreteTimeTrend(measure_t*** pMeasure,
                               count_t      *Cases[],
                               tract_t      nTracts,
                               int          nTimeIntervals,
                               count_t*     pTotalCases,
                               measure_t*   pTotalMeasure);

void AdjustForPercentageTimeTrend(double       nTimeAdjPercent,
                                  int          nTimeIntervals,
                                  int          nIntervalUnits,
                                  long         nIntervalLength,
                                  tract_t      nTracts,
                                  measure_t*   pTotalMeasure,
                                  measure_t*** pMeasure);

void DisplayInitialData(Julian  StartDate,
                        Julian  EndDate,
                        Julian* pIntvDates,
                        int     nTimeIntervals,
                        double* pAlpha,
                        int     nPops);

BOOL ValidateMeasures(measure_t** Measures,
							 measure_t   nTotalMeasure,
							 measure_t   nMaxCircleSize,
							 tract_t     nNumTracts,
							 int         nTimeIntervals,
							 int         nGeoSize);

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
BOOL ValidateAllCountsArePossitive(tract_t   nTracts,
                                   int       nTimeIntervals,
                                   count_t** Counts,
                                   count_t   nTotalCount);

BOOL ValidateAllPTCountsArePossitive(tract_t  nTracts,
                                   int      nTimeIntervals,
                                   count_t* Counts,
                                   count_t  nTotalCount);

#endif

