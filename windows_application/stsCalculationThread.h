//---------------------------------------------------------------------------
#ifndef stsCalculationThreadH
#define stsCalculationThreadH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <time.h>                                 
#include <process.h>
#include "SpaceTimeData.h"
#include "PurelySpatialData.h"
#include "PurelyTemporalData.h"
#include "SpaceTimeAnalysis.h"
#include "PurelySpatialAnalysis.h"
#include "PurelySpatialMonotoneAnalysis.h"
#include "PurelyTemporalAnalysis.h"
#include "SpaceTimeIncludePurelySpatialAnalysis.h"
#include "SpaceTimeIncludePurelyTemporalAnalysis.h"
#include "SpaceTimeIncludePureAnalysis.h"
//---------------------------------------------------------------------------
class CalcThread : public TThread
{
private:

protected:
       CParameters  *gpParams;
       CSaTScanData* gpData;
       CAnalysis* gpAnalysis;
       PrintWindow *gpPrintWindow;

       AnsiString  gsThreadTitle;
       TfrmAnalysisRun *gpFormStatus;
      // TForm1 * theMainForm;
       void __fastcall Execute();
       void  Init();
       void  Free();
public:
        __fastcall CalcThread(bool CreateSuspended, const CParameters& session, char *pTitle, TfrmAnalysisRun *pProgress);
        __fastcall ~CalcThread();
};
//---------------------------------------------------------------------------
#endif
