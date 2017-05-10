/* main */

/* program front-end subroutine */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 2002-06-18, David A­D­ Morano

	I grabbed this front-end from some other program.


*/

/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	Synopsis:

	$ bpsort <dbfile(s)> ...


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<pwd.h>
#include	<grp.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<exitcodes.h>
#include	<pwfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)


/* external subroutines */

extern int	matstr(char * const *,char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	process(struct proginfo *,vecitem *,char *) ;
extern int	dbdump(struct proginfo *,vecitem *) ;
extern int	isdigitlatin(int) ;

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


/* exported subroutines */


int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	struct proginfo	pi, *pip = &pi ;
	struct entry	e ;
	bfile		errfile, *efp = &errfile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		nisfile, *nfp = &nisfile ;

	vecitem	db ;

	time_t	daytime = 0 ;

	int	argr, argl, aol, akl, avl ;
	int	argvalue = -1 ;
	int	maxai, pan, npa, kwi, i, j, k ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	rs, len ;
	int	sl, ci ;
	int	ex = EX_INFO ;
	int	fd_debug ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_self = FALSE ;
	int	f_entok = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 1], *bp ;
	const char	*cp, *cp2 ;
	const char	*ofname = NULL ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getenv(VARDEBUGFD1)) == NULL)
	    cp = getenv(VARDEBUGFD2) ;
	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;
#endif

	memset(pip,0,sizeof(struct proginfo)) ;

	pip->version = VERSION ;
	pip->progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"dwca",0666) >= 0) {
	    pip->efp = &errfile ;
	    pip->f.errfile = TRUE ;
	    bcontrol(efp,BC_LINEBUF,0) ;
	}

/* initialize */

	pip->f.quiet = FALSE ;

	pip->debuglevel = 0 ;
	pip->verboselevel = 1 ;

	pip->programroot = NULL ;

/* start parsing the arguments */

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
		const int	ach = MKCHAR(argp[1]) ;

	        if (argl > 1) {

	            if (isdigitlatin(ach)) {

	                if (((argl - 1) > 0) && 
	                    (cfdeci(argp + 1,argl - 1,&argvalue) < 0))
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
	                debugprintf("main: about to check for a key word match\n") ;
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

#if	CF_DEBUGS
	                    debugprintf("main: got an option key letter\n") ;
#endif

	                    while (akl--) {

#if	CF_DEBUGS
	                        debugprintf("main: option key letters\n") ;
#endif

	                        switch ((int) *akp) {

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

/* version */
	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

/* output file name */
	                        case 'o':
	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
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

	                        case '?':
	                            f_usage = TRUE ;
	                            break ;

	                        default:
	                            ex = EX_USAGE ;
	                            f_usage = TRUE ;
	                            bprintf(efp,"%s: unknown option - %c\n",
	                                pip->progname,*aop) ;

	                        } /* end switch */

	                        akp += 1 ;

	                    } /* end while */

	                } /* end if (individual option key letters) */

	            } /* end if (digits as argument or not) */

	        } else {

/* we have a plus or minux sign character alone on the command line */

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
			ex = EX_USAGE ;
	                bprintf(efp,"%s: extra arguments specified\n",
	                    pip->progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: finished parsing arguments\n") ;
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


	vecitem_start(&db,100,VECITEM_PSORTED) ;


/* loop through the arguments processing them */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: checking for positional arguments\n") ;
#endif

	pan = 1 ;
	for (i = 0 ; i <= maxai ; i += 1) {

	    if (BATST(argpresent,i)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: positional argument i=%d pan=%d arg=%s\n",
	                i,pan,argv[i]) ;
#endif

		rs = process(pip,&db,argv[i]) ;


	    } /* end if (got a positional argument) */

	} /* end for (handling positional arguments) */


/* process the entire DB and dump everything out as we find it */

	rs = dbdump(pip,&db) ;


	vecitem_finish(&db) ;

	bclose(ofp) ;



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
	    "%s: USAGE> %s dbfile(s) ...\n",
	    pip->progname) ;

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



