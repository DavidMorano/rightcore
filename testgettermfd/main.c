/* main */

/* generic front-end subroutine */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	1		/* switchable debug print-outs */


/* revision history:

	= 96/03/01, David A­D­ Morano

	The program was written from scratch.


*/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<termios.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<bfile.h>
#include	<userinfo.h>
#include	<baops.h>
#include	<field.h>
#include	<getutmpent.h>
#include	<exitcodes.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#define	MAXARGINDEX	600
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)



/* external subroutines */

extern int	matpstr(const char **,int,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;

extern char	*strbasename(char *) ;


/* local forward references */


/* external variables */


/* global variables */


/* local variables */

static char *argopts[] = {
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	NULL
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

	struct proginfo		pi, *pip = &pi ;

	struct ustat	sb ;

	int	argr, argl, aol, akl, avl, npa, maxai, kwi ;
	int	pan ;
	int	rs, i ;
	int	ec = EC_BADARG ;
	int	newstateval = -1 ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;
	int	f_unknown = FALSE ;
	int	err_fd ;

	char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	termdevbuf1[MAXPATHLEN + 2] ;
	char	termdevbuf2[MAXPATHLEN + 2] ;
	char	*devbase = DEVBASE ;
	char	*termdevice = NULL ;
	char	*newstate = NULL ;
	char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	pip->progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"dwca",0666) >= 0)
	    bcontrol(efp,BC_LINEBUF,0) ;

/* early things to initialize */

	pip->efp = efp ;
	pip->debuglevel = 0 ;
	pip->verboselevel = 1 ;
	pip->tmpdname = NULL ;

	pip->f.verbose = FALSE ;
	pip->f.quiet = FALSE ;


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

/* do we have a keyword match or should we assume only key letters ? */

#if	CF_DEBUGS
	            debugprintf("main: about to check key word match\n") ;
#endif

	            if ((kwi = matpstr(argopts,1,aop,aol)) >= 0) {

#if	CF_DEBUGS
	                debugprintf("main: got an option keyword, kwi=%d\n",
	                    kwi) ;
#endif

	                switch (kwi) {

/* version */
	                case ARGOPT_VERSION:
#if	CF_DEBUGS
	                    debugprintf("main: version key-word\n") ;
#endif
	                    f_version = TRUE ;
	                    if (f_optequal) goto badargextra ;

	                    break ;

/* verbose */
	                case ARGOPT_VERBOSE:
#if	CF_DEBUGS
	                    debugprintf("main: version key-word\n") ;
#endif
	                    pip->f.verbose = TRUE ;
	                    if (f_optequal) goto badargextra ;

	                    break ;

/* temporary directory */
	                case ARGOPT_TMPDIR:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl) pip->tmpdname = avp ;

	                    } else {

	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            pip->tmpdname = argp ;

	                    }

	                    break ;

/* default action and user specified help */
	                default:
	                    f_usage = TRUE ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

#if	CF_DEBUGS
	                debugprintf("main: got an option key letter\n") ;
#endif

	                while (akl--) {

#if	CF_DEBUGS
	                    debugprintf("main: option key letters\n") ;
#endif

	                    switch (*aop) {

	                    case 'V':

#if	CF_DEBUGS
	                        debugprintf("main: version key-letter\n") ;
#endif
	                        f_version = TRUE ;
	                        break ;

	                    case 'D':
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            rs = cfdeci(avp,avl, &pip->debuglevel) ;

	                            if (rs < 0)
	                                goto badargval ;

	                        }

	                        break ;

/* device base directory */
	                    case 'd':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                devbase = avp ;

	                        } else {

	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                devbase = argp ;

	                        }

	                        break ;

/* say "NO" */
	                    case 'n':
	                        newstateval = 0 ;
	                        break ;

/* say "YES" */
	                    case 'y':
	                        newstateval = 1 ;
	                        break ;

	                    case 'q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* terminal device */
	                    case 't':
	                        if (argr <= 0)
	                            goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            termdevice = argp ;

	                        break ;

