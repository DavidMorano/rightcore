/* main */

/* invert a line coded file */
/* last modified %G% version %I% */


#define	CF_DEBUG		0


/* revision history :

	= 1998-10-01, David A­D­ Morano, version 0

	This program was written because I lost the program that I
	wrote years ago which performed line inversions.  That old
	program was much more efficient but I since I couldn't find it,
	I had to write this one in a pinch so it is not as efficient.

	= 1994-09-10, David A­D­ Morano, version 0a

	I enhanced this program slightly because I found my old program
	and dicovered that it is not robust in the face of total
	garbage binary input.  Since I want to penalize that behavior
	and reward good robust programs (like this one when used with
	binary input), I an enhancing this program as a reward for its
	robustness !  Unfortunately, the original program (when it
	works) is still way, way faster than this program.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	Synopsis:

	$ lineinvert [infile | - [outfile]]


**************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/mman.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<errno.h>

#include	<bfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"


/* local defines */

#define	NPARG		1
#define	LINELEN		257
#define	BUFLEN		(2 * MAXPATHLEN)


/* external subroutines */

extern int	mkapth2(char *,const char *,const char *) ;
extern int	mktmpfile(char *,mode_t,const char *) ;


/* external variables */


/* forward subroutine references */

static int	mkindexfile() ;


/* static data */

#ifdef	COMMENT
static char	*tmpdir[] = {
	"/tmp",
	"/var/tmp",
	"/var/spool/uucppublic",
	".",
	NULL
} ;
#endif /* COMMENT */


/* exported subroutines */


int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile		errfile, *efp = &errfile ;
	bfile		infile, *ifp = &infile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		indexfile, *xfp = &indexfile ;
	bfile		tmpfile, *tfp = &tmpfile ;

	offset_t	poffset, offset ;
	offset_t	tmpoff ;

	int	argl, aol ;
	int	pan, i ;
	int	len, rs ;
	int	mlen, rlen ;
	int	f_usage = FALSE ;
	int	f_debug = FALSE ;
	int	f_version = FALSE ;
	int	f_dash = FALSE ;
	int	f_verbose = FALSE ;
	int	f_notseek = FALSE ;
	int	f_index = FALSE ;
	int	nlines, line ;
	int	f_bol = FALSE, f_eol = FALSE ;

	const char	*argp, *aop ;
	const char	*progname ;
	char	linebuf[LINELEN + 1] ;
	const char	*ifname = NULL ;
	const char	*ofname = NULL ;
	char	xfnamebuf[MAXPATHLEN + 1], *xfname = xfnamebuf ;
	char	tfname[MAXPATHLEN + 1] ;
	char	buf[BUFLEN + 1] ;
	const char	*tmpdir = NULL ;


	progname = argv[0] ;
	if (bopen(efp,BIO_STDERR,"dwca",0666) >= 0)
		bcontrol(efp,BC_LINEBUF) ;


/* initial stuff */

	xfnamebuf[0] = '\0' ;
	tfname[0] = '\0' ;

