//---------------------------------------------------------------------------
#ifndef MainH
#define MainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <ComCtrls.hpp>
#include <Controls.hpp>
#include <Dialogs.hpp>
#include <ImgList.hpp>
#include <StdCtrls.hpp>
#include "zd540.h"
#include <ExtCtrls.hpp>
#include <Menus.hpp>
#include <ActnList.hpp>
#include <Registry.hpp>
#include <ToolWin.hpp>
#include <Buttons.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
//---------------------------------------------------------------------------

class TfrmMain : public TForm {
__published:	// IDE-managed Components
        TOpenDialog *OpenDialog;
        TImageList *ImageList;
        TPanel *pnlTop;
        TActionList *ActionList1;
        TAction *ActionStart;
        TLabel *lblSaTScanBatchExecutable;
        TEdit *edtBatchExecutableName;
        TLabel *lblParameterFileList;
        TEdit *edtParameterListFile;
        TButton *btnBrowseParametersListFile;
        TButton *btnBrowseBatchExecutable;
        TPanel *pnlClient;
        TListView *lstDisplay;
        TAction *ActionSaveResults;
        TToolBar *ToolBar1;
        TBevel *Bevel1;
        TToolButton *ToolButton2;
        TToolButton *ToolButton4;
        TBevel *Bevel2;
        TSpeedButton *sbtnSaveComparisonResults;
        TSpeedButton *sbtnStartComparing;
        TSaveDialog *SaveDialog;
        TSpeedButton *sbtnCompare;
        TToolButton *ToolButton1;
        TAction *ActionCompareResultFiles;
        void __fastcall btnBrowseBatchExecutableClick(TObject *Sender);
        void __fastcall btnBrowseParametersListFileClick(TObject *Sender);
        void __fastcall ActionStartExecute(TObject *Sender);
        void __fastcall edtChangeInput(TObject *Sender);
        void __fastcall ActionSaveResultsExecute(TObject *Sender);
        void __fastcall ActionCompareResultFilesExecute(TObject *Sender);
        void __fastcall lstDisplaySelectItem(TObject *Sender,
          TListItem *Item, bool Selected);
        void __fastcall FormShow(TObject *Sender);
   
  private:
    std::vector<ZdFileName>     gvParameterList;
    std::string                 gsComparisonProgram;

    static const char         * SCU_REGISTRY_KEY;
    static const char         * LASTAPP_DATA;
    static const char         * LASTPARAMLIST_DATA;
    static const char         * COMPARE_APP_DATA;
    static const char         * COMPARE_FILE_EXTENSION;

    void                        AddList(const char * sParameterFilename, const char * sMessage, bool bError);
    bool                        CompareResultFiles(std::string & sCorrectFile, std::string & sFileToValidate);
    void                        EnableCompareResultFilesAction();
    void                        EnableSaveResultsAction();
    void                        EnableStartAction();
    bool                        Execute(const AnsiString & sCommandLine);
    std::string               & GetCompareFilename(const ZdFileName & ParameterFilename, std::string & sResultFilename);
    std::string               & GetResultFileName(const ZdFileName & ParameterFilename, std::string & sResultFilename);

  public:
     __fastcall TfrmMain(TComponent* Owner);
     __fastcall ~TfrmMain();
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmMain *frmMain;
//---------------------------------------------------------------------------
#endif
 