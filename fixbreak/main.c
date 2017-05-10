/* main */

/* fix up a file which has page ejects in the middle of lines */


#define	CF_DEBUG	0		/* compile-time */


/* revistion history :

	= 1994-09-10, David A­D­ Morano

	I wrote this from scratch.


*/


/*******************************************************************

	This program will read the input file and search for page eject
	characters in the middle of lines.  The program will properly
	break these lines into two lines.  This program is called like
	a typical filter program.


*********************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<string.h>
#include	<ctype.h>

#include	<bfile.h>
#include	<exitcodes.h>

#include	"localmisc.h"



/* local defines */

#define	VERSION		"0"
#define	NPARG		2	/* number of positional arguments */

#define	LINELEN		200
#define	BUFLEN		(LINELEN + LINELEN)



/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;


/* forward references */


/* local variables */





int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	bfile		errfile, *efp = &errfile ;
	bfile		infile, *ifp = &infile ;
	bfile		outfile, *ofp = &outfile ;

	int		argl, aol ;
	int		pan, i ;
	int		len, l, rs ;
	int	ex = EX_INFO ;
	int	f_debug = FALSE ;
	int	f_usage = FALSE ;

	char	*progname, *argp, *aop ;
	char	linebuf[LINELEN + 1], *lbp ;
	char	*ifname = (char *) 0 ;
	char	*ofname = (char *) 0 ;
	char	*cp ;


	progname = argv[0] ;

	if (bopen(efp,BFILE_STDERR,"wca",0666) >= 0) 
		bcontrol(efp,BC_LINEBUF,0) ;


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

	                    case 'D':
	                        f_debug = TRUE ;
	                        break ;

	                    case 'V':
	                        bprintf(efp,"%s: version %s\n",
	                            progname,VERSION) ;

	                        break ;

	                    case 'i':
	                        if (argc <= 0) 
					goto badargnum ;

	                        argp = argv[i++] ;
	                        argc -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl) 
					ifname = argp ;

	                        break ;

	                    case '?':
	                        f_usage = TRUE ;
				break ;

	                    default:
				f_usage = TRUE ;
				ex = EX_USAGE ;
	                        bprintf(efp,"%s: unknown option - %c\n",
	                            progname,*aop) ;

	                    } /* end switch */

	                } /* end while */

	        } else {

	            pan += 1 ;	/* stand-alone dash, skip position */

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
				f_usage = TRUE ;
				ex = EX_USAGE ;

	            } /* end switch */

	            pan += 1 ;

	        } else {

	            bprintf(efp,"%s: extra arguments ignored\n",
	                progname) ;

	        }

	    } /* end if */

	} /* end while */


	if (f_debug) 
		bprintf(efp,"finished parsing arguments\n") ;

	if (f_usage) 
		goto usage ;


	ex = EX_DATAERR ;

/* check arguments */

	if (ifname == NULL) 
		ifname = BFILE_STDIN ;

	if (ofname == NULL)
		ofname = BFILE_STDOUT ;

/* open files */

	if ((rs = bopen(ifp,ifname,"r",0666)) < 0) 
		goto badinfile ;

	if ((rs = bopen(ofp,ofname,"wct",0666)) < 0) {

		goto badoutopen ;
	}

/* process the file */

	while ((len = breadline(ifp,linebuf,LINELEN)) > 0) {

		linebuf[len] = '\0' ;
	    lbp = linebuf ;
	    while ((len > 0) && (*lbp == '\014')) {

#if	CF_DEBUG
		if (f_debug) 
		bprintf(efp,"leading\n") ;
#endif

	        bputc(ofp,*lbp) ;

	        lbp += 1 ;
	        len -= 1 ;

	    } /* end while (FFs at the front of a line) */

	    while ((len > 0) && ((cp = strchr(lbp,'\014')) != NULL)) {

#if	CF_DEBUG
		if (f_debug) bprintf(efp,"middle\n") ;
#endif

	        bprintf(ofp,"%W\n\014",lbp,cp - lbp) ;

	        len -= (cp - lbp + 1) ;
	        lbp = cp + 1 ;

	    } /* end while (FFs in the middle of a line) */

	    if (len > 0) 
		bwrite(ofp,lbp,len) ;

	} /* end while (reading lines) */


	ex = EX_OK ;


/* finish up */
done:
ret2:
	bclose(ifp) ;

	bclose(ofp) ;

ret1:
	bclose(efp) ;

ret0:
	return ex ;

usage:
	bprintf(efp,
	    "usage: %s [-l lines] [-lines] [infile [outfile]]\n",
	    progname) ;

	bprintf(efp,
	    "usage: -l lines    number of lines per page\n") ;

	bprintf(efp,
	    "usage: -lines      number of lines per page\n") ;

	bprintf(efp,
	    "usage: infile      input file\n") ;

	bprintf(efp,
	    "usage: outfile     output file\n") ;

	bprintf(efp,
	    "usage: -D          debugging flag\n") ;

	bprintf(efp,
	    "usage: -V          program version\n") ;

	goto ret1 ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",progname) ;

	goto badarg ;

badarg:
	ex = EX_USAGE ;
	bprintf(efp,"%s: bad input argument specified\n",progname) ;

	goto badret ;

badinfile:
	bprintf(efp,"%s: could not open input file \"%s\" (rs %d)\n",
	    progname,ifname,rs) ;

	goto badret ;

badoutopen:
	bprintf(efp,"%s: could not open output file \"%s\" (rs %d)\n",
	    progname,ofname,rs) ;

	bclose(ifp) ;

	goto badret ;

badret:
	ex = EX_DATAERR ;
	goto ret2 ;

}
/* end subroutine (main) */



