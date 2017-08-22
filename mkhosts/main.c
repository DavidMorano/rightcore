/* main (mkhosts) */

/* convert a DNS database (in ASCII) into a 'hosts' table format */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1987-09-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1987 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This program will read a DNS database (in ASCII) and convert it into a
        'hosts' type of file.

	Synopsis:

	$ mkhosts [input_file] [-DV] 


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<time.h>
#include	<string.h>
#include	<stdlib.h>

#include	<bfile.h>
#include	<baops.h>
#include	<exitcodes.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	100
#define	NARGGROUPS	(MAXARGINDEX/8 + 1)
#define	DEFMAXLINES	66
#define	MAXLINES	180
#define	LINELEN		200
#define	BUFLEN		(MAXPATHLEN + (2 * LINELEN))
#define	DEFPOINT	10


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	isdigitlatin(int) ;

extern int	procfile() ;

extern char	*strbasename(char *) ;


/* forward references */

static void	helpfile(const char *,bfile *) ;


/* local structures */


/* global data */

struct global		g ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"DEBUG",
	"VERSION",
	"VERBOSE",
	"HELP",
	NULL,
} ;

enum argopts {
	argopt_root,
	argopt_debug,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_overlast
} ;






int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	struct global	*gp = &g ;

	bfile		outfile, *ofp = &outfile ;
	bfile		errfile, *efp = &errfile ;

	int	argr, argl, aol, avl ;
	int	maxai, pan, npa, kwi, i ;
	int	f_optminus, f_optplus, f_optequal ;
	int	rs, bl ;
	int	ex = EX_INFO ;
	int	argnum ;
	int	nhosts ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;

	const char	*argp, *aop, *avp ;
	char	argpresent[NARGGROUPS] ;
	char	buf[BUFLEN + 1] ;
	const char	*ifname = NULL ;
	const char	*ofname = NULL ;
	const char	*cp ;


	g.progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"wca",0666) < 0)
	    bcontrol(efp,BC_LINEBUF,0) ;

	g.efp = efp ;
	g.ofp = ofp ;
	g.debuglevel = 0 ;
	g.programroot = NULL ;
	g.helpfile = NULL ;

	g.f.verbose = FALSE ;

	f_help = FALSE ;


