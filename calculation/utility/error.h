// error.h

#ifndef __ERROR_H
#define __ERROR_H

enum {INVALIDNUMBEROFARGUMENTS=1, UNABLETOOPENPARAMETERFILE, INVALIDANALYSISTYPE,
      OUTOFMEMORY, MAXCLUSTERS, DATAERROR };

//void ReportError(int nError);
//void ReportWarning(int nWarning);
void FatalError(char* s);

#endif
