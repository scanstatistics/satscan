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
 */

#define ZZIP_DLL /* is part of zzip.dll */

#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "zzip.h"
#include "zzip-file.h"
#include "zzipformat.h"

#if defined ZZIP_HAVE_IO_H
#include <io.h> /* tell */
#else
#define tell(fd) lseek(fd,0,SEEK_CUR);
#endif

#if 0 /* debugging */
#include <stdio.h> 
#define DBG4(X1,X2,X3,X4) {fprintf(stderr,"\n"__FUNCTION__":"X1"\n",X2,X3,X4);}
#define zzip_errno(x) (fprintf(stderr,"\n"__FUNCTION__ ":ERROR:%s\n",zzip_strerror(x)),zzip_errno(x))
#else
#define DBG4(X1,X2,X3,X4) {}
#endif

/**
 * the direct function of => zzip_close(fp). it will cleanup the
 * inflate-portion of => zlib and free the structure given.
 * <p>
 * it is called quite from the error-cleanup parts
 * of the various => _open functions. 
 * <p>
 * the .refcount is decreased and if zero the fp->dir is closed just as well.
 */
void 
zzip_file_close(ZZIP_FILE * fp)
{
    ZZIP_DIR * dir = fp->dir;
    
    if (fp->method)

        inflateEnd(&fp->d_stream); /* inflateEnd() can be called many times */

    if (fp->buf32k)
    {
        if (dir->cache.buf32k == NULL) dir->cache.buf32k = fp->buf32k;
        else free(fp->buf32k);
    }

    if (dir->currentfp == fp)
        dir->currentfp = NULL;
  
    dir->refcount--;

    memset(fp, 0, sizeof(*fp)); /* ease to notice possible dangling reference errors */

    if (dir->cache.fp == NULL) dir->cache.fp = fp;
    else free(fp);
    
    if (! dir->refcount) zzip_dir_close(dir);
}
  

static int 
zzip_file_saveoffset(ZZIP_FILE * fp)
{
    if (fp)
    {
        int fd = fp->dir->fd;
        ZZIP_off_t off = lseek(fd, 0, SEEK_CUR);
        if (off < 0)
            return -1;

        fp->offset = off;
    }
    return 0;
}


static int zzip_inflate_init(ZZIP_FILE *, struct zzip_dir_hdr *);

/**
 * open an => ZZIP_FILE from an already open => ZZIP_DIR handle. Since
 * we have a chance to reuse a cached => buf32k and => ZZIP_FILE memchunk
 * this is the best choice to unpack multiple files.
 * <p>
 * Note: the zlib supports 2..15 bit windowsize, hence we provide a 32k
 *       memchunk here... just to be safe.
 */
