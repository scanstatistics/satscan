/*
 *	Copyright (c) 2000,2001 Guido Draheim <guidod@gmx.de>
 *      Use freely under the restrictions of the ZLIB license.
 */

#include <stdio.h>

#include "zziplib.h"

#ifndef O_BINARY
#define O_BINARY 0
#endif

static const char usage[] = 
{
    "zzdir <dir>.. \n"
    "  - prints a content table to stdout, but the dir can also be a zip-arch."
    "\n"
    " To show the contents of a zip-archive named 'test.zip', you may write \n"
    "     zzdir test \n"
};


//int main (int argc, char ** argv)
//{
//    int argn;
//    if (argc <= 1)
//    {
//        printf (usage);
//        exit(0);
//   }
//
//    for (argn=1; argn < argc; argn++)
//    {
//        ZZIP_DIR * dir;
//        ZZIP_DIRENT * d;
//
//        dir = zzip_opendir(argv[argn]);
//        if (! dir)
//        {
//            fprintf (stderr, "did not open %s:", argv[argn]);
//            perror(argv[argn]);
//            continue;
//        }
//
//        if (argc > 2) printf ("%s:\n", argv[argn]);
//
//	/* read each dir entry and show one line of info per file */
//        while ((d = zzip_readdir (dir)) != 0)
//        {
//	    /* orignalsize / compression-type / compression-ratio / filename */
//            if (d->st_size > 999999)
//            {
//                printf ("%5dK %-9s %2d%% %s\n",
//			d->st_size>>10,
//			zzip_compr_str(d->d_compr),
//			100 - (d->d_csize|1)/((d->st_size/100)|1),
//			d->d_name);
//            }else{
//                printf ("%6d %-9s %2d%% %s\n",
//			d->st_size,
//			zzip_compr_str(d->d_compr),
//			100 - (d->d_csize|1)*100/(d->st_size|1),
//			d->d_name);
//            }
//        }
//
//        zzip_closedir(dir);
//    }
//
//    return 0;
//}

/* 
 * Local variables:
 * c-file-style: "stroustrup"
 * End:
 */
