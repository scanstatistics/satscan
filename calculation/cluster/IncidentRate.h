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

/** Indicates whether an area has lower than expected cases for a clustering
    within a single data stream. */
inline bool LowRate(count_t nCases, measure_t nMeasure, count_t nTotalCases, measure_t nTotalMeasure)
{
  if (nMeasure == 0)
    return false;

  return (nCases*nTotalMeasure  < nMeasure*nTotalCases);
}

/** Indicates whether an area has high than expected cases for a clustering
    within a single data stream. Clusterings with less than two cases are not
    considered for high rates. Note this function should not be used for scannning
    for high rates with an analysis with multiple data streams; use MultipleSetsHighRate() */
inline bool HighRate(count_t nCases, measure_t nMeasure, count_t nTotalCases, measure_t nTotalMeasure)
{
 if (nMeasure == 0 || nCases < 2/*one case should not be considered a high rate*/)
    return false;

  return (nCases*nTotalMeasure  > nMeasure*nTotalCases);
}

/** Indicates whether an area has lower than expected cases for a clustering
    within a single data stream. */
inline bool HighOrLowRate(count_t nCases, measure_t nMeasure, count_t nTotalCases, measure_t nTotalMeasure)
{
  if (nMeasure == 0)
    return false;

  return (nCases*nTotalMeasure != nMeasure*nTotalCases);
}

inline bool MultipleSetsHighRate(count_t nCases, measure_t nMeasure, count_t nTotalCases, measure_t nTotalMeasure);

/** For multiple sets, the criteria that a high rate must have more than one case
    is not currently implemented. */
inline bool MultipleSetsHighRate(count_t nCases, measure_t nMeasure, count_t nTotalCases, measure_t nTotalMeasure)
{
 if (nMeasure == 0)
    return false;

  return (nCases*nTotalMeasure  > nMeasure*nTotalCases);
}
//*****************************************************************************
#endif
