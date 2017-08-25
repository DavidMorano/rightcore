/* main */

/* UTMP program */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_LOCALWTMP	0		/* use local WTMP */
#define	CF_LOCSETENT	0		/* |locinfo_setentry()| */


/* revision history:

	= 2000-12-13, David A­D­ Morano
        This program was originally written (as a test).

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ utmp [-y|-n] [-h <hostname>]


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	<utmpx.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<bits.h>
#include	<tmpx.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	WTMPFNAME
#if	CF_LOCALWTMP
#define	WTMPFNAME	"wtmpx"
#else
#define	WTMPFNAME	"/var/adm/wtmpx"
#endif
#endif

#ifndef	UTMPFNAME
#define	UTMPFNAME	"/var/adm/utmpx"
#endif

#ifndef	BUFLEN
#define	BUFLEN		1024
#endif

#define	TERMBUFLEN	(TMPX_LLINE + 5)

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	mkutmpid(char *,int,const char *,int) ;
extern int	termdevice(char *,int,int) ;
extern int	getusername(char *,int,uid_t) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(bfile *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		searchline:1 ;
	uint		yes:1 ;
	uint		no:1 ;
	uint		zap:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	vecstr		stores ;
	PROGINFO	*pip ;
	cchar		*searchline ;
} ;


/* forward references */

static int	usage(PROGINFO *) ;

static int	process(PROGINFO *,bfile *,pid_t) ;
static int	zaputx(struct utmpx *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;

#if	CF_LOCSETENT
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
#endif


/* local variables */

static cchar	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRLOCAL
} ;

static const struct mapex	mapexs[] = {
	{ SR_NOENT, EX_NOUSER },
	{ SR_AGAIN, EX_TEMPFAIL },
	{ SR_DEADLK, EX_TEMPFAIL },
	{ SR_NOLCK, EX_TEMPFAIL },
	{ SR_TXTBSY, EX_TEMPFAIL },
	{ SR_ACCESS, EX_NOPERM },
	{ SR_REMOTE, EX_PROTOCOL },
	{ SR_NOSPC, EX_TEMPFAIL },
	{ 0, 0 }
} ;

#if	CF_DEBUG || CF_DEBUGS
static const char	*utmptypes[] = {
	"empty",
	"runlevel",
	"boottime",
	"oldtime",
	"newtime",
	"initproc",
	"loginproc",
	"userproc",
	"deadproc",
	"account",
	"signature",
	NULL
} ;
#endif /* CF_DEBUG */


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	BITS		pargs ;
	bfile		errfile ;
	pid_t		pid = 0 ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		pan = 0 ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;
	int		f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

