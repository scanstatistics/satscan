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
    static const char                         * gsURLFormat;
    static const int                            giUpdateTokens;
    static const int                            giUpdateIndicatorIndex;
    static const int                            giUpdateVersionIdIndex;
    static const int                            giUpdateVersionIndex;
    static const int                            giUpdateAppNameIndex;
    static const int                            giUpdateAppUrlIndex;
    static const int                            giUpdateDataNameIndex;
    static const int                            giUpdateDataUrlIndex;

  private:
    std::pair<std::string,std::string>          gUpdateApplication;
    std::pair<std::string,std::string>          gUpdateArchive;
    std::string                                 gsUpdateVersion;
    bool                                        gbHasUpdates;

    bool                                        DisplayDownloadOption(const ZdString& sFilename);

   public:		// User declarations
      __fastcall TfrmUpdateCheck(TComponent* Owner);
      __fastcall virtual ~TfrmUpdateCheck();

      void                                      ConnectToServerForUpdateCheck();
      bool                                      HasUpdates() const;
      std::pair<std::string,std::string>      & GetUpdateApplicationInfo() {return gUpdateApplication;}
      std::pair<std::string,std::string>      & GetUpdateArchiveInfo() {return gUpdateArchive;}
      const std::string                       & GetUpdateVersion() const {return gsUpdateVersion;}
};
#endif
