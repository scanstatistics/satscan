//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include "uprMain.h"

USERES("updater.res");
USEFORM("uprMain.cpp", frmMain);
USEUNIT("uprException.cpp");
USELIB("ziplib\zziplibrary\zziplibrary.lib");
USELIB("ziplib\zziplibrary\zlib.lib");
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
             try {
               Application->Initialize();
               if (_argc < 3)
                 throw std::runtime_error("Invalid argument count.\nUsage: 'archive filename' 'launch application'");
               TfrmMain(Application).RunUpdate(_argv[1]);
             }
             catch (const archive_error& e) {
               AnsiString s;
               s.printf("SaTScan update was aborted due to an error while reading updates.\n"
                        "Please email SaTScan with the following information:\n\n%s.", e.what());
               Application->MessageBox(s.c_str(), "SaTScan Update Aborted", MB_OK);
             }
             catch (const file_access_error& e) {
               Application->MessageBox("SaTScan update was cancelled. Please try to update again at a later time.",
                                       "SaTScan Update Cancelled", MB_OK);
             }
             catch (const std::exception& e) {
               Application->MessageBox(e.what(), "SaTScan Update Error", MB_OK);
             }
             catch (Exception &exception) {
               Application->ShowException(&exception);
             }
             ShellExecute(NULL, "open", _argv[2], NULL, NULL, SW_SHOWNORMAL);
             return 0;
}
//---------------------------------------------------------------------------
