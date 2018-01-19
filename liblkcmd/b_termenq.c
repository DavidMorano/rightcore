/* b_termenq */

/* SHELL built-in to enquire about terminal information */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_STDIN	0		/* use standard-input */
#define	CF_SEC		1		/* call secondary */
#define	CF_ID		0		/* call ID */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	This subroutine was originally written.  

	= 2017-12-16, David A­D­ Morano
        Update puts terminal-type 'screen' ahead of 'vt100'. This still uses an
        internal database for the various terminal attributes. A reasonable
        future enhancement would be to read this database in from a file.

*/

/* Copyright © 2004,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ termenq [-s|-l] [-dev <device>|-line <line>]


*******************************************************************************/


#include	<envstandards.h>

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
#include	<sys/time.h>		/* for 'gethrtime(3c)' */
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<ascii.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<vecstr.h>
#include	<userinfo.h>
#include	<field.h>
#include	<uterm.h>
#include	<termcmd.h>
#include	<sbuf.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_termenq.h"
#include	"defs.h"
#include	"proglog.h"


/* local defines */

#define	UTOPTS		(FM_NOFILTER | FM_NOECHO | FM_RAWIN | FM_TIMED)

#define	CVTBUFLEN	100

#define	CONBUFLEN	100

#define	TBUFLEN		CONBUFLEN

