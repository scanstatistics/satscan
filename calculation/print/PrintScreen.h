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

   inline bool GetIsCanceled() const {return false;}    
   inline void PrintLine(char *s) {printf(s);}
   inline void PrintWarningLine(char *s) {printf(s);}
};

//*****************************************************************************
#endif
