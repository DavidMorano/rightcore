/* main */

/* filter a file that is in a 'catman' type format */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1985-08-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This program will process a file which is in a 'catman' like
	format (suitable for displaying with the 'cat' program) but
	which has too many extra lines on the top and bottom of
	each page (like produced by 'toolman').

	Execute as :

		prtcat [infile]


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	NPARG		1

#ifndef	BUFLEN
#define	BUFLEN		2048
#endif


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;

extern char	*strbasename(char *) ;


/* external variables */


/* local stuff */

static const char *argopts[] = {
	"pl",
	"head",
	"tail",
	NULL
} ;

enum argopts {
	argopt_pl,
	argopt_head,
	argopt_tail,
	argopt_overlast
} ;


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	bfile		errfile, *efp = &errfile ;
	bfile		infile, *ifp = &infile ;
	bfile		outfile, *ofp = &outfile ;

	int	argr, argl, aol ;
	int	pan, i ;
	int	rs, len ;
	int	addlines = 0 ;
	int	headlines = PRCAT_HEADLINES ;
	int	taillines = PRCAT_TAILLINES ;
	int	pagelength = PRCAT_PAGELENGTH ;
	int	lines, pageline, pages ;
	int	ex = EX_INFO ;
	int	fd_debug ;
	int	f_usage = FALSE ;
	int	f_debug = FALSE ;
	int	f_version = FALSE ;
	int	f_dash = FALSE ;
	int	f_verbose = FALSE ;
	int	f_bol = TRUE ;
	int	f_eol = FALSE ;
	int	f_formfeed ;

	char	*argp, *aop ;
	char	*progname ;
	char	linebuf[BUFLEN + 1] ;
	char	*infname = NULL ;
	char	*cp ;


	if ((cp = getenv(VARDEBUGFD1)) == NULL)
	    cp = getenv(VARDEBUGFD2) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


	progname = strbasename(argv[0]) ;

	efp = NULL ;
	if (bopen(efp,BFILE_STDERR,"dwca",0666) >= 0) {

		efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;

	}



	pan = 0 ;			/* number of positional so far */
	i = 0 ;
	argr = argc - 1 ;
	while (argr > 0) {

	    argp = argv[++i] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    if ((argl > 0) && (*argp == '-')) {

	        if (argl > 1) {

	            aop = argp ;
	            aol = argl ;
	            while (--aol) {

	                akp += 1 ;
	                switch ((int) *aop) {

	                case 'D':
	                    f_debug = TRUE ;
	                    break ;

	                case 'V':
	                    f_version = TRUE ;
	                    break ;

	                case 'v':
	                    f_verbose = TRUE ;
	                    break ;

	                case 'a':
	                    if (argr <= 0) 
				goto badargnum ;

	                    argp = argv[++i] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;

	                    if (argl) {

	                        rs = cfdeci(argp,argl,&addlines) ;

				if (rs < 0)
	                            goto badargval ;

	                    }

	                    break ;

/* header blank lines */
	                case 'h':
	                    if (argr <= 0) 
				goto badargnum ;

	                    argp = argv[++i] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;

	                    if (argl) {

	                        rs = cfdeci(argp,argl,&headlines) ;

				if (rs < 0)
	                            goto badargval ;

	                    }

	                    break ;

/* page length */
	                case 'l':
	                    if (argr <= 0) 
				goto badargnum ;

	                    argp = argv[++i] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;

	                    if (argl) {

	                        rs = cfdeci(argp,argl,&pagelength) ;

				if (rs < 0)
	                            goto badargval ;

	                    }

	                    break ;

/* tailing blank lines */
	                case 't':
	                    if (argr <= 0) 
				goto badargnum ;

	                    argp = argv[++i] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;

	                    if (argl) {

	                        rs = cfdeci(argp,argl,&taillines) ;

				if (rs < 0)
	                            goto badargval ;

	                    }

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

	        } else {

	            pan += 1 ;
	            f_dash = TRUE ;

	        } /* end if */

	    } else {

	        if (pan < NPARG) {

	            switch (pan) {

	            case 0:
	                if (argl > 0)
				infname = argp ;

	                break ;
	            }

	            pan += 1 ;

	        } else {

			ex = EX_USAGE ;
	            bprintf(efp,"%s: extra arguments ignored\n",
	                progname) ;

	        }

	    } /* end if */

	} /* end while (argument processing) */


/* handle the standard (no brainer) stuff */

	if (f_debug)
	    bprintf(efp,"%s: finished parsing arguments\n",
	        progname) ;

	if (f_version)
	    bprintf(efp,"%s: version %s\n",
	        progname,VERSION) ;

	if (f_usage) 
		goto usage ;

	if (f_version) 
		goto retearly ;


/* check arguments */

	if (addlines < 0)
	    addlines = 0 ;


/* open files */

	if (infname != NULL)
	rs = bopen(ifp,infname,"r",0666) ;

	else
	rs = bopen(ifp,BFILE_STDIN,"dr",0666) ;

	if (rs < 0)
	    goto badinfile ;

	rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

	if (rs < 0)
	    goto badoutopen ;


/* go through the loops */

	pages = 0 ;
	lines = 0 ;
	pageline = 0 ;
	f_formfeed = FALSE ;
	while ((len = breadline(ifp,linebuf,BUFLEN)) > 0) {

	    f_eol = FALSE ;
	    if (linebuf[len - 1] == '\n') f_eol = TRUE ;

	    if (f_formfeed) {

	        f_formfeed = FALSE ;
	        bprintf(ofp,"%c\n",12) ;

	    }

/* output any leading lines if user specified to have some */

	    if (pageline == 0) {

	        for (i = 0 ; i < addlines ; i += 1)
	            bputc(ofp,'\n') ;

	        if (pages == 0) 
			bputc(ofp,'\n') ;

	    }

/* output the content significant lines */

	    if ((pageline >= headlines) && 
	        (pageline < (pagelength - taillines)))
	        bwrite(ofp,linebuf,len) ;

/* overhead */

	    lines += 1 ;
	    pageline += 1 ;
	    if (pageline == pagelength) {

	        pages += 1 ;
	        pageline = 0 ;
	        f_formfeed = TRUE ;

	    }

	    f_bol = f_eol ;

	} /* end while (reading lines) */

	if (len < 0)
		rs = len ;


	bclose(ifp) ;


	if (f_debug) {

	if (efp != NULL)
	    bprintf(efp,"%s: lines=%d pages=%d\n",
	        lines,pages) ;

	}

	bclose(ofp) ;

	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;


ret2:

retearly:
ret1:
	bclose(efp) ;

ret0:
	return ex ;

/* usage */
usage:
	bprintf(efp,
	    "%s: USAGE> %s [infile] [-l pagelength] [-a additional] [-?VD]\n",
	    progname,progname) ;

	bprintf(efp,
	    "\t[-h headerlines] [-t trailinglines]\n") ;

	goto retearly ;

/* bad arguments (usage) */
badargnum:
	bprintf(efp,"%s: not enough arguments were given\n",
	    progname) ;

	goto badarg ;

badargval:
	bprintf(efp,"%s: a bad argument value was given\n",
	    progname) ;

	goto badarg ;

badarg:
	ex = EX_USAGE ;
	goto ret2 ;

badinfile:
	ex = EX_NOINPUT ;
	bprintf(efp,"%s: cannot open the input file (rs %d)\n",
	    progname,rs) ;

	goto ret2 ;

badoutopen:
	bclose(ifp) ;

	ex = EX_CANTCREAT ;
	bprintf(efp,"%s: could not open standard output (rs %d)\n",
	    progname,rs) ;

	goto ret2 ;

}
/* end subroutine (main) */



