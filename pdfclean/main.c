/* main */

/* strip garbage from the front of a PDF file */
/* last modified %G% version %I% */


#define	CF_DEBUG	0


/* revision history:

	= 92/03/01, David A­D­ Morano

	This program was originally written.


*/


/**************************************************************************

	Synopsis:

	moswidth [infile] [-i infile] [-o outfile]


	This program will process width specifications
	for MOSFETs.  Specifically, we will look for
	MOSFET width specifications that have an "X" in them
	and we will convert these to straight microns.


*********************************************************************/


#include	<sys/types.h>
#include	<sys/utsname.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<time.h>
#include	<pwd.h>
#include	<grp.h>
#include	<signal.h>
#include	<string.h>
#include	<errno.h>

#include	<bfile.h>
#include	<baops.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#define		MAXARGINDEX	100
#define		MAXARGGROUPS	(MAXARGINDEX/8 + 1)



/* external functions */

extern int	matstr(const char **,const char *,int) ;

extern char	*strbasename() ;


/* local structures */


/* forward references */


/* local globals */

struct global		g ;


/* local statics */


/* define command option words */

static const char *argopts[] = {
	"VERSION",		/* 0 */
	"TMPDIR",		/* 1 */
	NULL,
} ;

#define	ARGOPT_VERSION	0
#define	ARGOPT_TMPDIR	1






int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	bfile	errfile, *efp = &errfile ;
	bfile	infile, *ifp = &infile ;
	bfile	outfile ;

	struct global	*gdp = &g ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	npa, i, ai ;
	int	len, rs ;
	int	maxai ;
	int	l, pn ;
	int	blen ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_extra = FALSE ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_bol, f_eol ;
	int	line ;
	int	state, c, total ;

	const char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	linebuf[LINELEN + 1] ;
	char	linebuf2[LINELEN + 1] ;
	const char	*ifname = NULL ;
	const char	*ofname = NULL ;
	const char	*cp ;


	g.progname = strbasename(argv[0]) ;

	(void) bopen(efp,BFILE_STDERR,"dwca",0664) ;

	bcontrol(efp,BC_LINEBUF,0) ;


/* some very initial stuff */

	gdp->efp = efp ;
	gdp->ofp = &outfile ;
	gdp->debuglevel = 0 ;
	gdp->tmpdir = NULL ;

	gdp->f.verbose = FALSE ;


/* process program arguments */

	for (i = 0 ; i < MAXARGGROUPS ; i += 1) argpresent[i] = 0 ;

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
	i = 0 ;
	argr = argc - 1 ;
	while (argr > 0) {

#if	CF_DEBUGS
	    debugprintf("main: top of loop \n") ;
#endif

	    argp = argv[++i] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 0) && (f_optminus || f_optplus)) {

#if	CF_DEBUGS
	        debugprintf("main: inside loop\n") ;
#endif

	        if (argl > 1) {

#if	CF_DEBUGS
	            debugprintf("main: got an option\n") ;
#endif

#if	CF_DEBUGS
	            debugprintf("main: more inside \n") ;
#endif

	            aop = argp + 1 ;
	            aol = argl - 1 ;
	            akp = aop ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {

#if	CF_DEBUGS
	                debugprintf("main: got an option key w/ a value\n") ;
#endif

	                akl = avp - aop ;
	                aol = avp - aop ;
	                avp += 1 ;
	                avl = aop + argl - 1 - avp ;
	                f_optequal = TRUE ;

	            } else {

	                akl = aol ;
	                avl = 0 ;

	            }

/* do we have a keyword match or should we assume only key letters? */

	            if ((kwi = matstr(argopts,aop,aol)) >= 0) {

	                switch (kwi) {

	                case ARGOPT_TMPDIR:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl) g.tmpdir = avp ;

	                    } else {

	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl) g.tmpdir = argp ;

	                    }

	                    break ;

	                case ARGOPT_VERSION:
	                    f_version = TRUE ;
	                    if (f_optequal) goto badargextra ;

	                    break ;

/* default action and user specified help */
	                default:
	                    f_usage = TRUE ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {

	                    switch ((int) *aop) {

	                    case 'D':
	                        gdp->debuglevel = 1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (cfdec(avp,avl,&gdp->debuglevel) != OK)
	                                goto badargvalue ;

	                        }

	                        break ;

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* input file */
	                    case 'i':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                ifname = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                ifname = argp ;

	                        }

	                        break ;

/* output file when all subcircuit are combined */
	                    case 'o':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                ofname = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                ofname = argp ;

	                        }

	                        break ;

	                    default:
	                        bprintf(efp,"%s: unknown option - %c\n",
	                            g.progname,*aop) ;

	                    case '?':
	                        f_usage = TRUE ;

	                    } /* end switch */

	                    akp += 1 ;

	                } /* end while */

	            } /* end if (individual option key letters) */

	        } /* end if (something in addition to just the sign) */

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

	if (gdp->debuglevel > 1)
	    bprintf(gdp->efp,
	        "%s: finished parsing arguments\n",
	        gdp->progname) ;


