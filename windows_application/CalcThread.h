//---------------------------------------------------------------------------

#ifndef CalcThreadH
#define CalcThreadH

#include <Classes.hpp>
#include <time.h>
#include <process.h>

#include "param.h"
#include "utils.h"
#include "xmfrmAnalysisRun.h"
#include "display.h"
#include "PSanalysis.h"
#include "PSManalysis.h"
#include "PTanalysis.h"
#include "STanalysis.h"
#include "ST+PSanalysis.h"
#include "ST+PTanalysis.h"
#include "ST+PS+PTanalysis.h"
#include "PSdata.h"
#include "PTdata.h"
#include "STdata.h"
#include "TfrmViewer.h"
#include "PrintWindow.h"
#include "xbDlgException.h"

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
