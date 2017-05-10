/* main */

/* invert a line coded file */
/* last modified %G% version %I% */


#define	F_DEBUG	0


/* revision history:

	= 1994-09-01, David A.D. Morano

	lost the program that I wrote years ago)

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	Synopsis:

		lineinvert [infile | - [outfile]]


**************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<ctype.h>

#include	<bfile.h>
#include	<localmisc.h>


/* local defines */

#define		VERSION		"0"
#define		NPARG		1
#define		LINELEN		200


/* external subroutines */

extern int	mktmpfile(char *,mode_t,const char *) ;


/* external variables */


/* forward references */


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


	progname = argv[0] ;
	bopen(efp,BIO_STDERR,"wca",0666) ;

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
	                if (argl > 0) ifname = argp ;

	                break ;

	            case 1:
	                if (argl > 0) ofname = argp ;

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


/* open files */

	if (ifname == NULL) ifname = BFILE_STDIN ;

	if (ofname == NULL) ofname = BFILE_STDOUT ;

	if ((rs = bopen(ifp,ifname,"r",0666)) < 0)
	    goto badinfile ;

	if ((rs = bopen(ofp,ofname,"wct",0666)) < 0)
	    goto badoutfile ;

#if	F_DEBUG
	if (f_debug)
	    bprintf(efp,"%s: half way openning files\n",
	        progname) ;
#endif

	if (! f_index) {

	    rs = mktmpfile(xfnamebuf,0600,"/tmp/lixXXXXXXXXXXX") ;
	    if (rs < 0)
	        goto badindexmk ;

	    if (f_debug)
	        bprintf(efp,
	            "%s: made a temporary file \"%s\"\n",
	            progname,xfnamebuf) ;

	} /* end if */

#if	F_DEBUG
	if (f_debug)
	    bprintf(efp,"%s: three quarters way openning files\n",
	        progname) ;
#endif

	if ((rs = bopen(xfp,xfname,"rwct",0666)) < 0)
	    goto badindexopen ;

#if	F_DEBUG
	if (f_debug)
	    bprintf(efp,"%s: finished openning files\n",
	        progname) ;
#endif

/* other initializations */

	if (bseek(ifp,0,SEEK_SET) < 0) {

	    if (f_debug) bprintf(efp,
	        "we got a non-seekable input file\n") ;

	    f_notseek = TRUE ;
	    rs = mktmpfile( tfname, 0600, "/tmp/litXXXXXXXXXXX") ;
	    if (rs < 0) goto badtmpmk ;

	    if (f_debug) bprintf(efp,"made a temporary file \"%s\"\n",
	        tfname) ;

	    if ((rs = bopen(tfp,tfname,"rwct",0666)) < 0)
	        goto badtmpopen ;

	} /* end if */


/* go through the loops for indexing */

#if	F_DEBUG
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

	if (! f_eol)
	    line += 1 ;



	if (f_notseek) {

	    bclose(ifp) ;

	    ifp = tfp ;
	}

	if (f_debug)
	    bprintf(efp,
	        "%s: about to write it out - lines %d\n",
	        progname,line) ;

	rs = bseek(xfp,- sizeof(offset_t),SEEK_CUR) ;

	if (f_debug) bprintf(efp,
	    "%s: tried to seek rs=%d\n",
	    progname,rs) ;

	poffset = offset ;
	nlines = line ;
	while (offset > 0) {

	    if ((rs = bread(xfp,&offset,sizeof(offset_t))) < 0)
	        goto badindexread ;

	    bseek(ifp,offset,SEEK_SET) ;

	    line -= 1 ;
	    if (f_debug) bprintf(efp,
	        "%s: about to copy line %d - o=%08X po=%08X rs=%d\n",
	        progname,nlines - line,offset,poffset,rs) ;

	    mlen = poffset - offset ;
	    while (mlen > 0) {

	        rlen = MIN(mlen,LINELEN) ;
	        if ((len = bread(ifp,linebuf,rlen)) < 1)
	            goto badread ;

	        if ((rs = bwrite(ofp,linebuf,len)) < len)
	            goto badwrite ;

	        mlen -= len ;

	    } /* end while */

	    if (offset > 0) {

	        poffset = bseek(xfp,- (2 * sizeof(offset_t)),SEEK_CUR) ;

	        if (f_debug)
	            bprintf(efp,"%s: poffset=%08X\n",
	                progname,poffset) ;

	    }

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
	if (tfname[0] != '\0') unlink(tfname) ;

	if (xfnamebuf[0] != '\0') unlink(xfname) ;

	bclose(efp) ;

	return OK ;

badret:
	if (tfname[0] != '\0') unlink(tfname) ;

	if (xfnamebuf[0] != '\0') unlink(xfname) ;

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
	    "%s: bad 'bread' on file (rs %d)\n",
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


