//---------------------------------------------------------------------------
#ifndef StsMainH
#define StsMainH
//---------------------------------------------------------------------------
#include <Buttons.hpp>
#include <Classes.hpp>
#include <ComCtrls.hpp>
#include <Controls.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
#include <fcntl.h>
#include <ActnList.hpp>
#include <ImgList.hpp>

//---------------------------------------------------------------------------
class TfrmMainForm : public TForm{
  __published:	// IDE-managed Components
        TStatusBar *barStatusBar;
        TMainMenu *MainMenu1;
        TMenuItem *File1;
        TMenuItem *Help1;
        TMenuItem *Save1;
        TMenuItem *Exit1;
        TMenuItem *Session1;
        TMenuItem *Execute1;
        TMenuItem *Contents1;
        TMenuItem *N1;
        TMenuItem *AboutSatScan1;
        TMenuItem *Exit2;
        TBevel *Bevel1;
        TPanel *pnlSpeedButton;
        TSpeedButton *sbOpen;
        TSpeedButton *sbSave;
        TSpeedButton *sbHelp;
        TSpeedButton *sbExecute;
        TSpeedButton *sbEditSession;
        TBevel *Bevel2;
        TBevel *Bevel3;
        TBevel *Bevel4;
        TMenuItem *mitOpenSession;
        TMenuItem *Close1;
        TMenuItem *N2;
        TMenuItem *SaveAs1;
        TMenuItem *N3;
        TOpenDialog *OpenDialog1;
        TSaveDialog *SaveDialog1;
        TMenuItem *mitOpenParameterFile;
        TActionList *ActionList;
        TAction *NewSessionAction;
        TImageList *ImageList;
        TAction *OpenParameterFileAction;
        TAction *SaveSessionAction;
        TAction *SaveSessionAsAction;
        TAction *CloseSessionAction;
        TAction *PrintResultsAction;
        TAction *ExitAction;
        TAction *ExecuteAction;
        TAction *HelpAction;
        TAction *AboutSatscanAction;
        TMenuItem *ImportInputFile1;
        TBevel *Bevel5;
        TBevel *Bevel6;
        TMenuItem *mitReopen;
        TAction *ReopenAction;
        TAction *UpdateAction;
        TMenuItem *N4;
        TMenuItem *SaveSessionAs1;
        TAction *AdvancedParametersAction;
        TMenuItem *N5;
   TSpeedButton *sbUpdate;
        TMenuItem *Print1;
        TBevel *Bevel7;
        TSpeedButton *SpeedButton1;
        TMenuItem *SaTScanUserGuide1;
        TAction *AdobeUserGuideAction;
        void __fastcall PrintResultsActionExecute(TObject *Sender);
        void __fastcall NewSessionActionExecute(TObject *Sender);
        void __fastcall OpenParameterFileActionExecute(TObject *Sender);
        void __fastcall SaveSessionActionExecute(TObject *Sender);
        void __fastcall SaveSessionAsActionExecute(TObject *Sender);
        void __fastcall CloseSessionActionExecute(TObject *Sender);
        void __fastcall ExitActionExecute(TObject *Sender);
        void __fastcall ExecuteActionExecute(TObject *Sender);
        void __fastcall HelpActionExecute(TObject *Sender);
        void __fastcall AboutSatscanActionExecute(TObject *Sender);
        void __fastcall ReopenActionExecute(TObject *Sender);
        void __fastcall OnFormClose(TObject *Sender, TCloseAction &Action);
        void __fastcall OnFormActivate(TObject *Sender);
        void __fastcall UpdateActionExecute(TObject *Sender);
        void __fastcall AdvancedParametersActionExecute(TObject *Sender);
        void __fastcall AdobeUserGuideActionExecute(TObject *Sender);
  private:	// User declarations
    bool                        gbShowStartWindow;
    stsOutputFileRegister       gRegistry;

    bool                        CloseParameterSettingsWindows();
    void                        CloseRunningAnalysesWindows();
    void                        EnableActions(bool bEnable);
    bool                        GetAnalysesRunning();
    void                        Init() {gbShowStartWindow=true;}
    void                        OpenAFile();
    void                        Save();
    void                        SaveAs();
    void                        Setup();

  public:		// User declarations
    __fastcall TfrmMainForm(TComponent* Owner);
    virtual __fastcall ~TfrmMainForm();

    void                        RefreshOpenList();
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmMainForm *frmMainForm;
//---------------------------------------------------------------------------
#endif
