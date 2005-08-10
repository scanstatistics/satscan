//---------------------------------------------------------------------------

#ifndef QueueWindowH
#define QueueWindowH
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
    TPanel *pnlClient;
    TListView *ltvScheduledBatchs;
    void __fastcall ActionStartBatchesExecute(TObject *Sender);
    
  private:

  public:
    __fastcall TfrmQueueWindow(TComponent* Owner);

    void        AddBatch(const char * sExecutableFilename, const char * sParameterFilename);
};
#endif