/* done with argument procurement */

	if (f_version)
	    bprintf(efp,"%s: version %s\n",
	        gdp->progname,VERSION) ;

	if (f_usage) goto usage ;

	if (f_version) goto done ;

	if (gdp->debuglevel > 0) bprintf(gdp->efp,
	    "%s: debug level %d\n",
	    gdp->progname,gdp->debuglevel) ;


/* check arguments */


/* where is the TMP directory */

	if (g.tmpdir == NULL) {

	    g.tmpdir = "/tmp" ;
	    if ((cp = getenv("TMPDIR")) != NULL)
	        g.tmpdir = cp ;

	}


/* process some arguments */

/* initialize some basic stuff */

#if	CF_DEBUG
	if (gdp->debuglevel > 0) debugprintf(
	    "main: about to decide input\n") ;
#endif

	if (ifname == NULL) ifname = BFILE_STDIN ;

	if ((rs = bopen(ifp,ifname,"r",0666)) < 0) goto badopenin ;

#if	CF_DEBUG
	if (gdp->debuglevel > 0) debugprintf(
	    "main: about to check for copy of input\n") ;
#endif

	gdp->ifp = ifp ;

#if	CF_DEBUG
	if (gdp->debuglevel > 0) debugprintf(
	    "main: got input, about to separate\n") ;
#endif

/* find the start of the PDF document */

	state = 0 ;
	while ((c = bgetc(gdp->ifp)) >= 0) {

	    switch (state) {

	    case 0:
	        state = 0 ;
	        if (c == '%') state = 1 ;

	        break ;

	    case 1:
	        state = 0 ;
	        if (tolower(c) == 'p') state = 2 ;

	        break ;

	    case 2:
	        state = 0 ;
	        if (tolower(c) == 'd') state = 3 ;

	        break ;

	    case 3:
	        state = 0 ;
	        if (tolower(c) == 'f') state = 3 ;

	        break ;

	    } /* end switch */

	    if (state >= 3) break ;

	} /* end while */

/* write out the rest of the file as it is */

	total = 0 ;
	bprintf(gdp->ofp,"%PDF") ;

	while ((len = bread(ifp,linebuf,LINELEN)) >= 0) {

	    bwrite(gdp->ofp,linebuf,len) ;

	    total += len ;

	}

	if (total <= 0) {

	    bprintf(efp,"%s: this may not have been a PDF file\n") ;

	}

/* done */
done:
	bclose(ifp) ;

	bclose(gdp->ofp) ;

	bclose(efp) ;

	return OK ;

badret:
	bclose(efp) ;

	return BAD ;

usage:
	bprintf(efp,
	    "%s: USAGE> %s [input [output]] ",
	    g.progname,g.progname) ;

	bprintf(efp,
	    "[-i infile] [-o outfile] [-csVD?]\n") ;

	bprintf(efp,"\n") ;

	bprintf(efp,
	    "\t[-TMPDIR temporary_directory] \n") ;

	goto badret ;

badarg:
	bprintf(efp,"%s: bad argument given\n",g.progname) ;

	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",g.progname) ;

	goto badret ;

badargvalue:
	bprintf(efp,"%s: bad argument value specified\n",
	    g.progname) ;

	goto badret ;

badargextra:
	bprintf(efp,"%s: cannot have extra argument values for option\n",
	    g.progname) ;

	goto badret ;

badopenin:
	bprintf(efp,"%s: could not open input file (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

badopenout:
	bprintf(efp,"%s: could not open output file (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

badtmpmake:
	bprintf(efp,"%s: could not create a temporary file (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

badtmpopen:
	bprintf(efp,"%s: could not open a temporary file (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

badwrite:
	bprintf(efp,"%s: bad write (rs %d) - possible full filesystem\n",
	    g.progname,rs) ;

	goto badret ;

badalloc:
	bprintf(efp,"%s: could not allocate a circuit block\n",
	    g.progname) ;

	goto badret ;

badgotcir:
	bprintf(efp,"%s: bad return from 'gotcircuit' (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

badwritecir:
	bprintf(efp,"%s: bad return from 'writecir' (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



