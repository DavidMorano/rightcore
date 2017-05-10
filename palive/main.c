/* main */

/* program to check for existence of a process */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_GETEXECNAME	1		/* use 'getexecname(3c)' */


/* revision history:

	= 1999-06-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	Synopsis:

	$ palive [-x execfname] [-u username] {{-f pidfile} | pid}


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<pwd.h>
#include	<grp.h>
#include	<netdb.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<vecstr.h>
#include	<vechand.h>
#include	<exitcodes.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;

extern int	printhelp(bfile *,const char *,const char *,const char *) ;

extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */

static const char *argopts[] = {
	"VERSION",
	"VERBOSE",
	"HELP",
	"ROOT",
	"af",
	"of",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_root,
	argopt_af,
	argopt_of,
	argopt_overlast
} ;


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;
	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		nisfile, *nfp = &nisfile ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	argvalue = -1 ;
	int	maxai, pan, npa ;
	int	rs ;
	int	i, j, k ;
	int	len ;
	int	sl, cl, ci ;
	int	size, policy, count ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 1], *bp ;
	char	nodename[NODENAMELEN + 1] ;
	char	domainname[MAXHOSTNAMELEN + 1] ;
	char	usernamebuf[LOGNAME_MAX + 1] ;
	const char	*pr = NULL ;
	const char	*afname = NULL ;
	const char	*ofname = NULL ;
	const char	*sp, *cp, *cp2 ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	proginfo_start(pip,envv,argv[0],VERSION) ;

	proginfo_setbanner(pip,BANNER) ;

	if ((cp = getenv(VARERRORFNAME)) != NULL) {
	    rs = bopen(&errfile,cp,"wca",0666) ;
	} else
	    rs = bopen(&errfile,BFILE_STDERR,"dwca",0666) ;
	if (rs >= 0) {
	    pip->efp = &errfile ;
	    pip->f.errfile = TRUE ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	}

/* initialize */

	pip->verboselevel = 1 ;

/* start parsing the arguments */

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

	            if (isdigit(argp[1])) {

	                    rs = cfdeci((argp + 1),(argl - 1),&argvalue) ;

	        } else if (argp[1] == '-') {

	            ai_pos = ai ;
	            break ;

		} else {

	            aop = argp + 1 ;
	            akp = aop ;
	            aol = argl - 1 ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {
	                f_optequal = TRUE ;
	                akl = avp - aop ;
	                avp += 1 ;
	                avl = aop + argl - 1 - avp ;
	                aol = akl ;
	            } else {
			avp = NULL ;
	                avl = 0 ;
	                akl = aol ;
	            }

/* do we have a keyword match or should we assume only key letters? */

	                if ((kwi = matostr(argopts,1,akp,akl)) >= 0) {

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
	                            if (avl)
	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

	                        }

	                        break ;

/* program root */
	                    case argopt_root:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                pr = avp ;

	                        } else {

	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                pr = argp ;

	                        }

	                        break ;

	                    case argopt_help:
	                        f_help = TRUE ;
	                        break ;

	                case argopt_af:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            afname = avp ;

	                    } else {

	                        if (argr <= 0)
	                            goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            afname = argp ;

	                    }

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
	                        rs = SR_INVALID ;
	                        f_usage = TRUE ;
	                        bprintf(pip->efp,
	                        "%s: invalid key=%t\n",
	                        pip->progname,akp,akl) ;

	                    } /* end switch */

	                } else {

	                    while (akl--) {

	                        switch ((int) *akp) {

/* debug */
	                        case 'D':
	                            pip->debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    rs = cfdeci(avp,avl,
	                                        &pip->debuglevel) ;

	                            }

	                            break ;

/* quiet mode */
	                        case 'Q':
	                            pip->f.quiet = TRUE ;
	                            break ;

/* version */
	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

				case 'q':
	                            pip->verboselevel = 0 ;
	                            break ;

/* verbose mode */
	                        case 'v':
	                            pip->verboselevel = 2 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    rs = cfdeci(avp,avl,
	                                        &pip->verboselevel) ;

	                            }

	                            break ;

	                        case '?':
	                            f_usage = TRUE ;
	                            break ;

	                        default:
	                            rs = SR_INVALID ;
	                            f_usage = TRUE ;
	                            bprintf(pip->efp,
	                            "%s: invalid option=%c\n",
	                            pip->progname,*akp) ;

	                        } /* end switch */

	                        akp += 1 ;
				if (rs < 0)
				    break ;

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


