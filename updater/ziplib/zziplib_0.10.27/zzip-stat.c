/*
 * Author: 
 *	Guido Draheim <guidod@gmx.de>
 *	Tomi Ollila <Tomi.Ollila@iki.fi>
 *
 *	Copyright (c) 2000,2001 Guido Draheim
 * 	Copyright (c) 1999 Tomi Ollila
 * 	    All rights reserved,
 *	    use under the restrictions of the
 *	    Lesser GNU General Public License
 *
 * Description:
 *      although this file is defining a function called zzip_stat it
 *      will not need a real stat(2) exported by the Operating System.
 *      It will just try to fill the fields of the ZZIP_STAT structure
 *      of 
 */

#define ZZIP_DLL /* is part of zzip.dll */

#include <string.h>

#include "zzip.h"

/**
 * obtain information about a filename in a zip-archive without 
 * opening that file first. Mostly used to obtain the uncompressed 
 * size of file inside a zip-archive.
 */
int 
zzip_dir_stat(ZZIP_DIR * dir, const char * name, ZZIP_STAT * zs, int flags)
{
    struct zzip_dir_hdr * hdr = dir->hdr0;
    int (*cmp)(const char *, const char *);

    cmp = (flags & ZZIP_CASEINSENSITIVE) ? strcasecmp : strcmp;

    if (flags & ZZIP_IGNOREPATH)
    {
        char* n = strrchr(name, '/');
        if (n)  name = n + 1;
    }

    if (hdr)
    while (1)
    {
        register char* hdr_name = hdr->d_name;
        if (flags & ZZIP_IGNOREPATH)
        {
            register char* n = strrchr(hdr_name, '/');
            if (n)  hdr_name = n + 1;
        }

	if (! cmp(hdr_name, name))
            break;

	if (! hdr->d_reclen)
	{
            dir->errcode = ZZIP_ENOENT;
            return -1;
	}

	hdr = (struct zzip_dir_hdr *) ((char *)hdr + hdr->d_reclen);
    }

    zs->d_compr = hdr->d_compr;
    zs->d_csize = hdr->d_csize;
    zs->st_size = hdr->d_usize;
    zs->d_name  = hdr->d_name;

    return 0;
}

/* 
 * Local variables:
 * c-file-style: "stroustrup"
 * End:
 */
