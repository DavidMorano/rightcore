/* main */

/* program to exec a program with the named arg0 */
/* last modified %G% version %I% */


#define	F_DEBUGS	0		/* debug print-outs (non-switchable) */
#define	F_DEBUG		1		/* debug print-outs switchable */


/* revision history :

	= 1990-11-01, David A­D­ Morano
	This subroutine was originally written.

	= 2001-04-11, David A­D­ Morano
	This old dog program has been enhanced to serve as the
	environment wiper for executing MIPS programs.

*/

/* Copyright © 1990,2001 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	Execute as :

		envwipe [-e envfile] [-V] prog arg0 arg1 ...


**************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"misc.h"
#include	"defs.h"


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	matstr(char * const *,char *,int) ;
extern int	getpwd(char *,int) ;
extern int	findfilepath(const char *,char *,const char *,int) ;
extern int	isdigitlatin(int) ;

extern char	*strbasename(char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	usage(PROGINFO *) ;


/* local variables */

static char *argopts[] = {
	"VERSION",
	NULL,
} ;

#define	ARGOPT_VERSION	0






int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	vecstr		exports = obj_vecstr(20,0) ;

	struct ustat	sb ;

	FILE		*efp = NULL ;

	PROGINFO	pi, *pip = &pi ;

	int	argr, argl, aol, akl, avl ;
	int	kwi, npa, ai = 0, i ;
	int	f_optplus, f_optminus, f_optequal ;
	int	f_exitargs = FALSE ;
	int	f_programroot = FALSE ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	rs ;
	int	unknown ;
	int	ex = EX_INFO ;
	int	sl ;
	int	err_fd = -1 ;
	int	fd_stderr ;

	char	*argp, *aop, *akp, *avp ;
	char	*programroot = NULL ;
	char	progfnamebuf[MAXPATHLEN + 1] ;
	char	*progfname = NULL ;
	char	*envfname = NULL ;
	char	*arg0 = NULL ;
	char	*progfile ;
	char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	if (u_fstat(FD_STDERR,&sb) >= 0) {
		fd_stderr = u_dup(FD_STDERR) ;
		efp = fdopen(fd_stderr,"a") ;
	} /* end if */

	(void) memset(pip,0,sizeof(PROGINFO)) ;

	pip->efp = efp ;
	pip->progname = strbasename(argv[0]) ;

/* early things to initialize */

	pip->version = VERSION ;
	pip->programroot = NULL ;

	pip->debuglevel = 0 ;

	pip->f.verbose = FALSE ;

