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

enum CompareType {UNKOWN, MASTER_MISSING=0, COMPARE_MISSING, EQUAL, NOT_EQUAL, NOT_APPLICABLE};

class ParameterResultsInfo {
  private:
    CompareType         geClusterInformation;
    CompareType         geLocationInformation;
    CompareType         geRelativeRisks;
    CompareType         geSimulatedRatios;
    ZdFileName          gParameterFilename;

  public:
    ParameterResultsInfo(const char * sParameterFilename): geClusterInformation(UNKOWN),
                                                           geLocationInformation(UNKOWN),
                                                           geRelativeRisks(UNKOWN),
                                                           geSimulatedRatios(UNKOWN)
                                                          {gParameterFilename = sParameterFilename;}
    ~ParameterResultsInfo() {}

    CompareType         GetClusterInformationType() const {return geClusterInformation;}
    bool                GetHasMisMatches() const {return geClusterInformation == NOT_EQUAL ||
                                                         geLocationInformation == NOT_EQUAL ||
                                                         geRelativeRisks == NOT_EQUAL ||
                                                         geSimulatedRatios == NOT_EQUAL;}
    bool                GetHasMissingFiles() const {return geClusterInformation < EQUAL ||
                                                         geLocationInformation < EQUAL ||
                                                         geRelativeRisks < EQUAL ||
                                                         geSimulatedRatios < EQUAL;}
    CompareType         GetLocationInformationType() const {return geLocationInformation;}
    const ZdFileName  & GetFilename() const {return gParameterFilename;}
    const char        * GetFilenameString() const {return gParameterFilename.GetFullPath();}
    CompareType         GetRelativeRisksType() const {return geRelativeRisks;}
    CompareType         GetSimulatedRatiosType() const {return geSimulatedRatios;}
    void                SetClusterInformationType(CompareType eCompareType) {geClusterInformation = eCompareType;}
    void                SetLocationInformationType(CompareType eCompareType) {geLocationInformation = eCompareType;}
    void                SetRelativeRisksType(CompareType eCompareType) {geRelativeRisks = eCompareType;}
    void                SetSimulatedRatiosType(CompareType eCompareType) {geSimulatedRatios = eCompareType;}
};

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
        TAction *ActionSaveComparisonStats;
        TToolBar *ToolBar1;
        TBevel *Bevel1;
        TToolButton *ToolButton2;
        TToolButton *ToolButton4;
        TBevel *Bevel2;
        TSpeedButton *sbtnSaveComparisonResults;
        TSpeedButton *sbtnStartComparing;
        TSaveDialog *SaveDialog;
        TAction *ActionCompareResultFiles;
        TAction *ActionCompareSimulatedLLRs;
        TAction *ActionCompareClusterInformation;
        TAction *ActionCompareLocationInformation;
        TAction *ActionCompareRelativeRisks;
        TPanel *pnlClientRight;
        TSpeedButton *sbtnCompareResultsFile;
        TSpeedButton *sbtnCompareCluster;
        TSpeedButton *sbtnCompareLocation;
        TSpeedButton *sbtnCompareRelativeRisks;
        TSpeedButton *sbtnCompareLLR;
        void __fastcall btnBrowseBatchExecutableClick(TObject *Sender);
        void __fastcall btnBrowseParametersListFileClick(TObject *Sender);
        void __fastcall ActionStartExecute(TObject *Sender);
        void __fastcall edtChangeInput(TObject *Sender);
        void __fastcall ActionSaveComparisonStatsExecute(TObject *Sender);
        void __fastcall ActionCompareResultFilesExecute(TObject *Sender);
        void __fastcall lstDisplaySelectItem(TObject *Sender,TListItem *Item, bool Selected);
        void __fastcall FormShow(TObject *Sender);
        void __fastcall ActionCompareSimulatedLLRsExecute(TObject *Sender);
        void __fastcall ActionCompareClusterInformationExecute(TObject *Sender);
        void __fastcall ActionCompareLocationInformationExecute(TObject *Sender);
        void __fastcall ActionCompareRelativeRisksExecute(TObject *Sender);
        void __fastcall lstDisplayColumnClick(TObject *Sender,TListColumn *Column);
        void __fastcall lstDisplayCompare(TObject *Sender,TListItem *Item1, TListItem *Item2, int Data, int &Compare);
   
  private:
    std::vector<ParameterResultsInfo>   gvParameterResultsInfo;
    std::string                         gsComparisonProgram;
    int                                 giSortColumnIndex;
    std::vector<int>                    gvColumnSortOrder;

    static const char                 * SCU_REGISTRY_KEY;
    static const char                 * LASTAPP_DATA;
    static const char                 * LASTPARAMLIST_DATA;
    static const char                 * COMPARE_APP_DATA;
    static const char                 * COMPARE_FILE_EXTENSION;

    void                                AddList();
    void                                AddList(const char * sMessage);
    void                                AddSubItemForType(TListItem * pListItem, CompareType eType);
    void                                CompareClusterInformationFiles();
    void                                CompareLocationInformationFiles();
    void                                CompareRelativeRisksInformationFiles();
    void                                CompareSimulatedRatiosFiles();
    bool                                CompareTextFiles(const std::string & sMaster, const std::string & sCompare);
    void                                EnableCompareActions();
    void                                EnableSaveResultsAction();
    void                                EnableStartAction();
    bool                                Execute(const AnsiString & sCommandLine);
    std::string                       & GetCompareFilename(const ZdFileName & ParameterFilename, std::string & sResultFilename);
    std::string                       & GetResultFileName(const ZdFileName & ParameterFilename, std::string & sResultFilename);
    bool                                PromptForCompareProgram();

  public:
     __fastcall TfrmMain(TComponent* Owner);
     __fastcall ~TfrmMain();
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmMain *frmMain;
//---------------------------------------------------------------------------
#endif
 