/* process program arguments */

	rs = SR_OK ;
	for (i = 0 ; i < NARGGROUPS ; i += 1) argpresent[i] = 0 ;

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
	i = 0 ;
	argr = argc - 1 ;
	while ((rs >= 0) && (argr > 0)) {

	    argp = argv[++i] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 0) && (f_optminus || f_optplus)) {
		const int	ach = MKCHAR(argp[1]) ;

	        if (argl > 1) {

	            if (isdigitlatin(ach)) {

	                if (cfdeci(argp + 1,argl - 1,&argnum))
	                    goto badargvalue ;

	            } else {

	                aop = argp + 1 ;
	                aol = argl - 1 ;
	                f_optequal = FALSE ;
	                if ((avp = strchr(aop,'=')) != NULL) {

	                    aol = avp - aop ;
	                    avp += 1 ;
	                    avl = aop + argl - 1 - avp ;
	                    f_optequal = TRUE ;

	                } else
	                    avl = 0 ;

/* do we have a keyword match or should we assume only key letters? */

	                if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                    switch (kwi) {

/* program root */
	                    case argopt_root:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) g.programroot = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) g.programroot = argp ;

	                        }

	                        break ;

/* debug level */
	                    case argopt_debug:
	                        g.debuglevel = 1 ;
	                        if (f_optequal) {

#if	CF_DEBUGS
	                            debugprintf(
	                                "main: debug flag, avp=\"%W\"\n",
	                                avp,avl) ;
#endif

	                            f_optequal = FALSE ;
	                            if ((avl > 0) &&
	                                (cfdeci(avp,avl,
	                                &g.debuglevel) < 0))
	                                goto badargvalue ;

	                        }

	                        break ;

	                    case argopt_version:
	                        f_version = TRUE ;
	                        break ;

	                    case argopt_verbose:
	                        g.f.verbose = TRUE ;
	                        break ;

/* help file */
	                    case argopt_help:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) g.helpfile = avp ;

	                        }

	                        f_help  = TRUE ;
	                        break ;

	                    } /* end switch (key words) */

	                } else {

	                    while (akl--) {

	                        switch ((int) *akp) {

	                        case 'D':
	                            g.debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                rs = cfdeci(avp,avl, &g.debuglevel) ;

					if (rs < 0)
	                                    goto badargvalue ;

	                            }

	                            break ;

	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

	                        case 'v':
					g.f.verbose = TRUE ;
	                            break ;

	                        case '?':
	                            f_usage = TRUE ;
				    break ;

	                        default:
				    rs = SR_INVALID ;
	                            bprintf(efp,
	                            "%s: invalid option=%c\n",
	                            pip->progname,*akp) ;

	                        } /* end switch */

	                        akp += 1 ;
				if (rs < 0)
				    break ;

	                    } /* end while */

	                } /* end if (individual option key letters) */

	            } /* end if (digits as argument or not) */

	        } else {

/* we have a plus or minux sign character alone on the command line */

	            if (i < MAXARGINDEX) {

	                BASET(argpresent,i) ;
	                maxai = i ;
	                npa += 1 ;	/* increment position count */

	            }

	        } /* end if */

	    } else {

	        if (i < MAXARGINDEX) {

	            BASET(argpresent,i) ;
	            maxai = i ;
	            npa += 1 ;

	        } else {

	            if (! f_extra) {

	                f_extra = TRUE ;
	                bprintf(efp,"%s: extra arguments ignored\n",
	                    g.progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */


#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: finished parsing arguments\n") ;
#endif


/* get our program root (if we have one) */

	if (g.programroot == NULL) {

	    if (g.programroot == NULL)
	        g.programroot = getenv(VARPROGRAMROOT1) ;

	    if (g.programroot == NULL)
	        g.programroot = getenv(VARPROGRAMROOT2) ;

	    if (g.programroot == NULL)
	        g.programroot = PROGRAMROOT ;

	} /* end if */


/* continue w/ the trivia argument processing stuff */

	if (f_version) {

	    bprintf(efp,"%s: version %s\n",
	        g.progname,VERSION) ;

	}

	if (f_usage) goto usage ;

	if (f_version) goto exit ;


	if (f_help) {

	    if (g.helpfile == NULL) {

	        bl = mkpath2(buf,
	            g.programroot,HELPFILE) ;

	        g.helpfile = (char *) mallocbuf(buf,bl) ;

	    }

	    helpfile(g.helpfile,g.efp) ;

	    goto exit ;

	}


	if (g.debuglevel > 0)
	    bprintf(efp,"%s: debuglevel %d\n",
	        g.progname,g.debuglevel) ;


/* open output file */

	if ((ofname == NULL) || (ofname[0] == '-'))
		rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

	else
		rs = bopen(ofp,ofname,"wct",0666) ;

	if (rs < 0)
	    goto badoutopen ;


/* processing the input file arguments */

#if	CF_DEBUG
	if (g.debuglevel > 0) debugprintf(
	    "main: checking for positional arguments\n") ;
#endif

	nhosts = 0 ;
	pan = 0 ;
	if (npa > 0) {

	    for (i = 0 ; i <= maxai ; i += 1) {

	        if (BATST(argpresent,i)) {

#if	CF_DEBUG
	            if (g.debuglevel > 0) debugprintf(
	                "main: got a positional argument i=%d pan=%d arg=%s\n",
	                i,pan,argv[i]) ;
#endif


	            rs = procfile(&g,argv[i],pan + 1) ;

	            if (rs < 0) {

	                if (g.f.verbose)
	                    bprintf(g.efp,"%s: error processing file \"%s\"\n",
	                        g.progname,argv[i]) ;

	    } else
		nhosts += rs ;

	            pan += 1 ;

	        } /* end if (got a positional argument) */

	    } /* end for (loading positional arguments) */

	} else {

	    rs = procfile(&g,"-",pan + 1) ;

	    if (rs < 0) {

	        if (g.f.verbose)
	            bprintf(g.efp,"%s: error processing file \"%s\"\n",
	                g.progname,argv[i]) ;

	    } else
		nhosts += rs ;

	    pan += 1 ;

	} /* end if */



/* let's get out of here !! */
done:
	if ((g.debuglevel > 0) || g.f.verbose) {

	    bprintf(efp,"%s: files processed - %d\n",
	        g.progname,pan) ;

	    bprintf(efp,"%s: hosts found - %d\n",
	        g.progname,nhosts) ;

	}

	bclose(ofp) ;


/* close off and get out ! */
exit:
	bclose(efp) ;

	return OK ;

/* what are we about ? */
usage:
	bprintf(efp,
	    "%s: USAGE> %s [infile [...]]\n",
	    g.progname,g.progname) ;

	bprintf(efp,"\t\t[-DV]\n") ;

	bprintf(efp,
	    "\t-D          debugging flag\n") ;

	bprintf(efp,
	    "\t-V          program version\n") ;

	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",g.progname) ;

	goto badret ;

badargvalue:
	bprintf(efp,"%s: bad argument value was specified\n",
	    g.progname) ;

	goto badret ;

badinfile:
	bprintf(efp,"%s: could not open input file \"%s\" (rs %d)\n",
	    g.progname,ifname,rs) ;

	goto badret ;

badoutopen:
	bprintf(efp,"%s: could not open output file \"%s\" (rs %d)\n",
	    g.progname,ofname,rs) ;

	goto badret ;

badret:
	bclose(ofp) ;

	bclose(efp) ;

	return BAD ;
}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



/* print out some help ! */
void helpfile(f,ofp)
const char	f[] ;
bfile		*ofp ;
{
	bfile	file, *ifp = &file ;


	if ((f == NULL) || (f[0] == '\0')) 
	    return ;

	if (bopen(ifp,f,"r",0666) >= 0) {

	    bcopyblock(ifp,ofp,-1) ;

	    bclose(ifp) ;

	}

}
/* end subroutine (helpfile) */



