//*****************************************************************************
#ifndef __SMALLOC_H
#define __SMALLOC_H
//*****************************************************************************
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "BasePrint.h"
#include "SSException.h"

/**********************************************************************
 file: Salloc.h
 Header file for "safe allocation" functions -- they duplicate
 standard "malloc()", etc., but are guaranteed always to succeed.  If
 they cannot return a good value, they will call "SallocError()", and
 return its return value.  The default SallocError() function prints
 a message on stderr, and exits.
 **********************************************************************/

extern void *(*SallocError)(size_t size, BasePrint *pPrintDirection);

void *Scalloc(size_t, size_t, BasePrint *pPrintDirection);
void *Smalloc(size_t, BasePrint *pPrintDirection);
void *Srealloc(void *, size_t, BasePrint *pPrintDirection);
char *Sstrdup(char *s, BasePrint *pPrintDirection);
char* Sstrcpy(char** s1, char* s2, BasePrint *pPrintDirection);

//*****************************************************************************
#endif
