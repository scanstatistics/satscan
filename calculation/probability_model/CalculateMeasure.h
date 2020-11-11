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

std::vector<double>& CalcRisk(RealDataSet& Set, 
                              const Julian StudyStartDate, 
                              const Julian StudyEndDate, 
                              std::vector<double>& vRisk, 
                              PopulationData * pAltPopulationData=0);

boost::shared_ptr<TwoDimMeasureArray_t> Calcm(RealDataSet& Set, 
                                              const Julian StudyStartDate, 
                                              const Julian StudyEndDate, 
                                              PopulationData * pAltPopulationData=0);

void CalcMeasure(RealDataSet& DataSet,
                 const TwoDimMeasureArray_t& PopMeasure,
                 const std::vector<Julian>& vIntervalStartDates,
                 const Julian StartDate,
                 const Julian EndDate,
                 PopulationData * pAltPopulationData=0);

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
