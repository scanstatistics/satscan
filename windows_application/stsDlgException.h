//Author Dave Hunt, Brian Simpson
//
//---------------------------------------------------------------------------
#ifndef __xbDlgException_H
#define __xbDlgException_H

//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <stdio.h>
#include <Mapi.hpp>       // VCL Header
#include <Printers.hpp>   // VCL Header
//---------------------------------------------------------------------------
class TBdlgException : public TForm
{
__published:	// IDE-managed Components
        TImage *imgAlarmLevelImage;
        TRichEdit *rteAlarmLevelInfo;
        TRichEdit *rteUserInfo;
        TRichEdit *rteExtendedInfo;
        TButton *btnOK;
        TButton *btnPrint;
        TButton *btnEmail;
        TButton *btnMoreLess;
        TBevel *Bevel1;
        void __fastcall btnPrintClick(TObject *Sender);
        void __fastcall btnEmailClick(TObject *Sender);
        void __fastcall btnMoreLessClick(TObject *Sender);

private:	// User declarations
   void         Setup ( const SSException &theException, const char *sAlarmLevelString );

protected:	// User declarations
   bool         gbIsAmplified;
   AnsiString     gsMoreLessButtonCaption_More;
   AnsiString    gsMoreLessButtonCaption_Less;
   SSException::Level geLevel;

   void ArrangeButtons(SSException::Level eLevel);
  // bool GetEmailButtonCanBeEnabled();
   void OnCreate(SSException::Level eLevel);
  // void OnEmailClick();
   void OnMoreLessClick();
   void OnPrintClick();
   void SetIcon(SSException::Level eLevel);

public:		// User declarations
   __fastcall TBdlgException(TComponent* Owner, const SSException &theException, const char * sAlarmLevelString = 0);
   __fastcall virtual ~TBdlgException();

   void         Abridge();
   void         Amplify();
   bool         IsAmplified() const;
};

void DisplayBasisException ( const SSException &theException, const char *sNotify, const char *sWarning );//deprecated
void DisplayBasisException ( const SSException &theException, const char *sAlarmLevelString = 0 );//deprecated
void DisplayBasisException ( Classes::TComponent * Owner, const SSException &theException, const char *sAlarmLevelString = 0 );

//---------------------------------------------------------------------------
extern PACKAGE TBdlgException *BdlgException;
//---------------------------------------------------------------------------
#endif
