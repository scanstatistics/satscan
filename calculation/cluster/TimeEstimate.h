// TimeEstimate.h

#ifndef __TIMEESTIMATE_H
#define __TIMEESTIMATE_H

#include <time.h>
#include "SSException.h"
#include "baseprint.h"

void ReportTimeEstimate(clock_t nStartTime, int nRepetitions, int nRepsCompleted, BasePrint *pPrintDirection);

#endif