#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage)
	    goto usage ;

	if (f_version)
	    goto retearly ;

/* get the program root */

	if (pr == NULL) {

	    pr = getenv(VARPROGRAMROOT1) ;

	    if (pr == NULL)
	        pr = getenv(VARPROGRAMROOT2) ;

	    if (pr == NULL)
	        pr = getenv(VARPROGRAMROOT3) ;

/* try to see if a path was given at invocation */

	    if ((pr == NULL) && (pip->progdname != NULL))
	        proginfo_rootprogdname(pip) ;

/* do the special thing */

#if	CF_GETEXECNAME && defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)
	    if ((pr == NULL) && (pip->pr == NULL)) {

	        const char	*pp ;


	        pp = getexecname() ;

	        if (pp != NULL)
	            proginfo_execname(pip,pp) ;

	    }
#endif /* SOLARIS */

	} /* end if (getting a program root) */

	if (pip->pr == NULL) {

	    if (pr == NULL)
	        pr = PROGRAMROOT ;

	    proginfo_setprogroot(pip,pr,-1) ;

	}

/* program search name */

	proginfo_setsearchname(pip,VARSEARCHNAME,SEARCHNAME) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: pr=%s\n",pip->pr) ;
#endif

	if (pip->debuglevel > 0) {

	    bprintf(pip->efp,"%s: pr=%s\n",
	        pip->progname,pip->pr) ;

	    bprintf(pip->efp,"%s: sn=%s\n",
	        pip->progname,pip->searchname) ;

	}

/* help file */

	if (f_help)
	    goto help ;


	ex = EX_OK ;

/* some initialization */

	if (pip->tmpdname == NULL)
	    pip->tmpdname = getenv(VARTMPDNAME) ;

	if (pip->tmpdname == NULL)
	    pip->tmpdname = TMPDNAME ;

/* open the output file */

	if (ofname != NULL)
	    rs = bopen(ofp,ofname,"wct",0666) ;

	else
	    rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

	if (rs < 0)
	    goto badoutopen ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: done w/ opening output \n") ;
#endif







/* we are done */
done:
ret2:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

	bclose(ofp) ;


/* early return thing */
retearly:
ret1:
	bclose(pip->efp) ;

ret0:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* the information type thing */
usage:
	bprintf(pip->efp,
	    "%s: USAGE> %s <pid>\n",
	    pip->progname,pip->progname) ;

	goto retearly ;

/* print out some help */
help:
	ex = EX_INFO ;
	printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	goto retearly ;

/* the bad things */
badargnum:
	bprintf(pip->efp,"%s: not enough arguments specified\n",
	    pip->progname) ;

	goto badarg ;

badargextra:
	bprintf(pip->efp,"%s: no value associated with this option key\n",
	    pip->progname) ;

	goto badarg ;

badargval:
	bprintf(pip->efp,"%s: bad argument value was specified\n",
	    pip->progname) ;

	goto badarg ;

badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: bad argument(s) given\n",
	    pip->progname) ;

	goto retearly ;

badoutopen:
	ex = EX_CANTCREAT ;
	bprintf(pip->efp,"%s: could not open the output file (%d)\n",
	    pip->progname,rs) ;

	goto badret ;

/* error types of returns */
badret:
	bclose(pip->efp) ;

	goto retearly ;
}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



static int sortheadcmp(e1p,e2p)
struct listentry	**e1p, **e2p ;
{
	int	rc ;


	if ((*e1p == NULL) && (*e2p == NULL))
	    return 0 ;

	if (*e1p == NULL)
	    return 1 ;

	if (*e2p == NULL)
	    return -1 ;

	rc = strcasecmp((*e1p)->name,(*e2p)->name) ;

#if	CF_DEBUGS
	debugprintf("msg/sortheadcmp: rc=%d n1=%s n2=%s\n",
		rc,(*e1p)->name,(*e2p)->name) ;
#endif

	return rc ;
}
/* end subroutine (sortheadcmp) */



