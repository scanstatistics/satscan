//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
#include "MeasureDeterminant.h"

measure_t Standard(measure_t tMeasure, measure_t tMeasureSquared) {
  return tMeasure;
}
measure_t EnhancedMeasure(measure_t tMeasure, measure_t tMeasureSquared) {
  return tMeasureSquared/tMeasure;
}


measure_t Standard_(unsigned int iStart, unsigned int iEnd, const measure_t* tMeasure, const measure_t* tMeasureSquared) {
  return tMeasure[iStart] - tMeasure[iEnd];
}
measure_t EnhancedMeasure_(unsigned int iStart, unsigned int iEnd, const measure_t* tMeasure, const measure_t* tMeasureSquared) {
  return (tMeasureSquared[iStart] - tMeasureSquared[iEnd])/(tMeasure[iStart] - tMeasure[iEnd]);
}

measure_t _Standard_(unsigned int iStart, const measure_t* tMeasure, const measure_t* tMeasureSquared) {
  return tMeasure[iStart];
}

measure_t _EnhancedMeasure_(unsigned int iStart, const  measure_t* tMeasure, const measure_t* tMeasureSquared) {
  return tMeasureSquared[iStart]/tMeasure[iStart];
}

