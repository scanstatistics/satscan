//---------------------------------------------------------------------------
#ifndef stsFrmAboutH
#define stsFrmAboutH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
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
        TLabel *lblWebSite;
        TLabel *Label2;
        TLabel *Label4;
        TLabel *Label5;
        TLabel *Label6;
        TLabel *lblVersion;
        TLabel *Label7;
        TLabel *Label3;
        TLabel *Label8;
        TLabel *lblTrademark;
        TLabel *lblLicense;
        TLabel *Label14;
        void __fastcall Button1Click(TObject *Sender);
        void __fastcall lblWebSiteClick(TObject *Sender);
        void __fastcall lblEmailClick(TObject *Sender);
        void __fastcall lblLicenseClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TfrmAbout(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmAbout *frmAbout;
//---------------------------------------------------------------------------
#endif
