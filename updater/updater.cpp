//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include "uprMain.h"
#include "dlgULA.h"
#include <memory>

USERES("updater.res");
USEFORM("uprMain.cpp", frmMain);
USEUNIT("uprException.cpp");
USELIB("ziplib\zziplibrary\zziplibrary.lib");
USELIB("ziplib\zziplibrary\zlib.lib");
USEFORM("dlgULA.cpp", ssDlgULA);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
static const char VER_ID_OPTION_STRING[] = "-ver_id";
             try {
               Application->Initialize();
               if (_argc < 3)
                 throw std::runtime_error("Invalid argument count.\nUsage: 'archive filename' 'launch application' [-ver_id[=val]]");
               //check and show End User License Agreement if not "unrequested":
               char * * ppcOmitULA_Parameter(_argv);
               while ( (ppcOmitULA_Parameter < (_argv + _argc)) && (strnicmp(*ppcOmitULA_Parameter, VER_ID_OPTION_STRING, sizeof(VER_ID_OPTION_STRING)-1)) )
                 ++ppcOmitULA_Parameter;
               if (ppcOmitULA_Parameter == (_argv+_argc)) {//option "-ver_id" not found
                 std::auto_ptr<TssDlgULA> pula(new TssDlgULA(Application));
                 if (pula->ShowModal() != mrYes) {
                   remove(_argv[1]); // delete the archive file
                   throw update_cancelled("User License Agreement not accepted.");
                 }
               }
               else {//option "-ver_id" found: no logic right now.  Later we might check for "=something".
               }
               //run the update:
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
             catch (const update_cancelled& e) {
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
