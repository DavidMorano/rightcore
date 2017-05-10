/* main */

/* fix up the stupid Micro$oft character problems */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1987-09-10, David A­D­ Morano

	This subroutine was originally written but it was probably started
	from any one of the numerous subroutine which perform a similar
	"file-processing" fron end.


*/

/* Copyright © 1987 David A­D­ Morano.  All rights reserved. */

/*******************************************************************

	This program will read the input file and format it into
	'troff' constant width font style source input language.

	Synopsis:

	$0 [input_file [outfile]] [-DV] [-o offset] [-l lines] 
		[-f font] [-p point_size] [-v vertical_space]


*********************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<estrings.h>
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

extern int	cfdeci(const char *,int,int *) ;
extern int	isdigitlatin(int) ;

extern int	procfile() ;

extern char	*strbasename(char *) ;


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


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct global	*gp = &g ;

	bfile		outfile, *ofp = &outfile ;
	bfile		errfile, *efp = &errfile ;

	int	argr, argl, aol, avl ;
	int	maxai, pan, npa, kwi, i ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	pages = 0 ;
	int	l, rs ;
	int	maxlines = DEFMAXLINES ;
	int	blanklines = DEFBLANKLINES ;
	int	coffset = 0 ;
	int	xoffset = 0 ;
	int	yoffset = 0 ;
	int	ps = DEFPOINT, vs = 0 ;
	int	pointlen ;
	int	f_help = FALSE ;

	const char	*argp, *aop, *avp ;
	char	argpresent[NARGGROUPS] ;
	char	buf[BUFLEN + 1] ;
	char	blanks[LINELEN + 1] ;
	char	blankstring[MAXBLANKLINES + 1] ;
	const char	*fontname = "CW" ;
	const char	*ifname = NULL ;
	const char	*ofname = NULL ;
	const char	*pointstring  = NULL ;
	const char	*cp ;


	g.progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"wca",0666) < 0)
	    bcontrol(efp,BC_LINEBUF,0) ;

	g.efp = efp ;
	g.ofp = ofp ;
	g.debuglevel = 0 ;
	g.pr = NULL ;
	g.helpfile = NULL ;
	g.headerstring = NULL ;

	g.f.verbose = FALSE ;
	g.f.headers = FALSE ;

	f_help = FALSE ;


/* process program arguments */

	for (i = 0 ; i < NARGGROUPS ; i += 1) argpresent[i] = 0 ;

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
	i = 0 ;
	argr = argc - 1 ;
	while (argr > 0) {

	    argp = argv[++i] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 0) && (f_optminus || f_optplus)) {

	        if (argl > 1) {
		    const int	ach = MKCHAR(argp[1]) ;

	            if (isdigitlatin(ach)) {

	                if (cfdeci(argp + 1,argl - 1,&maxlines))
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
	                            if (avl) g.pr = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) g.pr = argp ;

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
	                                (cfdeci(avp,avl,
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
	                                if (cfdeci(avp,avl, 
						&g.debuglevel) != OK)
	                                    goto badargvalue ;

	                            }

	                            break ;

	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

/* blank lines at the top of a page */
	                        case 'b':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl && 
	                                (cfdeci(argp,argl,&blanklines) < 0))
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

	                            if (argl) fontname = argp ;

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
	                                (cfdeci(argp,argl,&maxlines) < 0))
	                                goto badargvalue ;

	                            break ;

	                        case 'o':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl && 
	                                (cfdeci(argp,argl,&coffset) < 0))
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

	                        case 'v':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl && (cfdeci(argp,argl,&vs) < 0))
	                                goto badargvalue ;

	                            break ;

	                        case 'x':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl && 
	                                (cfdeci(argp,argl,&xoffset) < 0))
	                                goto badargvalue ;

	                            break ;

	                        case 'y':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl && 
	                                (cfdeci(argp,argl,&yoffset) < 0))
	                                goto badargvalue ;

	                            break ;

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


#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: finished parsing arguments\n") ;
#endif


/* get our program root (if we have one) */

	if (g.pr == NULL) {

	    if (g.pr == NULL)
	        g.pr = getenv(VARPROGRAMROOT1) ;

	    if (g.pr == NULL)
	        g.pr = getenv(VARPROGRAMROOT2) ;

	    if (g.pr == NULL)
	        g.pr = PROGRAMROOT ;

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
	            g.pr,HELPFILE) ;

	        g.helpfile = (char *) malloc_sbuf(buf,l) ;

	    }

	    helpfile(g.helpfile,g.efp) ;

	    goto exit ;

	}


	if (g.debuglevel > 0)
	    bprintf(efp,"%s: debuglevel %d\n",
	        g.progname,g.debuglevel) ;


/* how many lines per page */

	if (maxlines < 1) maxlines = DEFMAXLINES ;

	if (maxlines > MAXLINES) maxlines = MAXLINES ;

	g.maxlines = maxlines ;