/* initialize */

	pip->verboselevel = 1 ;

	pip->lip = &li ;
	if (rs >= 0) rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* start parsing the arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	ai_max = 0 ;
	ai_pos = 0 ;
	argr = argc ;
	for (ai = 0 ; (ai < argc) && (argv[ai] != NULL) ; ai += 1) {
	    if (rs < 0) break ;
	    argr -= 1 ;
	    if (ai == 0) continue ;

	    argp = argv[ai] ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 0) && (f_optminus || f_optplus)) {
	        const int	ach = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ach)) {

	            argval = (argp + 1) ;

	        } else if (ach == '-') {

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

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
	                    break ;

/* verbose mode */
	                case argopt_verbose:
	                    pip->verboselevel = 2 ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optvalue(avp,avl) ;
	                            pip->verboselevel = rs ;
	                        }
	                    }
	                    break ;

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* program search-name */
	                case argopt_sn:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sn = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                sn = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* argument-list file */
	                case argopt_af:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            afname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                afname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* error file name */
	                case argopt_ef:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            efname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                efname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* output file name */
	                case argopt_of:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            ofname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                ofname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* handle all keyword defaults */
	                default:
	                    rs = SR_INVALID ;
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
	                            if (avl) {
	                                rs = optvalue(avp,avl) ;
	                                pip->debuglevel = rs ;
	                            }
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

	                    case 'h':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->hostname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'y':
	                        lip->f.yes = TRUE ;
	                        break ;

	                    case 'n':
	                        lip->f.no = TRUE ;
	                        break ;

	                    case 'p':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = optvalue(argp,argl) ;
	                                pid = rs ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* verbose mode */
	                    case 'v':
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optvalue(avp,avl) ;
	                                pip->verboselevel = rs ;
	                            }
	                        }
	                        break ;

	                    case 'z':
	                        lip->f.zap = TRUE ;
	                        break ;

	                    case '?':
	                        f_usage = TRUE ;
	                        break ;

	                    default:
	                        rs = SR_INVALID ;
	                        break ;

	                    } /* end switch */
	                    akp += 1 ;

	                    if (rs < 0) break ;
	                } /* end while */

	            } /* end if (individual option key letters) */

	        } /* end if (digits as argument or not) */

	    } else {

	        rs = bits_set(&pargs,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (efname == NULL) efname = getenv(VAREFNAME) ;
	if (efname == NULL) efname = BFILE_STDERR ;
	if ((rs1 = bopen(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    bcontrol(&errfile,BC_SETBUFLINE,TRUE) ;
	} else if (! isNotPresent(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;
	}

/* get the program root */

	if (rs >= 0) {
	    if ((rs = proginfo_setpiv(pip,pr,&initvars)) >= 0) {
	        rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;
	    }
	}

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main: pr=%s\n",pip->pr) ;
	    debugprintf("main: sn=%s\n",pip->searchname) ;
	}
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	} /* end if */

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* some initialization */

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (afname == NULL) afname = getenv(VARAFNAME) ;

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* loop through the arguments processing them */

	for (ai = 1 ; ai < argc ; ai += 1) {
	    f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (f) {
	        cp = argv[ai] ;
	        if (cp[0] != '\0') {
	            switch (pan) {
	            case 0:
	                lip->searchline = cp ;
	                break ;
	            } /* end switch */
	            pan += 1 ;
	        } /* end if (non-nul) */
	    }
	} /* end for */

/* open the output file */

	if ((ofname == NULL) || (ofname[0] == '\0') || (ofname[0] == '-'))
	    ofname = BFILE_STDOUT ;

	if (rs >= 0) {
	    bfile	ofile, *ofp = &ofile ;
	    if ((rs = bopen(ofp,ofname,"wct",0666)) >= 0) {
	        rs = process(pip,ofp,pid) ;
	        bclose(ofp) ;
	    } else {
	        ex = EX_CANTCREAT ;
	        bprintf(pip->efp,"%s: inaccesssible output (%d)\n",
	            pip->progname,rs) ;
	    }
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	}

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        break ;
	    case SR_NOENT:
	        ex = EX_CANTCREAT ;
	        break ;
	    case SR_AGAIN:
	        ex = EX_TEMPFAIL ;
	        break ;
	    default:
	        ex = mapex(mapexs,rs) ;
	        break ;
	    } /* end switch */
	} /* end if */

/* we are done */
retearly:
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u (%d)\n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	bits_finish(&pargs) ;

badpargs:
	locinfo_finish(lip) ;

badlocstart:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("main: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* the bad things */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: bad argument(s) given (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (main) */


/* local subrouines */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [-y|-n] [-h <hostname>] <searchline> [-z]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=n]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int process(PROGINFO *pip,bfile *ofp,pid_t pid)
{
	struct utmpx	uc ;
	struct utmpx	*up ;
	LOCINFO		*lip = pip->lip ;
	const pid_t	sid = u_getsid(pid) ;
	int		rs = SR_OK ;
	int		si ;
	int		cl ;
	char		lognamebuf[TMPX_LUSER + 1] ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;


	lognamebuf[0] = '\0' ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main: ent sid=%d\n",sid) ;
#endif

	if (pip->debuglevel > 0) {
	    fmt = "%s: sid=%d\n" ;
	    bprintf(pip->efp,fmt,pn,sid) ;
	}

/* loop */

	setutxent() ;

	si = 0 ;
	while ((up = getutxent()) != NULL) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        int	i ;
#ifdef	COMMENT
	        debugprintf("main: si=%u up=%p\n",si,up) ;
#endif
	        for (i = 0 ; utmptypes[i] != NULL ; i += 1) {
	            if (i == up->ut_type)
	                break ;
	        } /* end for */
	        if (utmptypes[i] != NULL)
	            debugprintf("main: type=%s(%u)\n",utmptypes[i],i) ;
	        debugprintf("main: id=%t\n",
	            up->ut_id,strnlen(up->ut_id,TMPX_LID)) ;
	        debugprintf("main: line=%t\n",
	            up->ut_line,strnlen(up->ut_line,TMPX_LLINE)) ;
	        debugprintf("main: user=%t\n",
	            up->ut_line,strnlen(up->ut_user,TMPX_LUSER)) ;
	        debugprintf("main: sid=%d\n",
	            up->ut_pid) ;
	        debugprintf("main: host=%t\n",
	            up->ut_host,strnlen(up->ut_host,TMPX_LHOST)) ;
	    }
#endif /* CF_DEBUG */

	    if (lip->f.zap && (lip->searchline != NULL)) {
		cchar	*searchline = lip->searchline ;

	        if (strncmp(up->ut_line,searchline,TMPX_LLINE) == 0)
	            break ;

	    } else if ((up->ut_type == TMPX_TUSERPROC) &&
	        (up->ut_pid == sid)) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: SID match \n") ;
#endif

	        break ;

	    } /* end if */

	    si += 1 ;

	} /* end while (positioning within the UTMPX file) */

	if (up != NULL) {
	    strwcpy(lognamebuf,up->ut_user,TMPX_LUSER) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: loguser=%s f_zap=%u up=%p\n",
	        lognamebuf,lip->f.zap,up) ;
