#include "SaTScan.h"
#pragma hdrstop
#include "Salloc.h"

/**********************************************************************
 file: Salloc.c
 "Safe allocation" functions -- they duplicate
 standard "malloc()", etc., but are guaranteed always to succeed.  If
 they cannot return a good value, they will call "SallocError()", and
 return its return value.  The default SallocError() function prints
 a message on stderr, and exits.
 **********************************************************************/

void *Scalloc(size_t nitems, size_t size, BasePrint*) {
  void *rtn;

  try {
    if (size && rtn == NULL)
      ZdMemoryException::Generate("Error: Failed not allocate enough memory for %d elements of size %d.\n", "Scalloc()", nitems, size);
  }
  catch (ZdException &x) {
    x.AddCallpath("Scalloc()","Salloc.cpp");
    throw;
  }
  return rtn;
}

void *Smalloc(size_t size, BasePrint*) {
  void *rtn;

  try {
    rtn = malloc(size);
    if (size && rtn == NULL)
      ZdMemoryException::Generate("Error: Failed not allocate %d bytes.\n", "Smalloc()", size);
  }
  catch (ZdException &x) {
    x.AddCallpath("Smalloc()","Salloc.cpp");
    throw;
  }
  return rtn;
}

void *Srealloc(void *block, size_t size, BasePrint*) {
  void *rtn;

  try {
    rtn = realloc(block, size);
    if (size && rtn == NULL)
      ZdMemoryException::Generate("Error: Failed not allocate %d bytes.\n", "Srealloc()", size);
  }
  catch (ZdException &x) {
    x.AddCallpath("Srealloc()","Salloc.cpp");
    throw;
  }
  return rtn;
}

