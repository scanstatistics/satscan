//*****************************************************************************
#ifndef __INCIDENTRATE_H
#define __INCIDENTRATE_H
//*****************************************************************************
#include "SaTScan.h"
#include "UtilityFunctions.h"

typedef bool(*RATE_FUNCPTRTYPE)(count_t, measure_t, count_t, measure_t);

inline bool LowRate(count_t nCases, measure_t nMeasure, count_t nTotalCases, measure_t nTotalMeasure);
inline bool HighRate(count_t nCases, measure_t nMeasure, count_t nTotalCases, measure_t nTotalMeasure);
inline bool HighOrLowRate(count_t nCases, measure_t nMeasure, count_t nTotalCases, measure_t nTotalMeasure);

inline bool LowRate(count_t nCases, measure_t nMeasure, count_t nTotalCases, measure_t nTotalMeasure)
{
  if (nMeasure == 0)
    return false;

  return (nCases*nTotalMeasure  < nMeasure*nTotalCases);
}

inline bool HighRate(count_t nCases, measure_t nMeasure, count_t nTotalCases, measure_t nTotalMeasure)
{
 if (nMeasure == 0 || nCases < 2/*one case should not be considered a high rate*/)
    return false;

  return (nCases*nTotalMeasure  > nMeasure*nTotalCases);
}

inline bool HighOrLowRate(count_t nCases, measure_t nMeasure, count_t nTotalCases, measure_t nTotalMeasure)
{
  if (nMeasure == 0)
    return false;

  return (nCases*nTotalMeasure != nMeasure*nTotalCases);
}
//*****************************************************************************
#endif
