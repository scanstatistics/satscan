//---------------------------------------------------------------------------

#ifndef OptionsH
#define OptionsH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>
#include <Buttons.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TfrmOptions : public TForm
{
__published:	// IDE-managed Components
        TPanel *pnlClient;
        TLabel *lblComparisonApplication;
        TEdit *edtComparisonApplication;
        TButton *btnBrowseComparisonApp;
        TCheckBox *chkArchiveResults;
        TLabel *lblArchiveApplication;
        TEdit *edtArchiveApplication;
        TButton *btnBrowseArchiveApplication;
        TEdit *edtArchiveApplicationOptions;
        TLabel *lblArchiveApplicationOptions;
        TOpenDialog *OpenDialog;
        void __fastcall btnBrowseComparisonAppClick(TObject *Sender);
        void __fastcall btnBrowseArchiveApplicationClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TfrmOptions(TComponent* Owner);
};
//---------------------------------------------------------------------------
#endif
