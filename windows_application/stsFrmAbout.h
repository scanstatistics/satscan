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
        TLabel *lblReleaseDate;
        TButton *Button1;
        TLabel *lblWebSiteLabel;
        TLabel *lblWebSite;
        TLabel *lblAuthors;
        TLabel *lblUsuage;
        TLabel *lblCitation;
        TLabel *lblVersion;
        TLabel *lblFurtherInfomation;
        TLabel *lblTitle;
        TLabel *lblEmail;
        TLabel *lblEmailAddress;
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
