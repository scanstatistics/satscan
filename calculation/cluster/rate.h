// rate.h

#ifndef __RATE_H
#define __RATE_H

#include "spatscan.h"

typedef bool(*RATE_FUNCPTRTYPE)(count_t, measure_t, count_t, measure_t);

bool LowRate(count_t nCases, measure_t nMeasure, count_t nTotalCases, measure_t nTotalMeasure);
bool HighRate(count_t nCases, measure_t nMeasure, count_t nTotalCases, measure_t nTotalMeasure);
bool HighOrLowRate(count_t nCases, measure_t nMeasure, count_t nTotalCases, measure_t nTotalMeasure);

#endif
