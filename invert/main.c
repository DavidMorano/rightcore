/* invert */

/* program to invert a line coded file */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-tune debugging */


/* revision history:

	= 1998-07-01, David A­D­ Morano

	This program was written from scratch but inspired by some 
	previous program that I wrote (and lost) by the same name.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	Synopsis:

	$ invert [infile [outfile]]


**************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<ctype.h>

#include	<bfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	NPARG		2

#define	BUFLEN		1000

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern int	mktmplock(const char **,const char *,mode_t,char *) ;

extern char	*strbasename(char *) ;


/* external variables */


/* local variables */

static const char	*tmpdirs[] = {
	"/tmp",
	"/var/tmp",
	"/var/spool/uucppublic",
	"/usr/tmp",
	".",
	NULL
} ;


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	struct ustat	ss ;

	bfile		errfile, *efp = &errfile ;
	bfile		infile, *ifp = &infile ;
	bfile		tmpfile, *tfp = &tmpfile ;
	bfile		outfile, *ofp = &outfile ;

	offset_t	boff ;
	offset_t	offset ;

	long		buflen ;

	int	argl, aol ;
	int	npa, i, j ;
	int	len, rs ;
	int	linebegin, lineend ;
	int	linelen ;
	int	pid ;
	int	argvalue = -1 ;
	int	ex = EX_INFO ;
	int	f_debug = FALSE ;
	int	f_usage = FALSE ;
	int	f_tof = FALSE ;

	const char	*argp, *aop ;
	char	buf[BUFLEN + 1] ;
	char	*lbp ;
	char	tmpfname[MAXPATHLEN + 1] ;
	const char	*progname ;
	const char	*ifname = NULL ;
	const char	*ofname = NULL ;


	progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"dwca",0666) >= 0)
		bcontrol(efp,BC_LINEBUF,0) ;

	npa = 0 ;			/* number of positional so far */
	i = 1 ;
	argc -= 1 ;
	while (argc > 0) {

	    argp = argv[i++] ;
	    argc -= 1 ;
	    argl = strlen(argp) ;

	    if ((argl > 0) && (*argp == '-')) {

	        if (argl > 1) {

	            if (isdigit(argp[1])) {

	                bprintf(efp,"%s: numeric option ignored\n",
	                    progname) ;

	            } else {

	                aop = argp ;
	                aol = argl ;
	                while (--aol) {

	                    akp += 1 ;
	                    switch ((int) *aop) {

	                    case 'D':
	                        f_debug = TRUE ;
	                        break ;

	                    case 'V':
	                        bprintf(efp,"%s version: %s\n",
	                            progname,VERSION) ;

	                        break ;

	                    case '?':
	                        f_usage = TRUE ;
				break ;

	                    default:
				ex = EX_USAGE ;
	                        f_usage = TRUE ;
	                        bprintf(efp,"%s: unknown option - %c\n",
	                            progname,*aop) ;

	                    } /* end switch */

	                } /* end while */
	            }

	        } else {

	            npa += 1 ;	/* increment position count */

	        } /* end if */

	    } else {

	        if (npa < NPARG) {

	            switch (npa) {

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

	            } /* end switch */

	            npa += 1 ;

	        } else {

	            bprintf(efp,"%s: extra arguments ignored\n",
	                progname) ;

	        }

	    } /* end if */

	} /* end while */

	if (f_usage) 
		goto usage ;

	if (f_debug)
		bprintf(efp,"%s: finished parsing arguments\n",
	    progname) ;


/* check arguments */


/* open files */

	if (ifname == NULL)
		ifname = BFILE_STDIN ;

	if ((rs = bopen(ifp,ifname,"r",0666)) < 0)
	    goto badinfile ;

	if (ofname == NULL) 
		ofname = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofname,"wct",0666)) < 0)
	    goto badoutopen ;