/* verbose output */
	                    case 'v':
	                        pip->f.verbose = TRUE ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            rs = cfdeci(avp,avl, &pip->verboselevel) ;

	                            if (rs < 0)
	                                goto badargval ;

	                        }

	                        break ;

	                    case '?':
	                        if (! f_unknown)
	                            ec = EC_INFO ;

	                        f_usage = TRUE ;
	                        break ;

	                    default:
	                        f_usage = TRUE ;
	                        f_unknown = TRUE ;
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

	        if (i < MAXARGINDEX) {

	            BASET(argpresent,i) ;
	            maxai = i ;
	            npa += 1 ;

	        } else {

	            if (! f_extra) {

	                f_extra = TRUE ;
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

	if (pip->debuglevel > 1) {

	    bprintf(pip->efp,
	        "%s: debuglevel %d\n",
	        pip->progname,pip->debuglevel) ;

	    bcontrol(pip->efp,BC_LINEBUF,0) ;

	    bflush(pip->efp) ;

	}

	if (f_version) {

	    bprintf(efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	}

	if (f_usage || f_unknown)
	    goto usage ;

	ec = EC_INFO ;
	if (f_version)
	    goto done ;


/* get the switch argument ('YES' or 'NO') if there is one */

	pan = 1 ;
	if (npa > 0) {

	    for (i = 1 ; i <= maxai ; i += 1) {

	        if (BATST(argpresent,i)) {

	            switch (pan) {

	            case 1:
	                if (argv[i][0] != '\0')
	                    newstate = argv[i] ;

	                break ;

	            case 2:
	                if (argv[i][0] != '\0')
	                    termdevice = argv[i] ;

	                break ;

	            } /* end switch */

	            pan += 1 ;

	        } /* end if (we had an argument) */

	    } /* end for (looping through requested circuits) */

	} /* end if */


/* what is the terminal device we want to pop ? */

	if ((termdevice == NULL) || (termdevice[0] == '\0')) {

	    if ((termdevice = getenv(TERMDEVICEVAR)) == NULL) {

	        rs = getutmpline(termdevbuf1,MAXPATHLEN,0) ;

	        if (rs < 0)
	            goto badnoterm ;

	        termdevice = termdevbuf1 ;

	    } /* end if (getting control terminal) */

	} /* end if */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: termdevice=%s\n",termdevice) ;
#endif

	if (termdevice[0] != '/') {

/* is the device base directory there ? */

	    if (u_access(devbase,R_OK) != 0) {

	        bprintf(pip->efp,
	            "%s: could not access device directory \"%s\"\n",
	            pip->progname,devbase) ;

	        ec = EC_ERROR ;
	        goto done ;
	    }

	    if ((u_stat(devbase,&sb) < 0) || (! S_ISDIR(sb.st_mode)))
	        goto baddir ;

	    mkpath2(termdevbuf2, devbase,termdevice) ;

	    termdevice = termdevbuf2 ;

	} /* end if */


/* OK, we do it */


	if (pip->verboselevel > 0) {

	    if ((rs = bopen(ofp,BFILE_STDOUT,"dwct",0644)) < 0)
	        goto badoutopen ;

	}

	if (pip->verboselevel > 1)
	    bprintf(ofp,"terminal device %s\n",termdevice) ;


	{


/* get the current state */

	    if ((rs = u_stat(termdevice,&sb)) < 0)
	        goto badstat ;

	        bprintf(ofp,"termdevice inode=%d device=\\%08lx\n",
			sb.st_ino,sb.st_rdev) ;


		if ((rs = u_stat("/dev/tty",&sb)) < 0)
			goto badstat ;

	        bprintf(ofp,"termdevice inode=%d device=\\%08lx\n",
			sb.st_ino,sb.st_rdev) ;



	} /* end block */


retout:
	if (pip->verboselevel > 0)
	    bclose(ofp) ;



	ec = EC_OK ;

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


/* we are out of here */
done:
earlyret:
	bclose(pip->efp) ;

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ec ;

/* program usage */
usage:
	bprintf(pip->efp,
	    "%s: USAGE> %s [y|n] [-v[=n]] [-t terminal]",
	    pip->progname,pip->progname) ;

	bprintf(pip->efp, 
	    " [-V]\n") ;

	goto earlyret ;

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

badarg:
	ec = EC_BADARG ;
	goto earlyret ;


/* error type returns */
badnoterm:
	bprintf(pip->efp,
	    "%s: could not find controlling terminal (rs=%d)\n",
	    pip->progname,rs) ;

	goto baderr ;

baddir:
	bprintf(efp,"%s: not enough arguments specified\n",
	    pip->progname) ;

	goto baderr ;

badoutopen:
	bprintf(pip->efp,"%s: could not open output (rs=%d)\n",
	    pip->progname,rs) ;


/* come here for a bad return from the program */
baderr:
	ec = EC_ERROR ;

#if	CF_DEBUGS
	debugprintf("main: exiting program BAD\n") ;
#endif

	goto done ;


/* bad stuff with STDOUT open */
badstat:
	bprintf(efp,"%s: could not get status on terminal (%d)\n",
	    pip->progname,rs) ;

	goto retout ;

badchmod:
	bprintf(efp,"%s: could not change mode on terminal (%d)\n",
	    pip->progname,rs) ;

	goto retout ;

}
/* end subroutine (main) */



