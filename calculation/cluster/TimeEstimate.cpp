#include "SaTScan.h"
#pragma hdrstop
#include "TimeEstimate.h"

void ReportTimeEstimate(clock_t nStartTime, int nRepetitions, int nRepsCompleted, BasePrint *pPrintDirection) {
  clock_t nStopTime     = clock();
  double  nClocksPerMin = 60.0*CLOCKS_PER_SEC;
  double  nDivisor      = nClocksPerMin*nRepsCompleted;
  double  nStepTime     = floor( (nRepetitions*(double)(nStopTime-nStartTime)/nDivisor) + 0.5 );

  if (nStepTime > 1.5)
    pPrintDirection->SatScanPrintf(".... this will take approximately %.0f minutes.\n", nStepTime);
  else if (nStepTime > 0.75)
    pPrintDirection->SatScanPrintf(".... this will take approximately %.0f minute.\n", nStepTime);
}