/* check if the input file is not seekable */

	tmpfname[0] = '\0' ;
	if ((rs = bseek(ifp,0L,SEEK_CUR)) < 0) {

/* create a temporary file */

	    rs = mktmplock(tmpdirs,"invXXXXXXXXXXX",0600,tmpfname) ;
	    if (rs < 0)
	        goto badtmpfile ;

	    if (f_debug) 
		bprintf(efp,
	        "%s: made the temporary file \n",
	        progname) ;

	    if ((rs = bopen(tfp,tmpfname,"rwct",0600)) < 0) 
		goto badtmpopen ;

	    if (f_debug) 
		bprintf(efp,
	        "%s: opened the temporary file \n",
	        progname) ;

/* copy the entire input file over to the temporary file */

	    while ((len = bread(ifp,buf,BUFLEN)) > 0) {

	        bwrite(tfp,buf,len) ;

	    }

	    bclose(ifp) ;

#ifdef	COMMENT
/* rewind new file */

	    bseek(tfp,0L,SEEK_SET) ;
#endif

/* swap pointers to file structures */

	    bclose(ifp) ;

	    ifp = tfp ;
	    if (f_debug)
		bprintf(efp,"%s: input file was not seekable\n",
	        progname) ;

	} /* end if (seekable or not) */


/* go to the end of the file */

	bseek(ifp,0L,SEEK_END) ;

	btell(ifp,&offset) ;

	buflen = (offset < BUFLEN) ? offset : BUFLEN ;
	boff = (offset - buflen) ;
	bseek(ifp,boff,SEEK_SET) ;

	btell(ifp,&offset) ;

	f_tof = FALSE ;
	while ((! f_tof) && ((len = bread(ifp,buf,(int) buflen)) > 0)) {

	    if (f_debug)
	        bprintf(efp,"%s: read an input block of len %d\n",
	            progname,len) ;

	    lineend = len ;

/* find the next line */

	    j = lineend - 1 ;
	    while (j >= 0) {

/* verify a line ending */

	        if (f_debug && (buf[j] != '\n'))
		    bprintf(efp,"%s: not at line end\n",
	            progname) ;

/* find the beginning of this line */

	        for (j -= 1 ; j >= 0 ; j -= 1)
	            if (buf[j] == '\n') 
			break ;

	        if (j >= 0) {

	            linebegin = j + 1 ;
	            lbp = buf + linebegin ;
	            linelen = lineend - linebegin ;

	            bwrite(ofp,lbp,linelen) ;

	            lineend = linebegin ;

	        } else {

	            if (offset == 0) {

	                f_tof = TRUE ;
	                bwrite(ofp,buf,lineend) ;

	            } else {

	                if ((offset + lineend) > BUFLEN) {

	                    buflen = BUFLEN ;
	                    offset = offset + lineend - BUFLEN ;

	                } else {

	                    buflen = offset + lineend ;
	                    offset = 0 ;

	                }

	                bseek(ifp,offset,SEEK_SET) ;

	            }
	        }
	    }

	} /* end while */


	ex = EX_OK ;
	if (len < 0) {

		ex = EX_DATAERR ;
	    bprintf(efp,
	    "%s: error during input file read (%d)\n",
	    progname,len) ;

	}


	if (f_debug)
	    bprintf(efp,"%s: done\n",progname) ;

	bclose(ifp) ;

	bclose(ofp) ;

/* we are done */
done:
	if (tmpfname[0] != '\0') {

	    bclose(tfp) ;

	    u_unlink(tmpfname) ;

	}

earlyret:
ret1:
	bclose(efp) ;

ret0:
	return ex ;

usage:
	bprintf(efp,
	    "%s: USAGE> %s [-?] [-V]",
	    progname,progname) ;

	bprintf(efp," [infile [outfile]]\n") ;

	goto earlyret ;

badnumarg:
	bprintf(efp,"%s: not enough arguments specified\n",progname) ;

	goto badarg ;

badargval:
	bprintf(efp,"%s: bad argument value was specified\n",
	    pip->progname) ;

	goto badarg ;

badarg:
	ex = EX_USAGE ;
	bprintf(efp,"%s: bad argument given\n",progname) ;

	goto earlyret ;

badinfile:
	bprintf(efp,"%s: cannot open the input file (%d)\n",
	    progname,rs) ;

	goto badret ;

badoutopen:
	bprintf(efp,"%s: cannot open the output file (%d)\n",
	    progname,rs) ;

	goto badret ;

badtmpfile:
	bprintf(efp,"%s: cannot create the temporary file (rs=%d)\n",
	    progname,rs) ;

	goto badret ;

badtmpopen:
	bprintf(efp,"%s: cannot open the temporary file (rs=%d)\n",
	    progname,rs) ;

	goto badret ;
}
/* end subroutine (main) */