ZZIP_FILE * 
zzip_file_open(ZZIP_DIR * dir, const char * name, int flags)
{
    zzip_error_t err = 0;
    struct zzip_file * fp = 0;
    struct zzip_dir_hdr * hdr = dir->hdr0;
    int (*cmp)(const char *, const char *);
  
    cmp = (flags & ZZIP_CASEINSENSITIVE)? strcasecmp: strcmp;

    if (flags & ZZIP_IGNOREPATH)
    {
        register char* n = strrchr(name, '/');
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

     	DBG4("name %s, compr %d, size %d\n", hdr->name, hdr->compr, hdr->usize);

      	if (!cmp(hdr_name, name))
        {
            switch (hdr->d_compr)
            {
            case 0: /* store */
            case 8: /* inflate */
                break;
            default:
                { err = ZZIP_UNSUPP_COMPR; goto error; }
            }

            if (dir->cache.fp) 
            {
                fp = dir->cache.fp; dir->cache.fp = NULL;
                /* memset(zfp, 0, sizeof *fp); cleared in zzip_file_close() */
            }else
            {
                if (!(fp = (ZZIP_FILE *)calloc(1, sizeof(*fp))))
                    { err =  ZZIP_OUTOFMEM; goto error; }
            }

            fp->dir = dir;
            dir->refcount++;
	
            if (dir->cache.buf32k) 
              { fp->buf32k = dir->cache.buf32k; dir->cache.buf32k = NULL; }
            else
            {
                if ((fp->buf32k = (char *)malloc(ZZIP_32K)) == NULL)
                    { err = ZZIP_OUTOFMEM; goto error; }
            }

            /*
             * In order to support simultaneous open files in one zip archive
             * we'll fix the fd offset when opening new file/changing which
	     * file to read...
	     */ 

            if (zzip_file_saveoffset(dir->currentfp) < 0)
                { err = ZZIP_DIR_SEEK; goto error; }

            fp->offset = hdr->d_off;
            dir->currentfp = fp;

            if (lseek(dir->fd, hdr->d_off, SEEK_SET) < 0)
                { err = ZZIP_DIR_SEEK; goto error; }

            { 	/* skip local header */
                /* should test tons of other info, but trust that those are correct*/
                int dataoff;
                struct zzip_file_header * p = (void*) fp->buf32k;

                if (read(dir->fd, (void*)p, sizeof(*p)) < sizeof(*p))
                    { err = ZZIP_DIR_READ; goto error; }

                if (! ZZIP_FILE_HEADER_CHECKMAGIC(p)) /* PK\3\4 */
                    { err = ZZIP_CORRUPTED; goto error; }

	   	dataoff = ZZIP_GET16(p->z_namlen) + ZZIP_GET16(p->z_extras);
	      
                if (lseek(dir->fd, dataoff, SEEK_CUR) < 0)
                { err = ZZIP_DIR_SEEK; goto error; }

		/* 2000-12-30 save data offset and uncompressed size */
		fp->dataoffset = tell(dir->fd);
		fp->usize = hdr->d_usize;
		fp->csize = hdr->d_csize;
            }

            err = zzip_inflate_init (fp, hdr);
            if (err) { goto error; }
                			
            return fp;
        }else
        {
            if (hdr->d_reclen == 0)
                break;
            hdr = (struct zzip_dir_hdr *)((char *)hdr + hdr->d_reclen);
        }/*cmp name*/
    }/*forever*/
    dir->errcode = ZZIP_ENOENT; 	zzip_errno(ZZIP_ENOENT);
    return NULL;
error:
    if (fp) zzip_file_close(fp);
    dir->errcode = err;   		zzip_errno(err);
    return NULL;
}

/**
 *  call => inflateInit and setup fp's iterator variables, 
 *  used by lowlevel => _open functions.
 */
static int 
zzip_inflate_init(ZZIP_FILE * fp, struct zzip_dir_hdr* hdr)
{
    int err;
    fp->method = hdr->d_compr;
    fp->restlen = hdr->d_usize;
    
    if (fp->method)
    {
        /* memset(&zfp->d_stream, 0, sizeof zfp->d_stream); no need, done earlier */
  
        err = inflateInit2(&fp->d_stream, -MAX_WBITS);
        if (err != Z_OK) { goto error; }

        fp->crestlen = hdr->d_csize;
    }
    return 0;
error:
    if (fp) zzip_file_close(fp);
    return err;
}

/**
 * close an ZZIP_FILE handle. If the ZZIP_FILE wraps a normal stat'fd then it
 * is just that int'fd that is being closed and the otherwise empty ZZIP_FILE
 * gets freed.
 */
void 
zzip_close(ZZIP_FILE * fp)
{
    if (! fp) return;
    if (! fp->dir) 
      { close (fp->fd); free(fp); } /* stat fd */
    else zzip_file_close(fp);
}

/**
 * works like => read(2), it will fill the given buffer with bytes from
 * the opened file. It will return the number of bytes read, so if the => EOF
 * is encountered you will be prompted with the number of bytes actually read.
 * <p>
 * This is the routines that needs the => buf32k buffer, and it would have
 * need for much more polishing but it does already work quite well.
 * <p>
 * Note: the 32K buffer is rather big. The original inflate-algorithm
 *       required just that but the latest zlib would work just fine with
 *       a smaller buffer.
 */
