// rate.cpp

#include "rate.h"
#include "display.h"

bool LowRate(count_t nCases, measure_t nMeasure, count_t nTotalCases, measure_t nTotalMeasure)
{
  if (nMeasure == 0)
    return false;

  if (nCases*nTotalMeasure  < nMeasure*nTotalCases)
    return true;
  else
    return false;
}

bool HighRate(count_t nCases, measure_t nMeasure, count_t nTotalCases, measure_t nTotalMeasure)
{
  if (nMeasure == 0)
    return false;

  if (nCases*nTotalMeasure  > nMeasure*nTotalCases)
    return true;
  else
    return false;
}

bool HighOrLowRate(count_t nCases, measure_t nMeasure, count_t nTotalCases, measure_t nTotalMeasure)
{
  if (nMeasure == 0)
    return false;

  if (nCases*nTotalMeasure != nMeasure*nTotalCases)
    return true;
  else
    return false;
}
