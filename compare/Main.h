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
#include <dos.h>
#include "Options.h"
//---------------------------------------------------------------------------

enum CompareType {UNKNOWN=0, MASTER_MISSING, COMPARE_MISSING, EQUAL, NOT_EQUAL, NOT_APPLICABLE};
enum TimeDifferenceType {INCOMPLETE=0, SLOWER, SAME, FASTER};

class ParameterResultsInfo {
  private:
    CompareType         geClusterInformation;
    CompareType         geLocationInformation;
    CompareType         geRelativeRisks;
    CompareType         geSimulatedRatios;
    ZdFileName          gParameterFilename;
    TimeDifferenceType  geTimeDifferenceType;
    unsigned short      guHoursDifferent;
    unsigned short      guMinutesDifferent;
    unsigned short      guSecondsDifferent;
    float               gfTimeDifferencePercentage;

  public:
    ParameterResultsInfo(const char * sParameterFilename);
    ~ParameterResultsInfo();

    CompareType         GetClusterInformationType() const {return geClusterInformation;}
    bool                GetHasMisMatches() const;
    bool                GetHasMissingFiles() const;
    unsigned short      GetHoursDifferent() const {return guHoursDifferent;}
    CompareType         GetLocationInformationType() const {return geLocationInformation;}
    const ZdFileName  & GetFilename() const {return gParameterFilename;}
    const char        * GetFilenameString() const {return gParameterFilename.GetFullPath();}
    unsigned short      GetMinutesDifferent() const {return guMinutesDifferent;}
    CompareType         GetRelativeRisksType() const {return geRelativeRisks;}
    unsigned short      GetSecondsDifferent() const {return guSecondsDifferent;}
    CompareType         GetSimulatedRatiosType() const {return geSimulatedRatios;}
    float               GetTimeDifferencePercentage() const {return gfTimeDifferencePercentage;}
    TimeDifferenceType  GetTimeDifferenceType() const {return geTimeDifferenceType;}
    void                SetClusterInformationType(CompareType eCompareType) {geClusterInformation = eCompareType;}
    void                SetLocationInformationType(CompareType eCompareType) {geLocationInformation = eCompareType;}
    void                SetRelativeRisksType(CompareType eCompareType) {geRelativeRisks = eCompareType;}
    void                SetSimulatedRatiosType(CompareType eCompareType) {geSimulatedRatios = eCompareType;}
    void                SetTimeDifferencePercentage(float fPercentage) {gfTimeDifferencePercentage = fPercentage;}
    void                SetTimeDifference(unsigned short uHours, unsigned short uMinutes, unsigned short uSeconds, TimeDifferenceType geTimeDifferenceType);
};