#ifndef	DEVDNAME
#define	DEVDNAME	"/dev"
#endif

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	snsds(char *,int,cchar *,cchar *) ;
extern int	snwcpy(char *,int,cchar *,int) ;
extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	mkpath2w(char *,cchar *,cchar *,int) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	matocasestr(cchar **,int,cchar *,int) ;
extern int	nleadstr(cchar *,cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecui(cchar *,int,uint *) ;
extern int	cfdecti(cchar *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	mkplogid(char *,int,cchar *,int) ;
extern int	mksublogid(char *,int,cchar *,int) ;
extern int	bufprintf(char *,int,cchar *,...) ;
extern int	termescseq(char *,int,int,int,int,int,int) ;
extern int	termconseq(char *,int,int,int,int,int,int) ;
extern int	termconseqi(char *,int,int,cchar *,int,int,int,int) ;
extern int	getutmpterm(char *,int,pid_t) ;
extern int	termdevice(char *,int,int) ;
extern int	uterm_readcmd(UTERM *,TERMCMD *,int,int) ;
extern int	tcgetws(int,struct winsize *) ;
extern int	tcgetlines(int) ;
extern int	tcsetlines(int,int) ;
extern int	isdigitlatin(int) ;
extern int	iscmdstart(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const PIVARS *) ;

extern int	termtype_vt(char *,int,const short *,const short *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		termfname:1 ;		/* have filename */
	uint		list:1 ;		/* list mode */
	uint		set:1 ;			/* "set" mode */
	uint		ws:1 ;			/* retrieved */
	uint		ansi:1 ;		/* ANSI confirmance mode */
	uint		latin1:1 ;		/* set ISO-Latin-1 in GR */
	uint		opened:1 ;		/* terminal was opened */
	uint		ut:1 ;
	uint		poll:1 ;
} ;

struct locinfo {
	vecstr		stores ;
	LOCINFO_FL	have, init, f, changed, final ;
	LOCINFO_FL	open ;
	UTERM		ut ;
	PROGINFO	*pip ;
	cchar		*termline ;	/* terminal "line" */
	cchar		*termfname ;	/* terminal file-name */
	cchar		*db ;
	struct winsize	ws ;
	int		tfd ;
	int		intpoll ;
	int		ansi ;		/* ANSI conformance level */
	int		to ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	process(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;

static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;
static int	procspecs(PROGINFO *,SHIO *,cchar *,int) ;
static int	procspec(PROGINFO *,SHIO *, cchar *,int) ;
static int	procget(PROGINFO *,SHIO *,int) ;
static int	procsetlatin1(PROGINFO *) ;
static int	procsetansi(PROGINFO *) ;
static int	procenq(PROGINFO *,SHIO *) ;
static int	procdevattr(PROGINFO *,SHIO *) ;
static int	procdevattr_pri(PROGINFO *,SHIO *,TERMCMD *,char *,int) ;
static int	procdevattr_sec(PROGINFO *,SHIO *,TERMCMD *,char *,int) ;

#if	CF_ID
static int	procdevattr_id(PROGINFO *,SHIO *,char *,int) ;
#endif

static int	procuserinfo_begin(PROGINFO *,USERINFO *) ;
static int	procuserinfo_end(PROGINFO *) ;
static int	procuserinfo_logid(PROGINFO *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
static int	locinfo_termbegin(LOCINFO *) ;
static int	locinfo_termend(LOCINFO *) ;
static int	locinfo_setline(LOCINFO *,cchar *,int) ;
static int	locinfo_utermbegin(LOCINFO *) ;
static int	locinfo_utermend(LOCINFO *) ;
static int	locinfo_utermread(LOCINFO *,TERMCMD *,char *,int) ;
static int	locinfo_utermwrite(LOCINFO *,cchar *,int) ;

static int	sicmdstart(cchar *,int) ;


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	"tf",
	"dev",
	"line",
	"db",
	"to",
	"ansi",
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
	argopt_tf,
	argopt_dev,
	argopt_line,
	argopt_db,
	argopt_to,
	argopt_ansi,
	argopt_overlast
} ;

static const PIVARS	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRNAME
} ;

static const MAPEX	mapexs[] = {
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

static const char	*progopts[] = {
	"poll",
	"pollint",
	"intpoll",
	"logprog",
	"ansi",
	"toopen",
	"toread",
	"latin1",
	NULL
} ;

enum progopts {
	progopt_poll,
	progopt_intpoll,
	progopt_intpoller,
	progopt_logprog,
	progopt_ansi,
	progopt_toopen,
	progopt_toread,
	progopt_latin1,
	progopt_overlast
} ;

/* define the configuration keywords */
static const char	*qopts[] = {
	"answerback",
	"type",
	NULL
} ;

enum qopts {
	qopt_answerback,
	qopt_type,
	qopt_overlast
} ;

static const uchar	aterms[] = {
	0x00, 0x2E, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int b_termenq(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_termenq) */


int p_termenq(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_termenq) */


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
	int		ai_continue = 1 ;
	int		rs, rs1 ;
	int		cl ;
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
	cchar		*efname = NULL ;
	cchar		*ofname = NULL ;
	cchar		*termline = NULL ;
	cchar		*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_termenq: starting DFD=%d\n",rs) ;
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
	pip->to_open = TO_OPEN ;
	pip->to_read = TO_READ ;
	pip->f.logprog = OPT_LOGPROG ;

#ifdef	COMMENT
	pip->daytime = time(NULL) ;
#endif

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
	        const int	ach = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ach)) {

	            argval = (argp+1) ;

	        } else if (ach == '-') {

	            ai_pos = ai ;
	            break ;

	        } else {
		    cp = NULL ;
		    cl = -1 ;

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

/* program-root */
	                case argopt_root:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pr = argp ;
			    } else
	            		rs = SR_INVALID ;
	                    break ;

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

/* argument file */
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

	                case argopt_tf:
			case argopt_dev:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            lip->termfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                lip->termfname = argp ;
				} else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

			case argopt_line:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
				    termline = argp ;
				}
			    } else
	              		rs = SR_INVALID ;
			    break ;

/* data-base */
			case argopt_db:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            lip->db = argp ;
			    } else
	          		rs = SR_INVALID ;
			    break ;

/* time-out */
	                case argopt_to:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            cp = avp ;
	                            cl = avl ;
	                        }
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                cp = argp ;
	                                cl = argl ;
	                            }
				} else
	                            rs = SR_INVALID ;
	                    }
	                    if ((rs >= 0) && (cp != NULL) && (cl > 0)) {
	                        rs = cfdecti(cp,cl,&v) ;
	                        pip->to_read = v ;
	                    }
	                    break ;