int 
zzip_file_read(ZZIP_FILE * fp, char * buf, int len)
{
    ZZIP_DIR * dir; 
    int l;
    int rv;
    
    if (! fp || ! fp->dir) return 0;

    dir = fp->dir;
    l = fp->restlen > len ? len : fp->restlen;
    if (fp->restlen == 0)
        return 0;

    /*
     * If this is other handle than previous, save current seek pointer
     * and read the file position of `this' handle.
     */
     if (dir->currentfp != fp)
     {
         if (zzip_file_saveoffset(dir->currentfp) < 0 
         || lseek(dir->fd, fp->offset, SEEK_SET) < 0)
           { dir->errcode = ZZIP_DIR_SEEK; return -1; }
         else
           { dir->currentfp = fp; }
     }
  
     /* if more methods is to be supported, change this to `switch ()' */
     if (fp->method) /* method != 0   == 8, inflate */
     {
         fp->d_stream.avail_out = l;
         fp->d_stream.next_out = (unsigned char *)buf;

         do {
             int err;
             int startlen;

             if (fp->crestlen > 0 && fp->d_stream.avail_in == 0)
             {
	         int cl = fp->crestlen > ZZIP_32K ? ZZIP_32K : fp->crestlen;
             /*	 int cl = fp->crestlen > 128? 128: fp->crestlen; */

                 int i = read(dir->fd, fp->buf32k, cl);
                 if (i <= 0)
                 {
                     dir->errcode = ZZIP_DIR_READ; /* 0 == ZZIP_DIR_READ_EOF ? */
                     return -1;
                 }
                 fp->crestlen -= i;
                 fp->d_stream.avail_in = i;
                 fp->d_stream.next_in = (unsigned char *)fp->buf32k;
             }

             startlen = fp->d_stream.total_out;
             err = inflate(&fp->d_stream, Z_NO_FLUSH);

             if (err == Z_STREAM_END) 
               { fp->restlen = 0; }
             else 
             if (err == Z_OK)
               { fp->restlen -= (fp->d_stream.total_out - startlen); }
             else
               { dir->errcode = err; return -1; }
         } while (fp->restlen && fp->d_stream.avail_out);

         return l - fp->d_stream.avail_out;
     }else
     {   /* method == 0 -- unstore */
         rv = read(dir->fd, buf, l);
         if (rv > 0)
             { fp->restlen-= rv; }
         else 
         if (rv < 0)
             { dir->errcode = ZZIP_DIR_READ; }
         return rv;
     }
}  

/**
 * the replacement for => read(2), it will fill the given buffer with bytes from
 * the opened file. It will return the number of bytes read, so if the EOF
 * is encountered you will be prompted with the number of bytes actually read.
 * <p>
 * If the file-handle is wrapping a stat'able file then it will actually just
 * perform a normal => read(2)-call, otherwise => zzip_file_read is called
 * to decompress the data stream and any error is mapped to => errno(3).
 */
int 
zzip_read(ZZIP_FILE * fp, char * buf, int len)
{
    if (! fp) return 0;
    if (! fp->dir) 
      { return read (fp->fd, buf, len); } /* stat fd */
    else
    {   register int v;
        v = zzip_file_read(fp, buf, len);
        if (v == -1) { errno = zzip_errno(fp->dir->errcode); }
        return v;
    }
}


#ifndef O_BINARY
#define O_BINARY 0
#endif

