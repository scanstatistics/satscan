// This class will query a perlscript to determine if updates are needed for a
// client. If the client is current the original client will run, else the
// class will ask the user to download the updates (auto-download) and then try
// to launch the updated version of the client.
//---------------------------------------------------------------------------
#include "stsSaTScan.h"
#pragma hdrstop
#include "stsFrmUpdateCheck.h"
#include <Masks.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

/** update application filename - returned from host
    -- we need to know this before communicating with host so that we know
       what file to delete when SaTScan restarts. We could set an ini section
       or registory value if this is becomes a problem. */
const char * TfrmUpdateCheck::gsUpdaterFilename   = "update_app.exe";
/** perl script for determining updates */
const char * TfrmUpdateCheck::gsURLFormat         = "http://satscan.us/cgi-bin/satscan/update/satscan_version_update.pl?todo=return_update_version_info&form_current_version_id=value%s&form_current_version_number=value%s";
const int TfrmUpdateCheck::giUpdateTokens         = 9;
const int TfrmUpdateCheck::giUpdateIndicatorIndex = 0;
const int TfrmUpdateCheck::giUpdateVersionIdIndex = 3;
const int TfrmUpdateCheck::giUpdateVersionIndex   = 4;
const int TfrmUpdateCheck::giUpdateAppNameIndex   = 5;
const int TfrmUpdateCheck::giUpdateAppUrlIndex    = 6;
const int TfrmUpdateCheck::giUpdateDataNameIndex  = 7;
const int TfrmUpdateCheck::giUpdateDataUrlIndex   = 8;

/** constructor */
__fastcall TfrmUpdateCheck::TfrmUpdateCheck(TComponent* Owner) : TForm(Owner), gbHasUpdates(false) {
}

/** destructor */
__fastcall TfrmUpdateCheck::~TfrmUpdateCheck() {}

/** this function connects to the server to request the update information */
void TfrmUpdateCheck::ConnectToServerForUpdateCheck() {
  bool                  bReturn=false, bDone=false;
  unsigned long         lToken;
  ZdString              sFilename, sUpdateURL;
  ZdStringTokenizer     sHTTP_Body("", ",");

  try {
    Show();
    sUpdateURL.printf(gsURLFormat, VERSION_ID, VERSION_NUMBER);
    try {
      // let the Get() do the connecting since reading results from perlscript
      // is the only purpose for connecting to remote host ... currently.
      pHTTPConnect->Get(sUpdateURL.GetCString());
    }
    catch (...) {
      ZdException::GenerateNotification("Failed to connect to server. Server may be down or\n"
                                        "internet connection may not be established.\n"
                                        "Please check your connection status and try again.",
                                        "ConnectToServerForUpdateCheck()");
    }
    remove(pHTTPConnect->Header.c_str());
    // get perlscript results -- list of files descriptions at specified url
    sHTTP_Body.SetString(pHTTPConnect->Body.c_str());
    if (sHTTP_Body.GetNumTokens() < (unsigned int)giUpdateTokens || !stricmp(sHTTP_Body.GetToken(0).GetCString(), "no"))
      gbHasUpdates = false;
    else if (sHTTP_Body.GetToken(giUpdateVersionIdIndex) == VERSION_ID) {
      gbHasUpdates = true;
      //get update information
      gsUpdateVersion = sHTTP_Body.GetToken(giUpdateVersionIndex);
      gUpdateApplication.first = sHTTP_Body.GetToken(giUpdateAppNameIndex);
      gUpdateApplication.second = sHTTP_Body.GetToken(giUpdateAppUrlIndex);
      gUpdateArchive.first = sHTTP_Body.GetToken(giUpdateDataNameIndex);
      gUpdateArchive.second = sHTTP_Body.GetToken(giUpdateDataUrlIndex);
      if (gUpdateArchive.second.EndsWith('\n'))
        gUpdateArchive.second.Truncate(gUpdateArchive.second.GetLength() - 1);
    }
    Close();
  }
  catch (ZdException &x) {
    Close();
    x.AddCallpath("ConnectToServerForUpdateCheck()", "TfrmUpdateCheck");
    throw;
  }
}

// displays a message box to user promting whether or not they want to update
bool TfrmUpdateCheck::DisplayDownloadOption(const ZdString& sFilename) {
  ZdString      sMessage;
  bool          bReturn=false;

  try {
    sMessage.printf("SaTScan v%s is available. Do you want to install now?", gsUpdateVersion.GetCString());
    bReturn = (TBMessageBox::Response(this, "SaTScan Update Available", sMessage.GetCString(), MB_YESNO) == IDYES);
  }
  catch (ZdException &x) {
    x.AddCallpath("DisplayDownloadOption()", "TBfrmImsUpdate");
    throw;
  }
  return bReturn;
}

bool TfrmUpdateCheck::HasUpdates() const {
  return gbHasUpdates;
}

void __fastcall TfrmUpdateCheck::OnCommandFailure(CmdType Cmd) {
  switch(Cmd) {
    case CmdGET         : /*HTTP GET Failed*/
                          // since connection isn't attempted until call
                          // to TNMHTTP::Get(), this event being triggered
                          // is superseded by connection failure exception
    case CmdPOST        : /*HTTP Post Failed*/
    case CmdHEAD        : /*HTTP HEAD Failed*/
    case CmdOPTIONS     : /*HTTP OPTIONS Failed*/
    case CmdTRACE       : /*HTTP TRACE Failed*/
    case CmdPUT         : /*HTTP PUT Failed*/
    case CmdDELETE      : /*HTTP Delete Failed*/
    default             : break;
  };
}

void __fastcall TfrmUpdateCheck::OnConnectionFailed(TObject *Sender)  {
 // since we never directly call TNMHTTP::Connect(), this event is never triggered
}







