//----------------------------------------------------------------------------
#ifndef dlgULAH
#define dlgULAH
//----------------------------------------------------------------------------
#include <vcl\System.hpp>
#include <vcl\Windows.hpp>
#include <vcl\SysUtils.hpp>
#include <vcl\Classes.hpp>
#include <vcl\Graphics.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\Controls.hpp>
#include <vcl\Buttons.hpp>
#include <vcl\ExtCtrls.hpp>
#include <ComCtrls.hpp>
//----------------------------------------------------------------------------
class TssDlgULA : public TForm
{
__published:
        TPanel *pnlULAAndExplanatoryText;
        TPanel *pnlButtons;
        TButton *btnYes;
        TButton *btnNo;
        TRichEdit *rdtExplanation;
        TRichEdit *rdtULA;
        void __fastcall pnlULAAndExplanatoryTextResize(TObject *Sender);
private:
  static int const VERT_PIXELS_BETWEEN_ULA_AND_EXPLANATION = 6;
  static int const VERT_PIXELS_BETWEEN_LINES_IN_EXPLANATION = 3;
  int giLastObservedExplanationLineCount;//the number of lines used to display the explanation last time anyone looked.
  
public:
	virtual __fastcall TssDlgULA(TComponent* AOwner);
};
//----------------------------------------------------------------------------
extern PACKAGE TssDlgULA *ssDlgULA;
//----------------------------------------------------------------------------
#endif    