#endif /* CF_DEBUG */

	if (lip->f.zap && (lip->searchline != NULL)) {

	    uc = *up ;		/* copy the record found */

	    uc.ut_exit.e_termination = 0 ;
	    uc.ut_exit.e_exit = 0 ;
	    uc.ut_type = TMPX_TEMPTY ;

	    rs = zaputx(&uc) ;

	    if (rs < 0) {
		fmt = "%s: operation not allowed (%d)\n" ;
	        bprintf(pip->efp,fmt,pn,rs) ;
	    }

	} else if ((! lip->f.yes) && (! lip->f.no)) {
	    fmt = "id=%-4t line=%-12t user=%-12t sid=%-5u ses=%u host=%t\n" ;

	    if (up != NULL) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: found at si=%u ut_pid=%u\n",
	                si,up->ut_pid) ;
#endif

	        bprintf(ofp,fmt,
	            up->ut_id,strnlen(up->ut_id,TMPX_LID),
	            up->ut_line,strnlen(up->ut_line,TMPX_LLINE),
	            up->ut_user,strnlen(up->ut_user,TMPX_LUSER),
	            up->ut_pid,
	            up->ut_session,
	            up->ut_host,strnlen(up->ut_host,TMPX_LHOST)) ;

	    } else {
	        bprintf(ofp,"not logged in\n") ;
	    }

	} else if (lip->f.yes) {

	    if (up == NULL) {
		const int	termlen = TERMBUFLEN ;
	        int		tl ;
	        char		termbuf[TERMBUFLEN + 1] ;

	        if ((rs = termdevice(termbuf,termlen,FD_STDIN)) >= 0) {
	            char	idbuf[TMPX_LID + 1] ;
	            tl = rs ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: termdevice() rs=%d term=%t\n",
	                rs,
	                termbuf,((rs > 0) ? rs : 0)) ;
#endif


	            memset(&uc,0,sizeof(struct utmpx)) ;

	            mkutmpid(idbuf,TMPX_LID,termbuf,tl) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("main: idbuf=%t\n",
	                    idbuf,strnlen(idbuf,TMPX_LID)) ;
#endif

	            strncpy(uc.ut_id,idbuf,TMPX_LID) ;

	            strncpy(uc.ut_line,(termbuf + 5),TMPX_LLINE) ;

	            if (lognamebuf[0] == '\0') {
	                getusername(lognamebuf,USERNAMELEN,-1) ;
		    }

	            strncpy(uc.ut_user,lognamebuf,TMPX_LUSER) ;

	            if (pip->hostname != NULL) {

	                strncpy(uc.ut_host,pip->hostname,TMPX_LHOST) ;

	                cl = strnlen(pip->hostname,TMPX_LHOST) ;

#if	defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)
	                uc.ut_host[cl] = '\0' ;
#endif

	                uc.ut_syslen = (cl + 1) ;

	            } else {
	                uc.ut_host[0] = '\0' ;
	                uc.ut_syslen = 0 ;
	            }

	            gettimeofday(&uc.ut_tv,NULL) ;

	            uc.ut_pid = sid ;
	            uc.ut_exit.e_termination = 0 ;
	            uc.ut_exit.e_exit = 0 ;
	            uc.ut_type = TMPX_TUSERPROC ;

	            up = pututxline(&uc) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("main: pututxline() up=%p\n",up) ;
#endif

	            if (up == NULL) {
	                rs = SR_ACCESS ;
	                bprintf(pip->efp,
	                    "%s: operation not allowed\n",
	                    pip->progname) ;
	            }

	        } else {
	            bprintf(ofp,"no terminal on standard input\n") ;
		}

	    } else {
	        bprintf(ofp,"already logged in\n") ;
	    }

	} else {

	    if (up != NULL) {

	        uc = *up ;		/* copy the record found */

	        uc.ut_exit.e_termination = 0 ;
	        uc.ut_exit.e_exit = 0 ;
	        uc.ut_type = TMPX_TDEADPROC ;

	        up = pututxline(&uc) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: pututxline() up=%p\n",up) ;
#endif

	        if (up == NULL) {
	            rs = SR_ACCESS ;
		    fmt = "%s: operation not allowed\n" ;
	            bprintf(pip->efp,fmt,pn) ;
	        }

	    } /* end if */

	} /* end if (what to do) */

	endutxent() ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/process: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (process) */


