//***************************************************************************
#ifndef __ScanLineParameterFileAccess_H
#define __ScanLineParameterFileAccess_H
//***************************************************************************
#include "ParameterFileAccess.h"

/** Implements class that reads/writes parameter settings of an ASCII file,
    where each line is associated with particular parameter. This process is
    maintained to permit parameter files created with version prior to 3.1.0 (in
    which the structure was switched to an ini format). Note that until the
    implementation of mutliple data streams, this reader provided the same read
    functionality as ini version.*/
class ScanLineParameterFileAccess : public AbtractParameterFileAccess {
  private:
    virtual const char        * GetParameterLabel(ParameterType eParameterType) const;

  public:
     ScanLineParameterFileAccess(CParameters& Parameters, BasePrint& PrintDirection);
     virtual ~ScanLineParameterFileAccess();

     virtual bool               Read(const char* szFilename);
     virtual void               Write(const char * szFilename);
};
//***************************************************************************
#endif