/* go to the arguments */

	pan = 0 ;			/* number of positional so far */
	i = 1 ;
	argc -= 1 ;

	while (argc > 0) {

	    argp = argv[i++] ;
	    argc -= 1 ;
	    argl = strlen(argp) ;

	    if ((argl > 0) && (*argp == '-')) {

	        if (argl > 1) {

	            aop = argp ;
	            aol = argl ;
	            while (--aol) {

	                aop += 1 ;
	                switch (*aop) {

	                case 'D':
	                    f_debug = TRUE ;
	                    break ;

	                case 'V':
	                    bprintf(efp,"%s: version %s\n",
	                        progname,VERSION) ;

	                    f_version = TRUE ;
	                    break ;

	                case 'v':
	                    f_verbose = TRUE ;
	                    break ;

	                case 'i':
	                    if (argc <= 0) goto badargnum ;

	                    argp = argv[i++] ;
	                    argc -= 1 ;
	                    argl = strlen(argp) ;

	                    if (argl) {

	                        f_index = TRUE ;
	                        xfname = argp ;
	                    }
	                    break ;

	                default:
	                    bprintf(efp,"%s: unknown option - %c\n",
	                        progname,*aop) ;

	                case '?':
	                    f_usage = TRUE ;

	                } /* end switch */

	            } /* end while */

	        } else {

	            pan += 1 ;	/* increment position count */
	            f_dash = TRUE ;

	        } /* end if */

	    } else {

	        if (pan < NPARG) {

	            switch (pan) {

	            case 0:
	                if (argl > 0) 
				ifname = argp ;

	                break ;

	            case 1:
	                if (argl > 0) 
				ofname = argp ;

	                break ;

	            default:
	                break ;
	            }

	            pan += 1 ;

	        } else {

	            bprintf(efp,"%s: extra arguments ignored\n",
	                progname) ;

	        }

	    } /* end if */

	} /* end while */

	if (f_debug)
	    bprintf(efp,
	        "%s: finished parsing arguments\n",
	        progname) ;

	if (f_usage) goto usage ;

	if (f_version) goto done ;


/* check arguments */

	if (tmpdir == NULL)
	    tmpdir = getenv("TMPDIR") ;

	if (tmpdir == NULL)
	    tmpdir = TMPDIR ;


/* open files */

	if (ifname == NULL) ifname = BIO_STDIN ;

	if ((rs = bopen(ifp,ifname,"r",0666)) < 0)
	    goto badinfile ;


	if (ofname == NULL) ofname = BIO_STDOUT ;

	if ((rs = bopen(ofp,ofname,"wct",0666)) < 0)
	    goto badoutfile ;

#if	CF_DEBUG
	if (f_debug)
	    bprintf(efp,"%s: half way openning files\n",
	        progname) ;
#endif


/* if an index file was not specified, we build it */

	if (! f_index) {

	    mkpath2(buf, tmpdir, "lixXXXXXXXXXXX") ;

	    rs = mktmpfile(xfnamebuf,0644,buf)) < 0)
	    if (rs < 0)
	        goto badindexmk ;

	    if (f_debug)
	        bprintf(efp,
	            "%s: made a temporary file \"%s\"\n",
	            progname,xfnamebuf) ;

	} /* end if */

#if	CF_DEBUG
	if (f_debug)
	    bprintf(efp,"%s: three quarters way openning files\n",
	        progname) ;
#endif

	if ((rs = bopen(xfp,xfname,"rwct",0666)) < 0)
	    goto badindexopen ;

#if	CF_DEBUG
	if (f_debug)
	    bprintf(efp,"%s: finished openning files\n",
	        progname) ;
#endif

/* other initializations */

	if (bseek(ifp,0,SEEK_SET) < 0) {

	    if (f_debug) 
		bprintf(efp,
	        "we got a non-seekable input file\n") ;

	    f_notseek = TRUE ;
	    mkpath2(buf, tmpdir, "litXXXXXXXXXXX") ;

	    rs = mktmpfile(tfname,0664,buf) ;
	    if (rs < 0)
	        goto badtmpmk ;

	    if (f_debug)
	        bprintf(efp,"made a temporary file \"%s\"\n",
	            tfname) ;

	    if ((rs = bopen(tfp,tfname,"rwct",0666)) < 0)
	        goto badtmpopen ;

	} /* end if */


