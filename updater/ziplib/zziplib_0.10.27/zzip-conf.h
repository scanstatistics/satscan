/*
 * Author: 
 *      Guido Draheim <guidod@gmx.de>
 *
 *      Copyright (c) 2001 Guido Draheim
 *          All rights reserved,
 *          use under the restrictions of the
 *          Lesser GNU General Public License
 */

#ifndef _ZZIP_CONF_H
#define _ZZIP_CONF_H 1

#if !defined ZZIP_OMIT_CONFIG_H
# if defined _MSC_VER || defined __BORLANDC__
# include <zzip-msvc.h>
# else
# include <zzip-config.h>
# endif
#endif

/* especially on win32 platforms */
#ifndef ZZIP_off_t
#define ZZIP_off_t long
#endif

/* currently unused, all current zziplib-users do have ansi-C89 compilers. */
#ifndef ZZIP_const
#define ZZIP_const const
#endif

#define strcasecmp stricmp

#include <errno.h>

/* mingw32msvc errno : would be in winsock.h */
#ifndef EREMOTE
#define EREMOTE ESPIPE
#endif

#define  EILSEQ 92

#ifndef ELOOP
#define ELOOP EILSEQ
#endif

#ifdef _MSC_VER
#include <io.h>

//# if !defined __cplusplus && !defined inline
# define inline __inline
//# endif

# if !__STDC__
#  ifndef lseek
#  define lseek _lseek
#  endif
#  ifndef read
#  define read _read
#  endif
#  ifndef stat
#  define stat _stat
#  endif
# endif /* !__STDC__ */

# ifndef strcasecmp
# define strcasecmp _stricmp
# endif

#endif
  /*MSVC*/

#if !defined __GNUC__ && !defined __attribute__
#define __attribute__(X)
#endif

/* based on zconf.h : */
/* compile with -DZZIP_DLL for Windows DLL support */
//#if defined ZZIP_DLL
#  if defined _WINDOWS || defined WINDOWS
#  include <windows.h>
#  endif
#    define _zzip_export __declspec(dllimport)
//#  if !defined _zzip_export && defined _MSC_VER && defined WIN32
//#    define _zzip_export __declspec(dllexport) WINAPI
//#  endif
//#  if !defined _zzip_export && defined __BORLANDC__
//#    if __BORLANDC__ >= 0x0500 && defined WIN32
//#    include <windows.h>
//#    define _zzip_export __declspec(dllexport) WINAPI
//#    else
//#      if defined _Windows && defined __DLL__
//#      define _zzip_export _export
//#      endif
//#    endif
//#  endif
//#  if !defined _zzip_export && defined __GNUC__
//#    if defined __declspec
//#      define _zzip_export extern __declspec(dllexport)
//#    else
//#      define _zzip_export extern
//#    endif
//#  endif
//#  if !defined _zzip_export && defined __BEOS__
//#    define _zzip_export extern __declspec(export)
//#  endif
//#endif

//#if !defined _zzip_export
//#  if defined __GNUC__ /* || !defined HAVE_LIBZZIP */
//#  define _zzip_export extern
//#  else
//#    if defined __declspec || defined _MSC_VER
//#    define _zzip_export extern __declspec(dllimport)
//#    else
//#    define _zzip_export extern
//#    endif
//#  endif
//#endif

#endif


