// display.cpp

#include <stdio.h>
#include "display.h"

void DisplayVersion(FILE* fp=stdout, int nPos=0)
{
  if (nPos==1)
    fprintf(fp,"                        ");
  fprintf(fp, "SaTScan V.2.1.3\n");
}

void HoldForEnter()
{
  char c;
  printf("Press <Enter> to continue.");
  c = getc(stdin);
}

void HoldForEnter(const char* szMessage)
{
  char c;
  printf(szMessage);
  c = getc(stdin);
}


