/* main (TESTCPUSPEED) */


#define	CF_DEBUGS	0
#define	CF_DEBUG	0
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_CPUSPEED	0
#define	CF_DLOPEN	1
#define	CF_PAUSE	0


/* revision history:

	- 1998-12-01, David A­D­ Morano

	This module was originally written for hardware CAD support.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine is thr front-end for a test program of the module
	CPUSPED.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<dlfcn.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifdef	PROGRAMEOOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	MODNAME		"cpuspeed"
#define	NRUNS		10000000


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cpuspeed(const char *,const char *,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* local variables */

static const char *argopts[] = {
	    "VERSION",
	    "VERBOSE",
	    "HELP",
	    "of",
	    NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_of,
	argopt_overlast
} ;

static const char	*names[] = {
	"dhry",
	"dhry.so",
	"dhry.o",
	NULL
} ;


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint	mo_start = 0 ;
#endif

	int	argr, argl, aol, akl, avl ;
	int	maxai, pan, npa, kwi, ai ;
	int	rs, i, j, k, len ;
	int	sl, ci ;
	int	ex = EX_INFO ;
	int	nruns = NRUNS ;
	int	speed ;
	int	dlmode ;
	int	(*fp)(int) ;
	int	fd_debug ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_self = FALSE ;
	int	f_entok = FALSE ;
	int	f_name = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	char	argpresent[MAXARGGROUPS] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	const char	*progname ;
	const char	*ofname = NULL ;
	const char	*pr = NULL ;
	const char	*cp ;

	void	*dhp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getenv(VARDEBUGFNAME)) == NULL) {
	    if ((cp = getenv(VARDEBUGFD1)) == NULL)
	        cp = getenv(VARDEBUGFD2) ;
	}
	if (cp != NULL)
	    debugopen(cp) ;
	debugprintf("main: starting\n") ;
#endif /* CF_DEBUGS */

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif
1
	memset(pip,0,sizeof(struct proginfo)) ;
	pip->verboselevel = 1 ;

	sfbasename(argv[0],-1,&progname) ;

#if	CF_DEBUGS
	debugprintf("main: progname=%s\n",progname) ;
#endif


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

	        if (argl > 1) {

	            if (isdigit(argp[1])) {

	                    argval = (argp + 1) ;

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

/* do we have a keyword match or should we assume only key letters? */

	                if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

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
	                            if (avl) {

	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

	                                if (rs < 0)
	                                    goto badargval ;

	                            }
	                        }

	                        break ;

	                    case argopt_help:
	                        f_help = TRUE ;
	                        break ;

/* output file name */
	                    case argopt_of:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                ofname = avp ;

	                        } else {

	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                ofname = argp ;

	                        }

	                        break ;

