/* b_qotd */

/* this is a SHELL built-in version of 'qotd(1)' */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory allocation */
#define	CF_BUFLINEIN	1		/* line-buffering for STDIN */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	This subroutine was originally written as a KSH built-in command.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ qotd [<day>|<mjd> -m] [-af <afile>] [-r] [-e] [-l <qfile>] [-V]

	Arguments:

	<day>		quote for this day (default today): <mon><mday>
	-af <afile>	argument file of <day(s)>
	-m		the day(s) are MJDs in decimal
	-r		flush locallly cached quote-of-the-day
	-e		request expiration maintenance of local cache
	-l <qfile>	load file into local cache
	-V		print command version to standard-error and then exit


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#if	defined(SFIO) && (SFIO > 0)
#define	CF_SFIO	1
#else
#define	CF_SFIO	0
#endif

#if	(defined(KSHBUILTIN) && (KSHBUILTIN > 0))
#include	<shell.h>
#endif

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<tzfile.h>		/* for TM_YEAR_BASE */

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<vecstr.h>
#include	<userinfo.h>
#include	<tmtime.h>
#include	<dayspec.h>
#include	<openqotd.h>
#include	<filebuf.h>
#include	<termout.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"msgdata.h"
#include	"proglog.h"
#include	"b_qotd.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(2048,LINE_MAX)
#else
#define	LINEBUFLEN	2048
#endif
#endif /* LINEBUFLEN */

