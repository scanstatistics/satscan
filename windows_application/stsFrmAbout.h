//---------------------------------------------------------------------------
#ifndef stsFrmAboutH
#define stsFrmAboutH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TfrmAbout : public TForm
{
__published:	// IDE-managed Components
        TPanel *Panel1;
        TLabel *lblVersion;
        TLabel *lblTitle;
        TLabel *lblWebSite;
        TLabel *lblWebSiteLabel;
        TLabel *lblEmail;
        TLabel *lblEmailAddress;
        TLabel *lblReleaseDate;
        TMemo *Memo1;
        TButton *Button1;
        void __fastcall Button1Click(TObject *Sender);
        void __fastcall lblWebSiteClick(TObject *Sender);
        void __fastcall lblEmailAddressClick(TObject *Sender);
private:	// User declarations
        void    Setup();
public:		// User declarations
        __fastcall TfrmAbout(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmAbout *frmAbout;
//---------------------------------------------------------------------------
#endif
