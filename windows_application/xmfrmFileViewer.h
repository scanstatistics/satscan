//---------------------------------------------------------------------------

#ifndef xmfrmFileViewerH
#define xmfrmFileViewerH
//---------------------------------------------------------------------------
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Classes.hpp>
#include <ComCtrls.hpp>
#include <Dialogs.hpp>
#include "SSException.h" 
//---------------------------------------------------------------------------
class TfrmOutputViewer : public TForm
{
__published:	// IDE-managed Components
        TButton *btnClose;
        TRichEdit *reOutputBox;
        TButton *btnPrint;
        TPrintDialog *PrintDialog1;
        void __fastcall btnCloseClick(TObject *Sender);
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
        void __fastcall btnPrintClick(TObject *Sender);
        void __fastcall FormCanResize(TObject *Sender, int &NewWidth,
          int &NewHeight, bool &Resize);
private:	// User declarations
         AnsiString FFileName;
public:		// User declarations
        __fastcall TfrmOutputViewer(TComponent* Owner, char *sOutputFileName = 0);
        void  AddLine(char *sLine);
        void  LoadFromFile(char *sFileName);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmOutputViewer *frmOutputViewer;
//---------------------------------------------------------------------------
#endif
