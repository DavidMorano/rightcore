/* main */

/* part of 'rmlinks' -- remove dead soft links in directory trees */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable debug print-outs */


/* revision history:

	= 1996-03-01, David A­D­ Morano

	The program was written from scratch.


*/


/******************************************************************************

	This is the front end for a program that removed dangling
	file links.


******************************************************************************/


#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<userinfo.h>
#include	<baops.h>
#include	<field.h>
#include	<mallocstuff.h>
#include	<exitcodes.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#define	MAXARGINDEX	600
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)



/* external subroutines */

extern int	matstr(const char **,const char *,int) ;

extern char	*strbasename(char *) ;


/* external variables */


/* local structures */


/* forward references */

extern int	procfile(struct proginfo *,const char *) ;


/* local variables */

static const char *argopts[] = {
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	NULL
} ;

#define	ARGOPT_VERSION		0
#define	ARGOPT_VERBOSE		1
#define	ARGOPT_TMPDIR		2






int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct proginfo	g, *pip = &g ;

	struct userinfo	u ;

	bfile	errfile, *efp = &errfile ;
	bfile	outfile, *ofp = &outfile ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_pos, ai_max, maxai ;
	int	argvalue = -1 ;
	int	pan, npa ;
	int	rs, rs1, i ;
	int	ex = EX_INFO ;
	int	fd_debug = -1 ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f ;

	char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	*cp ;


	if ((cp = getenv(VARDEBUGFD1)) != NULL) {
	    cp = getenv(VARDEBUGFD2) ;
	}

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


	(void) memset(pip,0,sizeof(struct proginfo)) ;

	pip->progname = strbasename(argv[0]) ;


	if (bopen(efp,BFILE_STDERR,"dwca",0666) >= 0)
	    bcontrol(efp,BC_LINEBUF,0) ;


/* early things to initialize */

	pip->efp = efp ;
	pip->ofp = ofp ;
	pip->debuglevel = 0 ;
	pip->tmpdname = NULL ;

	pip->f.print = FALSE ;
	pip->f.no = FALSE ;


/* process program arguments */

	rs = SR_OK ;
	for (ai = 0 ; ai < MAXARGGROUPS ; ai += 1) 
		argpresent[ai] = 0 ;

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
	ai = 0 ;
	ai_pos = 0 ;
	ai_max = 0 ;
	argr = argc - 1 ;
	while ((rs >= 0) && (argr > 0)) {

	    argp = argv[++ai] ;
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

	            if ((kwi = matstr(argopts,akp,akl)) >= 0) {

	                switch (kwi) {

/* version */
	                case ARGOPT_VERSION:
			    f_version = TRUE ;
	                    if (f_optequal) 
				rs = SR_INVALID ;

	                    break ;

/* verbose */
	                case ARGOPT_VERBOSE:
			    pip->verboselevel += 1 ;
	                    if (f_optequal) 
				rs = SR_INVALID ;

	                    break ;

/* temporary directory */
	                case ARGOPT_TMPDIR:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl) 
					pip->tmpdname = avp ;

	                    } else {

	                        if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				}

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl) 
					pip->tmpdname = argp ;

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
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
				    if (avl)
	                            rs = cfdeci(avp,avl, &pip->debuglevel) ;

	                        }

	                        break ;

/* don't do anything */
	                    case 'n':
	                        pip->f.no = TRUE ;
	                        break ;

/* print out the bad links */
	                    case 'p':
	                        pip->f.print = TRUE ;
	                        break ;

/* verbose output */
	                    case 'v':
	                        pip->verboselevel += 1 ;
	                        break ;

	                    case '?':
	                        f_usage = TRUE ;
				break ;

	                    default:
				rs = SR_INVALID ;
	                        bprintf(efp,"%s: unknown option - %c\n",
	                            pip->progname,*aop) ;

	                    } /* end switch */

	                    akp += 1 ;
			    if (rs < 0)
				break ;

	                } /* end while */

	            } /* end if (individual option key letters) */

	        } else {

	            if (ai < MAXARGINDEX) {

	                BASET(argpresent,ai) ;
	                ai_max = ai ;
	                npa += 1 ;	/* increment position count */

	            }

	        } /* end if */

	    } else {

	        if (i < MAXARGINDEX) {

	            BASET(argpresent,ai) ;
	            ai_max = ai ;
	            npa += 1 ;

	        } else {

			rs = SR_INVALID ;
	                bprintf(efp,"%s: extra arguments ignored\n",
	                    pip->progname) ;

	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */

	if (rs < 0)
		goto badarg ;

#if	CF_DEBUGS
	debugprintf("main: finished parsing command line arguments\n") ;
#endif

	if (pip->debuglevel > 0) {

	    bprintf(pip->efp,
	        "%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	    bcontrol(pip->efp,BC_LINEBUF,0) ;

	    bflush(pip->efp) ;

	}

	if (f_version)
	    bprintf(efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage) 
		goto usage ;

	if (f_version) 
		goto done ;


/* check a few more things */

	if (pip->tmpdname == NULL)
	    pip->tmpdname = getenv("TMPDIR") ;

	if (pip->tmpdname == NULL)
	    pip->tmpdname = TMPDNAME ;


/* OK, we do it */

	ex = EX_DATAERR ;

	pan = 0 ;
	if (npa > 0) {

	    if (pip->f.print || (pip->verboselevel > 0))
	        rs = bopen(ofp,BFILE_STDOUT,"dwct",0644) ;

	if (rs >= 0) {

	    for (ai = 0 ; ai <= maxai ; ai += 1) {

	        if ((! BATST(argpresent,ai)) &&
	            (argv[ai][0] != '\0')) continue ;

		cp = argv[ai] ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	debugprintf("main: processing name=%s\n",cp) ;
#endif

		pan += 1 ;
	        rs = procfile(pip,cp) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	debugprintf("main: procfile() ss=%d\n",rs) ;
#endif

		if (rs < 0)
			break ;

	    } /* end for (looping through requested circuits) */

	    if (pip->f.print || (pip->verboselevel > 0))
	    	bclose(ofp) ;

	}

	}

	if ((rs >= 0) && (pan == 0))
		goto badnodirs ;

	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

/* good return from program */
retgood:

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: exiting ex=%u\n",ex) ;
#endif

	    bprintf(pip->efp,"%s: program exiting ex=%u\n",
	        pip->progname,ex) ;

/* we are out of here */
done:
retearly:
	bclose(pip->efp) ;

	return ex ;

/* program usage */
usage:
	ex = EX_USAGE ;
	bprintf(pip->efp,
	    "%s: USAGE> %s [directory(s) ...] [-nv]\n",
	    pip->progname,pip->progname) ;

	goto retearly ;

badarg:
	ex = EX_USAGE ;
	bprintf(efp,"%s: invalid argument specified\n",
	    pip->progname) ;

	goto retearly ;

badoutopen:
	ex = EX_CANTCREAT ;
	bprintf(pip->efp,"%s: could not open output (%d)\n",
	    pip->progname,rs) ;

	goto retearly ;

badnodirs:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: no files or directories were specified\n",
	    pip->progname) ;

	goto retearly ;

}
/* end subroutine (main) */



