//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "zd546.h"
//---------------------------------------------------------------------------
USERES("SaTScanCompare.res");
USEFORM("Main.cpp", frmMain);
USEFORM("Options.cpp", frmOptions);
USEFORM("QueueWindow.cpp", frmQueueWindow);
USEFORM("QueueItemDefinition.cpp", frmBatchDefinition);
USEUNIT("mexcel.cpp");
USELIB("..\..\zero.dimension\zd546.lib");
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
   try
   {
       ZdInit();
       Application->Initialize();
       Application->Title = "SaTScan Version Comparsion Utility v1.6";
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
