//*****************************************************************************
#ifndef _CALCULATEMEASURE_H
#define _CALCULATEMEASURE_H
//*****************************************************************************
#include "SaTScan.h"
#include "Tracts.h"
#include "JulianDates.h"
#include "PopulationCategories.h"

int AssignMeasure(
            const TractHandler  *pTInfo,
            count_t      *Cases[],
            Julian**     Times,
            tract_t      NumTracts,
            Julian       StartDate,
            Julian       EndDate,
            Julian*      pStartDates,
            bool         bExactTimes,
            int          nTimeAdjust,
            double       nTimeAdjPercent,
            int          nTimeIntervals,
            int          nIntervalUnits,
            long         nIntervalLength,
            measure_t*** pMeasure,
            count_t*     pTotalCases,
            double*      pTotalPop,
            measure_t*   pTotalMeasure,
            BasePrint *pPrintDirection);

int CalcRisk(const TractHandler *pTInfo,
             double** pRisk,
             double*  pAlpha,
             int      nCats,
             tract_t  nTracts,
             int      nPops,
             double*    pTotalPop,
             count_t* pTotalCases,
             BasePrint *pPrintDirection);

int Calcm(const TractHandler *pTInfo,
          measure_t*** m,
          double*      pRisk,
          int          nCats,
          tract_t      nTracts,
          int          nPops,
          BasePrint *pPrintDirection);


int CalcMeasure(const TractHandler *pTInfo,
                measure_t*** pMeasure,
                measure_t**  m,
                Julian*      pStartDates,
                Julian       StartDate,
                Julian       EndDate,
                int          nCats,
                tract_t      nTracts,
                int          nPops,
                int          nTimeIntervals,
                measure_t*   pTotalMeasure,
                BasePrint *pPrintDirection);

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

//*****************************************************************************
#endif

