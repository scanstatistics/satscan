//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "dlgULA.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
TssDlgULA *ssDlgULA;
//---------------------------------------------------------------------
//ClassDesc Begin TssDlgULA
//This is a dialog which displays a (possibly 'rtf' formatted) message
//(which is generated after construction by operating on the
//'rdtULA->Lines' member).
//It displays below the rdtULA an explanatory message, the gist of which
//is that the above is a "user license agreement" and clicking the "yes"
//button is equivalent to acceptance and clicking the "no" button is
//equivalent to rejection.
//The result of ShowModal will be one of [mrYes, mrNo].
//
//ClassDesc End TssDlgULA
//---------------------------------------------------------------------
__fastcall TssDlgULA::TssDlgULA(TComponent* AOwner)
	: TForm(AOwner)
{
  int iExplanationLineCount(rdtExplanation->Lines->Count);
  giLastObservedExplanationLineCount = iExplanationLineCount;
  int iLineHeight(rdtExplanation->DefAttributes->Height + VERT_PIXELS_BETWEEN_LINES_IN_EXPLANATION);
  int iExplanationInitialHeight(iExplanationLineCount * iLineHeight);

  rdtULA->Height = pnlULAAndExplanatoryText->ClientHeight - iExplanationInitialHeight - VERT_PIXELS_BETWEEN_ULA_AND_EXPLANATION;
  rdtExplanation->Height = iExplanationInitialHeight;
  rdtExplanation->Top = rdtULA->Top + rdtULA->Height + VERT_PIXELS_BETWEEN_ULA_AND_EXPLANATION;
}
//---------------------------------------------------------------------



void __fastcall TssDlgULA::pnlULAAndExplanatoryTextResize(
      TObject *Sender)
{
  int iExplanationLineCount(rdtExplanation->Lines->Count);
  int iLineCountDifference(iExplanationLineCount - giLastObservedExplanationLineCount);
  giLastObservedExplanationLineCount = iExplanationLineCount;//we observed it, so we'd better keep it in mind.
  int iLineHeight(rdtExplanation->DefAttributes->Height + VERT_PIXELS_BETWEEN_LINES_IN_EXPLANATION);
  int iHeightChange(iLineCountDifference * iLineHeight);

  if (iHeightChange > 0)
  {
    //shrink rdtEULA:
    rdtULA->Height = rdtULA->Height - iHeightChange;
    //expand rdtExplanation
    rdtExplanation->Height = rdtExplanation->Height + iHeightChange;
    rdtExplanation->Top = rdtExplanation->Top - iHeightChange;
  }
  else//if iHeightChange == 0, this will have the expected effect.
  {
    //shrink rdtExplanation
    rdtExplanation->Height = rdtExplanation->Height + iHeightChange;
    rdtExplanation->Top = rdtExplanation->Top - iHeightChange;
    //expand rdtEULA
    rdtULA->Height = rdtULA->Height - iHeightChange;
  }
}
//---------------------------------------------------------------------------