#ifndef	QBUFLEN
#define	QBUFLEN		LINEBUFLEN
#endif

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	snsds(char *,int,cchar *,cchar *) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	mkplogid(char *,int,cchar *,int) ;
extern int	mksublogid(char *,int,cchar *,int) ;
extern int	vstrcmp(const void *,const void *) ;
extern int	vstrkeycmp(const void *,const void *) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecui(cchar *,int,uint *) ;
extern int	cfdecti(cchar *,int,int *) ;
extern int	ctdeci(char *,int,int) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	getmjd(int,int,int) ;
extern int	dialudp(cchar *,cchar *,int,int,int) ;
extern int	hasourmjd(cchar *,int) ;
extern int	hasalldig(cchar *,int) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;
extern int	isNotValid(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const PIVARS *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugprinthex(cchar *,int,cchar *,int) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*strdcpy1w(char *,int,cchar *,int) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		cvtcase:1 ;
	uint		cvtuc:1 ;
	uint		cvtlc:1 ;
	uint		termout:1 ;
	uint		outer:1 ;
	uint		curdate:1 ;
	uint		mjd:1 ;
	uint		gmt:1 ;
	uint		year:1 ;
	uint		ttl:1 ;
	uint		expire:1 ;
	uint		del:1 ;
	uint		intrun:1 ;
	uint		separate:1 ;
	uint		dgram:1 ;
	uint		rate:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	TERMOUT		outer ;
	DAYSPEC		ds ;
	vecstr		stores ;
	PROGINFO	*pip ;
	cchar		*termtype ;
	cchar		*qfname ;
	cchar		*hostspec ;
	int		year ;
	int		ttl ;
	int		intrun ;
	int		af ;
	int		rate ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procdgram(PROGINFO *,int) ;
static int	procdgramer(PROGINFO *,MSGDATA *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;
static int	procquery(PROGINFO *,void *,cchar *,int) ;
static int	procqueryload(PROGINFO *,int) ;
static int	procqueryout(PROGINFO *,void *,int) ;
static int	procqueryout_remote(PROGINFO *,void *,int) ;
static int	procqueryout_local(PROGINFO *,void *,int) ;
static int	procopenquery(PROGINFO *,int) ;
static int	procopenqueryhost(PROGINFO *,int) ;
static int	procqueryouter(PROGINFO *,void *,int) ;
static int	procquerytermout(PROGINFO *,void *,int) ;

static int	procuserinfo_begin(PROGINFO *,USERINFO *) ;
static int	procuserinfo_end(PROGINFO *) ;
static int	procuserinfo_logid(PROGINFO *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_qfname(LOCINFO *,cchar *) ;
static int	locinfo_termoutbegin(LOCINFO *,void *) ;
static int	locinfo_termoutend(LOCINFO *) ;
static int	locinfo_termoutprint(LOCINFO *,void *,cchar *,int) ;
static int	locinfo_defspec(LOCINFO *,DAYSPEC *) ;
static int	locinfo_curdate(LOCINFO *) ;
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
static int	locinfo_netparse(LOCINFO *,cchar *,int) ;
static int	locinfo_mjd(LOCINFO *) ;


/* local variables */

static cchar *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	"to",
	"tr",
	"dgram",
	"rate",
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
	argopt_to,
	argopt_tr,
	argopt_dgram,
	argopt_rate,
	argopt_overlast
} ;

static const PIVARS		initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRNAME
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
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;

static cchar *akonames[] = {
	"bufwhole",
	"bufline",
	"bufnone",
	"whole",
	"line",
	"none",
	"un",
	"termout",
	"gmt",
	"ttl",
	"intrun",
	"separate",
	"",
	NULL
} ;

enum akonames {
	akoname_bufwhole,
	akoname_bufline,
	akoname_bufnone,
	akoname_whole,
	akoname_line,
	akoname_none,
	akoname_un,
	akoname_termout,
	akoname_gmt,
	akoname_ttl,
	akoname_intrun,
	akoname_separate,
	akoname_empty,
	akoname_overlast
} ;


/* exported subroutines */


int b_qotd(int argc,cchar *argv[],void *contextp)
{
	int		rs ;
	int		rs1 ;
	int		ex = EX_OK ;

	if ((rs = lib_kshbegin(contextp,NULL)) >= 0) {
	    cchar	**envv = (cchar **) environ ;
	    ex = mainsub(argc,argv,envv,contextp) ;
	    rs1 = lib_kshend() ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ksh) */

	if ((rs < 0) && (ex == EX_OK)) ex = EX_DATAERR ;

	return ex ;
}
/* end subroutine (b_qotd) */


int p_qotd(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_qotd) */


/* local subroutines */


/* ARGSUSED */
static int mainsub(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	ARGINFO		ainfo ;
	BITS		pargs ;
	KEYOPT		akopts ;
	SHIO		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs, rs1 ;
	int		v ;
	int		wlen = 0 ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;

	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	cchar		*pr = NULL ;
	cchar		*sn = NULL ;
	cchar		*afname = NULL ;
	cchar		*ofname = NULL ;
	cchar		*efname = NULL ;
	cchar		*qfname = NULL ;
	cchar		*tos_open = NULL ;
	cchar		*tos_read = NULL ;
	cchar		*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_qotd: starting DFD=%u\n",rs) ;
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

	if ((cp = getourenv(envv,VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

/* initialize */

	pip->verboselevel = 1 ;
	pip->daytime = time(NULL) ;
	pip->f.logprog = OPT_LOGPROG ;

	pip->lip = lip ;
	if (rs >= 0) rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* start parsing the arguments */

	if (rs >= 0) rs = bits_start(&pargs,0) ;
	if (rs < 0) goto badpargs ;

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

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
	    if ((argl > 1) && (f_optminus || f_optplus)) {
	        const int ach = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ach)) {

	            argval = (argp+1) ;

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

/* program root */
	                case argopt_root:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pr = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pr = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* time-out */
	                case argopt_to:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            tos_open = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                tos_open = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* read time-out */
	                case argopt_tr:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            tos_read = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                tos_read = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* data-gram mode */
	                case argopt_dgram:
	                    lip->final.dgram = TRUE ;
	                    lip->f.dgram = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = cfdecti(avp,avl,&v) ;
	                            lip->intrun = v ;
	                        }
	                    }
	                    break ;

/* rate control mode */
	                case argopt_rate:
	                    lip->final.rate = TRUE ;
	                    lip->have.rate = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = cfdecti(avp,avl,&v) ;
	                            lip->rate = v ;
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

/* program-root */
	                    case 'R':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pr = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* terminal-type */
	                    case 'T':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                lip->termtype = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* expiration maintenance */
	                    case 'e':
	                        lip->f.expire = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = cfdecti(avp,avl,&v) ;
	                                lip->ttl = v ;
	                            }
	                        }
	                        break ;

/* query a specified host */
	                    case 'h':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                lip->hostspec = argp ;
				    }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'i':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = cfdecti(argp,argl,&v) ;
					lip->intrun = v ;
				    }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* specify that queries are MJDs */
	                    case 'l':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                qfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* specify that queries are MJDs */
	                    case 'm':
	                        lip->f.mjd = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.mjd = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* options */
	                    case 'o':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
					KEYOPT	*kop = &akopts ;
	                                rs = keyopt_loads(kop,argp,argl) ;
				    }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

	                    case 'r':
	                        lip->f.del = TRUE ;
	                        break ;

	                    case 't':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                tos_open = argp ;
	                                tos_read = argp ;
	                                rs = cfdecti(argp,argl,&v) ;
	                                pip->to = v ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* line-buffered */
	                    case 'u':
	                        pip->have.bufnone = TRUE ;
	                        pip->f.bufnone = TRUE ;
	                        pip->final.bufnone = TRUE ;
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

