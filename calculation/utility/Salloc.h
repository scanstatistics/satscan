/**********************************************************************
 file: salloc.h
 Header file for "safe allocation" functions -- they duplicate
 standard "malloc()", etc., but are guaranteed always to succeed.  If
 they cannot return a good value, they will call "SallocError()", and
 return its return value.  The default SallocError() function prints
 a message on stderr, and exits.
 **********************************************************************/

#ifndef SMALLOC_H
#define SMALLOC_H

#include <stdlib.h>

extern void *(*SallocError)(size_t size);

void *Scalloc(size_t, size_t);
void *Smalloc(size_t);
void *Srealloc(void *, size_t);
char *Sstrdup(char *s);
char* Sstrcpy(char** s1, char* s2);

#endif
