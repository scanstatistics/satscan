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

/** update application filename */
const char * TfrmUpdateCheck::gsUpdaterFilename = "updater.exe";
/** url detailing perlscript location */
const char * TfrmUpdateCheck::gsURL = "http://install.cancer.gov/cgi-bin/versioncheck100.pl?app=satscan";
/** mask to determing which file returned from url is latest version release */
const char * TfrmUpdateCheck::gsVersionMask = "satscanv*.zip";
/** index of filename in file description string of perlscript results */
const int TfrmUpdateCheck::giFilenameIndex = 0;
/** index of url path in file description string of perlscript results */
const int TfrmUpdateCheck::giURLIndex = 3;

/** constructor */
__fastcall TfrmUpdateCheck::TfrmUpdateCheck(TComponent* Owner) : TForm(Owner) {
}

/** destructor */
__fastcall TfrmUpdateCheck::~TfrmUpdateCheck() {}

/** this function connects to the server to request the update information */
void TfrmUpdateCheck::ConnectToServerForUpdateCheck() {
  bool                  bReturn=false, bDone=false;
  unsigned long         lToken;
  ZdString              sFilename;
  ZdStringTokenizer     sHTTP_Body("", "\n"), sFileDescriptorParser("", ",");

  try {
    Show();
    try {
      // let the Get() do the connecting since reading results from perlscript
      // is the only purpose for connecting to remote host ... currently.
      pHTTPConnect->Get(gsURL);
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
    if (! sHTTP_Body.GetNumTokens())
      ZdException::GenerateNotification("No updates currently available.\nPlease try again later.",
                                        "ConnectToServerForUpdateCheck()");

    // parse each file description -- looking for satscanX.zip where X = version number
    for (lToken=0; lToken < sHTTP_Body.GetNumTokens() - 1 && !bDone; ++lToken) {
        sFileDescriptorParser.SetString(sHTTP_Body.GetToken(lToken));
        if (sFileDescriptorParser.GetNumTokens() != 4)
          ZdGenerateException("Update text '%s'\ncontains %d file details, want 4.", "ConnectToServerForUpdateCheck()",
                              sFileDescriptorParser.GetString(), sFileDescriptorParser.GetNumTokens());
        sFilename = sFileDescriptorParser.GetToken(giFilenameIndex);
        if (TMask(gsVersionMask).Matches(sFilename.GetCString())) {
          if (GetVersion(sFilename, gsUpdateVersion) > VERSION_NUMBER)
            gUpdateArchive.first = sFilename;
            gUpdateArchive.second = sFileDescriptorParser.GetToken(giURLIndex);
        }
        else if (sFilename == gsUpdaterFilename) {
          // found update application
          gUpdateApplication.first = sFilename;
          gUpdateApplication.second = sFileDescriptorParser.GetToken(giURLIndex);
        }
        bDone = gUpdateArchive.first.GetLength() && gUpdateApplication.first.GetLength();
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
  ZdString      sMessage, sVersion;
  bool          bReturn=false;

  try {
    if (GetVersion(sFilename, sVersion) > VERSION_NUMBER) {
      sMessage.printf("SaTScan v%s is available. Do you want to install now?", sVersion.GetCString());
      bReturn = (TBMessageBox::Response(this, "SaTScan Update Available", sMessage.GetCString(), MB_YESNO) == IDYES);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("DisplayDownloadOption()", "TBfrmImsUpdate");
    throw;
  }
  return bReturn;
}

/** returns version for passed update file */
ZdString& TfrmUpdateCheck::GetVersion(const ZdString& sFilename, ZdString& sVersion) const {
  const char  * vptr, * zipptr;

  vptr = strstr(sFilename.GetCString(), "v");
  zipptr = strstr(sFilename.GetCString(), ".zip");

  sVersion.Clear();
  while (++vptr != zipptr)
       sVersion += *vptr;
  return sVersion;
}

bool TfrmUpdateCheck::HasUpdates() const {
  return gUpdateArchive.first.GetLength() && gUpdateApplication.first.GetLength();
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