/* set ANSI confirmance (to ANSI Level 1) */
	                case argopt_ansi:
	                    lip->final.ansi = TRUE ;
	                    lip->f.ansi = TRUE ;
			    lip->ansi = 1 ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optvalue(avp,avl) ;
	                            lip->ansi = rs ;
	                        }
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

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* terminal-device */
	                    case 'd':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                lip->termfname = argp ;
				} else
	                            rs = SR_INVALID ;
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

	                    case 's':
	                        lip->f.set = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.set = (rs > 0) ;
	                            }
	                        }
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

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("b_termenq: args rs=%d\n",rs) ;
	    debugprintf("b_termenq: debuglevel=%u\n",pip->debuglevel) ;
	}
#endif

	if (rs < 0)
	    goto badarg ;

	if (f_version) {
	    shio_printf(pip->efp,"%s: version %s\n",pip->progname,VERSION) ;
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
	} /* end if */

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* statvfs initialization */

	if ((rs >= 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if (termline == NULL) {
	    termline = getourenv(envv,VARTERMLINE) ;
	}

	if (lip->db == NULL) {
	    lip->db = getourenv(envv,VARTERMDB) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_termenq: termline=%s\n",lip->termline) ;
#endif

	if ((rs = locinfo_setline(lip,termline,-1)) >= 0) {
	     KEYOPT	*kop = &akopts ;
	     rs = procopts(pip,kop) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_termenq: termline=%s\n",lip->termline) ;
#endif

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;
	ainfo.ai_continue = ai_continue ;

	if (rs >= 0) {
	    USERINFO	u ;
	    if ((rs = userinfo_start(&u,NULL)) >= 0) {
	        if ((rs = procuserinfo_begin(pip,&u)) >= 0) {
		    if ((rs = proglog_begin(pip,&u)) >= 0) {
			if ((rs = locinfo_termbegin(lip)) >= 0) {
			    {
			        ARGINFO	*aip = &ainfo ;
			        BITS	*bop = &pargs ;
	         	        cchar	*afn = afname ;
	     		        cchar	*ofn = ofname ;
	 		        rs = process(pip,aip,bop,ofn,afn) ;
				wlen = rs ;
			    }
			    rs1 = locinfo_termend(lip) ;
			    if (rs >= 0) rs = rs1 ;
			} /* end if (locinfo-term) */
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
	    ex = EX_USAGE ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	}

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        if (! pip->f.quiet) {
		    cchar	*pn = pip->progname ;
		    cchar	*fmt = "%s: invalid query (%d)\n" ;
	            shio_printf(pip->efp,fmt,pn,rs) ;
	        }
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
	} else if ((rs >= 0) && (ex == EX_OK)) {
	    if ((rs = lib_sigterm()) < 0) {
	        ex = EX_TERM ;
	    } else if ((rs = lib_sigintr()) < 0) {
	        ex = EX_INTR ;
	    }
	} else if ((rs >= 0) && (ex == EX_OK)) {
	    if (wlen == 0) ex = 1 ; /* "no-term" failure indication */
	} /* end if */

/* early return thing */
retearly:
	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("b_termenq: exiting ex=%u (%d)\n",ex,rs) ;
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
	    debugprintf("b_termenq: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* the bad things */
badarg:
	ex = EX_USAGE ;
	shio_printf(pip->efp,"%s: invalid argument specified (%d)\n",
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

	fmt = "%s: USAGE> %s [-l|-s]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [{-line <line>|-dev <termdev>}]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procopts(PROGINFO *pip,KEYOPT *kop)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		v ;
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

	            if ((oi = matostr(progopts,3,kp,kl)) >= 0) {

	                vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	                switch (oi) {
	                case progopt_poll:
	                    lip->f.poll = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        lip->f.poll = (rs > 0) ;
	                    }
	                    break ;
	                case progopt_intpoll:
	                case progopt_intpoller:
	                    if (vl > 0) {
	                        pip->have.intpoll = TRUE ;
	                        rs = cfdecti(vp,vl,&v) ;
	                        pip->intpoll = v ;
	                    }
	                    break ;
	                case progopt_logprog:
	                    if (! pip->final.logprog) {
	                        pip->have.logprog = TRUE ;
	                        pip->final.logprog = TRUE ;
	                        pip->f.logprog = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.logprog = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case progopt_latin1:
	                    if (! lip->final.latin1) {
	                        lip->have.latin1 = TRUE ;
	                        lip->final.latin1 = TRUE ;
	                        lip->f.latin1 = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.latin1 = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case progopt_ansi:
	                    if (! lip->final.ansi) {
	                        lip->have.ansi = TRUE ;
	                        lip->final.ansi = TRUE ;
	                        lip->f.ansi = TRUE ;
			        lip->ansi = 1 ;
	                        if (vl > 0) {
	                            rs = optvalue(vp,vl) ;
	                            lip->ansi = rs ;
	                        }
	                    }
	                    break ;
	                case progopt_toopen:
	                    if (vl > 0) {
	                        rs = cfdecti(vp,vl,&v) ;
	                        pip->to_open = v ;
	                    }
	                    break ;
	                case progopt_toread:
	                    if (vl > 0) {
	                        rs = cfdecti(vp,vl,&v) ;
	                        pip->to_read = v ;
	                    }
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


static int process(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *ofn,cchar *afn)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if ((rs = locinfo_utermbegin(lip)) >= 0) {
	    if (lip->f.set) {
	        if ((rs = procsetansi(pip)) >= 0) {
		    rs = procsetlatin1(pip) ;
	        }
	    } else {
	        rs = procargs(pip,aip,bop,ofn,afn) ;
		wlen = rs ;
	    }
	    rs1 = locinfo_utermend(lip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (locinfo-uterm) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (process) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *ofn,cchar *afn)
{
	SHIO		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		pan = 0 ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

	if ((rs = shio_open(ofp,ofn,"wct",0666)) >= 0) {
	    int		cl ;
	    cchar	*cp ;

	    if (rs >= 0) {
	        const int	ai_continue = aip->ai_continue ;
	        int		ai ;
	        int		f ;
	        for (ai = ai_continue ; ai < aip->argc ; ai += 1) {

	            f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	            f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	            if (f) {
	                cp = aip->argv[ai] ;
	                if (cp[0] != '\0') {
	                    pan += 1 ;
	                    rs = procspec(pip,ofp,cp,-1) ;
	                    wlen += rs ;
	                }
	            }

	            if (rs < 0) break ;
	        } /* end for (handling positional arguments) */
	    } /* end if (ok) */

	    if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	        SHIO	afile, *afp = &afile ;

	        if (strcmp(afn,"-") == 0)
	            afn = STDINFNAME ;

	        if ((rs = shio_open(afp,afn,"r",0666)) >= 0) {
	            const int	llen = LINEBUFLEN ;
	            int		len ;
	            char	lbuf[LINEBUFLEN + 1] ;

	            while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	                len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

	                if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                    if (cp[0] != '#') {
	                        pan += 1 ;
	                        rs = procspecs(pip,ofp,cp,cl) ;
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
	            if (! pip->f.quiet) {
			fmt = "%s: inaccessible argument-list (%d)\n" ;
	                shio_printf(pip->efp,fmt,pn,rs) ;
	                shio_printf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	            }
	        } /* end if */

	    } /* end if (processing file argument file list) */

	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    if (! pip->f.quiet) {
		fmt = "%s: inaccessible output (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
		fmt = "%s: ofile=%s\n" ;
	        shio_printf(pip->efp,fmt,pn,ofn) ;
	    }
	} /* end if */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procargs) */


static int procspecs(PROGINFO *pip,SHIO *ofp,cchar *sp,int sl)
{
	FIELD		fsb ;
	int		rs ;
	int		wlen = 0 ;

	if ((rs = field_start(&fsb,sp,sl)) >= 0) {
	    int		fl ;
	    cchar	*fp ;

	    while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
	        if (fl > 0) {
	            rs = procspec(pip,ofp,fp,fl) ;
	            wlen += rs ;
	        }
	        if (fsb.term == '#') break ;
	        if (rs < 0) break ;
	    } /* end while */

	    field_finish(&fsb) ;
	} /* end if (field) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procspecs) */


/* process a specification name */
static int procspec(PROGINFO *pip,SHIO *ofp,cchar *rp,int rl)
{
	int		rs = SR_OK ;
	int		ri ;
	int		vl = 0 ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	cchar		*tp ;
	cchar		*vp = rp ;

	if (rp == NULL) return SR_FAULT ;

	if (rl < 0) rl = strlen(rp) ;

	if ((tp = strnchr(rp,rl,'=')) != NULL) {
	    vp = (tp+1) ;
	    vl = ((rp+rl) - (tp+1)) ;
	    rl = (tp-rp) ;
	}

	if ((pip->debuglevel > 0) && (vp != NULL)) {
	    fmt = "%s: v=%t\n" ;
	    if (vl < 0) vl = strlen(vp) ;
	    if (vl > 0) {
	        shio_printf(pip->efp,fmt,pn,vp,vl) ;
	    }
	}

	if ((ri = matocasestr(qopts,2,rp,rl)) >= 0) {
	    if (pip->debuglevel > 0) {
		fmt = "%s: spec=%t (%d)\n" ;
	        shio_printf(pip->efp,fmt, pn,rp,rl,ri) ;
	    }
	    rs = procget(pip,ofp,ri) ;
	    wlen += rs ;
	} else {
	    if (pip->debuglevel > 0) {
		fmt = "%s: spec=%t notfound\n" ;
	        shio_printf(pip->efp,fmt, pn,rp,rl) ;
	    }
	    rs = SR_INVALID ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procspec) */


/* process a specification name */
static int procget(PROGINFO *pip,SHIO *ofp,int ri)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	switch (ri) {
	case qopt_answerback:
	    rs = procenq(pip,ofp) ;
	    wlen = rs ;
	    break ;
	case qopt_type:
	    rs = procdevattr(pip,ofp) ;
	    wlen = rs ;
	    break ;
	default:
	    rs = SR_INVALID ;
	    break ;
	} /* end switch */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_termenq/procget: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procget) */


/* set ANSI confirmance level */
static int procsetansi(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	if ((rs >= 0) && lip->f.ansi) {
	    const int	clen = CONBUFLEN ;
	    int		name = 'L' ;
	    char	cbuf[CONBUFLEN+1] ;
	    switch (lip->ansi) {
	    case 1:
		break ;
	    case 2:
		name = 'M' ;
		break ;
	    case 3:
		name = 'N' ;
		break ;
	    default:
		name = 0 ;
		break ;
	    } /* end switch */
	    if (name > 0) {
	        if ((rs = termescseq(cbuf,clen,name,' ',-1,-1,-1)) >= 0) {
		    rs = uc_write(lip->tfd,cbuf,rs,-1) ;
	        }
	    } /* end if (valid name) */
	} /* end if (ANSI confirmance level) */
	return rs ;
}
/* end subroutine (procsetansi) */


static int procsetlatin1(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	if ((rs >= 0) && lip->f.latin1) {
	    SBUF	b ;
	    const int	clen = CONBUFLEN ;
	    int		sl = 0 ;
	    char	cbuf[CONBUFLEN+1] ;
	    if ((rs = sbuf_start(&b,cbuf,clen)) >= 0) {
		sbuf_strw(&b,"\033-A",-1) ; /* ISO-Latin-1 to G1 */
		sbuf_strw(&b,"\033~",-1) ; /* G1 lock to GR */
		sl = sbuf_finish(&b) ;
	        if (rs >= 0) rs = sl ;
	    } /* end if (sbuf) */
	    if (rs >= 0) {
		rs = uc_write(lip->tfd,cbuf,sl,-1) ;
	    }
	} /* end if (ANSI confirmance level) */
	return rs ;
}
/* end subroutine (procsetlatin1) */


static int procenq(PROGINFO *pip,SHIO *ofp)
{
	LOCINFO		*lip = pip->lip ;
	const int	clen = CONBUFLEN ;
	int		rs ;
	int		cl = 1 ;
	char		cbuf[CONBUFLEN+1] ;
	cbuf[0] = CH_ENQ ;
	if ((rs = locinfo_utermwrite(lip,cbuf,cl)) >= 0) {
	     TERMCMD	c ;
	     if ((rs = locinfo_utermread(lip,&c,cbuf,clen)) >= 0) {
		cl = rs ;
#if	CF_DEBUG
		if (DEBUGLEVEL(4)) {
	            TERMCMD	*cmdp = &c ;
		    const int	max = COLUMNS ;
	            int		i ;
		    cchar	*ids = "termenq/procenq" ;
	            debugprintf("termenq/procenq: rs=%d\n",rs) ;
	            debugprintf("termenq/procenq: ab=>%t<\n",cbuf,cl) ;
		    debugprinthexblock(ids,max,cbuf,cl) ;
	            debugprintf("termenq/procenq: type=%u\n",
				cmdp->type) ;
	            debugprintf("termenq/procenq: name=%04x\n",
				cmdp->name) ;
	            debugprintf("termenq/procenq: parameters¬\n") ;
	   	    for (i = 0 ; i < TERMCMD_NP ; i += 1) {
			    if (cmdp->p[i] == TERMCMD_PEOL) break ;
	        	    debugprintf("termenq/procenq: "
			        "p[%2u]=%04x(%u)\n",i,cmdp->p[i],cmdp->p[i]) ;
		    } /* end for */
		}
#endif /* CF_DEBUG */
		if (rs > 0) {
		    rs = shio_println(ofp,cbuf,cl) ;
		}
	     } /* end if (locinfo_utermread) */
	} /* end if (locinfo_utermwrite) */
	return (rs >= 0) ? cl : rs ;
}
/* end subroutine (procenq) */


int procdevattr(PROGINFO *pip,SHIO *ofp)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("termenq/procdevattr: ent\n") ;
#endif

	if (rs >= 0) {
	    TERMCMD	pcmd ;
	    const int	plen = CONBUFLEN ;
	    char	pbuf[CONBUFLEN+1] ;
	    if ((rs = procdevattr_pri(pip,ofp,&pcmd,pbuf,plen)) >= 0) {
	        TERMCMD		scmd ;
	        const int	slen = CONBUFLEN ;
	        char		sbuf[CONBUFLEN+1] ;
	        if ((rs = procdevattr_sec(pip,ofp,&scmd,sbuf,slen)) >= 0) {
		    const int	tlen = TBUFLEN ;
		    char	tbuf[TBUFLEN+1] ;
		    if ((rs = termtype_vt(tbuf,tlen,pcmd.p,scmd.p)) > 0) {
			rs = shio_println(ofp,tbuf,rs) ;
			wlen += rs ;
		    }
	        }
	    } else if (rs == SR_TIMEDOUT) {
#if	CF_ID
		rs = procdevattr_id(pip,ofp,cbuf,clen) ;
#else /* CF_ID */
		rs = 0 ;
#endif /* CF_ID */
	    }
	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("termenq/procdevattr: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procdevattr) */


static int procdevattr_pri(PROGINFO *pip,SHIO *ofp,TERMCMD *cmdp,
		char *cbuf,int clen)
{
	LOCINFO		*lip = pip->lip ;
	const int	name = 'c' ;
	int		rs ;
	int		len = 0 ;
	if ((rs = termconseq(cbuf,clen,name,-1,-1,-1,-1)) >= 0) {
	    if ((rs = locinfo_utermwrite(lip,cbuf,rs)) >= 0) {
	        if ((rs = locinfo_utermread(lip,cmdp,cbuf,clen)) >= 0) {
		    len = rs ;
#if	CF_DEBUG
	            if (DEBUGLEVEL(4)) {
	                int	i ;
	                debugprintf("termenq/procdevattr_pri: type=%u\n",
				cmdp->type) ;
	                debugprintf("termenq/procdevattr_pri: name=%04x\n",
				cmdp->name) ;
	                debugprintf("termenq/procdevattr_pri: parameters¬\n") ;
	    		for (i = 0 ; i < TERMCMD_NP ; i += 1) {
			    if (cmdp->p[i] == TERMCMD_PEOL) break ;
	        	    debugprintf("termenq/procdevattr_pri: "
			        "p[%2u]=%04x(%u)\n",i,cmdp->p[i],cmdp->p[i]) ;
	    	        } /* end for */
		    }
#endif /* CF_DEBUG */
	        } /* end if (lockinfo_utermread) */
	    } /* end if (lockinfo_utermwrite) */
	} /* end if (termconseq) */
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (procdevattr_pri) */


static int procdevattr_sec(PROGINFO *pip,SHIO *ofp,TERMCMD *cmdp,
		char *cbuf,int clen)
{
	LOCINFO		*lip = pip->lip ;
	const int	name = 'c' ;
	int		rs ;
	int		len = 0 ;
	cchar		*is = ">" ;
	if ((rs = termconseqi(cbuf,clen,name,is,-1,-1,-1,-1)) >= 0) {
	    if ((rs = locinfo_utermwrite(lip,cbuf,rs)) >= 0) {
	        if ((rs = locinfo_utermread(lip,cmdp,cbuf,clen)) >= 0) {
		    len = rs ;
#if	CF_DEBUG
	            if (DEBUGLEVEL(4)) {
	                int	i ;
	                debugprintf("termenq/procdevattr_sec: type=%u\n",
				cmdp->type) ;
	    		debugprintf("termenq/procdevattr_sec: name=%04x\n",
				cmdp->name) ;
	    		debugprintf("termenq/procdevattr_sec: parameters¬\n") ;
	    		for (i = 0 ; i < TERMCMD_NP ; i += 1) {
			    if (cmdp->p[i] == TERMCMD_PEOL) break ;
	        	    debugprintf("termenq/procdevattr_sec: "
				"p[%2u]=%04x(%u)\n",i,cmdp->p[i],cmdp->p[i]) ;
	    	        } /* end for */
		    }
#endif /* CF_DEBUG */
	        } /* end if (locinfo_utermread) */
	    } /* end if (locinfo_utermwrite) */
	} /* end if (termconseqi) */
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (procdevattr_sec) */


#if	CF_ID
static int procdevattr_id(PROGINFO *pip,SSHIO *ofp,char *cbuf,int clen)
{
	int		rs ;
	int		len = 0 ;
	if ((rs = sncpy1(cbuf,clen,"\033Z")) >= 0) {
	    if ((rs = locinfo_utermwrite(lip,cbuf,rs)) >= 0) {
	        TERMCMD		c ;
	        if ((rs = locinfo_utermread(lip,&c,cbuf,clen)) >= 0) {
		    len = rs ;
#if	CF_DEBUG
		    if (DEBUGLEVEL(4)) {
	    	        TERMCMD	*cmdp = &c ;
	    	        int	i ;
	    	        debugprintf("termenq/procdevattr_proc: type=%u\n",
			    cmdp->type) ;
	    	        debugprintf("termenq/procdevattr_proc: name=%04x\n",
			    cmdp->name) ;
	    	        debugprintf("termenq/procdevattr_proc: parameters¬\n") ;
	    	        for (i = 0 ; i < TERMCMD_NP ; i += 1) {
			    if (cmdp->p[i] == TERMCMD_PEOL) break ;
	        	    debugprintf("termenq/procdevattr_proc: "
				"p[%2u]=%04x(%u)\n",i,cmdp->p[i],cmdp->p[i]) ;
	    	        } /* end for */
		    }
#endif /* CF_DEBUG */
	        } /* end if (locinfo_utermread) */
	    } /* end if (locinfo_utermwrite) */
	} /* end if (termconseq) */
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (procdevattr_id) */
#endif /* CF_ID */


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

	if (lip == NULL) return SR_FAULT ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->to = -1 ;
#if	CF_STDIN
	lip->tfd = FD_STDIN ;
#else
	lip->tfd = FD_STDOUT ;
#endif

	return SR_OK ;
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


static int locinfo_termbegin(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	const mode_t	om = 0666 ;
	const int	of = (O_RDWR|O_NOCTTY) ;
	int		rs = SR_OK ;
	if (lip->termfname != NULL) {
	    cchar	*fn = lip->termfname ;
	    if ((rs = u_open(fn,of,om)) >= 0) {
	        lip->tfd = rs ;
	        lip->f.opened = TRUE ;
	    }
	} else {
	    const int	rlen = MAXPATHLEN ;
	    char	rbuf[MAXPATHLEN+1] ;
	    if ((rs = getutmpterm(rbuf,rlen,0)) >= 0) {
		const int	rl = rs ;
	        if ((rs = u_open(rbuf,of,om)) >= 0) {
		    cchar	**vpp = &lip->termfname ;
		    lip->tfd = rs ;
	            lip->f.opened = TRUE ;
	    	    rs = locinfo_setentry(lip,vpp,rbuf,rl) ;
		}
	    }
	}
	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: termdev=%s\n" ;;
	    shio_printf(pip->efp,fmt,pn,lip->termfname) ;
	}
	return rs ;
}
/* end subroutine (locinfo_termbegin) */


static int locinfo_termend(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (lip->f.opened && (lip->tfd >= 0)) {
	    rs1 = u_close(lip->tfd) ;
	    if (rs >= 0) rs = rs1 ;
	    lip->tfd = -1 ;
	    lip->f.opened = FALSE ;
	}
	return rs ;
}
/* end subroutine (locinfo_termend) */


static int locinfo_setline(LOCINFO *lip,cchar *argp,int argl)
{
	int		rs = SR_OK ;
	if (argp != NULL) {
	    if (lip->termfname == NULL) {
	        int	m ;
	        cchar	*dev = "/dev/" ;
	        cchar	**vpp ;
	        if ((m = nleadstr(dev,argp,argl)) >= 6) {
	            vpp = &lip->termfname ;
	            rs = locinfo_setentry(lip,vpp,argp,argl) ;
	            argp += m ;
	            argl -= m ;
	        }
	        if (rs >= 0) {
		    char	tbuf[MAXNAMELEN+1] ;
		    if ((rs = mkpath2w(tbuf,dev,argp,argl)) >= 0) {
		        cchar	**vpp = &lip->termfname ;
		        rs = locinfo_setentry(lip,vpp,tbuf,rs) ;
		    }
	        }
	    } /* end if */
	} /* end if */
	return rs ;
}
/* end subroutine (locinfo_setline) */


static int locinfo_utermbegin(LOCINFO *lip)
{
	int		rs = SR_OK ;
	if (! lip->open.ut) {
	    UTERM	*utp = &lip->ut ;
	    const int	tfd = lip->tfd ;
	    if ((rs = uterm_start(utp,tfd)) >= 0) {
		lip->open.ut = TRUE ;
	    }
	}
	return rs ;
}
/* end subroutine (locinfo_utermbegin) */


static int locinfo_utermend(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (lip->open.ut) {
	    UTERM	*utp = &lip->ut ;
	    lip->open.ut = TRUE ;
	    rs1 = uterm_finish(utp) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (locinfo_utermend) */


static int locinfo_utermread(LOCINFO *lip,TERMCMD *cmdp,char *rbuf,int rlen)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		len = 0 ;
	rbuf[0] = '\0' ;
	termcmd_clear(cmdp) ;
	if (lip->open.ut) {
	    UTERM	*utp = &lip->ut ;
	    const int	ro = UTOPTS ;
	    const int	to = pip->to_read ;
	    if ((rs = uterm_reade(utp,rbuf,rlen,to,ro,NULL,NULL)) > 0) {
		int	si ;
		len = rs ;
	        if ((si = sicmdstart(rbuf,len)) >= 0) {
		    const int	ich = rbuf[si] ;
		    len = si ;
	            rs = uterm_readcmd(utp,cmdp,to,ich) ;
		}
	    }
	}
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_utermread) */


static int locinfo_utermwrite(LOCINFO *lip,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	if (lip->open.ut) {
	    UTERM	*utp = &lip->ut ;
	    rs = uterm_write(utp,sp,sl) ;
	}
	return rs ;
}
/* end subroutine (locinfo_utermwrite) */


static int sicmdstart(cchar *rbuf,int rlen)
{
	int		i ;
	int		f = FALSE ;
	for (i = 0 ; (i < rlen) ; i += 1) {
	    const int	cmd = MKCHAR(rbuf[i]) ;
	    f = iscmdstart(cmd) ;
	    if (f) break ;
	}
	return (f) ? i : -1 ;
}
/* end subroutine (sicmdstart) */


