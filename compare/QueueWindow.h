//---------------------------------------------------------------------------
#ifndef QueueWindow_h
#define QueueWindow_h
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
#include <ActnList.hpp>
#include <Buttons.hpp>
#include <Menus.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TfrmQueueWindow : public TForm {
  __published:	// IDE-managed Components
        TPanel *pnlButtons;
        TPanel *pnlClient;
        TButton *btnAdd;
        TButton *btnRemove;
        TListView *ltvScheduledBatchs;
        TActionList *ActionList1;
        TAction *ActionScheduleBatch;
        TAction *ActionCancelBatches;
        TAction *ActionStartBatches;
        TAction *ActionRemoveBatch;
        TBitBtn *btnStart;
        TBitBtn *btnSaveBatchDefinitions;
        TAction *ActionSaveDefinitions;
        TBitBtn *btnLoadDefinitions;
        TAction *ActionLoadDefinitions;
        TOpenDialog *OpenDialog;
        TSaveDialog *SaveDialog;
        TAction *ActionCompareOutputFiles;
        TBitBtn *btnSelectCompareExecutable;
        TOpenDialog *OpenDialog2;
        TAction *ActionOptions;
        void __fastcall ActionScheduleBatchExecute(TObject *Sender);
        void __fastcall ActionStartBatchesExecute(TObject *Sender);
        void __fastcall ActionRemoveBatchExecute(TObject *Sender);
        void __fastcall ActionCancelBatchesExecute(TObject *Sender);
        void __fastcall OnScheduledBatchsSelectItem(TObject *Sender,
          TListItem *Item, bool Selected);
        void __fastcall ActionSaveDefinitionsExecute(TObject *Sender);
        void __fastcall ActionLoadDefinitionsExecute(TObject *Sender);
        void __fastcall ActionCompareOutputFilesExecute(TObject *Sender);
        void __fastcall ltvScheduledBatchsKeyDown(TObject *Sender,
          WORD &Key, TShiftState Shift);
  private:
    bool                        gProcessRunning;
    std::string                 gsLastExe;

    void                ClearBatches();
    void                EnableBatchExecutingButtons(bool b);
    void                EnableCompareFilesButton();
    void                EnableRemoveBatchButton();
    void                EnableSaveBatchDefinitionsButton();
    void                EnableStartBatchButton();
    void                ExecuteCreateProcessEachAnalysis();
    void                ExecuteThroughBatchFile();
    std::string       & GetOutputFilename(const ZdFileName & ParameterFilename, std::string& OutputFilename);
    void                RunVersionComparison();
    void                Setup();

  public:
    __fastcall TfrmQueueWindow(TComponent* Owner);

    void                AddBatch(const char * sExecutableFilename, const char * sParameterFilename, const char * sStatus="queued");

};
#endif
