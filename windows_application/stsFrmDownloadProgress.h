//---------------------------------------------------------------------------
#ifndef stsFrmDownloadProgressH
#define stsFrmDownloadProgressH

#include <ComCtrls.hpp>
#include <NMHttp.hpp>
#include <Psock.hpp>
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <vector>
#include <string>
#include "SaTScan.h"
extern const char *   	UPDATE_FILE_NAME;
//---------------------------------------------------------------------------
class TfrmDownloadProgress : public TForm {
__published:	// IDE-managed Components
   TLabel *LabelStep;
   TProgressBar *ProgressBarDownload;
   TButton *ButtonCancel;
        TNMHTTP *NMWebDownload;
   void __fastcall OnCancelClick(TObject *Sender);
   void __fastcall OnPacketReceived(TObject *Sender);
   void __fastcall OnCommandFailure(CmdType Cmd);
   
  private:	// User declarations
    void                                        Init() {gbCompleted=false; gbUpdateProgressCaption=false; giCurrentDownload=0;}

    size_t                                      giCurrentDownload;
    bool                                        gbCompleted;
    bool                                        gbUpdateProgressCaption;
    std::vector<std::pair<ZdString,ZdString> >  gvDownloads;

    ZdFileName                                & GetFullPath(const ZdString& sFileName, ZdFileName& Filename);

  public:		// User declarations
    __fastcall TfrmDownloadProgress(TComponent* Owner);

    void                                        Add(std::pair<ZdString, ZdString>& FileInfo);
    void                                        DownloadFiles();
    bool                                        GetDownloadCompleted() const {return gbCompleted;}
};
#endif