/* establish an offset if any */

	if (coffset < 0)
	    coffset = 0 ;

	else if (coffset > LINELEN)
	    coffset = LINELEN ;

	g.coffset = coffset ;

/* establish working point size and vertical spacing */

	if (pointstring != NULL) {

	    i = substring(pointstring,pointlen,".") ;

	    if (i < 0)
	        i = substring(pointstring,pointlen,"/") ;

	    if (i >= 0) {

	        if ((i > 0) && (cfdeci(pointstring,i,&ps) < 0))
	            goto badargvalue ;

	        if ((pointlen - i) > 1)
	            if (cfdeci(pointstring + i + 1,pointlen - i - 1,&vs) < 0)
	                goto badargvalue ;

	    } else {

	        if (cfdeci(pointstring,pointlen,&ps) < 0)
	            goto badargvalue ;

	    }

	} /* end if (handling the 'pointstring') */

	if (ps < 2) ps = 6 ;

	if (vs == 0)
	    vs = ps + 2 ;

	else if (vs < ps)
	    vs = ps + 1 ;

	if (g.debuglevel > 0) bprintf(efp,
	    "%s: ps %ld - vs %ld\n",g.progname,ps,vs) ;


/* open output file */

	if ((ofname == NULL) || (ofname[0] == '-'))
		rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

	else
		rs = bopen(ofp,ofname,"wct",0666) ;

	if (rs < 0)
	    goto badoutopen ;


/* perform initialization processing */

	for (i = 0 ; i < LINELEN ; i += 1) blanks[i] = ' ' ;

	gp->blanks = blanks ;

	cp = blankstring ;
	for (i = 0 ; (i < blanklines) && (i < MAXBLANKLINES) ; i += 1)
	    *cp++ = '\n' ;

	*cp = '\0' ;
	gp->blankstring = blankstring ;


/* output the header stuff */

	bprintf(ofp,".nf\n") ;

	bprintf(ofp,".fp 1 %s\n.ft %s\n",fontname,fontname) ;


/* change to running point size */

	bprintf(ofp,".ps %ld\n",ps) ;

	bprintf(ofp,".vs %d\n",vs) ;



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


	            rs = procfile(&g,argv[i],pan + 1,(pages & 1)) ;

	            if (rs < 0) {

	                if (g.f.verbose)
	                    bprintf(g.efp,"%s: error processing file \"%s\"\n",
	                        g.progname,argv[i]) ;

	            } else
	                pages += rs ;

#if	CF_DEBUG
	            if (g.debuglevel > 1)
	                debugprintf("main: file pages %d - total pages %d\n",
	                    rs,pages) ;
#endif

	            pan += 1 ;

	        } /* end if (got a positional argument) */

	    } /* end for (loading positional arguments) */

	} else {

	    rs = procfile(&g,"-",pan + 1,(pages & 1)) ;

	    if (rs < 0) {

	        if (g.f.verbose)
	            bprintf(g.efp,"%s: error processing file \"%s\"\n",
	                g.progname,argv[i]) ;

	    } else
	        pages += rs ;

	    pan += 1 ;

	} /* end if */



/* let's get out of here !! */
done:
	if ((g.debuglevel > 0) || g.f.verbose) {

	    bprintf(efp,"%s: files processed - %d\n",
	        g.progname,pan) ;

	    bprintf(efp,"%s: pages processed - %d\n",
	        g.progname,pages) ;

	}

	bclose(ofp) ;


/* close off and get out ! */
exit:
	bclose(efp) ;

	return OK ;

/* what are we about? */
usage:
	bprintf(efp,
	    "%s: USAGE> %s [-l lines] [-lines] [-w outfile] [infile [...]]\n",
	    g.progname,g.progname) ;

	bprintf(efp,"\t\t[-f font] [-o offset] [-DV]\n") ;

	bprintf(efp,"\t\t[-p point_size] [-v vertical_space]\n") ;

	bprintf(efp,
	    "\t-l lines    number of lines per page\n") ;

	bprintf(efp,
	    "\t-lines      number of lines per page\n") ;

	bprintf(efp,
	    "\tinfile      input file\n") ;

	bprintf(efp,
	    "\t-f font     TROFF type font specification\n") ;

	bprintf(efp,
	    "\t-o offset   left margin shift offset\n") ;

	bprintf(efp,
	    "\t-w outfile  output file\n") ;

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



/* print out some help! */
static void helpfile(f,ofp)
const char	f[] ;
bfile		*ofp ;
{
	bfile	file, *ifp = &file ;

	char	buf[BUFLEN + 1] ;


	if ((f == NULL) || (f[0] == '\0')) 
	    return ;

	if (bopen(ifp,f,"r",0666) >= 0) {

	    bcopyblock(ifp,ofp,-1) ;

	    bclose(ifp) ;

	}

}
/* end subroutine (helpfile) */