class TfrmMain : public TForm {
__published:	// IDE-managed Components
        TOpenDialog *OpenDialog;
        TImageList *ImageList;
        TPanel *pnlTop;
        TActionList *ActionList1;
        TAction *ActionStart;
        TPanel *pnlClient;
        TListView *lstDisplay;
        TAction *ActionSaveComparisonStats;
        TToolBar *ToolBar1;
        TBevel *Bevel1;
        TToolButton *ToolButton2;
        TToolButton *ToolButton4;
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
        TAction *ActionAddParameterFile;
        TAction *ActionRemoveParameterFile;
        TAction *ActionSaveParametersList;
        TAction *ActionLoadParameterList;
        TPanel *pnlTopTop;
        TLabel *lblSaTScanBatchExecutableComparator;
        TEdit *edtBatchExecutableComparatorName;
        TLabel *lblSaTScanBatchExecutable;
        TEdit *edtBatchExecutableName;
        TButton *btnBrowseBatchExecutable;
        TButton *btnBrowseBatchComparatorExecutable;
        TPanel *Panel1;
        TLabel *lblParameterFileList;
        TListView *ltvScheduledBatchs;
        TPanel *pnlTopButtons;
        TBitBtn *btnSaveBatchDefinitions;
        TButton *btnAdd;
        TButton *btnRemove;
        TBitBtn *btnLoadDefinitions;
        TSplitter *Splitter1;
        TToolButton *btnOptions;
        TToolButton *ToolButton3;
        TAction *ActionOptions;
        TPanel *pnlBottom;
        TMemo *memRunErrors;
        TLabel *lblRunErrors;
        void __fastcall btnBrowseBatchExecutableClick(TObject *Sender);
        void __fastcall btnBrowseBatchExecutableComparatorClick(TObject *Sender);
        void __fastcall ActionStartExecute(TObject *Sender);
        void __fastcall edtChangeInput(TObject *Sender);
        void __fastcall ActionSaveComparisonStatsExecute(TObject *Sender);
        void __fastcall ActionCompareResultFilesExecute(TObject *Sender);
        void __fastcall lstDisplaySelectItem(TObject *Sender,TListItem *Item, bool Selected);
        void __fastcall ActionCompareSimulatedLLRsExecute(TObject *Sender);
        void __fastcall ActionCompareClusterInformationExecute(TObject *Sender);
        void __fastcall ActionCompareLocationInformationExecute(TObject *Sender);
        void __fastcall ActionCompareRelativeRisksExecute(TObject *Sender);
        void __fastcall lstDisplayColumnClick(TObject *Sender,TListColumn *Column);
        void __fastcall lstDisplayCompare(TObject *Sender,TListItem *Item1, TListItem *Item2, int Data, int &Compare);
        void __fastcall ActionAddParameterFileExecute(TObject *Sender);
        void __fastcall ActionRemoveParameterFileExecute(TObject *Sender);
        void __fastcall ActionSaveParametersListExecute(TObject *Sender);
        void __fastcall ActionLoadParameterListExecute(TObject *Sender);
        void __fastcall ltvScheduledBatchsKeyDown(TObject *Sender,
          WORD &Key, TShiftState Shift);
        void __fastcall ltvScheduledBatchsSelectItem(TObject *Sender,
          TListItem *Item, bool Selected);
        void __fastcall ActionOptionsExecute(TObject *Sender);
   
  private:
    std::vector<ParameterResultsInfo>   gvParameterResultsInfo;
    int                                 giSortColumnIndex;
    std::vector<int>                    gvColumnSortOrder;

    static const char                 * SCU_REGISTRY_KEY;
    static const char                 * LASTAPPCOMPARATOR_DATA;
    static const char                 * LASTAPP_DATA;
    static const char                 * PARAMETER_DATA;
    static const char                 * COMPARE_APP_DATA;
    static const char                 * COMPARE_FILE_EXTENSION;
    static const char                 * ARCHIVE_APP_DATA;
    static const char                 * USE_ARCHIVE_APP_DATA;
    static const char                 * ARCHIVE_APP_OPTIONS_DATA;

    TfrmOptions                       * gpFrmOptions;  

    void                                AddList();
    void                                AddList(const char * sMessage);
    void                                AddSubItemForType(TListItem * pListItem, CompareType eType);
    void                                ArchiveResults();
    void                                CompareClusterInformationFiles();
    void                                CompareLocationInformationFiles();
    void                                CompareRelativeRisksInformationFiles();
    void                                CompareSimulatedRatiosFiles();
    bool                                CompareTextFiles(const std::string & sMaster, const std::string & sCompare);
    void                                CompareTimes();
    void                                CreateStatsFile(const char * sFilename);
    void                                EnableCompareActions();
    void                                EnableRemoveParameterAction();
    void                                EnableSaveParametersListAction();
    void                                EnableSaveResultsAction();
    void                                EnableStartAction();
    bool                                Execute(const AnsiString & sCommandLine, bool bWindowed=true);
    std::string                       & GetCompareFilename(const ZdFileName & ParameterFilename, std::string & sResultFilename);
    AnsiString                        & GetDisplayTime(AnsiString & sDisplay);
    std::string                       & GetResultFileName(const ZdFileName & ParameterFilename, std::string & sResultFilename);
    bool                                GetRunTime(const char * sResultFile, unsigned short& uHours, unsigned short& uMinutes, unsigned short& uSeconds);
    bool                                PromptForCompareProgram();

  public:
     __fastcall TfrmMain(TComponent* Owner);
     __fastcall ~TfrmMain();
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmMain *frmMain;
//---------------------------------------------------------------------------
#endif
 