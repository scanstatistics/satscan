//---------------------------------------------------------------------------

#ifndef TfrmAboutH
#define TfrmAboutH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>

#include "SatScan.h"
//---------------------------------------------------------------------------
class TfrmAbout : public TForm
{
__published:	// IDE-managed Components
        TLabel *Label1;
        TLabel *lblVersionDate;
        TLabel *Label12;
        TLabel *lblEmail;
        TButton *Button1;
        TLabel *Label9;
        TLabel *Label10;
        TLabel *Label11;
        TLabel *Label13;
        TLabel *Label2;
        TLabel *Label4;
        TLabel *Label5;
        TLabel *Label6;
        TLabel *lblVersion;
        TLabel *Label7;
        void __fastcall Button1Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TfrmAbout(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmAbout *frmAbout;
//---------------------------------------------------------------------------
#endif
