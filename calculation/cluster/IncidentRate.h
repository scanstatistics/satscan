//*****************************************************************************
#ifndef __INCIDENTRATE_H
#define __INCIDENTRATE_H
//*****************************************************************************
#include "SaTScan.h"
#include "UtilityFunctions.h"

typedef bool(*RATE_FUNCPTRTYPE)(count_t, measure_t, count_t, measure_t);

bool LowRate(count_t nCases, measure_t nMeasure, count_t nTotalCases, measure_t nTotalMeasure);
bool HighRate(count_t nCases, measure_t nMeasure, count_t nTotalCases, measure_t nTotalMeasure);
bool HighOrLowRate(count_t nCases, measure_t nMeasure, count_t nTotalCases, measure_t nTotalMeasure);
//*****************************************************************************
#endif
