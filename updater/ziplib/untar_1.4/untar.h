#ifndef ___untar_h
#define ___untar_h

#define VERSION "1.4"

/* DESCRIPTION:
 *	Untar extracts files from an uncompressed tar archive, or one which
 *	has been compressed with gzip. Usually such archives will have file
 *	names that end with ".tar" or ".tgz" respectively, although untar
 *	doesn't depend on any naming conventions.  For a summary of the
 *	command-line options, run untar with no arguments.
 *
 * HOW TO COMPILE:
 *	Untar doesn't require any special libraries or compile-time flags.
 *	A simple "cc untar.c -o untar" (or the local equivalent) is
 *	sufficient.  Even "make untar" works, without needing a Makefile.
 *	For Microsoft Visual C++, the command is "cl /D_WEAK_POSIX untar.c"
 *	(for 32 bit compilers) or "cl /F 1400 untar.c" (for 16-bit).
 *
 *	IF YOU SEE COMPILER WARNINGS, THAT'S NORMAL; you can ignore them.
 *	Most of the warnings could be eliminated by adding #include <string.h>
 *	but that isn't portable -- some systems require <strings.h> and
 *	<malloc.h>, for example.  Because <string.h> isn't quite portable,
 *	and isn't really necessary in the context of this program, it isn't
 *	included.
 *
 * PORTABILITY:
 *	Untar only requires the <stdio.h> header.  It uses old-style function
 *	definitions.  It opens all files in binary mode.  Taken together,
 *	this means that untar should compile & run on just about anything.
 *
 *	If your system supports the POSIX chmod(2), utime(2), link(2), and
 *	symlink(2) calls, then you may wish to compile with -D_POSIX_SOURCE,
 *	which will enable untar to use those system calls to restore the
 *	timestamp and permissions of the extracted files, and restore links.
 *	(For Linux, _POSIX_SOURCE is always defined.)
 *
 *	For systems which support some POSIX features but not enough to support
 *	-D_POSIX_SOURCE, you might be able to use -D_WEAK_POSIX.  This allows
 *	untar to restore time stamps and file permissions, but not links.
 *	This should work for Microsoft systems, and hopefully others as well.
 *
 * AUTHOR & COPYRIGHT INFO:
 *	Written by Steve Kirkendall, kirkenda@cs.pdx.edu
 *	Placed in public domain, 6 October 1995
 *
 *	Portions derived from inflate.c -- Not copyrighted 1992 by Mark Adler
 *	version c10p1, 10 January 1993
 */

#include <stdio.h>
#include <io.h>
#include <dir.h>
#include <stdlib.h>
#include <string>
#include <vector>


#ifndef SEEK_SET
# define SEEK_SET 0
#endif

#ifdef _WEAK_POSIX
# ifndef _POSIX_SOURCE
#  define _POSIX_SOURCE
# endif
#endif

#ifdef _POSIX_SOURCE
# include <sys/types.h>
# include <sys/stat.h>
# include <utime.h>
# ifdef _WEAK_POSIX
#  define mode_t int
# else
#  include <unistd.h>
# endif
#endif

#define WSIZE	32768	/* size of decompression buffer */
#define TSIZE	512	/* size of a "tape" block */
#define CR	13	/* carriage-return character */
#define LF	10	/* line-feed character */

typedef unsigned char	Uchar_t;
typedef unsigned short	Ushort_t;
typedef unsigned long	Ulong_t;

typedef struct
{
	Uchar_t	magic[2];	/* magic: 0x1F, 0x8b */
	Uchar_t	compression;	/* compression method: 8=deflated */
	Uchar_t	flags;		/* content flags: 0x08 bit -> name present */
	Uchar_t	mtime[4];	/* time_t when archive created */
	Uchar_t	extraflags;	/* ? */
	Uchar_t	os;		/* operating system: 3=UNIX */
	/* if flags&0x08, then original file name goes here, '\0'-terminated */
} gzhdr_t;
#define MAGIC0	0x1f
#define MAGIC1	0x8b
#define DEFLATE	0x08
#define NAME	0x08

typedef struct
{
	char	filename[100];	/*   0  name of next file */
	char	mode[8];	/* 100  Permissions and type (octal digits) */
	char	owner[8];	/* 108  Owner ID (ignored) */
	char	group[8];	/* 116  Group ID (ignored) */
	char	size[12];	/* 124  Bytes in file (octal digits) */
	char	mtime[12];	/* 136  Modification time stamp (octal digits)*/
	char	checksum[8];	/* 148  Header checksum (ignored) */
	char	type;		/* 156  File type (see below) */
	char	linkto[100];	/* 157  Linked-to name */
	char	brand[8];	/* 257  Identifies tar version (ignored) */
	char	ownername[32];	/* 265  Name of owner (ignored) */
	char	groupname[32];	/* 297  Name of group (ignored) */
	char	devmajor[8];	/* 329  Device major number (ignored) */
	char	defminor[8];	/* 337  Device minor number (ignored) */
	char	prefix[155];	/* 345  Prefix of name (optional) */
	char	RESERVED[12];	/* 500  Pad header size to 512 bytes */
} tar_t;
#define ISREGULAR(hdr)	((hdr).type < '1' || (hdr).type > '6')

typedef struct huft {
	Uchar_t e;	/* number of extra bits or operation */
	Uchar_t b;	/* number of bits in this code or subcode */
	union {
		Ushort_t	n; /* literal, length base, or distance base */
		struct huft	*t;/* pointer to next level of table */
	} v;
} huft_t;

#define error(desc)	{fprintf(stderr, "%s:%s", inname, (desc)); exit(1);}

extern int	force;
extern int	quiet;
extern int	listing;

void doarchive(char * filename);
FILE * createpath(char * name);
void cvtwrite(Uchar_t * blk, Ulong_t size, FILE	* fp);
long checksum(tar_t* tblk, int sunny);
void linkorcopy(char * src, char * dst, int sym);
void untar(Uchar_t * blk);
void flush_output(unsigned w);
int huft_build(unsigned * b, unsigned n, unsigned s, Ushort_t * d, Ushort_t * e, huft_t ** t, int * m);
int huft_free(huft_t * t);
int inflate_codes(huft_t * tl, huft_t * td, int bl, int bd);
int inflate_block(int * e);
void usage(char * argv0, int exitcode);

void                            AddZipFileName(const char * sZipFileName);
void                            ClearZipFileNames();
std::vector<std::string>      & GetZipFileNames();
void                            SetExtractionDirectory(const char * sLocation);
#endif