/**
 * see => open(2).
 * <p>
 * This functions has some magic builtin - it will first try to open
 * the given <em>filename</em> as a normal file. If it does not
 * exist, the given path to the filename (if any) is split into
 * its directory-part and the file-part. A ".zip" extension is
 * then added to the directory-part to create the name of a
 * zip-archive. That zip-archive (if it exists) is being searched
 * for the file-part, and if found a zzip-handle is returned. 
 * <p>
 * Note that if the file is found in the normal fs-directory the
 * returned structure is mostly empty and the => zzip_read call will
 * use the libc => read to obtain data. Otherwise a => zzip_file_open 
 * is performed and any error mapped to => errno(3).
 */
ZZIP_FILE*
zzip_open(const char* filename, int flags)
{
    int o_flags = flags &~ ZZIP_O_FLAGS;

    /* prefer an existing real file */
    {   int fd =  open (filename, o_flags);
        if (fd != -1)
        {
            ZZIP_FILE* fp = calloc (1, sizeof(ZZIP_FILE));
            if (!fp) { close (fd); return 0; }
            
            fp->fd = fd;
            return fp;
        }
    }
        
    /* if the user had it in place of a normal xopen, then
       we better defend this lib against illegal usage */
    if (o_flags & (O_CREAT|O_WRONLY))
        { errno = EINVAL; return 0; }
    
    /* see if we can open a file that is a zip file */
    { char basename[PATH_MAX];
      char* p;
      strcpy (basename, filename);
      
      /* per each slash in filename, check if it there is a zzip around */
      while ((p = strrchr (basename, '/'))) 
      {
	  zzip_error_t e = 0;
	  ZZIP_DIR* dir;
	  ZZIP_FILE* fp;
	  int fd;

	  *p = '\0'; /* cut at path separator == possible zipfile basename */
	  fd = __zzip_open_zip (basename, o_flags|O_RDONLY|O_BINARY);
	  if (fd == -1) { continue; }
/*    found: */
	  /* found zip-file, now try to parse it */
	  dir = zzip_dir_fdopen(fd, &e);
	  if (e) { errno = zzip_errno(e); close(fd); return 0; }

	  /* (p - basename) is the lenghtof zzip_dir part of the filename */
	  fp = zzip_file_open (dir, filename + (p - basename) +1, 
				     flags&ZZIP_O_FLAGS);
	  if (! fp) { errno = zzip_errno(dir->errcode); }

	  zzip_dir_close(dir); 
	  /* note: since (fp) is attached that (dir) will survive */
	  /* but (dir) is implicitly closed on next zzip_close(fp) */
	  return fp;
      } /*again*/

      errno = ENOENT; return 0;
    }
}

/**
 * Rewind an ZZIP_FILE handle. Seeks to the beginning of this file's
 * data in the zip, or the beginning of the file for a stat'fd.
 */
int
zzip_rewind(ZZIP_FILE *fp)
{
  ZZIP_DIR *dir;
  int err;

  if(!fp)
    return -1;

  if(!fp->dir) { /* stat fd */
    lseek(fp->fd,0,SEEK_SET);
    return 0;
  }
  
  dir = fp->dir;
  /*
   * If this is other handle than previous, save current seek pointer
   */
  if (dir->currentfp != fp)  {
    if (zzip_file_saveoffset(dir->currentfp) < 0)
      { dir->errcode = ZZIP_DIR_SEEK; return -1; }
    else
      { dir->currentfp = fp; }
  }

  /* seek to beginning of this file */
  if(lseek(dir->fd, fp->dataoffset, SEEK_SET) < 0)
     return -1;

  /* reset the inflate init stuff */
  fp->restlen = fp->usize;
  fp->offset = fp->dataoffset;
    
  if (fp->method) { /* == 8, deflate */
    memset(&fp->d_stream, 0, sizeof fp->d_stream);
    err = inflateInit2(&fp->d_stream, -MAX_WBITS);
    if (err != Z_OK) { goto error; }

    fp->crestlen = fp->csize;
  }
    

  return 0;
  
 error:
  if (fp) zzip_file_close(fp);
  return err;
}

