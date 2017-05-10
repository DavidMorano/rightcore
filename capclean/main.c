/* main */

/* capclean */


#define	CF_DEBUG	0
#define	CF_DEBUGS	0


/* revision history:

	= 96/03/01, David A­D­ Morano

	The program was written from scratch to do what
	the previous program by the same name did.


*/


/******************************************************************************

	This is the front-end subroutine for CAPCLEAN.



******************************************************************************/


#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<bfile.h>
#include	<userinfo.h>
#include	<baops.h>
#include	<field.h>
#include	<exitcodes.h>
#include	<mallocstuff.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local subroutine defines */

#define		MAXARGINDEX	100
#define		MAXARGGROUPS	(MAXARGINDEX/8 + 1)



/* external subroutines */

extern int	matstr(char *const *,const char *,int) ;

extern char	*strbasename(char *) ;


/* local forward references */


/* external variables */


/* global variables */


/* local variables */

static char *const argopts[] = {
	"VERSION",
	"VERBOSE",
	NULL,
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_overlast
} ;








int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct userinfo		u ;

	struct proginfo		pi, *pip = &pi ;

	bfile	errfile, *efp = &errfile ;
	bfile	outfile, *ofp = &outfile ;

	int	argr, argl, aol, akl, avl, npa, maxai, kwi ;
	int	rs, i ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;

	char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;


	pip->progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"dwca",0644) >= 0)
		bcontrol(efp,BC_LINEBUF,0) ;

#if	CF_DEBUGS
	debugsetfd(2) ;

	debugprintf("main: efp=%08X\n",efp) ;
#endif

	pip->efp = efp ;

/* early things to initialize */

	if ((rs = bopen(ofp,BFILE_STDOUT,"wct",0644)) < 0)
	    goto badoutopen ;

	pip->ofp = ofp ;
	pip->debuglevel = 0 ;

	pip->f.verbose = FALSE ;
	pip->f.remove = FALSE ;

	if ((rs = userinfo(&u,NULL)) < 0) goto baduser ;

#if	CF_DEBUGS
	debugprintf("main: user=%s homedir=%s\n",
	    u.username,u.homedname) ;
#endif


/* process program arguments */

	for (i = 0 ; i < MAXARGGROUPS ; i += 1) argpresent[i] = 0 ;

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

#if	CF_DEBUGS
	            debugprintf("main: got an option\n") ;
#endif

	            aop = argp + 1 ;
	            akp = aop ;
	            aol = argl - 1 ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {

#if	CF_DEBUGS
	                debugprintf("main: got an option key w/ a value\n") ;
#endif

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
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal) 
				goto badargextra ;

	                    break ;

/* verbose */
	                case argopt_verbose:
	                    pip->f.verbose = TRUE ;
	                    if (f_optequal) 
				goto badargextra ;

	                    break ;

/* default action and user specified help */
	                default:
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
	                            if (cfdec(avp,avl, &pip->debuglevel) < 0)
	                                goto badargval ;

	                        }

	                        break ;

/* verbose output */
	                    case 'v':
	                        pip->f.verbose = TRUE ;
	                        break ;

/* actually remove the files */
				case 'r':
				pip->f.remove = TRUE ;
				break ;

	                    case '?':
	                        f_usage = TRUE ;
				break ;

	                    default:
				f_usage = TRUE ;
				ex = EX_USAGE ;
	                        bprintf(efp,"%s: unknown option - %c\n",
	                            pip->progname,*aop) ;

	                    } /* end switch */

	                    akp += 1 ;

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

	        if (i < MAXARGGROUPS) {

	            BASET(argpresent,i) ;
	            maxai = i ;
	            npa += 1 ;

	        } else {

	            if (! f_extra) {

	                f_extra = TRUE ;
			f_usage = TRUE ;
			ex = EX_USAGE ;
	                bprintf(efp,"%s: extra arguments ignored\n",
	                    pip->progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */


/* check arguments */

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

	if (f_version) {

	    bprintf(efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	    bprintf(efp,"%s: m=%s u=%s\n",
	        pip->progname,u.nodename,
	        u.username) ;

	}

	if (f_usage) 
		goto usage ;

	if (f_version) 
		goto done ;


/* OK, we do it */


	if (npa <= 0) 
		goto goodret ;


	for (i = 0 ; i <= maxai ; i += 1) {

	    if ((! BATST(argpresent,i)) &&
	        (argv[i][0] != '\0')) continue ;

	    process(argv[i]) ;

	} /* end for (looping through requested circuits) */


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: exiting\n") ;
#endif

/* good return from program */
goodret:

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    bprintf(pip->efp,"%s: program exiting\n",
	        pip->progname) ;
#endif

	ex = EX_OK ;


done:
ret2:
	bclose(pip->ofp) ;

ret1:
retearly:
	bclose(pip->efp) ;

ret0:
	return ex ;

/* program usage */
usage:
	bprintf(pip->efp,
	    "%s: USAGE> %s [directory(s) ...] \n",
	    pip->progname,pip->progname) ;

	goto retearly ;

badargextra:
	bprintf(efp,"%s: no value associated with this option key\n",
	    pip->progname) ;

	goto badarg ;

badargval:
	bprintf(efp,"%s: bad argument value was specified\n",
	    pip->progname) ;

	goto badarg ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",
	    pip->progname) ;

	goto badarg ;

badarg:
	ex = EX_USAGE ;
	goto retearly ;

badoutopen:
	bprintf(pip->efp,"%s: could not open output (rs=%d)\n",
	    pip->progname,rs) ;

	goto badret ;

badinopen:
	bprintf(pip->efp,"%s: could not open standard input (rs=%d)\n",
	    pip->progname,rs) ;

	goto badret ;

baduser:
	bprintf(pip->efp,"%s: could not user information (rs=%d)\n",
	    pip->progname,rs) ;

	goto badret ;

badret:
	ex = EX_DATAERR ;
	goto retearly ;
}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */




