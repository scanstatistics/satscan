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

static void *AllocationError(size_t size);

/* User-definable function to handle allocation errors */
void *(*SallocError)(size_t size) = AllocationError;

static void *AllocationError(size_t size)
{
   char c;

   fprintf(stderr,
      "\nOut of memory: unable to allocate %ld bytes.  Program aborted.\n",
      (long)size);

   printf("\nPress <Enter> to exit the SaTScan program.");
   c = getc(stdin);

   exit(1);

   return 0;
}

void *Scalloc(size_t nitems, size_t size)
{
    void *rtn = calloc(nitems, size);
    if (size && rtn == NULL)
        rtn = SallocError(size);
    return rtn;
}

void *Smalloc(size_t size)
{
    void *rtn = malloc(size);
    if (size && rtn == NULL)
        rtn = SallocError(size);
    return rtn;
}

void *Srealloc(void *block, size_t size)
{
    void *rtn = realloc(block, size);
    if (size && rtn == NULL)
        rtn = SallocError(size);
    return rtn;
}

char *Sstrdup(char *s)
{
   char *rtn = strdup(s);
   if (s && rtn == 0)
      rtn = (char*)SallocError((size_t) strlen(s) + 1);
   return rtn;
}

char* Sstrcpy(char** s1, char* s2)
{
  if (s2 != NULL)
    *s1 = (char*) malloc(strlen(s2)+1);

  if (*s1 == NULL)
    return((char*)SallocError((size_t) strlen(s2)+1));

  strcpy(*s1, s2);
  return(*s1);
}

