/* xbase/xbconfig.h.  Generated automatically by configure.  */
/* xbase/xbconfig.h.in.  Generated automatically from configure.in by autoheader.  */

/* Define if on AIX 3.
   System headers sometimes define this.
   We just want to avoid a redefinition error message.  */
#ifndef _ALL_SOURCE
/* #undef _ALL_SOURCE */
#endif

/* Define if on MINIX.  */
/* #undef _MINIX */

/* Define if the system does not provide POSIX.1 features except
   with this defined.  */
/* #undef _POSIX_1_SOURCE */

/* Define if you need to in order for stat and other things to work.  */
/* #undef _POSIX_SOURCE */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#define TIME_WITH_SYS_TIME 1

/* Define if the C++ compiler supports BOOL */
#define HAVE_BOOL 1

/* define if you have setenv */
#define HAVE_FUNC_SETENV 1

/* Define if you need the GNU extensions to compile */
#define _GNU_SOURCE 1

/* Define if you need to have .ndx indexes */
#define XB_INDEX_NDX 1

/* Define if you need to have .ntx indexes */
#define XB_INDEX_NTX 1

/* Define if you need to support memo fields */
#define XB_MEMO_FIELDS 1

/* Define if you need expressions */
#define XB_EXPRESSIONS 1

/* Define if you need html support */
#define XB_HTML 1

/* Define if you need locking support */
#define XB_LOCKING_ON 1

/* Define if you need to turn on XBase specific debug */
#define XBASE_DEBUG 1

/* Define if your compiler support exceptions */
/* #undef HAVE_EXCEPTIONS */

/* Define if you want Castellano (Spanish) Dates */
/* #undef XB_CASTELLANO */

/* Define if using real deletes */
#define XB_REAL_DELETE 1

/* Define if need filters */
#define XB_FILTERS 1

/* Define if you have the fcntl function.  */
#define HAVE_FCNTL 1

/* Define if you have the flock function.  */
#define HAVE_FLOCK 1

/* Define if you have the getdomainname function.  */
#define HAVE_GETDOMAINNAME 1

/* Define if you have the snprintf function.  */
#define HAVE_SNPRINTF 1

/* Define if you have the socket function.  */
#define HAVE_SOCKET 1

/* Define if you have the strcasecmp function.  */
#define HAVE_STRCASECMP 1

/* Define if you have the vsnprintf function.  */
#define HAVE_VSNPRINTF 1

/* Define if you have the vsprintf function.  */
#define HAVE_VSPRINTF 1

/* Define if you have the <ctype.h> header file.  */
#define HAVE_CTYPE_H 1

/* Define if you have the <dirent.h> header file.  */
#define HAVE_DIRENT_H 1

/* Define if you have the <exception> header file.  */
#define HAVE_EXCEPTION 1

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <g++/exception.h> header file.  */
/* #undef HAVE_G___EXCEPTION_H */

/* Define if you have the <io.h> header file.  */
/* #undef HAVE_IO_H */

/* Define if you have the <ndir.h> header file.  */
/* #undef HAVE_NDIR_H */

/* Define if you have the <stdarg.h> header file.  */
#define HAVE_STDARG_H 1

/* Define if you have the <string.h> header file.  */
#define HAVE_STRING_H 1

/* Define if you have the <strings.h> header file.  */
#define HAVE_STRINGS_H 1

/* Define if you have the <sys/dir.h> header file.  */
/* #undef HAVE_SYS_DIR_H */

/* Define if you have the <sys/locking.h> header file.  */
/* #undef HAVE_SYS_LOCKING_H */

/* Define if you have the <sys/ndir.h> header file.  */
/* #undef HAVE_SYS_NDIR_H */

/* Define if you have the <sys/types.h> header file.  */
#define HAVE_SYS_TYPES_H 1

/* Define if you have the <tvision/tv.h> header file.  */
/* #undef HAVE_TVISION_TV_H */

/* Should we include generic index support? */
#if defined(XB_INDEX_NDX) || defined(XB_INDEX_NTX)
#define  XB_INDEX_ANY 1
#endif

/* expressions required for indexes */
#if defined(XB_INDEX_ANY) && !defined(XB_EXPRESSIONS)
#define XB_EXPRESSIONS 1
#endif

/* default memo block size */
#define XB_DBT_BLOCK_SIZE  512

/* filename path separator */
#define PATH_SEPARATOR '/'

#ifndef HAVE_BOOL
#define HAVE_BOOL 1
typedef int bool;
const bool false = 0;
const bool true = 1;
#endif
