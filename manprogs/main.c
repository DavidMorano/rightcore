/* main */

/* part of the 'manprogs' program */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1996-03-01, David A­D­ Morano

	The program was written from scratch to do what
	the previous program by the same name did.


*/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<termios.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<bfile.h>
#include	<baops.h>
#include	<field.h>
#include	<wdt.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#define		MAXARGINDEX	600
#define		MAXARGGROUPS	(MAXARGINDEX/8 + 1)



/* external subroutines */

extern int	matstr() ;
extern int	wdt() ;
extern int	checkname() ;

extern char	*strbasename() ;


/* local forward references */


/* external variables */


/* global variables */

struct global		g ;


/* local variables */

static char *argopts[] = {
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	NULL,
} ;

#define	ARGOPT_VERSION		0
#define	ARGOPT_VERBOSE		1
#define	ARGOPT_TMPDIR		2




int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile	errfile, *efp = &errfile ;
	bfile	outfile, *ofp = &outfile ;

	int	argr, argl, aol, akl, avl, npa, maxai, kwi ;
	int	i ;
	int	rs ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;
	int	err_fd ;

	char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	*cp ;


	if (((cp = getenv("ERROR_FD")) != NULL) &&
	    (cfdeci(cp,-1,&err_fd) >= 0))
	    debugsetfd(err_fd) ;


	g.progname = strbasename(argv[0]) ;

	g.efp = efp ;
	if (bopen(efp,BFILE_STDERR,"dwca",0666) >= 0)
	    bcontrol(efp,BC_LINEBUF,0) ;


/* early things to initialize */

	g.ofp = ofp ;
	g.debuglevel = 0 ;
	g.tmpdir = NULL ;
	g.suffix = NULL ;
	g.namelen = MAXNAMELEN ;
	g.suffixlen = -1 ;

	g.f.verbose = FALSE ;
	g.f.nochange = FALSE ;
	g.f.print = FALSE ;


/* process program arguments */

	rs = SR_OK ;
	for (i = 0 ; i < MAXARGGROUPS ; i += 1) argpresent[i] = 0 ;

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

	            aop = argp + 1 ;
	            akp = aop ;
	            aol = argl - 1 ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {

	                akl = avp - aop ;
	                avp += 1 ;
	                avl = aop + argl - 1 - avp ;
	                aol = akl ;
	                f_optequal = TRUE ;

	            } else {

	                avl = 0 ;
	                akl = aol ;

	            }

/* do we have a keyword match or should we assume only key letters? */

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

/* version */
	                case ARGOPT_VERSION:
	                    f_version = TRUE ;
	                    if (f_optequal) goto badargextra ;

	                    break ;

/* verbose */
	                case ARGOPT_VERBOSE:
#if	CF_DEBUGS
	                    debugprintf("main: version key-word\n") ;
#endif
	                    g.f.verbose = TRUE ;
	                    if (f_optequal) goto badargextra ;

	                    break ;

/* temporary directory */
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

/* default action and user specified help */
	                default:
			    rs = SR_INVALID ;
	                    f_usage = TRUE ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {

	                    switch ((int) *akp) {

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'D':
	                        g.debuglevel = 1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
				    if (avl)
	                            rs = cfdeci(avp,avl, &g.debuglevel) ;

	                        }

	                        break ;

/* file name length restriction */
	                    case 'l':
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
				if (avl)
	                            rs = (cfdec(avp,avl, &g.namelen) ;

	                    } else {

	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

				if (argl)
	                            rs = cfdeci(argp,argl,&g.namelen) ;

	                    }

				break ;

/* no-change */
	                    case 'n':
	                        g.f.nochange = TRUE ;
	                        break ;

/* print something !! */
	                    case 'p':
	                        g.f.print = TRUE ;
	                        break ;

/* require a suffix for file names */
	                    case 's':
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl) g.suffix = avp ;

	                    } else {

	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl) g.suffix = argp ;

	                    }

				break ;

/* verbose output */
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

	        } else {

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


/* check arguments */

#if	CF_DEBUGS
	debugprintf("main: finished parsing command line arguments\n") ;
#endif

	if (g.debuglevel > 0) {

	    bprintf(g.efp,
	        "%s: debuglevel %d\n",
	        g.progname,g.debuglevel) ;

	    bcontrol(g.efp,BC_LINEBUF,0) ;

	    bflush(g.efp) ;

	}

	if (f_version) {

	    bprintf(efp,"%s: version %s\n",
	        g.progname,VERSION) ;

	}

	if (f_usage) goto usage ;

	if (f_version) goto done ;


/* check a few more things */

	if (g.tmpdir == NULL)
	    g.tmpdir = getenv("TMPDIR") ;

	if (g.tmpdir == NULL)
	    g.tmpdir = TMPDIR ;


	g.suffixlen = -1 ;
	if (g.suffix != NULL)
		g.suffixlen = strlen(g.suffix) ;



/* OK, we do it */

	if (npa > 0) {


	    if (g.f.print || g.f.verbose) {

	        if ((rs = bopen(ofp,BFILE_STDOUT,"dwct",0644)) < 0)
	            goto badoutopen ;

	    }

	    for (i = 0 ; i <= maxai ; i += 1) {

	        if ((! BATST(argpresent,i)) &&
	            (argv[i][0] != '\0')) continue ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: calling process name=\"%s\"\n",argv[i]) ;
#endif

	        if ((rs = process(&g,argv[i])) < 0)
			break ;

	    } /* end for (looping through requested circuits) */

	    if (g.f.print || g.f.verbose)
	    bclose(ofp) ;


	} else
		goto badnodirs ;


#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: exiting\n") ;
#endif


/* good return from program */
goodret:

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    bprintf(g.efp,"%s: program exiting\n",
	        g.progname) ;
#endif


/* we are out of here */
done:
	bclose(g.ofp) ;

	bclose(g.efp) ;

	return OK ;

/* come here for a bad return from the program */
badret:

#if	CF_DEBUGS
	debugprintf("main: exiting program BAD\n") ;
#endif

	bclose(g.ofp) ;

	bclose(g.efp) ;

	return BAD ;

/* program usage */
usage:
	bprintf(g.efp,
	    "%s: USAGE> %s [directory(s) ...] [-s suffix] [-rpv]\n",
	    g.progname,g.progname) ;

	goto badret ;

badargextra:
	bprintf(efp,"%s: no value associated with this option key\n",
	    g.progname) ;

	goto badret ;

badargvalue:
	bprintf(efp,"%s: bad argument value was specified\n",
	    g.progname) ;

	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",
	    g.progname) ;

	goto badret ;

badoutopen:
	bprintf(g.efp,"%s: could not open output (rs=%d)\n",
	    g.progname,rs) ;

	goto badret ;

badinopen:
	bprintf(g.efp,"%s: could not open standard input (rs=%d)\n",
	    g.progname,rs) ;

	goto badret ;

badnodirs:
	bprintf(g.efp,"%s: no files or directories were specified\n",
	    g.progname) ;

	goto badret ;

}
/* end subroutine (main) */



