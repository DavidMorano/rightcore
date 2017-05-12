/* main */

/* generic front-end subroutine */


#define	F_DEBUGS	0		/* non-switchable debug print-outs */
#define	F_DEBUG		1		/* switchable debug print-outs */


/* revision history :

	= 1998-03-01, David A­D­ Morano
	The program was written from scratch.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is the front-end (main) subroutine for the MESG program. This
        program is so small that this subroutine is pretty much it!


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<termios.h>
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<bfile.h>
#include	<userinfo.h>
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

extern int	matstr(const char **,const char *,int) ;
extern int	getloginterm(int,char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;

extern char	*strbasename(char *) ;


/* local forward references */


/* external variables */


/* global variables */


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


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	bfile	errfile, *efp = &errfile ;
	bfile	outfile, *ofp = &outfile ;

	struct proginfo		pi, *pip = &pi ;

	struct ustat	sb ;

	int	argr, argl, aol, akl, avl, npa, maxai, kwi ;
	int	pan ;
	int	rs, i ;
	int	ex = EX_INFO ;
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


	if (((cp = getenv("ERROR_FD")) != NULL) &&
	    (cfdeci(cp,-1,&err_fd) >= 0))
	    esetfd(err_fd) ;

	    else
	    eclose() ;


	pip->progname = strbasename(argv[0]) ;

	if (bopen(efp,BIO_STDERR,"dwca",0666) >= 0)
	    bcontrol(efp,BC_LINEBUF,0) ;


/* early things to initialize */

	pip->efp = efp ;
	pip->debuglevel = 0 ;
	pip->verboselevel = 1 ;
	pip->tmpdir = NULL ;

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

#if	F_DEBUGS
	            eprintf("main: got an option\n") ;
#endif

	            aop = argp + 1 ;
	            akp = aop ;
	            aol = argl - 1 ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {

#if	F_DEBUGS
	                eprintf("main: got an option key w/ a value\n") ;
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

#if	F_DEBUGS
	            eprintf("main: about to check for a key word match\n") ;
#endif

	            if ((kwi = matstr(argopts,aop,aol)) >= 0) {

#if	F_DEBUGS
	                eprintf("main: got an option keyword, kwi=%d\n",
	                    kwi) ;
#endif

	                switch (kwi) {

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    break ;

/* verbose */
	                case argopt_verbose:
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            rs = cfdeci(avp,avl, &pip->verboselevel) ;

	                            if (rs < 0)
	                                goto badargval ;

	                        }

	                    break ;

/* temporary directory */
	                case argopt_tmpdir:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl) 
					pip->tmpdir = avp ;

	                    } else {

	                        if (argr <= 0) 
					goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            pip->tmpdir = argp ;

	                    }

	                    break ;

/* default action and user specified help */
	                default:
			ex = EX_USAGE ;
	                    f_usage = TRUE ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

#if	F_DEBUGS
	                eprintf("main: got an option key letter\n") ;
#endif

	                while (aol--) {

#if	F_DEBUGS
	                    eprintf("main: option key letters\n") ;
#endif

	                    switch (*aop) {

	                    case 'V':

#if	F_DEBUGS
	                        eprintf("main: version key-letter\n") ;
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
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            rs = cfdeci(avp,avl, &pip->verboselevel) ;

	                            if (rs < 0)
	                                goto badargval ;

	                        }

	                        break ;

	                    case '?':
	                        if (! f_unknown)
	                            ex = EX_INFO ;

	                        f_usage = TRUE ;
	                        break ;

	                    default:
				ex = EX_USAGE ;
	                        f_usage = TRUE ;
	                        f_unknown = TRUE ;
	                        bprintf(efp,"%s: unknown option - %c\n",
	                            pip->progname,*aop) ;

	                    } /* end switch */

	                    aop += 1 ;

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

#if	F_DEBUGS
	eprintf("main: finished parsing command line arguments\n") ;
#endif

	if (pip->debuglevel > 1) {

	    bprintf(pip->efp,
	        "%s: debuglevel=%d\n",
	        pip->progname,pip->debuglevel) ;

	    bcontrol(pip->efp,BC_LINEBUF,0) ;

	    bflush(pip->efp) ;

	}

	if (f_version) {

	    bprintf(efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	}

	ex = (f_unknown) ? EX_USAGE : EX_OK ;
	if (f_usage || f_unknown)
	    goto usage ;

	ex = EX_INFO ;
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

	    if ((termdevice = getenv(TERMDEVICEVAR1)) == NULL)
	    	termdevice = getenv(TERMDEVICEVAR2) ;

	    if (termdevice == NULL) {

	        rs = getloginterm(0,termdevbuf1,MAXPATHLEN) ;

	        if (rs < 0)
	            goto badnoterm ;

	        termdevice = termdevbuf1 ;

	    } /* end if (getting control terminal) */

	} /* end if */

