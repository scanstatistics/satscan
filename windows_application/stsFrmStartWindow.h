//---------------------------------------------------------------------------
#ifndef stsFrmStartWindowH
#define stsFrmStartWindowH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TfrmStartWindow : public TForm {
__published:	// IDE-managed Components
        TPanel *pnlAlignRight;
        TPanel *pnlClient;
        TRadioGroup *rdgOpenChoices;
        TButton *btnOk;
        TButton *btnCancel;
        void __fastcall rdgOpenChoicesClick(TObject *Sender);
  private:	// User declarations
    int                 giSelectedIndex;

    void                Setup();

  public:		// User declarations
    __fastcall TfrmStartWindow(TComponent* Owner);

    int                 GetSelectedItemIndex() const {return giSelectedIndex;}
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmStartWindow *frmStartWindow;
//---------------------------------------------------------------------------
#endif
