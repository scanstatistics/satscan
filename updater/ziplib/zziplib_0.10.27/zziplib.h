/*
 * Author: 
 *	Guido Draheim <guidod@gmx.de>
 *	Tomi Ollila <Tomi.Ollila@iki.fi>
 *
 *	Copyright (c) 2000,2001 Guido Draheim
 * 	Copyright (c) 1999 Tomi Ollila
 * 	    All rights reserved, 
 *          usage allowed under the restrictions of the
 *	    Lesser GNU General Public License 
 */

#ifndef _ZZIPLIB_H
#define _ZZIPLIB_H

#include <zzip-conf.h>

#include <fcntl.h>

#ifdef __cplusplus
extern "C" {
#endif

/* the zzip_error_t is also used to pass back ZLIB errors... */
#define ZZIP_ERROR -4096

typedef enum 
{
    ZZIP_NO_ERROR = 0,	/* no error, may be used if user sets it. */
    ZZIP_OUTOFMEM =     ZZIP_ERROR-20, /* out of memory */
    ZZIP_DIR_OPEN =      ZZIP_ERROR-21, /* failed to open zipfile, see errno for details */
    ZZIP_DIR_STAT =      ZZIP_ERROR-22, /* failed to fstat zipfile, see errno for details */
    ZZIP_DIR_SEEK =      ZZIP_ERROR-23, /* failed to lseek zipfile, see errno for details */
    ZZIP_DIR_READ =      ZZIP_ERROR-24, /* failed to read zipfile, see errno for details */
    ZZIP_DIR_TOO_SHORT = ZZIP_ERROR-25,
    ZZIP_DIR_EDH_MISSING = ZZIP_ERROR-26,
    ZZIP_DIRSIZE =      ZZIP_ERROR-27,
    ZZIP_ENOENT =       ZZIP_ERROR-28,
    ZZIP_UNSUPP_COMPR = ZZIP_ERROR-29,
    ZZIP_CORRUPTED =    ZZIP_ERROR-31,
    ZZIP_UNDEF =        ZZIP_ERROR-32,
} zzip_error_t;

/*
 * zzip_open flags.
 */
#define ZZIP_CASEINSENSITIVE	O_APPEND
#define ZZIP_IGNOREPATH	O_TRUNC
#define ZZIP_O_FLAGS           (ZZIP_CASEINSENSITIVE|ZZIP_IGNOREPATH)

typedef struct zzip_dir		ZZIP_DIR;
typedef struct zzip_file	ZZIP_FILE;
typedef struct zzip_dirent 	ZZIP_DIRENT;
typedef struct zzip_dirent 	ZZIP_STAT;

struct zzip_dirent
{
    int	 	d_compr;	/* compression method */
    int         d_csize;        /* compressed size */
    int	 	st_size;	/* file size */
    char * 	d_name;		/* file name */
};

/*
 * Getting error strings 
 * zzip-err.c
 */
_zzip_export    /* error in _opendir : */
const char * 	zzip_strerror(int errcode); 
_zzip_export    /* error in other functions : */
const char * 	zzip_strerror_of(ZZIP_DIR * dir); 
_zzip_export    /* error mapped to errno.h defines : */
int    	 	zzip_errno(int errcode); 


/*
 * Functions to grab information from ZZIP_DIR/ZZIP_FILE structure 
 * (if ever needed)
 * zzip-info.c
 */
_zzip_export
int  	 	zzip_error(ZZIP_DIR * dir);
_zzip_export
void 	 	zzip_seterror(ZZIP_DIR * dir, int errcode);
_zzip_export
const char * 	zzip_compr_str(int compr);

_zzip_export
ZZIP_DIR * 	zzip_dirhandle(ZZIP_FILE * fp);
_zzip_export
int           	zzip_dirfd(ZZIP_DIR * dir);
_zzip_export
int            	zzip_dir_real(ZZIP_DIR * dir);
_zzip_export
int      	zzip_file_real(ZZIP_FILE * fp);
_zzip_export
void*           zzip_realdir(ZZIP_DIR * dir);
_zzip_export
int             zzip_realfd(ZZIP_FILE * fp);

/*
 * zip handle management
 * zzip-zip.c
 */
_zzip_export
ZZIP_DIR *      zzip_dir_alloc(const char** fileext);
_zzip_export
int             zzip_dir_free(ZZIP_DIR *);

/*
 * Opening/closing a zip archive
 * zzip-zip.c
 */
_zzip_export
ZZIP_DIR *  	zzip_dir_fdopen(int fd, zzip_error_t * errcode_p);
_zzip_export
ZZIP_DIR *  	zzip_dir_open(const char * filename, zzip_error_t * errcode_p);
_zzip_export
int	  	zzip_dir_close(ZZIP_DIR * dir);
_zzip_export
int             zzip_dir_read(ZZIP_DIR * dir, ZZIP_DIRENT * dirent);


/*
 * Scanning files in zip archive
 * zzip-dir.c
 * zzip-zip.c
 */
_zzip_export
ZZIP_DIR * 	zzip_opendir(const char * filename);
_zzip_export
int          	zzip_closedir(ZZIP_DIR * dir);
_zzip_export
ZZIP_DIRENT * 	zzip_readdir(ZZIP_DIR * dir);
_zzip_export
void 	 	zzip_rewinddir(ZZIP_DIR * dir);
_zzip_export
ZZIP_off_t  	zzip_telldir(ZZIP_DIR * dir);
_zzip_export
void	 	zzip_seekdir(ZZIP_DIR * dir, ZZIP_off_t offset);

/*
 * 'opening', 'closing' and reading invidual files in zip archive.
 * zzip-file.c
 */

_zzip_export
ZZIP_FILE * 	zzip_file_open(ZZIP_DIR * dir, const char * name, int flags);
_zzip_export
void  		zzip_file_close(ZZIP_FILE * fp);
_zzip_export
int		zzip_file_read(ZZIP_FILE * fp, char* buf, int len);

_zzip_export
ZZIP_FILE * 	zzip_open(const char* name, int flags);
_zzip_export
void	 	zzip_close(ZZIP_FILE * fp);
_zzip_export
int	 	zzip_read(ZZIP_FILE * fp, char * buf, int len);
/* 12-30-2000,2001 - added rewind, seek and tell */
_zzip_export
int             zzip_rewind(ZZIP_FILE *fp);
_zzip_export
int             zzip_seek(ZZIP_FILE * fp, int offset, int
			       whence);
_zzip_export
int             zzip_tell(ZZIP_FILE * fp);


/*
 * reading info of a single file 
 * zzip-stat.c
 */
_zzip_export
int		zzip_dir_stat(ZZIP_DIR * dir, const char * name, 
			      ZZIP_STAT * zs, int flags);

#ifdef __cplusplus
};
#endif

#endif /* _ZZIPLIB_H */

