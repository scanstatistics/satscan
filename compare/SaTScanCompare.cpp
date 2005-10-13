//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "zd540.h"
//---------------------------------------------------------------------------
USERES("SaTScanCompare.res");
USEFORM("Main.cpp", frmMain);
USELIB("C:\Program Files\Borland\CBuilder5\Lib\zd540.lib");
USEFORM("Options.cpp", frmOptions);
USEFORM("QueueWindow.cpp", frmQueueWindow);
USEFORM("QueueItemDefinition.cpp", frmBatchDefinition);
USEUNIT("mexcel.cpp");
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
   try
   {
       ZdInit();
       Application->Initialize();
       Application->Title = "SaTScan Version Comparsion Utility v1.5";
       Application->CreateForm(__classid(TfrmMain), &frmMain);
                 Application->Run();
       ZdExit();
   }
   catch (Exception &exception)
   {
       Application->ShowException(&exception);
   }
   return 0;
}
//---------------------------------------------------------------------------
