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
        TPanel *pnlClient;
        TButton *btnCreateNew;
        TButton *btnOpenSaved;
        TButton *btnOpenLast;
        TButton *btnCancel;
        void __fastcall FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
        void __fastcall btnCreateNewClick(TObject *Sender);
        void __fastcall btnOpenSavedClick(TObject *Sender);
        void __fastcall btnOpenLastClick(TObject *Sender);
        void __fastcall btnCancelClick(TObject *Sender);

  public:
    enum StartWindowOpenType {NEW, SAVED, LAST, CANCEL};

  private:	// User declarations
    StartWindowOpenType geOpenType;

    void                Setup();

  public:		// User declarations
    __fastcall TfrmStartWindow(TComponent* Owner);

    StartWindowOpenType GetOpenType() const {return geOpenType;}
};
#endif