/* zap the specified UTMPX entry */
static int zaputx(struct utmpx *up)
{
	TMPX		ut ;
	TMPX_CUR	cur ;
	TMPX_ENT	ue ;
	int		rs ;
	int		ei = 0 ;
	int		f ;

	if ((rs = tmpx_open(&ut,NULL,O_RDWR)) >= 0) {
	    if ((rs = tmpx_curbegin(&ut,&cur)) >= 0) {

	        f = FALSE ;
	        while ((ei = tmpx_enum(&ut,&cur,&ue)) >= 0) {

	            f = ((strncmp(up->ut_id,ue.ut_id,TMPX_LID) == 0) &&
	                (strncmp(up->ut_line,ue.ut_line,TMPX_LLINE) == 0)) ;

	            if (f) break ;
	        } /* end while */

	        if (f) {

	            ue.ut_exit.e_termination = up->ut_exit.e_termination ;
	            ue.ut_exit.e_exit = up->ut_exit.e_termination ;

	            ue.ut_type = up->ut_type ;
	            ue.ut_host[0] = '\0' ;
	            ue.ut_syslen = 0 ; /* or '1' like stupid Solaris wants? */

	            rs = tmpx_write(&ut,ei,&ue) ;

	        } /* end if (got a match) */

	        tmpx_curend(&ut,&cur) ;
	    } /* end if (cursor) */
	    tmpx_close(&ut) ;
	} /* end if (tmpx) */

	return (rs >= 0) ? ei : rs ;
}
/* end subroutine (zaputx) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip == NULL) return SR_FAULT ;

	if (lip->open.stores) {
	    lip->open.stores = FALSE ;
	    rs1 = vecstr_finish(&lip->stores) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (locinfo_finish) */


#if	CF_LOCSETENT
int locinfo_setentry(LOCINFO *lip,cchar **epp,cchar *vp,int vl)
{
	VECSTR		*slp ;
	int		rs = SR_OK ;
	int		len = 0 ;

	if (lip == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_FAULT ;

	slp = &lip->stores ;
	if (! lip->open.stores) {
	    rs = vecstr_start(slp,4,0) ;
	    lip->open.stores = (rs >= 0) ;
	}

	if (rs >= 0) {
	    int	oi = -1 ;
	    if (*epp != NULL) {
		oi = vecstr_findaddr(slp,*epp) ;
	    }
	    if (vp != NULL) {
	        len = strnlen(vp,vl) ;
	        rs = vecstr_store(slp,vp,len,epp) ;
	    } else {
	        *epp = NULL ;
	    }
	    if ((rs >= 0) && (oi >= 0)) {
	        vecstr_del(slp,oi) ;
	    }
	} /* end if (ok) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_setentry) */
#endif /* CF_LOCSETENT */


