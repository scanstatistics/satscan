//---------------------------------------------------------------------------
#include "stsSaTScan.h"
#pragma hdrstop
//#include "stsFrmDownloadProgress.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
__fastcall TfrmDownloadProgress::TfrmDownloadProgress(TComponent* Owner) : TForm(Owner) {
  Init();
}

/** adds file info to download list */
void TfrmDownloadProgress::Add(std::pair<std::string, std::string>& FileInfo) {
  gvDownloads.push_back(FileInfo);
}

/** downloads files */
void TfrmDownloadProgress::DownloadFiles() {
  FileName         fDownloadFile;
  std::string      s, buffer;

  try {                                  
    Show();
    // Download the new files
    for (giCurrentDownload=0; giCurrentDownload < gvDownloads.size(); ++giCurrentDownload) {
       GetFullPath(gvDownloads[giCurrentDownload].first, fDownloadFile);
       ZdIOInterface::Delete(fDownloadFile.getFullPath(buffer).c_str());
       NMWebDownload->Body = fDownloadFile.getFullPath(buffer).c_str();
       gbUpdateProgressCaption = true;
       try {
         NMWebDownload->Get(gvDownloads[giCurrentDownload].second.c_str());
       }
       catch (EAbortError &x) {
         try {
           for (int t=giCurrentDownload; t >= 0; --t) {
              GetFullPath(gvDownloads[t].first, fDownloadFile);
              ZdIOInterface::Delete(fDownloadFile.getFullPath(buffer).c_str());
           }
         }
         catch (...){}
         throw ZdException("SaTScan update cancelled.","DownloadFiles()", ZdException::Notify);
       }
       catch (...) {
         throw ZdException("SaTScan was unable to download version updates. Internet connection may have been lost.\n"
                           "Please check your connection status and try again.",
                           "DownloadFiles()", ZdException::Notify);
       }
    }
    gbCompleted = true;
    Close();
  }
  catch (ZdException &x) {
    Close();
    x.AddCallpath("DownloadFiles()","TfrmDownloadProgress");
    throw;
  }
}

/** returns full path of download file */
FileName& TfrmDownloadProgress::GetFullPath(const std::string& sFileName, FileName& Filename) {
  Filename.setFullPath(sFileName.c_str());
  Filename.setLocation(ExtractFilePath(Application->ExeName).c_str());
  return Filename;
}

/** cancel click response */
void __fastcall TfrmDownloadProgress::OnCancelClick(TObject *Sender) {
  gbCompleted = false;
  NMWebDownload->Cancel();
}

void __fastcall TfrmDownloadProgress::OnPacketReceived(TObject *Sender) {
  try {
    if (gbUpdateProgressCaption) {
      ZdString      s;
      s.printf("Downloading file %d of %d (%i bytes) ...", giCurrentDownload + 1, gvDownloads.size(), NMWebDownload->BytesTotal);
      LabelStep->Caption = s.GetCString();
      gbUpdateProgressCaption = false;
    }
    ProgressBarDownload->Position = (NMWebDownload->BytesRecvd * 100) / NMWebDownload->BytesTotal;
  }
  catch (ZdException &x) {
    x.AddCallpath("OnPacketReceived()","TfrmDownloadProgress");
    DisplayBasisException(this, x);
  }
}

void __fastcall TfrmDownloadProgress::OnCommandFailure(CmdType Cmd) {
  switch(Cmd) {
    case CmdGET         : /*HTTP GET Failed*/
    case CmdPOST        : /*HTTP Post Failed*/
    case CmdHEAD        : /*HTTP HEAD Failed*/
    case CmdOPTIONS     : /*HTTP OPTIONS Failed*/
    case CmdTRACE       : /*HTTP TRACE Failed*/
    case CmdPUT         : /*HTTP PUT Failed*/
    case CmdDELETE      : /*HTTP Delete Failed*/
    default             : break;
  }
}


