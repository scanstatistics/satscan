//*****************************************************************************
#ifndef __PrintScreen_H
#define __PrintScreen_H
//*****************************************************************************
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

//*****************************************************************************
#endif
