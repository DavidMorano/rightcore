/* main */

/* generic */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0
#define	CF_DEBUG	0


/* revision history:

	= 1989-03-01, David A­D­ Morano

	This subroutine was originally written.  This whole program,
	LOGDIR, is needed for use on the Sun CAD machines because Sun
	doesn't support LOGDIR or LOGNAME at this time.  There was a
	previous program but it is lost and not as good as this one
	anyway.  This one handles NIS+ also.  (The previous one
	didn't.) 


	= 1998-06-01, David A­D­ Morano

	I enhanced the program a little to print out some other user
	information besides the user's name and login home directory.


	= 1999-03-01, David A­D­ Morano

	I enhanced the program to also print out effective UID and
	effective GID.


	= 2002-04-16, David A­D­ Morano

	This little front-end was borrowed to write this little
	dittie program! :-)



*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	This little dittie shifts a group of arguments (after the
	double '--', to the left.

	Execute as :

	$ shiftargs [-n] -- args



*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<netdb.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<pwfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	PWENTRY_BUFLEN
#define	PWENTRY_BUFLEN		256
#endif


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	sfshrink(const char *,int,char **) ;

extern char	*strbasename(char *) ;
extern char	*strshrink(char *) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* forward references */


/* local structures */

/* define command option words */

static char *const argopts[] = {
	"VERSION",
	"VERBOSE",
	NULL
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
	struct proginfo	pi, *pip = &pi ;

	bfile		errfile, *efp = &errfile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		argfile, *afp = &argfile ;

	time_t	daytime = 0 ;

	int	argr, argl, aol, akl, avl ;
	int	argvalue = -1 ;
	int	maxai, pan, npa, kwi, i, j, k ;
	int	rs = SR_OK ;
	int	ai, len ;
	int	sl, ci ;
	int	ex = EX_INFO ;
	int	fd_debug ;
	int	nshifts = -1 ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_double = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 1], *bp ;
	const char	*ofname = NULL ;
	const char	*cp, *cp2 ;


	if ((cp = getenv(VARDEBUGFD1)) == NULL)
	    cp = getenv(VARDEBUGFD2) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


	memset(pip,0,sizeof(struct proginfo)) ;

	pip->version = VERSION ;
	pip->progname = strbasename(argv[0]) ;

	if (bopen(&errfile,BFILE_STDERR,"dwca",0666) >= 0) {
	    pip->efp = &errfile ;
	    pip->f.errfile = TRUE ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	}

/* initialize */

	pip->debuglevel = 0 ;
	pip->verboselevel = 1 ;
	pip->f.quiet = FALSE ;
	pip->pr = NULL ;

/* start parsing the arguments */

	for (i = 0 ; i < MAXARGGROUPS ; i += 1) argpresent[i] = 0 ;

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
	ai = 0 ;
	argr = argc - 1 ;
	while ((! f_double) && (argr > 0)) {

#if	CF_DEBUGS
	                debugprintf("main: incrementing to next\n") ;
#endif

	    argp = argv[++ai] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

#if	CF_DEBUGS
	                debugprintf("main: arlg=%d argp=>%s<\n",argl,argp) ;
#endif

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 0) && (f_optminus || f_optplus)) {

	        if (argl > 1) {

	            if (isdigit(argp[1])) {

	                if (((argl - 1) > 0) && 
	                    (cfdeci(argp + 1,argl - 1,&argvalue) < 0))
	                    goto badarg ;

	            } else {

#if	CF_DEBUGS
	                debugprintf("main: got an option\n") ;
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
	                    avp += 1 ;
	                    avl = aop + aol - avp ;
	                    f_optequal = TRUE ;

#if	CF_DEBUGS
	                    debugprintf("main: aol=%d avp=\"%s\" avl=%d\n",
	                        aol,avp,avl) ;

	                    debugprintf("main: akl=%d akp=\"%s\"\n",
	                        akl,akp) ;
#endif

	                } else {

	                    akl = aol ;
	                    avl = 0 ;

	                }

/* do we have a keyword match or should we assume only key letters ? */

#if	CF_DEBUGS
	                debugprintf("main: ? keyword match on >%W<\n",akp,akl) ;
#endif

	                if ((kwi = matstr(argopts,akp,akl)) >= 0) {

#if	CF_DEBUGS
	                    debugprintf("main: option keyword=%W kwi=%d\n",
	                        akp,akl,kwi) ;
#endif

	                    switch (kwi) {

/* version */
	                    case argopt_version:
	                        f_version = TRUE ;
	                        if (f_optequal)
	                            goto badargextra ;

	                        break ;

/* verbose mode */
	                    case argopt_verbose:
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

/* handle all keyword defaults */
	                    default:
	                        ex = EX_USAGE ;
	                        f_usage = TRUE ;
	                        bprintf(efp,"%s: option (%s) not supported\n",
	                            pip->progname,akp) ;

	                        goto badarg ;

	                    } /* end switch */

	                } else {

	                    while (akl--) {

	                        switch ((int) *akp) {

/* debug */
	                        case 'D':
	                            pip->debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl) {

	                                    rs = cfdeci(avp,avl,
						&pip->debuglevel) ;

	                                    if (rs < 0)
	                                        goto badargval ;

	                                }
	                            }

	                            break ;

/* version */
	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

/* number of shifts */
	                        case 'n':
	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) {

	                                rs = cfdeci(argp,argl,&nshifts) ;

#if	CF_DEBUGS
	                        debugprintf("main: rs=%d nshifts=%d\n",
					rs,nshifts) ;
#endif

					if (rs < 0)
						goto badargval ;

					}

	                            break ;

/* output file name */
	                        case 'o':
	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                ofname = argp ;

	                            break ;

/* quiet mode */
	                        case 'q':
	                            pip->f.quiet = TRUE ;
	                            break ;

/* verbose mode */
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

				case '-':

#if	CF_DEBUGS
	                        debugprintf("main: double dash\n") ;
#endif

					f_double = TRUE ;
					break ;

	                        case '?':
	                            f_usage = TRUE ;
	                            break ;

	                        default:
	                            ex = EX_USAGE ;
	                            f_usage = TRUE ;
	                            bprintf(efp,"%s: unknown option - %c\n",
	                                pip->progname,*aop) ;

	                        } /* end switch */

#if	CF_DEBUGS
	                        debugprintf("main: end switch (key letter)\n") ;
#endif

	                        akp += 1 ;

	                    } /* end while */