/* go through the loops for indexing */

	if (f_notseek) {

#if	CF_DEBUG
	    if (f_debug)
	        eprintf("main: about to go through the index loops\n") ;
#endif

	    f_bol = TRUE ;
	    offset = 0 ;
	    line = 0 ;
	    while ((len = bgetline(ifp,linebuf,LINELEN)) > 0) {

	        f_eol = FALSE ;
	        if (linebuf[len - 1] == '\n') f_eol = TRUE ;

	        if (f_bol)
	            bwrite(xfp,&offset,sizeof(offset_t)) ;

	        offset += len ;
	        if (f_notseek)
	            if ((rs = bwrite(tfp,linebuf,len)) < 0)
	                goto badwrite ;

	        if (f_eol)
	            line += 1 ;

	        f_bol = f_eol ;

	    } /* end while */

	    rs = len ;
	    if (! f_eol)
	        line += 1 ;

/* finish up (the case where the original file was not seekable) */

	    bclose(ifp) ;

	    ifp = tfp ;

	} else {

#if	CF_DEBUG
	    if (f_debug)
	        eprintf("main: about to make the index file (the easy way)\n") ;
#endif

	    rs = mkindexfile(ifp,xfp,&offset,&line) ;

	} /* end if (file indexing) */

	if (rs < 0)
	    goto badindexwrite ;


/* enter the write-out phase of the inversion operation */

	if (f_debug)
	    bprintf(efp,
	        "%s: about to write it out - lines %d\n",
	        progname,line) ;

	rs = bseek(xfp,(offset_t) (- sizeof(offset_t)),SEEK_CUR) ;

	if (f_debug) 
		bprintf(efp,"%s: tried to seek rs=%d\n",
	    progname,rs) ;

	poffset = offset ;
	nlines = line ;
	while (offset > 0) {

	    if ((rs = bread(xfp,&offset,sizeof(offset_t))) < 0)
	        goto badindexread ;

	    bseek(ifp,offset,SEEK_SET) ;

	    line -= 1 ;
	    if (f_debug) bprintf(efp,
	        "%s: about to copy line %d - o=%08lX po=%08lX rs=%d\n",
	        progname,nlines - line,offset,poffset,rs) ;

	    mlen = poffset - offset ;

#ifdef	COMMENT
	    while (mlen > 0) {

	        rlen = MIN(mlen,LINELEN) ;
	        if ((len = bread(ifp,linebuf,rlen)) < 1)
	            goto badread ;

	        if ((rs = bwrite(ofp,linebuf,len)) < len)
	            goto badwrite ;

	        mlen -= len ;

	    } /* end while */
#else
	    bcopyblock(ifp,ofp,mlen) ;
#endif

/* prepare for the next iteration of the loop */

	    if (offset > 0) {

	        tmpoff = - (2 * sizeof(offset_t)) ;
	        (void) bseek(xfp,tmpoff,SEEK_CUR) ;

	        if (f_debug) {

	            btell(xfp,&poffset) ;

	            bprintf(efp,"%s: poffset=%08X\n",
	                progname,poffset) ;

	        }

	    } /* end if (offset greater than zero) */

	    poffset = offset ;

	} /* end while */


	if (f_debug)
	    bprintf(efp,"%s: out of it now\n",
	        progname) ;


	bclose(ofp) ;

	bclose(xfp) ;

	bclose(ifp) ;

/* let's get outr of here ! */
done:
	if (tfname[0] != '\0') 
		u_unlink(tfname) ;

	if ((xfnamebuf[0] != '\0') && (! f_debug))
	    u_unlink(xfname) ;

	bclose(efp) ;

	return OK ;

badret:
	if (tfname[0] != '\0') 
		u_unlink(tfname) ;

	if (xfnamebuf[0] != '\0') 
		u_unlink(xfname) ;

	bclose(efp) ;

	return BAD ;

usage:
	bprintf(efp,
	    "%s: USAGE> %s [infile | - [outfile]] [-?VD]",
	    progname,progname) ;

	goto badret ;

badarg:
	bprintf(efp,"%s: bad argument given\n",progname) ;

	goto badret ;

badinfile:
	bprintf(efp,"%s: cannot open the input file (rs %d)\n",
	    progname,rs) ;

	goto badret ;

badoutfile:
	bprintf(efp,"%s: could not open standard output (rs %d)\n",
	    progname,rs) ;

	goto badret ;

badindexmk:
	bprintf(efp,"%s: could not make temporary index file (rs %d)\n",
	    progname,rs) ;

	goto badret ;

badindexopen:
	bprintf(efp,"%s: could not open temporary index file (rs %d)\n",
	    progname,rs) ;

	goto badret ;

