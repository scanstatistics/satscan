/**********************************************************************
 file: salloc.c
 "Safe allocation" functions -- they duplicate
 standard "malloc()", etc., but are guaranteed always to succeed.  If
 they cannot return a good value, they will call "SallocError()", and
 return its return value.  The default SallocError() function prints
 a message on stderr, and exits.
 **********************************************************************/

#include <stdio.h>
#include <string.h>
#include "salloc.h"

static void *AllocationError(size_t size, BasePrint *pPrintDirection);

/* User-definable function to handle allocation errors */
void *(*SallocError)(size_t size, BasePrint *pPrintDirection) = AllocationError;

static void *AllocationError(size_t size, BasePrint *pPrintDirection)
{
   char c;

   try
      {
      fprintf(stderr,"\nOut of memory: unable to allocate %ld bytes.  Program aborted.\n",(long)size);
      SSGenerateException("\nCould not allocate enough memory.  This SaTScan run has been terminated.", "AllocationError()");
      }
   catch (SSException & x)
      {
      x.AddCallpath("AllocationError()", "Salloc.cpp");
      throw;
      }
   return 0;
}

void *Scalloc(size_t nitems, size_t size, BasePrint *pPrintDirection)
{
   void *rtn;

   try
      {
      rtn = calloc(nitems, size);
      if (size && rtn == NULL)
         rtn = SallocError(size, pPrintDirection);
      }
   catch (SSException & x)
      {
      x.AddCallpath("Scalloc()", "Salloc.cpp");
      throw;
      }
   return rtn;
}

void *Smalloc(size_t size, BasePrint *pPrintDirection)
{
   void *rtn;
   try
      {
      rtn = malloc(size);
      if (size && rtn == NULL)
         rtn = SallocError(size, pPrintDirection);
      }
   catch (SSException & x)
      {
      x.AddCallpath("Smalloc()", "Salloc.cpp");
      throw;
      }
   return rtn;
}

void *Srealloc(void *block, size_t size, BasePrint *pPrintDirection)
{
   void *rtn;
   try
      {
      rtn = realloc(block, size);
      if (size && rtn == NULL)
         rtn = SallocError(size, pPrintDirection);
      }
   catch (SSException & x)
      {
      x.AddCallpath("Srealloc()", "Salloc.cpp");
      throw;
      }
   return rtn;
}

char *Sstrdup(char *s, BasePrint *pPrintDirection)
{
   char *rtn;
   try
      {
      rtn = strdup(s);
      if (s && rtn == 0)
         rtn = (char*)SallocError((size_t) (strlen(s) + 1), pPrintDirection);
      }
   catch (SSException & x)
      {
      x.AddCallpath("Sstrdup()", "Salloc.cpp");
      throw;
      }
   return rtn;
}

char* Sstrcpy(char** s1, char* s2, BasePrint *pPrintDirection)
{
   try
      {
      if (s2 != NULL)
         *s1 = (char*) malloc(strlen(s2)+1);

      if (*s1 == NULL)
         return((char*)SallocError((size_t) strlen(s2)+1, pPrintDirection));

      strcpy(*s1, s2);
      }
   catch (SSException & x)
      {
      x.AddCallpath("Sstrcpy()", "Salloc.cpp");
      throw;
      }
  return(*s1);
}

