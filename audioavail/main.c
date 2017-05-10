/* main */

/* part of 'audiavail' */


#define	CF_DEBUGS	0
#define	CF_DEBUG	0


/* revision history:

	= 1996-03-01, David A­D­ Morano

	The program was written from scratch.


*/



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

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#define	MAXARGINDEX	600
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	DEVBASE
#define	DEVBASE		"/dev"
#endif



/* external subroutines */

extern int	matstr(const char **,char *,int) ;

extern int	process(struct proginfo *,bfile *,char *,char *) ;

extern char	*strbasename(char *) ;


/* local forward references */


/* external variables */


/* local variables */

static const char *argopts[] = {
	    "VERSION",
	    "VERBOSE",
	    "TMPDIR",
	    NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_overlast
} ;







int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct proginfo		pi, *pip = &pi ;

	struct userinfo		u ;

	bfile	errfile ;
	bfile	outfile, *ofp = &outfile ;

	int	argr, argl, aol, akl, avl, npa, maxai, kwi ;
	int	rs, i ;
	int	ex = EX_INFO ;
	int	fd_debug ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;

	char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	*pr = NULL ;
	char	*devbase = DEVBASE ;
	char	*cp ;


	if ((cp = getenv(VARDEBUGFD1)) == NULL) {
	    cp = getenv(VARDEBUGFD2) ;
	}

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


	memset(pip,0,sizeof(struct proginfo)) ;

	pip->progname = strbasename(argv[0]) ;


	if (bopen(&errfile,BFILE_STDERR,"dwca",0666) >= 0) {

	    pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;

	}

/* early things to initialize */

	pip->verboselevel = 1 ;

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

	            if ((kwi = matstr(argopts,aop,aol)) >= 0) {

	                switch (kwi) {

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal) 
				goto badargextra ;

	                    break ;

/* verbosity */
	                case argopt_verbose:
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) {

	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

	                                if (rs < 0)
	                                    goto badargval ;

	                            }
	                        }

	                    break ;

/* temporary directory */
	                case argopt_tmpdir:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl) pip->tmpdname = avp ;

	                    } else {

	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl) pip->tmpdname = argp ;

	                    }

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

/* device base directory */
	                    case 'd':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) devbase = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) devbase = argp ;

	                        }
	                        break ;

	                    case 'v':
	                            pip->verboselevel = 2 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl > 0) {

	                                    rs = cfdeci(avp,avl,
	                                        &pip->verboselevel) ;

	                                    if (rs < 0)
	                                        goto badargval ;

	                                }
	                            }

	                            break ;

	                    case '?':
	                        f_usage = TRUE ;
				break ;

	                    default:
				ex = EX_USAGE ;
				f_usage = TRUE ;
	                        bprintf(pip->efp,
					"%s: unknown option - %c\n",
	                            pip->progname,*aop) ;

				break ;

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
	                bprintf(pip->efp,
				"%s: extra arguments specified\n",
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

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage)
	    goto usage ;

	if (f_version)
	    goto done ;


/* check a few more things */

	if (pip->tmpdname == NULL)
	    pip->tmpdname = getenv("TMPDIR") ;

	if ((pip->tmpdname == NULL) || (u_access(pip->tmpdname,W_OK) < 0))
	    pip->tmpdname = TMPDNAME ;


/* is the device base directory there ? */

	if (u_access(devbase,R_OK) != 0) {

	    bprintf(pip->efp,
	        "%s: could not access device directory \"%s\"\n",
	        pip->progname,devbase) ;

	    goto done ;
	}


/* OK, we do it */

	{
	    struct ustat	sb ;

	    int	f_avail = TRUE ;


	    if (npa > 0) {

	        for (i = 0 ; i <= maxai ; i += 1) {

	            if ((! BATST(argpresent,i)) &&
	                (argv[i][0] != '\0')) continue ;

	            rs = process(pip,ofp,devbase,argv[i]) ;

	            if (rs < 0) {

	                f_avail = FALSE ;
	                break ;
	            }

	        } /* end for (looping through requested circuits) */

	    } else if ((cp = getenv(AUDIODEVVAR)) != NULL) {

	        rs = process(pip,ofp,devbase,cp) ;

	        f_avail = (rs >= 0) ;

	    } else if (u_stat(AUDIODEV,&sb) >= 0) {

	        rs = process(pip,ofp,devbase,AUDIODEV) ;

	        f_avail = (rs >= 0) ;

	    } else
	        goto badnoaudio ;


	    ex = (f_avail) ? EX_OK : EX_TEMPFAIL ;

	if (pip->debuglevel > 0)
		bprintf(pip->efp,"%s: available=%u\n",
		pip->progname,f_avail) ;

	} /* end block */


#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting rs=%d ex=%d\n",rs,ex) ;
#endif


/* we are out of here */
done:
ret2:
	if (pip->debuglevel > 0)
		bprintf(pip->efp,"%s: exiting ex=%u\n",
		pip->progname,ex) ;

	bclose(pip->efp) ;

ret0:
	return ex ;

/* program usage */
usage:
	bprintf(pip->efp,
	    "%s: USAGE> %s [device(s) ...] [-n] [-d devbase]\n",
	    pip->progname,pip->progname) ;

	bprintf(pip->efp, 
		"%s: \t[-V]\n",
		pip->progname) ;

	goto done ;

badargextra:
	bprintf(pip->efp,"%s: no value associated with this option key\n",
	    pip->progname) ;

	goto badarg ;

badargval:
	bprintf(pip->efp,"%s: bad argument value was specified\n",
	    pip->progname) ;

	goto badarg ;

badargnum:
	bprintf(pip->efp,"%s: not enough arguments specified\n",
	    pip->progname) ;

	goto badarg ;

badarg:
	ex = EX_USAGE ;
	goto ret2 ;

/* other bad */
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

badnoaudio:
	bprintf(pip->efp,"%s: no audio devices found or specified\n",
	    pip->progname) ;

	goto badret ;

/* come here for a bad return from the program */
badret:
	ex = EX_DATAERR ;
	goto ret2 ;

}
/* end subroutine (main) */



