/* main */

/* main subroutine to the "fortune-info" program */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1998-08-01, David A­D­ Morano

	This subroutine was originally written.


*/


/*******************************************************************

	This program is used to print out information about
	fortune data files.

	Synopsis:

	fortune-info [input_file [outfile]] [-id] [-Vv]


*********************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<time.h>

#include	<bfile.h>
#include	<baops.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#define	MAXARGINDEX	100
#define	NARGGROUPS	(MAXARGINDEX/8 + 1)
#ifndef	LINELEN
#define	LINELEN		200
#endif
#define	BUFLEN		(MAXPATHLEN + (2 * LINELEN))



/* external subroutines */

extern int	matstr() ;
extern int	cfdec() ;
extern int	isdigitlatin(int) ;

extern int	procfile() ;


/* forward references */

static void	helpfile(const char *,bfile *) ;


/* local structures */


/* global data */

struct global		g ;


/* local data */

/* define command option words */

static char *argopts[] = {
	"ROOT",
	"DEBUG",
	"VERSION",
	"VERBOSE",
	"HELP",
	NULL,
} ;

#define	ARGOPT_ROOT		0
#define	ARGOPT_DEBUG		1
#define	ARGOPT_VERSION		2
#define	ARGOPT_VERBOSE		3
#define	ARGOPT_HELP		4






int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile		outfile, *ofp = &outfile ;
	bfile		errfile, *efp = &errfile ;

	struct global	*gp = &g ;

	int	argr, argl, aol, avl ;
	int	maxai, pan, npa, kwi, i ;
	int	argnum ;
	int	rs ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	len, l ;
	int	len2 ;
	int	mode ;

	const char	*argp, *aop, *avp ;
	char	argpresent[NARGGROUPS] ;
	char	linebuf[LINELEN + 1], *lbp ;
	char	buf[BUFLEN + 1] ;
	const char	*ifname = NULL ;
	const char	*ofname = NULL ;
	const char	*cp ;


	g.progname = strbasename(argv[0]) ;

	(void) bopen(efp,BFILE_STDERR,"wca",0666) ;

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

	        if (argl > 1) {
		    const int	ach = MKCHAR(argp[1]) ;

	            if (isdigitlatin(ach)) {

	                if (cfdec(argp + 1,argl - 1,&argnum))
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
	                    case ARGOPT_ROOT:
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
	                    case ARGOPT_DEBUG:
	                        g.debuglevel = 1 ;
	                        if (f_optequal) {

#if	CF_DEBUGS
	                            debugprintf(
	                                "main: debug flag, avp=\"%W\"\n",
	                                avp,avl) ;
#endif

	                            f_optequal = FALSE ;
	                            if ((avl > 0) &&
	                                (cfdec(avp,avl,
	                                &g.debuglevel) < 0))
	                                goto badargvalue ;

	                        }

	                        break ;

	                    case ARGOPT_VERSION:
	                        f_version = TRUE ;
	                        break ;

	                    case ARGOPT_VERBOSE:
	                        g.f.verbose = TRUE ;
	                        break ;

/* help file */
	                    case ARGOPT_HELP:
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
	                                if (cfdec(avp,avl, &g.debuglevel) != OK)
	                                    goto badargvalue ;

	                            }

	                            break ;

	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

#ifdef	COMMENT
/* blank lines at the top of a page */
	                        case 'b':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl && 
	                                (cfdec(argp,argl,&blanklines) < 0))
	                                goto badargvalue ;

	                            break ;

	                        case 'w':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) ofname = argp ;

	                            break ;

	                        case 'f':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            fontname = argp ;

	                            break ;

/* output page headers */
	                        case 'h':
	                            g.f.headers = TRUE ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    g.headerstring = avp ;

	                            }

	                            break ;

	                        case 'l':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl && 
	                                (cfdec(argp,argl,&maxlines) < 0))
	                                goto badargvalue ;

	                            break ;

	                        case 'o':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl && 
	                                (cfdec(argp,argl,&coffset) < 0))
	                                goto badargvalue ;

	                            break ;

	                        case 'p':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

#if	CF_DEBUGS
	                            debugprintf("main: about to get pointsize\n") ;
#endif

	                            if (argl) {

#if	CF_DEBUGS
	                                debugprintf("main: got pointsize\n") ;
#endif

	                                pointstring = argp ;
	                                pointlen = argl ;
	                            }

	                            break ;

/* vertical spacing */
	                        case 'v':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl && (cfdec(argp,argl,&vs) < 0))
	                                goto badargvalue ;

	                            break ;

	                        case 'x':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl && 
	                                (cfdec(argp,argl,&xoffset) < 0))
	                                goto badargvalue ;

	                            break ;

	                        case 'y':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl && 
	                                (cfdec(argp,argl,&yoffset) < 0))
	                                goto badargvalue ;

	                            break ;
#endif /* COMMENT */

	                        case '?':
	                            f_usage = TRUE ;
				    break ;

	                        default:
				    rs = SR_OK ;
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


	if (g.debuglevel > 0) bprintf(efp,
	    "%s: finished parsing arguments\n",
	    g.progname) ;


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

	        l = bufprintf(buf,BUFLEN,"%s/%s",
	            g.programroot,HELPFILE) ;

	        g.helpfile = (char *) malloc_sbuf(buf,l) ;

	    }

	    helpfile(g.helpfile,g.efp) ;

	    goto exit ;

	} /* end if (help) */


	if (g.debuglevel > 0)
	    bprintf(efp,"%s: debuglevel %d\n",
	        g.progname,g.debuglevel) ;


	mode = 0 ;

/* open output file */

	if ((ofname == NULL) || (ofname[0] == '-'))
	    ofname = (char *) BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofname,"wct",0666)) < 0)
	    goto badoutopen ;


/* perform initialization processing */



/* processing the input file arguments */

#if	CF_DEBUG
	if (g.debuglevel > 0) debugprintf(
	    "main: checking for positional arguments\n") ;
#endif

	pan = 0 ;
	if (npa > 0) {

	    for (i = 0 ; i <= maxai ; i += 1) {

	        if (BATST(argpresent,i)) {

#if	CF_DEBUG
	            if (g.debuglevel > 0) debugprintf(
	                "main: got a positional argument i=%d pan=%d arg=%s\n",
	                i,pan,argv[i]) ;
#endif


	            rs = procfile(&g,argv[i],mode) ;

	            if (rs < 0) {

	                if (g.f.verbose)
	                    bprintf(g.efp,"%s: error processing file \"%s\"\n",
	                        g.progname,argv[i]) ;

	            }


	            pan += 1 ;

	        } /* end if (got a positional argument) */

	    } /* end for (loading positional arguments) */

	} else {

	    rs = procfile(&g,"-",mode) ;

	    if (rs < 0) {

	        if (g.f.verbose)
	            bprintf(g.efp,"%s: error processing file \"%s\"\n",
	                g.progname,argv[i]) ;

	    }

	    pan += 1 ;

	} /* end if (processing the input files) */



/* let's get out of here !! */
done:
	if ((g.debuglevel > 0) || g.f.verbose) {

	    bprintf(efp,"%s: files processed - %d\n",
	        g.progname,pan) ;

	}

	bclose(ofp) ;


/* close off and get out ! */
exit:
	bclose(efp) ;

	return OK ;

/* what are we about ? */
usage:
	bprintf(efp,
	    "%s: USAGE> %s [infile [output]] [-id]n",
	    g.progname,g.progname) ;

	bprintf(efp,"\t[-DV]\n") ;


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



static void helpfile(f,ofp)
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