/* gather the arguments */

	npa = 0 ;			/* number of positional so far */
	i = 0 ;
	argr = argc - 1 ;
	while ((! f_exitargs) && (argr > 0)) {

	    argp = argv[++i] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 0) && (f_optplus || f_optminus)) {

	        if (argl > 1) {
		    const int	ach = MKCHAR(argp[1]) ;

	            if (isdigitlatin(ach)) {

	                if (cfdeci(argp + 1,argl - 1,&unknown) < 0)
	                    goto badarg ;

	            } else {

	                aop = argp + 1 ;
	                aol = argl - 1 ;
	                akp = aop ;
	                f_optequal = FALSE ;
	                if ((avp = strchr(aop,'=')) != NULL) {

	                    akl = avp - aop ;
	                    avp += 1 ;
	                    avl = aop + aol - avp ;
	                    f_optequal = TRUE ;

	                } else {

	                    akl = aol ;
	                    avl = 0 ;

	                }

	                if ((kwi = matstr(argopts,akp,akl)) >= 0) {

	                    switch (kwi) {

/* version */
	                    case ARGOPT_VERSION:
	                        f_version = TRUE ;
	                        if (f_optequal)
	                            goto badargextra ;

	                        break ;

/* handle all keyword defaults */
	                    default:
				ex = EX_USAGE ;
	                        f_exitargs = TRUE ;
	                        f_usage = TRUE ;
				brea ;

	                    } /* end switch */

	                } else {

	                    while (akl--) {
				const int	kc = MKCHAR((*akp) ;

	                        switch (*akp) {

/* debug */
	                        case 'D':
	                            pip->debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl > 0) {

	                                    rs = cfdeci(avp,avl,
	                                        &pip->debuglevel) ;

	                                    if (rs < 0)
	                                        goto badargval ;

	                                }
	                            }

	                            break ;

	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

/* environment file */
	                        case 'e':
	                                if (argr <= 0)
	                                    goto badargnum ;

	                                argp = argv[++i] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;

	                                if (argl)
	                                    envfname = argp ;

	                            break ;

	                        case '?':
	                            f_usage = TRUE ;
	                            break ;

	                        default:
	                            ex = EX_USAGE ;
	                            f_usage = TRUE ;
	                            break ;

	                        } /* end switch */

	                        akp += 1 ;

	                    } /* end while */

	                } /* end if (individual option key letters) */

	            } /* end if (digits as argument or not) */

	        } else {

/* we have a plus or minus sign character alone on the command line */

	            npa += 1 ;

	        } /* end if */

	    } else {

	        switch (npa) {

	        case 0:
	            if (argl > 0)
	                progfname = argp ;

	            break ;

	        case 1:
	            arg0 = argp ;
	            ai = i ;
	            f_exitargs = TRUE ;
	            break ;

	        default:
	            ex = EX_USAGE ;
	            f_usage = TRUE ;
	            if (efp != NULL)
	                fprintf(efp,
	                    "%s: extra positional arguments ignored\n",
	                    pip->progname) ;

	        } /* end switch */

	        npa += 1 ;

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */

#if	F_DEBUGS
	eprintf("main: done looping on arguments\n") ;
#endif


	if (f_version && (efp != NULL)) {
	    fprintf(efp,"%s: version %s\n",pip->progname,VERSION) ;
	}

	if (f_usage) {
	    rs = usage(pip) ;
	    goto done ;
	}

	if (f_version)
	    goto done ;


/* check arguments */

#if	F_DEBUG
	if (pip->debuglevel > 1)
	eprintf("main: checking arguments\n") ;
#endif

	if (npa < 2)
	    goto badargnum ;

	if ((progfname == NULL) || (arg0 == NULL))
	    goto badarg ;


/* let's move on */

#if	F_DEBUG
	if (pip->debuglevel > 1)
	eprintf("main: programroot\n") ;
#endif

	if (pip->programroot == NULL) {

	    programroot = getenv(PROGRAMROOTVAR1) ;

	    if (programroot == NULL)
	        programroot = getenv(PROGRAMROOTVAR2) ;

	    if (programroot == NULL)
	        programroot = getenv(PROGRAMROOTVAR3) ;

	    if (programroot == NULL)
	        programroot = PROGRAMROOT ;

	    pip->programroot = programroot ;

	} else {

	    f_programroot = TRUE ;
	    programroot = pip->programroot ;

	}

#if	F_DEBUG
	if (pip->debuglevel > 1)
	eprintf("main: programroot=%s\n",pip->programroot) ;
#endif



	ex = EX_DATAERR ;



/* read in any environment that was specified */

#if	F_DEBUG
	if (pip->debuglevel > 1)
	eprintf("main: user specified environment, envfname=%s\n",
		envfname) ;
#endif

	if (envfname != NULL) {

		rs = procenv(pip->programroot,envfname,&exports) ;

#if	F_DEBUG
	if (pip->debuglevel > 1) {
	eprintf("main: procenv() rs=%d\n",rs) ;
		for (i = 0 ; rs = vecstr_get(&exports,i,&cp) >= 0 ; i += 1) {
			if (cp == NULL) continue ;
			eprintf("main: e> %s\n",cp) ;
		}
	}
#endif /* F_DEBUG */

	} /* end if (user specified environment) */

	if (efp != NULL)
	    fclose(efp) ;

/* find the file path if we need to */

#if	F_DEBUG
	if (pip->debuglevel > 1)
	eprintf("main: finding program file, progfname=%s \n",
		progfname) ;
#endif

	progfile = progfname ;
	if (progfile[0] != '/') {

	    rs = findfilepath(NULL,progfnamebuf,progfile,X_OK) ;

#if	F_DEBUG
	if (pip->debuglevel > 1)
	eprintf("main: findfilepath() rs=%d progfnamebuf=%s\n",
		rs,progfnamebuf) ;
#endif

	    if (rs == 0) {

	        sl = getpwd(progfnamebuf,-1) ;

		strwcpy(progfnamebuf + sl,"/",MAXPATHLEN - sl) ;

	        strwcpy(progfnamebuf + sl + 1,progfile,MAXPATHLEN - sl - 1) ;

	        progfile = progfnamebuf ;

	    } else
	        progfile = progfnamebuf ;

	} /* end if */

#if	F_DEBUG
	if (pip->debuglevel > 1)
	eprintf("main: progfile=%s\n",progfile) ;
#endif


/* create the environment for the program to execute */

#if	F_DEBUG
	if (pip->debuglevel > 1)
	eprintf("main: ai=%d arg0=%s\n",ai,arg0) ;
#endif



/* do it */

	{
	    const char	**eav = (const char **) (argv + ai) ;
	    const char	**eev = (const char **) exports.va ;
	    rs = u_execve(progfile,eav,eev) ;
	}

/* handle error condition */

	ex = EX_NOEXEC ;
	if (efp != NULL) {

	    fprintf(stderr,
	        "%s: program would not 'exec' correctly (rs %d)\n",
	        pip->progname,rs) ;

	    fprintf(stderr,
	        "%s: program attempted was \"%s\" \n",
	        pip->progname,progfile) ;

	}

	goto done ;

badargval:
	fprintf(efp,"%s: bad argument value was specified\n",
	    pip->progname) ;

	goto badarg ;


badargnum:
	if (efp != NULL)
	    fprintf(stderr,
	        "%s: not enough arguments given\n",
	        pip->progname) ;

	goto badarg ;

badargextra:
	if (efp != NULL)
	    fprintf(efp,
	        "%s: no value associated with this option key\n",
	        pip->progname) ;

	goto badarg ;

badarg:
	ex = EX_USAGE ;
	if (efp != NULL)
	    fprintf(stderr,
	        "%s: a bad (NULL, whatever !) argument was given\n",
	        pip->progname) ;

	goto done ;

badusage:
	ex = EX_USAGE ;
	rs = usage(pip) ;

	goto done ;

badprog:
	ex = EX_NOPROG ;

done:
	if (efp != NULL)
	    fclose(stderr) ;

	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


/* print out (standard error) some short usage */
static int usage(PROGINFO *pip)
{
	int		rs = SR_NOTOPEN ;

	if (pip->efp != NULL) {
	    rs = SR_OK ;
	    fprintf(pip->efp,
	        "%s: USAGE> %s [-e envfile] program [arg0 [args ...]]\n",
	        pip->progname,pip->progname) ;
	}

	return rs ;
}
/* end subroutine (usage) */