badindexwrite:
	bprintf(efp,"%s: we could not create the index (rs %d)\n",
	    progname,rs) ;

	goto badret ;

badindexread:
	bprintf(efp,"%s: bad index file read (rs %d)\n",
	    progname,rs) ;

	goto badret ;

badtmpmk:
	bprintf(efp,"%s: could not make temporary seek file (rs %d)\n",
	    progname,rs) ;

	goto badret ;

badtmpopen:
	bprintf(efp,"%s: could not open temporary seek file (rs %d)\n",
	    progname,rs) ;

	goto badret ;

badread:
	bclose(ifp) ;

	bclose(xfp) ;

	bclose(ofp) ;

	bprintf(efp,
	    "%s: bad read on file (rs %d)\n",
	    progname,rs) ;

	goto badret ;

badwrite:
	bclose(ifp) ;

	bclose(xfp) ;

	bclose(ofp) ;

	bprintf(efp,
	    "%s: could not perform the 'bwrite' to file system (rs %d)\n",
	    progname,rs) ;

	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",progname) ;

	goto badret ;
}
/* end subroutine (main) */


/* this is the fast indexing subroutine */

/*
	This indexing works by mapping the file (in successive
	segments) into the program memory where we scan quickly
	for the end-of-line characters delineating lines.
*/

#define	MAPSIZE		(8192 * 1024 * 4)


static int mkindexfile(ifp,xfp,offsetp,linesp)
bfile	*ifp, *xfp ;
offset_t	*offsetp ;
int	*linesp ;
{
	struct ustat	sb ;

	offset_t	filesize ;
	offset_t	lineoffset = 0L ;
	offset_t	mapoffset = 0L ;

	int	mapsize ;
	int	ifd ;
	int	rs ;
	int	line = 0 ;

	char	*filemem ;
	char	*cp ;


#if	CF_DEBUG
	eprintf("mkindexfile: entered\n") ;
#endif

	if ((rs = bcontrol(ifp,BC_FD,&ifd)) < 0)
	    return rs ;

#if	CF_DEBUG
	eprintf("mkindexfile: about to fstat\n") ;
#endif

	if ((rs = u_fstat(ifd,&sb)) < 0)
	    return rs ;

	filesize = sb.st_size ;

#if	CF_DEBUG
	eprintf("mkindexfile: entering the while\n") ;
#endif

	while (sb.st_size > mapoffset) {

	    filemem = (char *) mmap(0L, MAPSIZE,
	        PROT_READ,MAP_SHARED,ifd,mapoffset) ;

	    if (filemem == MAP_FAILED)
	        return (- errno) ;

/* find the EOF offset into the map of the file */

	    mapsize = MIN(MAPSIZE,filesize - mapoffset) ;

#if	CF_DEBUG
	    eprintf("mkindexfile: entering the for\n") ;
#endif

	    for (cp = filemem ; cp < (filemem + mapsize) ; cp += 1) {

	        if (*cp == '\n') {

	            bwrite(xfp,&lineoffset,sizeof(offset_t)) ;

	            line += 1 ;
	            lineoffset = (cp - filemem) + mapoffset + 1 ;

	        } /* end if */

	    } /* end for */

#if	CF_DEBUG
	    eprintf("mkindexfile: out of for\n") ;
#endif

	    u_munmap(filemem,MAPSIZE) ;

#if	CF_DEBUG
	    eprintf("mkindexfile: we unmapped\n") ;
#endif

	    mapoffset += MAPSIZE ;

	} /* end while */

#if	CF_DEBUG
	eprintf("mkindexfile: out of while\n") ;
#endif

/* write out the last line if there was one */

	if (lineoffset != filesize)
	    bwrite(xfp,&lineoffset,sizeof(offset_t)) ;


	*offsetp = filesize ;
	*linesp = line ;

#if	CF_DEBUG
	eprintf("mkindexfile: exiting subroutine\n") ;
#endif

	return OK ;
}
/* end subroutine (mkindexfile) */