/* handle all keyword defaults */
	                    default:
	                        ex = EX_USAGE ;
	                        f_usage = TRUE ;
	                        break ;

	                    } /* end switch */

	                } else {

	                    while (akl--) {
			        const int	kc = MKCHAR(*akp) ;

	                        switch (kc) {
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
	                            break ;

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

#if	CF_DEBUGS
	            debugprintf("main: pos arg=>%s<\n",argv[i]) ;
#endif

	            BASET(argpresent,i) ;
	            maxai = i ;
	            npa += 1 ;

	        } else {

	            if (! f_extra) {

	                f_extra = TRUE ;
	                ex = EX_USAGE ;
	                bprintf(pip->efp,"%s: extra arguments specified\n",
	                    pip->progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */


/* program root */

	pr = getenv(VARPROGRAMROOT1) ;

	if (pr == NULL)
	    pr = getenv(VARPROGRAMROOT2) ;

	if (pr == NULL)
	    pr = PROGRAMROOT ;


/* check some arguments */

	pan = 0 ;
	for (ai = 1 ; ai <= maxai ; ai += 1) {

	    if (BATST(argpresent,ai)) {

	        rs = SR_OK ;
	        switch (pan) {

	        case 0:
	            rs = cfdeci(argv[ai],-1,&nruns) ;

			if (rs < 0)
				nruns = NRUNS ;

	            break ;

	        } /* end switch */

	        if (rs < 0)
	            goto badargval ;

	        pan += 1 ;

	    } /* end if */

	} /* end for */


#if	CF_DEBUGS
	debugprintf("main: pr=%s\n",pr) ;
#endif

/* do it statically */

#if	CF_CPUSPEED

#if	CF_DEBUGS
	debugprintf("main: static cpuspeed()\n") ;
#endif

	speed = cpuspeed(pr,NULL,nruns) ;

#if	CF_DEBUGS
	debugprintf("main: static cpuspeed() speed=%d\n",speed) ;
#endif

	if (speed >= 0)
	    fprintf(stdout,"speed=%d\n",speed) ;

#endif /* CF_CPUSPEED */


#if	CF_DLOPEN

/* do it dynamically */

#if	CF_DEBUGS
	debugprintf("main: dynamically pr=%s\n",pr) ;
#endif

	for (i = 0 ; names[i] != NULL ; i += 1) {

	    mkpath3(tmpfname,pr,"lib/cpuspeed/S5",names[i]) ;

	    rs = u_access(tmpfname,(R_OK | X_OK)) ;

	    if (rs >= 0)
	        break ;

	} /* end for */

	if (rs < 0)
	    goto bad0 ;

#if	CF_DEBUGS
	debugprintf("main: tmpfname=%s\n",tmpfname) ;
#endif

	dlmode = RTLD_LAZY | RTLD_LOCAL ;
	dhp = dlopen(tmpfname,dlmode) ;

#if	CF_DEBUGS
	debugprintf("main: dlopen() returned=%p\n",dhp) ;
#endif

	if (dhp == NULL) {

#if	CF_DEBUGS
	    debugprintf("main: dlopen() NULL\n") ;
#endif

	    rs = SR_NOEXIST ;
	    goto bad1 ;
	}

	fp = (int (*)()) dlsym(dhp,MODNAME) ;

	if (fp == NULL) {

	    rs = SR_NOTFOUND ;
	    goto bad2 ;
	}

#if	CF_PAUSE
	sleep(60) ;
#endif

	speed = (*fp)(nruns) ;

#if	CF_DEBUGS
	debugprintf("main: dynamic cpuspeed() speed=%d\n",speed) ;
#endif

	dlclose(dhp) ;

	if (speed >= 0)
	    fprintf(stdout,"speed=%d\n",speed) ;

#endif /* CF_DLOPEN */


	ex = (speed >= 0) ? EX_OK : EX_DATAERR ;

ret2:
	fclose(stdout) ;

ret1:
retearly:

#if	CF_DEBUGS
	debugprintf("main: ret ex=%d\n",ex) ;
#endif

	fclose(stderr) ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("main: final mallout=%u\n",(mo-mo_start)) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* the bad things */
badargnum:
	fprintf(stderr,"%s: not enough arguments specified\n",
	    pip->progname) ;

	goto badarg ;

badargextra:
	fprintf(stderr,"%s: no value associated with this option key\n",
	    pip->progname) ;

	goto badarg ;

badargval:
	fprintf(stderr,"%s: bad argument value was specified\n",
	    pip->progname) ;

	goto badarg ;

badarg:
	ex = EX_USAGE ;
	fprintf(stderr,"%s: bad argument(s) given\n",
	    pip->progname) ;

	goto retearly ;

/* bad things */
bad0:
	ex = EX_UNAVAILABLE ;
	fprintf(stderr,"%s: could not find accessible CPUSPEED (%d)\n",
	    progname,rs) ;

	goto retearly ;

bad1:
	ex = EX_UNAVAILABLE ;
	fprintf(stderr,"%s: could not load CPUSPEED (%d)\n",
	    progname,rs) ;

	goto retearly ;

bad2:
	dlclose(dhp) ;

	ex = EX_SOFTWARE ;
	fprintf(stderr,"%s: internal protocol error (%d)\n",
	    progname,rs) ;

	goto ret2 ;

}
/* end subroutine (main) */



