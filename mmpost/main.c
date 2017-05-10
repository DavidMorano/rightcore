/* mmpost */

/* post a mail-like message to the PCS bulletin board system */
/* last modified %G% version %I% */


#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1996-04-10, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1996 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	This program will post meeting minutes onto the bulletin board
	system.


***************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<signal.h>
#include	<unistd.h>
#include	<string.h>
#include	<time.h>
#include	<pwd.h>
#include	<grp.h>

#include	<vsystem.h>
#include	<bfile.h>

#include	"localmisc.h"



/* local defines */

#define	VERSION		"0"
#define	LINELEN		256
#define	NPARG		2
#define	CLENHEADER	"content-length"



/* external subroutines */

extern int	cfdec() ;
extern int	mheader() ;

extern char	*strbasename() ;


/* external variables */


/* local data */

static char	*tmpdir[] = {
	"/tmp",
	"/var/tmp",
	"/var/spool/uucppublic",
	".",
	NULL
} ;





int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile		errfile, *efp = &errfile ;
	bfile		infile, *ifp = &infile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		tmpfile, *tfp = &tmpfile ;

	struct ustat	stat_i ;

	offset_t	offset, off_clen ;

	int	rs ;
	int	argl, aol ;
	int	pan ;
	int	i ;
	int	len, line ;
	int	flen ;
	int	f_version = FALSE ;
	int	f_debug = FALSE ;
	int	f_usage= FALSE  ;
	int	f_dash = FALSE ;
	int	f_header ;
	int	f_bol, f_eol ;
	int	f_clen ;
	int	f_tmpfile = FALSE ;
	int	f_writeback = FALSE ;
	int	ofd ;

	char	*argp, *aop ;
	char	*progname ;
	char	*ifname, *ofname ;
	char	*bp, buf[LINELEN] ;
	char	tmpfnamebuf[MAXPATHLEN + 1], *tmpfname = NULL ;


	progname = strbasename(argv[0]) ;

	bopen(efp,BFILE_STDERR,"dwca",0666) ;

/* initial initialization */

	tmpfnamebuf[0] = '\0' ;
	ifname = BFILE_STDIN ;
	ofname = BFILE_STDOUT ;

/* do the argument thing */

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

	                akp += 1 ;
	                switch ((int) *aop) {

	                case 'V':
	                    f_version = TRUE ;
	                    break ;

	                case 'D':
	                    f_debug = TRUE ;
	                    break ;

	                case '?':
	                    f_usage = TRUE ;
	                    break ;

	                default:
	                    printf("%s: unknown option - %c\n",progname,
	                        *aop) ;

	                    goto usage ;

	                } /* end switch */

	            }

	        } else {

	            f_dash = TRUE ;
	            pan += 1 ;		/* increment position count */

	        } /* end if */

	    } else {

	        if (pan < NPARG) {

	            switch (pan) {

	            case 0:
	                ifname = argp ;
	                break ;

	            case 1:
	                ofname = argp ;
	                break ;

	            } /* end switch */

	            pan += 1 ;

	        } else {

	            bprintf(efp,"%s: extra arguments ignored\n",progname) ;

	        }

	    } /* end if */

	} /* end while */


	if (f_version) {

	    bprintf(efp,"%s: version %s\n",progname,VERSION) ;

	    goto earlyret ;
	}

	if (f_usage) goto usage ;


/* check the arguments */

	if (f_dash) ifname = (char *) 0L ;

	if ((rs = bopen(ifp,ifname,"r",0666)) < 0) goto badin ;

	if ((rs = bopen(ofp,ofname,"wct",0666)) < 0) goto badout ;


/* get the total length of the input file (if we can) */

	flen = -1 ;
	if (bcontrol(ifp,BC_STAT,&stat_i) >= 0)
	    flen = stat_i.st_size ;

#if	CF_DEBUG
	if (f_debug)
	    debugprintf("main: input file len=%ld\n",
	        flen) ;
#endif

	if ((flen < 0) || (bseek(ifp,0L,SEEK_CUR) < 0)) 
		f_writeback = TRUE ;

#if	CF_DEBUG
	if (f_debug)
	    debugprintf("main: about to check output file - wb=%d\n",
	        f_writeback) ;

	bcontrol(ofp,BC_FD,&ofd) ;

	rs = lseek(ofd,0,SEEK_CUR) ;

	if (f_debug) debugprintf(
	    "main: UNIX says that we %s seekable on output\n",
	    (rs < 0) ? "ARE NOT" : "ARE") ;
