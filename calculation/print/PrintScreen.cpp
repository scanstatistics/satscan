#include "SaTScan.h"
#pragma hdrstop
#include "PrintScreen.h"

PrintScreen::PrintScreen()
            : BasePrint()
{
}
//---------------------------------------------------------------------------
PrintScreen::~PrintScreen()
{
}
//---------------------------------------------------------------------------
bool PrintScreen::GetIsCanceled()
{
   return false;
}
//---------------------------------------------------------------------------
void PrintScreen::PrintLine(char *s)
{
   printf(s);
}
//---------------------------------------------------------------------------
void PrintScreen::PrintWarningLine(char *s)
{
   printf(s);
}
//---------------------------------------------------------------------------


