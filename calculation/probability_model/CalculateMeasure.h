//*****************************************************************************
#ifndef _CALCULATEMEASURE_H
#define _CALCULATEMEASURE_H
//*****************************************************************************
#include "SaTScan.h"
#include "Tracts.h"
#include "JulianDates.h"
#include "DataStream.h"

int CalcRisk(DataStream & thisStream,
             double                  ** pRisk,
             double                   * pAlpha,
             tract_t                    nTracts,
             BasePrint                * pPrintDirection);

int Calcm(PopulationData & thisPopulationData,
          measure_t           ** m,
          double               * pRisk,
          int                    nCats,
          tract_t                nTracts,
          int                    nPops,
          BasePrint            * pPrintDirection);


measure_t CalcMeasure(PopulationData & thisPopulationData,
                      measure_t            ** pMeasure,
                      measure_t            ** m,
                      Julian                * pStartDates,
                      Julian                  StartDate,
                      Julian                  EndDate,
                      tract_t                 nTracts,
                      int                     nTimeIntervals,
                      BasePrint             * pPrintDirection);

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

