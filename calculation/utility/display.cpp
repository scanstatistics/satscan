// display.cpp

#include <stdio.h>
#include "display.h"

void DisplayVersion(FILE* fp=stdout, int nPos=0)
{
  if (nPos==1)
    fprintf(fp,"                        ");
  fprintf(fp, "SaTScan V.2.1.4\n");
}

/*void HoldForEnter()
{
#ifndef OUTPUT_TO_WINDOW
  char c;
  printf("Press <Enter> to continue.");
  c = getc(stdin);
#endif
}

void HoldForEnter(const char* szMessage)
{
#ifndef OUTPUT_TO_WINDOW
  char c;
  printf(szMessage);
  c = getc(stdin);
#endif
} */


