//---------------------------------------------------------------------------
#ifndef stsCalculationThreadH
#define stsCalculationThreadH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <time.h>                                 
#include <process.h>
#include "SVTTData.h"
#include "SpaceTimeData.h"
#include "SVTTAnalysis.h"
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
class CalcThread : public TThread {
  private:
    CParameters       * gpParameters;
    CSaTScanData      * gpData;
    CAnalysis         * gpAnalysis;
    PrintWindow       * gpPrintWindow;
    TfrmAnalysisRun   & gFormStatus;
    char              * gsPrintString;
    bool                gbJobCanceled;
    std::string         gsProgramErrorCallPath;      

    void                Free();
    void                Init();
    void __fastcall     EnableProgressEmailButton(void);
    void __fastcall     EnableProgressPrintButton(void);
    void __fastcall     ProcessAcknowledgesCancellation(void);
    void __fastcall     ProcessSignalsCompletion(void);
    void __fastcall     GetIsCanceledFromProgress(void);
    void __fastcall     LoadResultsFromFile();
    void __fastcall     PrintLineToProgress(void);
    void __fastcall     PrintWarningLineToProgress(void);
    void __fastcall     ResetProgressCloseButton(void);
    void __fastcall     SetJobCancelled(void);
    void __fastcall     SetProgramErrorCallPath(void); 
    void __fastcall     SetProgressWarnings(void);
    void                Setup(const CParameters& Parameters);

  protected:
    void __fastcall     Execute();

  public:
    __fastcall CalcThread(TfrmAnalysisRun & Progress, const CParameters & Parameters);
    __fastcall ~CalcThread();

    void                AddLineToProgress(char * sText);
    void                AddWarningToProgress(char * sText);
    void                CancellJob();
    bool                IsCancelled();
};
//---------------------------------------------------------------------------
#endif
