#include "SaTScan.h"
#pragma hdrstop
#include "IncidentRate.h"

bool LowRate(count_t nCases, measure_t nMeasure, count_t nTotalCases, measure_t nTotalMeasure)
{
  if (nMeasure == 0)
    return false;

  return (nCases*nTotalMeasure  < nMeasure*nTotalCases);
}

bool HighRate(count_t nCases, measure_t nMeasure, count_t nTotalCases, measure_t nTotalMeasure)
{
 if (nMeasure == 0 || nCases < 2/*one case should not be considered a high rate*/)
    return false;

  return (nCases*nTotalMeasure  > nMeasure*nTotalCases);
}

bool HighOrLowRate(count_t nCases, measure_t nMeasure, count_t nTotalCases, measure_t nTotalMeasure)
{
  if (nMeasure == 0)
    return false;

  return (nCases*nTotalMeasure != nMeasure*nTotalCases);
}