#endif

	if ((bseek(ofp,0L,SEEK_CUR) < 0) && f_writeback) {

#if	CF_DEBUG
	    if (f_debug) debugprintf("main: making temporary file - wb=%d\n",
	        f_writeback) ;
#endif

	    if (tmpfname == NULL) {

	        tmpfname = tmpfnamebuf ;

	        if ((rs = 
	            mktmplock(tmpdir,"acXXXXXXXXXXXX",0600,tmpfname)) < 0)
	            goto badtmpmk ;

	    }

	    if ((rs = bopen(tfp,tmpfname,"rwct",0600)) < 0)
	        goto badtmpopen ;

	    while ((len = bread(ifp,buf,LINELEN)) > 0) {

	        if ((rs = bwrite(tfp,buf,len)) < len)
	            goto badtmpwrite ;

	    }

	    bclose(ifp) ;

	    bflush(tfp) ;

	    bseek(tfp,0L,SEEK_SET) ;

	    ifp = tfp ;
	    rs = bcontrol(ifp,BC_STAT,&stat_i) ;
	    if (rs < 0)
		goto badstat ;

	    flen = stat_i.st_size ;
	    f_writeback = FALSE ;

	} /* end if (seekable) */

#if	CF_DEBUG
	if (f_debug)
	    debugprintf("main: before hard processing - wb=%d\n",f_writeback) ;
#endif

/* do the hard processing */

	f_header = TRUE ;
	line = 0 ;
	f_bol = TRUE ;
	f_clen = FALSE ;
	offset = 0 ;
	while ((len = breadline(ifp,buf,LINELEN)) > 0) {

	    f_eol = (buf[len - 1] == '\n') ;

#if	CF_DEBUG
	    if (f_debug) debugprintf(
	        "main: got a line - bol=%d eol=%d\n%W",f_bol,f_eol,buf,len) ;
#endif

	    offset += len ;
	    if (f_header) {

	        if (buf[0] == '\n') {

#if	CF_DEBUG
	            if (f_debug) debugprintf(
	                "main: we are at the end of the header part\n") ;
#endif

	            f_header = FALSE ;
	            if (! f_clen) {

	                if (! f_writeback) {

	                    bprintf(ofp,"%s: %d\n",CLENHEADER,
	                        (flen - offset)) ;

	                } else {

/* do not use "input" offset since we may have written differently than read */

				btell(ofp,&off_clen) ;

#if	CF_DEBUG
	                    if (f_debug)
	                        debugprintf("main: offset=%lld "
				"offset(bseek)=%lld\n",
	                            offset,off_clen) ;
#endif

	                    offset = 0 ;

	                    off_clen += (strlen(CLENHEADER) + 2) ;

	                    bprintf(ofp,"%s:           \n",CLENHEADER) ;

	                }
	            }

	        } else if (mheader(CLENHEADER,buf)) {

#if	CF_DEBUG
	            if (f_debug)
	                debugprintf("main: we already have a content length\n") ;
#endif
	            f_clen = TRUE ;

	        }

	    } else {

	        if (f_eol) line += 1 ;

	    } /* end if */

	    bwrite(ofp,buf,len) ;

	    f_bol = f_eol ;

	} /* end while */

#if	CF_DEBUG
	if (f_debug) debugprintf(
	    "main: we have read and writen %d bytes %d lines\n",
	    offset,line) ;
#endif

/* OK, do we have any writeback stuff to do ? */

	if ((! f_clen) && f_writeback) {

#if	CF_DEBUG
	    if (f_debug) debugprintf("main: doing the write back\n") ;
#endif

	    bseek(ofp,off_clen,SEEK_SET) ;

	    bprintf(ofp,"%d",offset) ;

	} /* end if */


/* whew, we can finish up */
earlyret:
	if (tmpfnamebuf[0] != '\0') unlink(tmpfname) ;

	bclose(ifp) ;

	bclose(ofp) ;

	bclose(efp) ;

	return OK ;

badin:
	bprintf(efp,"%s: can't open infile (%d)\n",progname,rs) ;

	goto badret ;

badout:
	bprintf(efp,"%s: can't open outfile (%d)\n",progname,rs) ;

	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments given\n",
		progname) ;

	goto badret ;

badparam:
	bprintf(efp,"%s: bad parameter specified\n",
		progname) ;

	goto badret ;

usage:
	bprintf(efp,"usage: %s [-|file] [-s]\n",
	    progname) ;

badret:
	if (tmpfnamebuf[0] != '\0') unlink(tmpfname) ;

	bclose(efp) ;

	return BAD ;

badtmpmk:
	bprintf(efp,
	    "%s: could not make necessary temporary file\n",
	    progname) ;

	goto badret ;

badtmpopen:
	bprintf(efp,
	    "%s: could not 'bopen' (rs %d) necessary temporary file\n",
	    progname,rs) ;

	goto badret ;

badtmpwrite:
	bprintf(efp,
	    "%s: could not 'bwrite' (rs %d) necessary temporary file\n",
	    progname,rs) ;

	goto badret ;

badstat:
	bprintf(efp,
	    "%s: could not 'bcontrol(stat)' (rs=%d) TMP file\n",
	    progname,rs) ;

	goto badret ;
}
/* end subroutine (main) */



