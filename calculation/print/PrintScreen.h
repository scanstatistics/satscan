//---------------------------------------------------------------------------

#ifndef PrintScreenH
#define PrintScreenH

#include <stdio.h>
#include "BasePrint.h"

class PrintScreen : public BasePrint
{
   public:
      PrintScreen();
      ~PrintScreen();

   bool GetIsCanceled();    
   void PrintLine(char *s);
   void PrintWarningLine(char *s);
};
//---------------------------------------------------------------------------
#endif
