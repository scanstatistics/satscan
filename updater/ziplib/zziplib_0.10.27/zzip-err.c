/*
 * Author: 
 *	Guido Draheim <guidod@gmx.de>
 *    	Tomi Ollila <Tomi.Ollila@iki.fi>
 *
 *	Copyright (c) 2000,2001 Guido Draheim
 * 	Copyright (c) 1999 Tomi Ollila
 * 	    All rights reserved,
 *	    use under the restrictions of the
 *	    Lesser GNU General Public License
 */

#define ZZIP_DLL /* is part of zzip.dll */

#include <zlib.h>
#include <string.h>
#include <errno.h>

#include "zzip.h"

static struct errlistentry { int code; char* mesg; } 
errlist[] = 
{
    { ZZIP_NO_ERROR, "No error" },
    { ZZIP_OUTOFMEM, "could not get temporary memory for internal structures" },
    { ZZIP_DIR_OPEN, "Failed to open zip-file %s" },
    { ZZIP_DIR_STAT, "Failed to fstat zip-file %s" },
    { ZZIP_DIR_SEEK, "Failed to lseek zip-file %s" },
    { ZZIP_DIR_READ, "Failed to read zip-file %s"},  
    { ZZIP_DIR_TOO_SHORT,   "zip-file %s too short" },
    { ZZIP_DIR_EDH_MISSING, "zip-file central directory not found" },
    { ZZIP_DIRSIZE, "Directory size too big..." },
    { ZZIP_ENOENT, "No such file found in zip-file %s" },
    { ZZIP_UNSUPP_COMPR, "Unsupported compression format" },
    { ZZIP_CORRUPTED, "Zipfile corrupted" }, 
    { ZZIP_UNDEF,      "Some undefined error occurred" },
    { 0, 0 },
};

#define errlistSIZE (sizeof(errlist)/sizeof(*errlist))

/**
 * returns the static string for the given error code. The
 * error code can be either a normal system error (a
 * positive error code will flag this), it can be => libz
 * error code (a small negative error code will flag this)
 * or it can be an error code from => libzzip, which is an
 * negative value lower than => ZZIP_ERROR
 */
const char*
zzip_strerror(int errcode)
{
  if (errcode < ZZIP_ERROR && errcode > ZZIP_ERROR-32)
  {
      struct errlistentry* err = errlist;
      for (; err->mesg ; err++)
      {
          if (err->code == errcode) 
              return err->mesg; 
      }
      errcode = EINVAL;
  }

  if (errcode < 0)
  {
      if (errcode == -1)
          return strerror(errcode);
      else
          //return zError(errcode);
          return strerror(errcode);
  }
  
  return strerror (errcode);
}

/**
 * read the errorcode from the => DIR-handle and run it
 * through => zzip_strerror to obtain the static string
 * describing the error.
 */
const char* 
zzip_strerror_of(ZZIP_DIR* dir)
{
    return zzip_strerror(dir->errcode);
}

static struct errnolistentry { int code; int e_no; } 
errnolist[] =
{
    { Z_STREAM_ERROR, EPIPE },
    { Z_DATA_ERROR, ESPIPE },
    { Z_MEM_ERROR, ENOMEM },
    { Z_BUF_ERROR, EMFILE },
    { Z_VERSION_ERROR, ENOEXEC },
      
    { ZZIP_DIR_OPEN, ENOTDIR },
    { ZZIP_DIR_STAT, EREMOTE },
    { ZZIP_DIR_SEEK, ESPIPE },
    { ZZIP_DIR_READ, EPIPE},  
    { ZZIP_DIR_TOO_SHORT, ENOEXEC },
    { ZZIP_DIR_EDH_MISSING, EIO },
    { ZZIP_DIRSIZE, EFBIG },
    { ZZIP_OUTOFMEM, ENOMEM },
    { ZZIP_ENOENT, ENOENT },
    { ZZIP_UNSUPP_COMPR, EACCES },
    { ZZIP_CORRUPTED, ELOOP }, 
    { ZZIP_UNDEF, EINVAL },
    { 0, 0 },
};    

/**
 * map the error code to a system error code. This is used
 * for the drop-in replacement functions to return a value
 * that can be interpreted correctly by code sections that
 * are unaware of the fact they their => open(2) call had been
 * diverted to a file inside a zip-archive.
 */
int
zzip_errno(int errcode)
{
    if (errcode >= -1) return errno;
    
    {	struct errnolistentry* err = errnolist;
        for (; err->code ; err++)
        {
            if (err->code == errcode) 
                return err->e_no; 
        }
    }
    return EINVAL;
}

/* 
 * Local variables:
 * c-file-style: "stroustrup"
 * End:
 */
