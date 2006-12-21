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
        TPanel *pnlRootPanel;
        TPanel *pnlRootPanelTop;
        TLabel *lblVersion;
        TPanel *pnlRootPanelRight;
        TPanel *pnlRootPanelLeft;
        TPanel *pnlRootPanelBottom;
        TPanel *Panel1;
        TLabel *lblTitle;
        TLabel *lblWebSiteLabel;
        TLabel *lblWebSite;
        TLabel *lblEmail;
        TLabel *lblEmailAddress;
        TLabel *lblReleaseDate;
        TLabel *Label1;
        TLabel *Label2;
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
