/* main */

/* part of LOGLSERIAL */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1996-03-01, David A­D­ Morano

	The program was written from scratch.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This program logs somethinpip->  One really has to write these
	little blurbs while the program is being developed! :-)


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<termios.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<bfile.h>
#include	<baops.h>
#include	<field.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	600
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)


/* external subroutines */

extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern int	process() ;

extern char	*strbasename(char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */

static const char *argopts[] = {
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	NULL,
} ;

#define	ARGOPT_VERSION		0
#define	ARGOPT_VERBOSE		1
#define	ARGOPT_TMPDIR		2


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	struct proginfo	pi, *pip = &pi ;
	bfile		errfile, *efp = &errfile ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	maxai ;
	int	argvalue = -1 ;
	int	npa ;
	int	rs, i ;
	int	ofd = 1 ;
	int	ex = 0 ;
	int	logsize = DEFSIZE ;
	int	ex = EX_INFO ;
	int	fd_debug = -1 ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;

	char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	*devbase = DEVBASE ;
	char	*filename = NULL ;
	char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	memset(pip,0,sizeof(struct proginfo)) ;
	pip->verboselevel = 1 ;

	pip->progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"dwca",0666) >= 0)
	    bcontrol(efp,BC_LINEBUF,0) ;


/* early things to initialize */

	pip->efp = efp ;
	pip->debuglevel = 0 ;
	pip->tmpdname = NULL ;

	pip->f.print = FALSE ;
	pip->f.fakeit = FALSE ;


/* process program arguments */

	rs = SR_OK ;
	for (i = 0 ; i < MAXARGGROUPS ; i += 1) 
		argpresent[i] = 0 ;

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
	                    pip->verboselevel = 2 ;
	                    if (f_optequal) 
				goto badargextra ;

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
	                            if (cfdeci(avp,avl, &pip->debuglevel) < 0)
	                                goto badargvalue ;

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

/* file to log to */
	                    case 'f':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) filename = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl > 0) filename = argp ;

	                        }
	                        break ;

/* size of log file output */
	                    case 's':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl > 0) 
					cfdeci(avp,avl,&logsize) ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl > 0)
					cfdeci(argp,argl,&logsize) ;

	                        }
	                        break ;

/* do not actually do it ! */
	                    case 'n':
	                        pip->f.fakeit = TRUE ;
	                        break ;

/* print out something */
	                    case 'p':
	                        pip->f.print = TRUE ;
	                        break ;

/* verbose output */
	                    case 'v':
	                        pip->verboselevel = 2 ;
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
	        "%s: debuglevel %d\n",
	        pip->progname,pip->debuglevel) ;

	    bcontrol(pip->efp,BC_LINEBUF,0) ;

	    bflush(pip->efp) ;

	}

	if (f_version) {

	    bprintf(efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	}

	if (f_usage) goto usage ;

	if (f_version) goto done ;


/* check a few more things */

	if (pip->tmpdname == NULL)
	    pip->tmpdname = getenv("TMPDIR") ;

	if (pip->tmpdname == NULL)
	    pip->tmpdname = TMPDNAME ;


/* is the device base directory there? */

	if (u_access(devbase,R_OK) != 0) {

	    bprintf(pip->efp,
	        "%s: could not access device directory \"%s\"\n",
	        pip->progname,devbase) ;
	    goto retearly ;
	}

	if (pip->verboselevel > 1)
	    bprintf(pip->efp,"%s: dev_directory=%s\n",
	        pip->progname,devbase) ;


/* can we open the output file? */

	if (filename != NULL) {

		if ((ofd = u_open(filename,O_WRONLY | O_CREAT,0666)) < 0)
			goto badoutopen ;

	}


/* round the log size up to the nearest page boundary */

	if ((logsize % (8 * 1024)) != 0)
		logsize = (logsize + (8 * 1024)) & ((8 * 1024) - 1) ;



/* OK, we do it */


	if (npa == 1) {

	for (i = 0 ; i <= maxai ; i += 1) {

	        if ((! BATST(argpresent,i)) &&
	            (argv[i][0] != '\0')) continue ;

	        process(pip,devbase,argv[i],ofd,logsize) ;

		break ;

	    } /* end for (looping through requested circuits) */

	} else
	    goto badtoomany ;

	u_close(ofd) ;


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
	bclose(pip->efp) ;

retearly:
	if (rs < 0) ex = EX_DATAERR ;

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* come here for a bad return from the program */
badret:

#if	CF_DEBUGS
	debugprintf("main: exiting program BAD\n") ;
#endif

	bclose(pip->efp) ;
	goto retearly ;

/* program usage */
usage:
	bprintf(pip->efp,
	    "%s: USAGE> %s [directory(s) ...] [-s search [-s search]] [-rpv]\n",
	    pip->progname,pip->progname) ;

	bprintf(pip->efp, 
	    "\t[-V]\n") ;

	goto done ;

badargextra:
	bprintf(efp,"%s: no value associated with this option key\n",
	    pip->progname) ;

	goto badret ;

badargvalue:
	bprintf(efp,"%s: bad argument value was specified\n",
	    pip->progname) ;

	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",
	    pip->progname) ;

	goto badret ;

badoutopen:
	bprintf(pip->efp,"%s: could not open output (rs=%d)\n",
	    pip->progname,ofd) ;

	goto badret ;

badtoomany:
	close(ofd) ;

	bprintf(efp,"%s: only one device can be monitored\n",
	    pip->progname) ;

	goto badret ;

}
/* end subroutine (main) */