/**
 * the replacement for => lseek(2), it will seek to the offset
 * specified by offset, relative to whence, which is one of
 * SEEK_SET, SEEK_CUR or SEEK_END.
 * <p>
 * If the file-handle is wrapping a stat'able file then it will actually just
 * perform a normal => lseek(2)-call. Otherwise the relative offset
 * is calculated, negative offsets are transformed into positive ones
 * by rewinding the file, and then data is read until the offset is
 * reached.  This can make the function terribly slow, but this is
 * how gzio implements it, so I'm not sure there is a better way
 * without using the internals of the algorithm.
 */
int
zzip_seek(ZZIP_FILE * fp, int offset, int whence)
{
  int cur_pos, rel_ofs, read_size, ofs;
  ZZIP_DIR *dir;
  
  if(!fp)
    return -1;

  if(!fp->dir) { /* stat fd */
    return lseek(fp->fd,offset,whence);
  }

  cur_pos = zzip_tell(fp);

  /* calculate relative offset */
  switch(whence) {
  case SEEK_SET: /* from beginning */
    rel_ofs = offset - cur_pos;
    break;
  case SEEK_CUR: /* from current */
    rel_ofs = offset;
    break;
  case SEEK_END: /* from end */
    rel_ofs = fp->usize + offset - cur_pos;
    break;
  default: /* something wrong */
    return -1;
  }

  if(rel_ofs == 0) /* don't have to move */
    return cur_pos;

  if(rel_ofs < 0) { /* convert backward into forward */
    if(zzip_rewind(fp) == -1) {
      return -1;
    }
    read_size = cur_pos + rel_ofs;
    cur_pos = 0;
  }
  else /* amount to read is positive relative offset */
    read_size = rel_ofs;

  if(read_size < 0) /* bad offset, before beginning of file */
    return -1;

  if(read_size + cur_pos > fp->usize) /* bad offset, past EOF */
    return -1;

  if(read_size == 0) /* nothing to read */
    return cur_pos;
  
  dir = fp->dir;
  /*
   * If this is other handle than previous, save current seek pointer
   * and read the file position of `this' handle.
   */
  if (dir->currentfp != fp)  {
    if (zzip_file_saveoffset(dir->currentfp) < 0 
	|| lseek(dir->fd, fp->offset, SEEK_SET) < 0)
      { dir->errcode = ZZIP_DIR_SEEK; return -1; }
    else
      { dir->currentfp = fp; }
  }
  
  if(fp->method == 0) { /* unstore, just lseek relatively */
    ofs = tell(dir->fd);
    ofs = lseek(dir->fd,read_size,SEEK_CUR);
    if(ofs > 0) { /* readjust from beginning of file */
      ofs -= fp->dataoffset;
      fp->restlen = fp->usize - ofs;
    }
    return ofs;
  }
  else { /* method == 8, inflate */
    char *buf;
    /*FIXME: use a static buffer! */
    buf = (char *)malloc(ZZIP_32K);
    if(!buf)
      return -1;
    
    while (read_size > 0)  {
      int size = ZZIP_32K;
      if (read_size < ZZIP_32K) size = (int)read_size;

      size = zzip_file_read(fp, buf, size);
      if (size <= 0) {
	free(buf);
	return -1;
      }
      
      read_size -= size;
    }

    free(buf);
  }

  return zzip_tell(fp);
}

/**
 * the replacement for => tell(2), it will return the current offset
 * in the file, in uncompressed bytes.
 * <p>
 * If the file-handle is wrapping a stat'able file then it will actually just
 * perform a normal => tell(2)-call, otherwise the offset is
 * calculated from the amount of data left and the total uncompressed
 * size;
 */
int
zzip_tell(ZZIP_FILE * fp)
{
  if(!fp)
    return -1;

  if(!fp->dir) { /* stat fd */
    return tell(fp->fd);
  }

  /* current uncompressed offset is uncompressed size - data left */
  return (fp->usize - fp->restlen);
}

/* 
 * Local variables:
 * c-file-style: "stroustrup"
 * End:
 */