#if	CF_DEBUGS
	                        debugprintf("main: end while (key letters)\n") ;
#endif

	                } /* end if (individual option key letters) */

	            } /* end if (digits as argument or not) */

	        } else {

/* we have a plus or minux sign character alone on the command line */

	            if (ai < MAXARGINDEX) {

	                BASET(argpresent,ai) ;
	                maxai = ai ;
	                npa += 1 ;	/* increment position count */

	            }

	        } /* end if */

	    } else {

	        if (i < MAXARGINDEX) {

	            BASET(argpresent,ai) ;
	            maxai = ai ;
	            npa += 1 ;

	        } else {

	            if (! f_extra) {

	                f_extra = TRUE ;
			ex= EX_USAGE ;
	                bprintf(efp,"%s: extra arguments specified\n",
	                    pip->progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

#if	CF_DEBUGS
	                        debugprintf("main: bottom while (outer)\n") ;
#endif

	} /* end while (all command line argument processing) */

#if	CF_DEBUGS
	                        debugprintf("main: end while (outer)\n") ;
	                        debugprintf("main: debuglevel=%u\n",
					pip->debuglevel) ;
#endif


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: finished parsing arguments ai=%d\n",ai) ;
#endif

	if (f_version)
	    bprintf(efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage)
	    goto usage ;

	if (f_version)
	    goto earlyret ;

	if (pip->debuglevel > 0)
	    bprintf(efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: special debugging turned on\n") ;
#endif


/* check some arguments */

	if (nshifts < 0) {

		nshifts = (argvalue >= 0) ? argvalue : 1 ;

	}

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: nshifts=%d\n",nshifts) ;
#endif


/* open the output file */

	if (ofname != NULL)
	    rs = bopen(ofp,ofname,"wct",0666) ;

	else
	    rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

	if (rs < 0)
	    goto badoutopen ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: done w/ opening output \n") ;
#endif


	ai += 1 ;
	for (i = 0 ; argv[ai] != NULL ; (ai += 1),(i += 1)) {

		if (i >= nshifts) {

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: argv[%d]=%s\n",ai,argv[ai]) ;
#endif

			bprintf(ofp," %s",argv[ai]) ;

		}

	} /* end for */

	bputc(ofp,'\n') ;



/* we are done */
done:
ret2:

#if	CF_DEBUG
	if (pip->debuglevel > 0)
	    bprintf(efp,"%s: program finishing\n",
	        pip->progname) ;
#endif

	bclose(ofp) ;

/* early return thing */
earlyret:
ret1:
	bclose(efp) ;

ret0:
	return ex ;

/* the information type thing */
usage:
	bprintf(efp,
	    "%s: USAGE> %s [-V] [-<n>] [-n <n>] -- [arg(s) ...]\n",
	    pip->progname,pip->progname) ;

	goto earlyret ;

/* the bad things */
badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",
	    pip->progname) ;

	goto badarg ;

badargextra:
	bprintf(efp,"%s: no value associated with this option key\n",
	    pip->progname) ;

	goto badarg ;

badargval:
	bprintf(efp,"%s: bad argument value was specified\n",
	    pip->progname) ;

	goto badarg ;

badarg:
	ex = EX_USAGE ;
	bprintf(efp,"%s: bad argument(s) given\n",
	    pip->progname) ;

	goto earlyret ;

badoutopen:
	ex = EX_CANTCREAT ;
	bprintf(efp,"%s: could not open the output file (rs %d)\n",
	    pip->progname,rs) ;

	goto badret ;

/* error types of returns */
badret:
	bclose(efp) ;

	goto earlyret ;
}
/* end subroutine (main) */