#if	F_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: termdevice=%s\n",termdevice) ;
#endif

	if (termdevice[0] != '/') {

/* is the device base directory there ? */

	    if (u_access(devbase,R_OK) != 0) {

	        bprintf(pip->efp,
	            "%s: could not access device directory \"%s\"\n",
	            pip->progname,devbase) ;

	        ex = EX_DATAERR ;
	        goto done ;
	    }

	    if ((u_stat(devbase,&sb) < 0) || (! S_ISDIR(sb.st_mode)))
	        goto baddir ;

	    mkpath2(termdevbuf2, devbase,termdevice) ;

	    termdevice = termdevbuf2 ;

	} /* end if */


/* OK, we do it */


	if (pip->verboselevel > 1) {

	    if ((rs = bopen(ofp,BIO_STDOUT,"dwct",0644)) < 0)
	        goto badoutopen ;

	}

	if (pip->verboselevel > 1)
	    bprintf(ofp,"terminal device %s\n",termdevice) ;


	{
	    int	f_on ;


/* get the current state */

	    if ((rs = u_stat(termdevice,&sb)) < 0)
	        goto badstat ;

	    f_on = sb.st_mode & S_IWGRP ;


	    if ((newstateval < 0) && (newstate != NULL))
		newstateval = (tolower(newstate[0]) == 'y') ? 1 : 0 ;


/* switch it if called for */

	    if (newstateval >= 0) {

	        int	f_change = FALSE ;


#if	F_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("main: newstateval=%d\n",newstateval) ;
#endif

	        if (newstateval == 1) {

	            if (! f_on) {

	                f_change = TRUE ;
	                sb.st_mode |= S_IWGRP ;
	                f_on = TRUE ;

	            }

	        } else {

	            if (f_on) {

	                f_change = TRUE ;
	                sb.st_mode &= (~ S_IWGRP) ;
	                f_on = FALSE ;

	            }

	        }


	        if (f_change)
	            rs = u_chmod(termdevice,sb.st_mode) ;

	        if (rs < 0)
	            goto badchmod ;

	    } /* end if (switching it) */


/* print current status */

	    if ((pip->verboselevel > 1) || (newstateval < 0))
	        bprintf(ofp,"is %s\n",
	            ((f_on) ? "y" : "n")) ;


	} /* end block */


	ex = EX_OK ;


retout:
	if (pip->verboselevel > 1)
	    bclose(ofp) ;


#if	F_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: exiting\n") ;
#endif


/* good return from program */
goodret:

#if	F_DEBUG
	if (pip->debuglevel > 1)
	    bprintf(pip->efp,"%s: program exiting\n",
	        pip->progname) ;
#endif


/* we are out of here */
done:
earlyret:
ret1:
	bclose(pip->efp) ;

ret0:
	return ex ;

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
	ex = EX_USAGE ;
	goto earlyret ;


/* error type returns */
badnoterm:
	ex = EX_DATAERR ;
	bprintf(pip->efp,
	    "%s: could not find controlling terminal (rs=%d)\n",
	    pip->progname,rs) ;

	goto baderr ;

baddir:
	ex = EX_DATAERR ;
	bprintf(efp,"%s: bad directory specified\n",
	    pip->progname) ;

	goto baderr ;

badoutopen:
	ex = EX_DATAERR ;
	bprintf(pip->efp,"%s: could not open output (rs=%d)\n",
	    pip->progname,rs) ;


/* come here for a bad return from the program */
baderr:
	ex = EX_DATAERR ;

#if	F_DEBUGS
	eprintf("main: exiting program BAD\n") ;
#endif

	goto done ;


/* bad stuff with STDOUT open */
badstat:
	ex = EX_DATAERR ;
	bprintf(efp,"%s: could not get status on terminal (%d)\n",
	    pip->progname,rs) ;

	goto retout ;

badchmod:
	ex = EX_DATAERR ;
	bprintf(efp,"%s: could not change mode on terminal (%d)\n",
	    pip->progname,rs) ;

	goto retout ;

}
/* end subroutine (main) */


