//---------------------------------------------------------------------------
#ifndef stsFrmSuggestedCitationH
#define stsFrmSuggestedCitationH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <AppEvnts.hpp>
#include <ExtCtrls.hpp>
#include <map>

//---------------------------------------------------------------------------
class TfrmSuggestedCitation : public TForm {
__published:	// IDE-managed Components
   TApplicationEvents *ApplicationEvents1;
   TRichEdit *RichEdit1;
   TStatusBar *StatusBar1;
   TPanel *pnlClientNorth;
   TPanel *pnlClientSouth;
   TPanel *pnlClientWest;
   TPanel *pnlClientEast;
   void __fastcall ApplicationEvents1Idle(TObject *Sender, bool &Done);
   void __fastcall FormResize(TObject *Sender);

private:	// User declarations
  void __fastcall TfrmSuggestedCitation::WndProc(Messages::TMessage &Message);

  std::map<long, size_t>        gmCharRangeMinToUrlIndex;
  std::vector<std::string>      gvCitationUrls;

  void                          FormatRichEditText(long lMin, long lMax);

public:		// User declarations
        __fastcall TfrmSuggestedCitation(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmSuggestedCitation *frmSuggestedCitation;
//---------------------------------------------------------------------------
#endif