/* default year */
	                    case 'y':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = optvalue(argp,argl) ;
	                                lip->year = rs ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* use GMT */
	                    case 'z':
	                        lip->final.gmt = TRUE ;
	                        lip->have.gmt = TRUE ;
	                        lip->f.gmt = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.gmt = (rs > 0) ;
	                            }
	                        }
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

	if (efname == NULL) efname = getourenv(envv,VAREFNAME) ;
	if (efname == NULL) efname = STDERRFNAME ;
	if ((rs1 = shio_open(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    shio_control(&errfile,SHIO_CSETBUFLINE,TRUE) ;
	} else if (! isFailOpen(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_qotd: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    shio_printf(pip->efp,"%s: version %s\n",
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

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: pr=%s\n",pip->progname,pip->pr) ;
	    shio_printf(pip->efp,"%s: sn=%s\n",pip->progname,pip->searchname) ;
	} /* end if */

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help) {
#if	CF_SFIO
	    printhelp(sfstdout,pip->pr,pip->searchname,HELPFNAME) ;
#else
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;
#endif
	}

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* some initialization */

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

#ifdef	COMMENT
	if (pip->tmpdname == NULL) pip->tmpdname = getourenv(envv,VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;
#endif

	if ((rs >= 0) && (lip->intrun == 0)) {
	    if ((cp = getourenv(envv,VARINTRUN)) != NULL) {
		rs = cfdecti(cp,-1,&v) ;
		lip->intrun = v ;
	    }
	}

	if (lip->intrun == 0) lip->intrun = INT_RUN ;

	if (pip->to <= 0) pip->to = TO_RECVMSG ;

	if ((rs >= 0) && (pip->to_open == 0) && (tos_open != NULL)) {
	    rs = cfdecti(tos_open,-1,&v) ;
	    pip->to_open = v ;
	}

	if ((rs >= 0) && (pip->to_read == 0) && (tos_read != NULL)) {
	    rs = cfdecti(tos_read,-1,&v) ;
	    pip->to_read = v ;
	}

	if (pip->to_open == 0) pip->to_open = TO_OPEN ;

	if (pip->to_read == 0) pip->to_read = TO_READ ;

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    if ((pip->to_open >= 0) || (pip->to_read >= 0)) {
		fmt = "%s: to_open=%d\n" ;
	        shio_printf(pip->efp,fmt,pn,pip->to_open) ;
		fmt = "%s: to_read=%d\n" ;
	        shio_printf(pip->efp,fmt,pn,pip->to_read) ;
	    }
	}

	if ((rs >= 0) && (qfname != NULL)) {
	    rs = locinfo_qfname(lip,qfname) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("b_qotd: to_open=%d\n",pip->to_open) ;
	    debugprintf("b_qotd: to_read=%d\n",pip->to_read) ;
	    debugprintf("b_qotd: f_bufline=%u\n",pip->f.bufline) ;
	    debugprintf("b_qotd: f_bufnone=%u\n",pip->f.bufnone) ;
	}
#endif /* CF_DEBUG */

/* go */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    USERINFO	u ;
	    if ((rs = userinfo_start(&u,NULL)) >= 0) {
	        if ((rs = procuserinfo_begin(pip,&u)) >= 0) {
	 	    if ((rs = proglog_begin(pip,&u)) >= 0) {
	    	        if (lip->f.dgram) {
	    	            const int	nfd = FD_STDIN ;
			    rs = procdgram(pip,nfd) ;
			    wlen = rs ;
	    	        } else {
	    	            cchar	*ofn = ofname ;
	    	            cchar	*afn = afname ;
			    rs = procargs(pip,&ainfo,&pargs,ofn,afn) ;
			    wlen = rs ;
	    	        }
	 		rs1 = proglog_end(pip) ;
	  		if (rs >= 0) rs = rs1 ;
	  	    } /* end if (proglog) */
		    rs1 = procuserinfo_end(pip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (procuserinfo) */
	        rs1 = userinfo_finish(&u) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt = "%s: userinfo failure (%d)\n" ;
	        ex = EX_NOUSER ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	    } /* end if (userinfo) */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    ex = EX_USAGE ;
	    usage(pip) ;
	} /* end if (ok) */

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    shio_printf(pip->efp,"%s: bytes=%u\n",
	        pip->progname,wlen) ;
	}

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    if (! pip->f.quiet) {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt = "%s: could not process (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	    }
	    ex = mapex(mapexs,rs) ;
	} else if (rs >= 0) {
	    if ((rs = lib_sigterm()) < 0) {
	        ex = EX_TERM ;
	    } else if ((rs = lib_sigintr()) < 0) {
	        ex = EX_INTR ;
	    }
	} /* end if */

retearly:
	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_qotd: exiting ex=%u (%d)\n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    shio_close(pip->efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
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
	    debugprintf("b_qotd: final mallout=%u\n",mo-mo_start) ;
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
	shio_printf(pip->efp,
	    "%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (mainsub) */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s [<day(s)>|-m <mjd(s)>] [-af <afile>] [-r]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-of <ofile>] [-e[=<ttl>]] [-l <qfile>] [-y <year>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-to <to_open>] [-tr <to_read>] [-h <host>]:<port>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* process the program ako-options */
static int procopts(PROGINFO *pip,KEYOPT *kop)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		c = 0 ;
	cchar		*cp ;

	if ((cp = getourenv(pip->envv,VAROPTS)) != NULL) {
	    rs = keyopt_loads(kop,cp,-1) ;
	}

	if (rs >= 0) {
	    KEYOPT_CUR	kcur ;
	    if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
	        int	oi ;
	        int	kl, vl ;
	        cchar	*kp, *vp ;

	        while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	            if ((oi = matostr(akonames,2,kp,kl)) >= 0) {

	                vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	                switch (oi) {
	                case akoname_bufwhole:
	                case akoname_whole:
	                    if (! pip->final.bufwhole) {
	                        pip->have.bufwhole = TRUE ;
	                        pip->final.bufwhole = TRUE ;
	                        pip->f.bufwhole = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.bufwhole = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_bufline:
	                case akoname_line:
	                    if (! pip->final.bufline) {
	                        pip->have.bufline = TRUE ;
	                        pip->final.bufline = TRUE ;
	                        pip->f.bufline = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.bufline = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_bufnone:
	                case akoname_none:
	                case akoname_un:
	                    if (! pip->final.bufnone) {
	                        pip->have.bufnone = TRUE ;
	                        pip->final.bufnone = TRUE ;
	                        pip->f.bufnone = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.bufnone = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_termout:
	                    if (! lip->final.termout) {
	                        lip->have.termout = TRUE ;
	                        lip->final.termout = TRUE ;
	                        lip->f.termout = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.termout = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_gmt:
	                    if (! lip->final.gmt) {
	                        lip->have.gmt = TRUE ;
	                        lip->final.gmt = TRUE ;
	                        lip->f.gmt = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.gmt = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_ttl:
	                    if (lip->ttl < 0) {
	                        if (vl > 0) {
	                            rs = optvalue(vp,vl) ;
	                            lip->ttl = rs ;
	                        }
	                    }
	                    break ;
	                case akoname_intrun:
	                    if (lip->intrun < 0) {
	                        if (vl > 0) {
	                            int	v ;
	                            rs = cfdecti(vp,vl,&v) ;
	                            lip->intrun = v ;
	                        }
	                    }
	                    break ;
	                case akoname_separate:
	                    if (! lip->final.separate) {
	                        lip->have.separate = TRUE ;
	                        lip->final.separate = TRUE ;
	                        lip->f.separate = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.separate = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_empty:
	                    break ;
	                default:
	                    rs = SR_INVALID ;
	                    break ;
	                } /* end switch */

	                c += 1 ;
	            } else
	                rs = SR_INVALID ;

	            if (rs < 0) break ;
	        } /* end while (looping through key options) */

	        keyopt_curend(kop,&kcur) ;
	    } /* end if (keyopt-cur) */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


/* this is a UDP server (it eventually times out) */
static int procdgram(PROGINFO *pip,int nfd)
{
	LOCINFO		*lip = pip->lip ;
	MSGDATA		m ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	int		c = 0 ;
	if ((rs = msgdata_init(&m,0)) >= 0) {
	    const int	to = lip->intrun ;
	    while ((rs = msgdata_recvto(&m,nfd,to)) >= 0) {
	        if ((rs = procdgramer(pip,&m)) >= 0) {
		    if (lip->rate > 0) sleep(lip->rate) ;
		    c += 1 ;
	            rs = msgdata_send(&m,nfd,rs,0) ;
	            wlen += rs ;
	        }
	        if (rs >= 0) rs = lib_sigterm() ;
	        if (rs >= 0) rs = lib_sigintr() ;
	        if (rs < 0) break ;
	    } /* end while */
	    rs1 = msgdata_fini(&m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (msgdata) */
	proglog_printf(pip,"reverse c=%u wl=%u",c,wlen) ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procdgram) */


static int procdgramer(PROGINFO *pip,MSGDATA *mip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		rs1 ;
	int		tlen = 0 ;
	int		mjd = 0 ;
	char		*mbuf ;
	if ((rs = msgdata_getdata(mip,&mbuf)) > 0) {
	    int		ql = rs ;
	    cchar	*tp ;
	    cchar	*qp  = mbuf ;
	    if ((tp = strnchr(qp,ql,'\n')) != NULL) {
	        ql = (tp-qp) ;
	    }
	    while (ql && (qp[ql-1] == '\0')) ql -= 1 ;
	    if ((rs = locinfo_netparse(lip,qp,ql)) >= 0) {
	        mjd = rs ;
	    } else if (isNotValid(rs)) {
	        rs = SR_OK ;
	    }
	} /* end if (msgdata_getdata) */
	if ((rs >= 0) && (mjd == 0)) {
	    rs = locinfo_mjd(lip) ;
	    mjd = rs ;
	}
	if (rs >= 0) {
	    const int	of = O_RDONLY ;
	    const int	to_open = pip->to_open ;
	    if (pip->debuglevel > 0) {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt = "%s: mjd=%u\n" ;
	        shio_printf(pip->efp,fmt,pn,mjd) ;
	    }
	    proglog_printf(pip,"mjd=%u",mjd) ;
	    if ((rs = openqotd(pip->pr,mjd,of,to_open)) >= 0) {
	        const int	qfd = rs ;
		if ((rs = msgdata_getbuf(mip,&mbuf)) >= 0) {
		    int		ml = rs ;
		    char	*mp = mbuf ;
	            while ((ml > 0) && ((rs = u_read(qfd,mp,ml)) > 0)) {
	                int	rlen = rs ;
			tlen += rlen ;
	                mp += rlen ;
	                ml -= rlen ;
	            } /* end while (reading) */
		    msgdata_setdatalen(mip,tlen) ;
		} /* end if (msgdata) */
	        rs1 = u_close(qfd) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (openqotd) */
	} /* end if (ok) */
	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (procdgramer) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *afn,cchar *ofn)
{
	LOCINFO		*lip = pip->lip ;
	SHIO		ofile, *ofp = &ofile ;
	const int	to_open = pip->to_open ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

	if ((rs = shio_opene(ofp,ofn,"wct",0666,to_open)) >= 0) {
	    int	pan = 0 ;

	    if (pip->have.bufnone)
	        shio_control(ofp,SHIO_CSETBUFNONE,TRUE) ;

	    if (pip->have.bufline)
	        shio_control(ofp,SHIO_CSETBUFLINE,pip->f.bufline) ;

	    if (pip->have.bufwhole)
	        shio_control(ofp,SHIO_CSETBUFWHOLE,pip->f.bufwhole) ;

/* go through the loops */

	    if ((rs = locinfo_termoutbegin(lip,ofp)) >= 0) {
	        int	cl ;
	        cchar	*cp ;

	        if (rs >= 0) {
	            const int	argc = aip->argc ;
	            int		ai ;
	            int		f ;
	            cchar	**argv = aip->argv ;
	            for (ai = 1 ; ai < argc ; ai += 1) {

	                f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	                f = f || ((ai > aip->ai_pos) && (argv[ai] != NULL)) ;
	                if (f) {
	                    cp = aip->argv[ai] ;
	                    if (cp[0] != '\0') {
	                        pan += 1 ;
	                        rs = procquery(pip,ofp,cp,-1) ;
	                        wlen += rs ;
	                    }
	                }

	                if (rs >= 0) rs = lib_sigterm() ;
	                if (rs >= 0) rs = lib_sigintr() ;
	                if (rs < 0) break ;
	            } /* end for */
	        } /* end if (ok) */

	        if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	            SHIO	afile, *afp = &afile ;

	            if (strcmp(afn,"-") == 0) afn = STDINFNAME ;

	            if ((rs = shio_open(&afile,afn,"r",0666)) >= 0) {
	                const int	llen = LINEBUFLEN ;
	                char		lbuf[LINEBUFLEN + 1] ;

	                while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	                    int	len = rs ;

	                    if (lbuf[len - 1] == '\n') len -= 1 ;
	                    lbuf[len] = '\0' ;

	                    if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                        if (cp[0] != '#') {
	                            pan += 1 ;
	                            rs = procquery(pip,ofp,cp,cl) ;
	                            wlen += rs ;
	                        }
	                    }

	                    if (rs >= 0) rs = lib_sigterm() ;
	                    if (rs >= 0) rs = lib_sigintr() ;
	                    if (rs < 0) break ;
	                } /* end while (reading lines) */

	                rs1 = shio_close(afp) ;
	                if (rs >= 0) rs = rs1 ;
	            } else {
	                fmt = "%s: inaccessible argument-list (%d)\n",
	                shio_printf(pip->efp,fmt,pn,rs) ;
	                shio_printf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	            } /* end if */

	        } /* end if (procesing file argument file list) */

	        if ((rs >= 0) && (pan == 0)) {

	            cp = "-" ;
	            pan += 1 ;
	            rs = procquery(pip,ofp,cp,-1) ;
	            wlen += rs ;

	        } /* end if (standard-input) */

	        rs1 = locinfo_termoutend(lip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (termout) */

	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    fmt = "%s: inaccessible output (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    shio_printf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	}
	proglog_printf(pip,"forward wl=%u",wlen) ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procargs) */


static int procquery(PROGINFO *pip,void *ofp,cchar qp[],int ql)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		mjd ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_qotd/procquery: q=%s\n",qp) ;
#endif

	if (qp == NULL)
	    return SR_FAULT ;

	if (ql < 0) ql = strlen(qp) ;

	if (lip->f.mjd && hasalldig(qp,ql)) {
	    uint	uv ;
	    rs = cfdecui(qp,ql,&uv) ;
	    mjd = (int) uv ;
	} else if ((rs = hasourmjd(qp,ql)) > 0) {
	    mjd = rs ;
	} else {
	    DAYSPEC	ds ;
	    if ((qp[0] == '+') || (qp[0] == '-')) {
	        rs = dayspec_default(&ds) ;
	    } else {
	        rs = dayspec_load(&ds,qp,ql) ;
	    }
	    if (rs >= 0) {
	        if ((rs = locinfo_defspec(lip,&ds)) >= 0) {
	            rs = getmjd(ds.y,ds.m,ds.d) ;
	            mjd = rs ;
	        }
	    }
	} /* end if */

	if (rs >= 0) {
	    if (pip->debuglevel > 0) {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt = "%s: mjd=%u\n" ;
	        shio_printf(pip->efp,fmt,pn,mjd) ;
	    }
	    if (lip->qfname != NULL) {
	        rs = procqueryload(pip,mjd) ;
	    } else {
	        const int	n = MAX(pip->n,1) ;
	        int		i ;
	        for (i = 0 ; i < n ; i += 1) {
	            if (lip->f.separate && (i > 0)) {
	                char	obuf[2] = { '÷', 0 } ;
	                rs = shio_print(ofp,obuf,1) ;
	                wlen += rs ;
	            }
	            if (rs >= 0) {
	                rs = procqueryout(pip,ofp,mjd) ;
	                wlen += rs ;
	                mjd += 1 ;
	            }
	            if (rs < 0) break ;
	        } /* end for */
	    } /* end if (program mode) */
	} /* end if (ok) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procquery) */


static int procqueryload(PROGINFO *pip,int mjd)
{
	LOCINFO		*lip = pip->lip ;
	const int	to_open = pip->to_open ;
	const int	of = (O_CREAT|O_WRONLY|O_TRUNC) ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if ((rs = openqotd(pip->pr,mjd,of,to_open)) >= 0) {
	    SHIO	ifile, *ifp = &ifile ;
	    const int	qfd = rs ;
	    cchar	*qfname = lip->qfname ;

	    if (qfname[0] == '-') qfname = STDINFNAME ;

	    if ((rs = shio_open(ifp,qfname,"r",0666)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        char		lbuf[LINEBUFLEN+1] ;

	        while ((rs = shio_read(ifp,lbuf,llen)) > 0) {
	            int	len = rs ;

	            rs = u_write(qfd,lbuf,len) ;
	            wlen += rs ;

	            if (rs < 0) break ;
	        } /* end while */

	        rs1 = shio_close(ifp) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (input-file) */

	    rs1 = u_close(qfd) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (quote-file) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procqueryload) */


static int procqueryout(PROGINFO *pip,void *ofp,int mjd)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		wlen = 0 ;

	if (lip->hostspec != NULL) {
	    rs = procqueryout_remote(pip,ofp,mjd) ;
	    wlen += rs ;
	} else {
	    rs = procqueryout_local(pip,ofp,mjd) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procqueryout) */


static int procqueryout_remote(PROGINFO *pip,void *ofp,int mjd)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_qotd/procqueryout_remote: ent mjd=%d\n",mjd) ;
#endif

	if ((rs = procopenquery(pip,mjd)) >= 0) {
	    const int	to = pip->to_read ;
	    const int	qlen = QBUFLEN ;
	    const int	qfd = rs ;
	    char	qbuf[QBUFLEN+1] ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("b_qotd/procqueryout_remote: uc_recve()\n") ;
#endif
	    if ((rs = uc_recve(qfd,qbuf,qlen,0,to,0)) >= 0) {
		const int	ql = rs ;
	        if (pip->verboselevel > 0) {
	            if (lip->open.outer) {
	                if (ql > 0) {
	                    rs = locinfo_termoutprint(lip,ofp,qbuf,ql) ;
	                    wlen += rs ;
	                } else {
	                    rs = shio_print(ofp,qbuf,ql) ;
	                    wlen += rs ;
	                }
		    } else {
	    		rs = shio_write(ofp,qbuf,ql) ;
	    		wlen += rs ;
		    }
		}
	    } /* end if (uc_recve) */
	    rs1 = u_close(qfd) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (procopenquery) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_qotd/procqueryout_remote: ret rs=%d wlen=%u\n",
		rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procqueryout_remote) */


static int procqueryout_local(PROGINFO *pip,void *ofp,int mjd)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_qotd/procqueryout_local: ent mjd=%d\n",mjd) ;
#endif

	if ((rs = procopenquery(pip,mjd)) >= 0) {
	    const int	qfd = rs ;

	    if (pip->verboselevel > 0) {
	        if (lip->open.outer) {
	            rs = procquerytermout(pip,ofp,qfd) ;
	            wlen = rs ;
	        } else {
	            rs = procqueryouter(pip,ofp,qfd) ;
	            wlen = rs ;
	        }
	    } /* end if (verbose enough) */

	    rs1 = u_close(qfd) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (open) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procqueryout_local) */


static int procopenquery(PROGINFO *pip,int mjd)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		fd = -1 ;

	if (lip->hostspec != NULL) {
	    rs = procopenqueryhost(pip,mjd) ;
	    fd = rs ;
	} else {
	    int		to_open = pip->to_open ;
	    int		of = O_RDONLY ;
	    if (lip->f.expire) {
	        of |= O_EXCL ;
	        to_open = lip->ttl ;
	    }
	    if (lip->f.del) {
	        of |= O_NOCTTY ;
	    }
	    rs = openqotd(pip->pr,mjd,of,to_open) ;
	    fd = rs ;
	} /* end if (local or remote) */

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (procopenquery) */


static int procopenqueryhost(PROGINFO *pip,int mjd)
{
	LOCINFO		*lip = pip->lip ;
	const int	hlen = MAXHOSTNAMELEN ;
	int		rs = SR_OK ;
	int		fd = -1 ;
	cchar		*tp ;
	cchar		*hs, *ps ;
	char		hbuf[MAXHOSTNAMELEN+1] ;
	if ((tp = strchr(lip->hostspec,':')) != NULL) {
	    hs = hbuf ;
	    strdcpy1w(hbuf,hlen,lip->hostspec,(tp-lip->hostspec)) ;
	    ps = (tp+1) ;
	} else {
	    hs = lip->hostspec ;
	    ps = PORTSPEC_QUOTE ;
	}
	if (hs[0] != '\0') {
	    const int	to_open = pip->to_open ;
	    const int	af = lip->af ;
	    if ((rs = dialudp(hs,ps,af,to_open,0)) >= 0) {
	        fd = rs ;
	        if ((rs = ctdeci(hbuf,hlen,mjd)) >= 0) {
		    rs = u_send(fd,hbuf,rs,0) ;
		    if (rs < 0) u_close(fd) ;
	        }
	    } /* end if (dialudp) */
	} else {
	    rs = SR_INVALID ;
	}
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (procopenqueryhost) */


static int procqueryouter(PROGINFO *pip,void *ofp,int qfd)
{
	const int	llen = LINEBUFLEN ;
	int		rs ;
	int		len ;
	int		wlen = 0 ;
	char		lbuf[LINEBUFLEN + 1] ;

	if (pip == NULL) return SR_FAULT ;

	while ((rs = u_read(qfd,lbuf,llen)) > 0) {
	    len = rs ;

	    rs = shio_write(ofp,lbuf,len) ;
	    wlen += rs ;

	    if (rs >= 0) rs = lib_sigterm() ;
	    if (rs >= 0) rs = lib_sigintr() ;
	    if (rs < 0) break ;
	} /* end while */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procqueryouter) */


static int procquerytermout(PROGINFO *pip,void *ofp,int qfd)
{
	LOCINFO		*lip = pip->lip ;
	FILEBUF		b ;
	const int	to = pip->to_read ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if ((rs = filebuf_start(&b,qfd,0L,0,0)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    int		len ;
	    char	lbuf[LINEBUFLEN + 1] ;

	    while ((rs = filebuf_readlines(&b,lbuf,llen,to,NULL)) > 0) {
	        len = rs ;

	            if (len > 0) {
	                rs = locinfo_termoutprint(lip,ofp,lbuf,len) ;
	                wlen += rs ;
	            } else {
	                rs = shio_print(ofp,lbuf,len) ;
	                wlen += rs ;
	            }

	        if (rs >= 0) rs = lib_sigterm() ;
	        if (rs >= 0) rs = lib_sigintr() ;
	        if (rs < 0) break ;
	    } /* end while */

	    rs1 = filebuf_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (filebuf) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_qotd/procquerytermout: ret rs=%d wlen=%u\n",
	        rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procquerytermout) */


static int procuserinfo_begin(PROGINFO *pip,USERINFO *uip)
{
	int		rs = SR_OK ;

	pip->nodename = uip->nodename ;
	pip->domainname = uip->domainname ;
	pip->username = uip->username ;
	pip->gecosname = uip->gecosname ;
	pip->realname = uip->realname ;
	pip->name = uip->name ;
	pip->fullname = uip->fullname ;
	pip->mailname = uip->mailname ;
	pip->org = uip->organization ;
	pip->logid = uip->logid ;
	pip->pid = uip->pid ;
	pip->uid = uip->uid ;
	pip->euid = uip->euid ;
	pip->gid = uip->gid ;
	pip->egid = uip->egid ;

	if (rs >= 0) {
	    const int	hlen = MAXHOSTNAMELEN ;
	    char	hbuf[MAXHOSTNAMELEN+1] ;
	    cchar	*nn = pip->nodename ;
	    cchar	*dn = pip->domainname ;
	    if ((rs = snsds(hbuf,hlen,nn,dn)) >= 0) {
	        cchar	**vpp = &pip->hostname ;
	        rs = proginfo_setentry(pip,vpp,hbuf,rs) ;
	    }
	}

	if (rs >= 0) {
	    rs = procuserinfo_logid(pip) ;
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (procuserinfo_begin) */


static int procuserinfo_end(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;

	return rs ;
}
/* end subroutine (procuserinfo_end) */


static int procuserinfo_logid(PROGINFO *pip)
{
	int		rs ;
	if ((rs = lib_runmode()) >= 0) {
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("procuserinfo_logid: rm=%08ß\n",rs) ;
#endif
	    if (rs & KSHLIB_RMKSH) {
	        if ((rs = lib_serial()) >= 0) {
	            const int	s = rs ;
	            const int	plen = LOGIDLEN ;
	            const int	pv = pip->pid ;
	            cchar	*nn = pip->nodename ;
	            char	pbuf[LOGIDLEN+1] ;
	            if ((rs = mkplogid(pbuf,plen,nn,pv)) >= 0) {
	                const int	slen = LOGIDLEN ;
	                char		sbuf[LOGIDLEN+1] ;
	                if ((rs = mksublogid(sbuf,slen,pbuf,s)) >= 0) {
	                    cchar	**vpp = &pip->logid ;
	                    rs = proginfo_setentry(pip,vpp,sbuf,rs) ;
	                }
	            }
	        } /* end if (lib_serial) */
	    } /* end if (runmode-KSH) */
	} /* end if (lib_runmode) */
	return rs ;
}
/* end subroutine (procuserinfo_logid) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;
	cchar		*varterm = VARTERM ;

	if (lip == NULL) return SR_FAULT ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->termtype = getourenv(pip->envv,varterm) ;

	lip->f.separate = TRUE ;

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


static int locinfo_qfname(LOCINFO *lip,cchar *qfname)
{
	int		rs = SR_OK ;
	cchar		**vpp ;

	if (qfname != NULL) {
	    vpp = &lip->qfname ;
	    rs = locinfo_setentry(lip,vpp,qfname,-1) ;
	}

	return rs ;
}
/* end subroutine (locinfo_qfname) */


static int locinfo_termoutbegin(LOCINFO *lip,void *ofp)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		f_termout = FALSE ;
	cchar		*tstr = lip->termtype ;

	if (lip->f.termout || ((rs = shio_isterm(ofp)) > 0)) {
	    int		ncols = COLUMNS ;
	    cchar	*vp ;
	    if ((vp = getourenv(pip->envv,VARCOLUMNS)) != NULL) {
	        int	v ;
	        rs1 = cfdeci(vp,-1,&v) ;
	        if (rs1 >= 0) ncols = v ;
	    }
	    if (rs >= 0) {
	        rs = termout_start(&lip->outer,tstr,-1,ncols) ;
	        lip->open.outer = (rs >= 0) ;
	    }
	} /* end if */

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    cchar	*pn = pip->progname ;
	    f_termout = lip->open.outer ;
	    shio_printf(pip->efp,"%s: termout=%u\n",pn,f_termout) ;
	    if (f_termout) {
	        shio_printf(pip->efp,"%s: termtype=%s\n",pn,tstr) ;
	    }
	}

	return (rs >= 0) ? f_termout : rs ;
}
/* end subroutine (locinfo_termoutbegin) */


static int locinfo_termoutend(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip->open.outer) {
	    lip->open.outer = FALSE ;
	    rs1 = termout_finish(&lip->outer) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (locinfo_termoutend) */


static int locinfo_termoutprint(LOCINFO *lip,void *ofp,cchar *lbuf,int llen)
{
	PROGINFO	*pip = lip->pip ;
	TERMOUT		*top = &lip->outer ;
	int		rs ;
	int		wlen = 0 ;

	if (pip == NULL) return SR_FAULT ;

	if ((rs = termout_load(top,lbuf,llen)) >= 0) {
	    int		ln = rs ;
	    int		i ;
	    int		ll ;
	    cchar	*lp ;
	    for (i = 0 ; i < ln ; i += 1) {
	        ll = termout_getline(top,i,&lp) ;
	        if (ll == SR_NOTFOUND) break ;
	        rs = ll ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("b_qotd/locinfo_termoutprint: ll=%u\n",ll) ;
	            debugprintf("b_qotd/locinfo_termoutprint: l=>%t<\n",
	                lp,strlinelen(lp,ll,40)) ;
	        }
#endif

	        if (rs >= 0) {
	            rs = shio_print(ofp,lp,ll) ;
	            wlen += rs ;
	        }

	        if (rs < 0) break ;
	    } /* end for */
	} /* end if (termout_load) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (locinfo_termoutprint) */


static int locinfo_defspec(LOCINFO *lip,DAYSPEC *dsp)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

	f = f || (dsp->y < 0) ;
	f = f || (dsp->m < 0)  ;
	f = f || (dsp->d < 0) ;
	if (f) {
	    if (! lip->f.curdate) rs = locinfo_curdate(lip) ;
	    if (dsp->y < 0) dsp->y = lip->ds.y ;
	    if (dsp->m < 0) dsp->m = lip->ds.m ;
	    if (dsp->d < 0) dsp->d = lip->ds.d ;
	}

	return rs ;
}
/* end subroutine (locinfo_defspec) */


static int locinfo_curdate(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	if (! lip->f.curdate) {
	    TMTIME	ct ;
	    lip->f.curdate = TRUE ;
	    if (lip->f.gmt) {
	        rs = tmtime_gmtime(&ct,pip->daytime) ;
	    } else {
	        rs = tmtime_localtime(&ct,pip->daytime) ;
	    }
	    lip->ds.y = (lip->year > 0) ? lip->year : (ct.year + TM_YEAR_BASE) ;
	    lip->ds.m = ct.mon ;
	    lip->ds.d = ct.mday ;
	} /* end if (needed) */
	return rs ;
}
/* end subroutine (locinfo_curdate) */


static int locinfo_netparse(LOCINFO *lip,cchar *qp,int ql)
{
	int		rs = SR_OK ;
	int		mjd = 0 ;
	if (ql < 0) ql = strlen(qp) ;
	if (hasalldig(qp,ql)) {
	    uint	uv ;
	    rs = cfdecui(qp,ql,&uv) ;
	    mjd = (int) uv ;
	} else if ((rs = hasourmjd(qp,ql)) > 0) {
	    mjd = rs ;
	} else {
	    DAYSPEC	ds ;
	    if ((qp[0] == '+') || (qp[0] == '-')) {
	        rs = dayspec_default(&ds) ;
	    } else {
	        rs = dayspec_load(&ds,qp,ql) ;
	    }
	    if (rs >= 0) {
	        if ((rs = locinfo_defspec(lip,&ds)) >= 0) {
	            rs = getmjd(ds.y,ds.m,ds.d) ;
	            mjd = rs ;
	        }
	    }
	} /* end if */
	return (rs >= 0) ? mjd : rs ;
}
/* end subroutine (locinfo_netparse) */


static int locinfo_mjd(LOCINFO *lip)
{
	int		rs ;
	int		mjd = 0 ;
	if ((rs = locinfo_curdate(lip)) >= 0) {
	    rs = getmjd(lip->ds.y,lip->ds.m,lip->ds.d) ;
	    mjd = rs ;
	}
	return (rs >= 0) ? mjd : rs ;
}
/* end subroutine (locinfo_mjd) */


