#ifndef xbfrmIMSUpdateH
#define xbfrmIMSUpdateH

//---------------------------------------------------------------------------
#include <ComCtrls.hpp>
#include <Psock.hpp>
#include <NMHttp.hpp>
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Buttons.hpp>
//---------------------------------------------------------------------------

class TfrmUpdateCheck : public TForm {
  __published:	// IDE-managed Components
    TLabel *lblMessage;
    TNMHTTP *pHTTPConnect;
    void __fastcall OnCommandFailure(CmdType Cmd);
    void __fastcall OnConnectionFailed(TObject *Sender);

  public:	// User declarations
    static const char                         * gsUpdaterFilename;
    static const char                         * gsURL;
    static const char                         * gsVersionMask;
    static const int                            giFilenameIndex;
    static const int                            giURLIndex;

  private:
    std::pair<ZdString,ZdString>                gUpdateApplication;
    std::pair<ZdString,ZdString>                gUpdateArchive;
    ZdString                                    gsUpdateVersion;

    bool                                        DisplayDownloadOption(const ZdString& sFilename);
    ZdString                                  & GetVersion(const ZdString& sFilename, ZdString& sVersion) const;

   public:		// User declarations
      __fastcall TfrmUpdateCheck(TComponent* Owner);
      __fastcall virtual ~TfrmUpdateCheck();

      void                                      ConnectToServerForUpdateCheck();
      bool                                      HasUpdates() const;
      std::pair<ZdString,ZdString>            & GetUpdateApplicationInfo() {return gUpdateApplication;}
      std::pair<ZdString,ZdString>            & GetUpdateArchiveInfo() {return gUpdateArchive;}
      const ZdString                          & GetUpdateVersion() const {return gsUpdateVersion;}
};
#endif
