/* b_logadm */

/* SHELL built-in for user login-administration */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_PERCACHE	1		/* use persistent cache */
#define	CF_LOCUTMPENT	0		/* using |locinfo_utmpent()| */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	This subroutine was originally written.  

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ logadm [-u <username>] [-y|-n] [-h <host>] [<spec(s)>]


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
#include	<sys/stat.h>
#include	<sys/mkdev.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<utmpx.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<ctdec.h>
#include	<tmpx.h>
#include	<field.h>
#include	<getutmpent.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_logadm.h"
#include	"defs.h"


/* local defines */

#define	CVTBUFLEN	100
#define	UTMPIDLEN	5

#ifndef	TTYDEV
#define	TTYDEV		"/dev/tty"
#endif

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	snwcpy(char *,int,cchar *,int) ;
extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecti(cchar *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	bufprintf(char *,int,cchar *,...) ;
extern int	logout(pid_t) ;
extern int	mkutmpid(char *,int,cchar *,int) ;
extern int	termdevice(char *,int,int) ;
extern int	getusername(char *,int,uid_t) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		yes:1 ;
	uint		no:1 ;
	uint		host:1 ;
	uint		termline:1 ;
	uint		session:1 ;
	uint		utmpent:1 ;
	uint		username:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	init ;
	PROGINFO	*pip ;
	cchar		*utfname ;
	cchar		*hostname ;
	cchar		*termline ;
	GETUTMPENT	ue ;
	pid_t		sid ;
	int		session ;
	int		to ;		/* time-out? */
	char		username[USERNAMELEN+1] ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_utfname(LOCINFO *,cchar *) ;
static int	locinfo_to(LOCINFO *,int) ;
static int	locinfo_username(LOCINFO *) ;
static int	locinfo_finish(LOCINFO *) ;

#if	CF_LOCUTMPENT
static int	locinfo_utmpent(LOCINFO *) ;
#endif

static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;
static int	procspecs(PROGINFO *,void *,cchar *,int) ;
static int	procspec(PROGINFO *,void *, cchar *,int) ;
static int	procout(PROGINFO *,SHIO *,cchar *,int) ;
static int	proclogin(PROGINFO *) ;


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
	"utf",
	"db",
	"pid",
	"sid",
	"ses",
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
	argopt_utf,
	argopt_db,
	argopt_pid,
	argopt_sid,
	argopt_ses,
	argopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRNAME
} ;

static const struct mapex	mapexs[] = {
	{ SR_NOENT, EX_NOUSER },
	{ SR_PERM, EX_NOPERM },
	{ SR_AGAIN, EX_TEMPFAIL },
	{ SR_DEADLK, EX_TEMPFAIL },
	{ SR_NOLCK, EX_TEMPFAIL },
	{ SR_TXTBSY, EX_TEMPFAIL },
	{ SR_ACCESS, EX_NOPERM },
	{ SR_REMOTE, EX_PROTOCOL },
	{ SR_NOSPC, EX_TEMPFAIL },
	{ SR_ISDIR, EX_NOINPUT },
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;

/* define the configuration keywords */
static cchar *qopts[] = {
	"line",
	"name",
	"user",
	"id",
	"session",
	"ses",
	"host",
	"time",
	"date",
	"pid",
	"sid",
	NULL
} ;

enum qopts {
	qopt_line,
	qopt_name,
	qopt_user,
	qopt_id,
	qopt_session,
	qopt_ses,
	qopt_host,
	qopt_time,
	qopt_date,
	qopt_pid,
	qopt_sid,
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


int b_logadm(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_logadm) */


int p_logadm(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_logadm) */


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
	int		to = -1 ;
	int		v ;
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
	cchar		*utfname = NULL ;
	cchar		*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_logadm: starting DFD=%d\n",rs) ;
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

/* keyword match or only key letters? */

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

/* UTMP file */
	                case argopt_utf:
	                case argopt_db:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            utfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            utfname = argp ;
				} else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

			case argopt_sid:
	                        if (argr > 0) {
	                    argp = argv[++ai] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;
	                    if (argl) {
	                        rs = optvalue(argp,argl) ;
				lip->sid = rs ;
			    }
				} else
	                            rs = SR_INVALID ;
	                    break ;

/* session */
			case argopt_ses:
	                        lip->have.session = TRUE ;
	                        lip->final.session = TRUE ;
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = optvalue(argp,argl) ;
	                            lip->session = rs ;
	                        }
				} else
	                            rs = SR_INVALID ;
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

/* host */
	                    case 'h':
	                        lip->have.host = TRUE ;
	                        lip->f.host = TRUE ;
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            lip->hostname = argp ;
				} else
	                            rs = SR_INVALID ;
	                        break ;

/* termline */
	                    case 'l':
	                        lip->have.termline = TRUE ;
	                        lip->f.termline = TRUE ;
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            lip->termline = argp ;
				} else
	                            rs = SR_INVALID ;
	                        break ;

/* no */
	                    case 'n':
	                        lip->have.no = TRUE ;
	                        lip->f.no = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.no = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* yes */
	                    case 'y':
	                        lip->have.yes = TRUE ;
	                        lip->f.yes = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.yes = (rs > 0) ;
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

/* proces (session) ID */
	                    case 'p':
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = optvalue(argp,argl) ;
	                            lip->sid = rs ;
				}
				} else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* session */
	                    case 's':
	                        lip->have.session = TRUE ;
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = optvalue(argp,argl) ;
	                            lip->session = rs ;
	                        }
				} else
	                            rs = SR_INVALID ;
	                        break ;

/* time-out */
	                    case 't':
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = cfdecti(argp,argl,&v) ;
	                            to = v ;
	                        }
				} else
	                            rs = SR_INVALID ;
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

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_logadm: debuglevel=%u\n",pip->debuglevel) ;
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
	} /* end if */

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* initialization */

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if (utfname == NULL) utfname = getourenv(envv,VARUTFNAME) ;

	if ((argval != NULL) && (lip->sid == 0)) {
	    rs = optvalue(argval,-1) ;
	    lip->sid = rs ;
	}

	locinfo_utfname(lip,utfname) ;

	locinfo_to(lip,to) ;

	if ((rs >= 0) && lip->f.yes) {
	    if (lip->f.no) {
	        shio_printf(pip->efp,"%s: invalid argument combination\n",
	            pip->progname) ;
	        goto badargcombo ;
	    }
	    rs1 = getutmpent(&lip->ue,lip->sid) ;
	    if (rs1 == SR_NOENT) {
	        rs = proclogin(pip) ;
	        if (! pip->f.quiet) {
	            if (rs == SR_PERM) {
	                shio_printf(pip->efp,
	                    "%s: no permission for action (%d)\n",
	                    pip->progname,rs) ;
	            } else
	                shio_printf(pip->efp,
	                    "%s: could not log in (%d)\n",
	                    pip->progname,rs) ;
	        } /* end if (not quiet) */
	    } else {
	        ex = EX_PROTOCOL ; /* bad protocol to log in when already */
	        if (! pip->f.quiet)
	            shio_printf(pip->efp,"%s: already logged in (%u)\n",
	                pip->progname,rs1) ;
	    }
	    goto done ;
	} /* end if (yes) */

	if ((rs >= 0) && lip->f.no) {
	    rs = logout(-1) ;
	    if (! pip->f.quiet) {
	        if (rs == SR_PERM) {
	            shio_printf(pip->efp,"%s: no permission for action (%d)\n",
	                pip->progname,rs) ;
	        } else if (rs == SR_NOENT) {
	            ex = EX_NOUSER ;
	            shio_printf(pip->efp,"%s: not logged in (%d)\n",
	                pip->progname,rs) ;
	        }
	    } /* end if (not quiet) */
	    goto done ;
	}

/* continue with default action */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    if ((rs = getutmpent(&lip->ue,lip->sid)) >= 0) {
	        cchar	*afn = afname ;
	        cchar	*ofn = ofname ;
	        rs = procargs(pip,&ainfo,&pargs,ofn,afn) ;
	    } else {
	        if (! pip->f.quiet) {
	            if (rs == SR_NOENT) {
	                ex = EX_NOUSER ;
	                shio_printf(pip->efp,
	                    "%s: not logged in (%d)\n",
	                    pip->progname,rs) ;
	            } else {
	                shio_printf(pip->efp,
	                    "%s: error accessing data-base (%d)\n",
	                    pip->progname,rs) ;
		    }
	        } /* end if (not quiet) */
	    } /* end if */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    ex = EX_USAGE ;
	    usage(pip) ;
	} /* end if (ok) */

/* finish */
badargcombo:
done:
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        if (! pip->f.quiet) {
	            shio_printf(pip->efp,"%s: invalid query (%d)\n",
	                pip->progname,rs) ;
		}
	        break ;
	    case SR_ISDIR:
	        ex = EX_NOINPUT ;
	        if (! pip->f.quiet) {
	            shio_printf(pip->efp,"%s: could not find terminal (%d)\n",
	                pip->progname,rs) ;
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
	} else if (rs >= 0) {
	    if ((rs = lib_sigterm()) < 0) {
	        ex = EX_TERM ;
	    } else if ((rs = lib_sigintr()) < 0) {
	        ex = EX_INTR ;
	    }
	} /* end if */

/* early return thing */
retearly:
	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_logadm: exiting ex=%u (%d)\n",ex,rs) ;
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
	    debugprintf("b_logadm: final mallout=%u\n",(mo-mo_start)) ;
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
	int		i ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s [-y|-n] [-q] [<queries> ...] [-af <afile>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-p <sid>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-l <termline>] [-h <host>] [-ses <ses>] [-db <utmpx>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:   possible specifications are: \n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	for (i = 0 ; (rs >= 0) && (qopts[i] != NULL) ; i += 1) {

	    if ((i % USAGECOLS) == 0) {
	        rs = shio_printf(pip->efp,"%s: \t",pn) ;
	        wlen += rs ;
	    }

	    if (rs >= 0) {
	        rs = shio_printf(pip->efp,"%-16s",qopts[i]) ;
	        wlen += rs ;
	        if ((rs >= 0) && ((i % USAGECOLS) == 3)) {
	            rs = shio_printf(pip->efp,"\n") ;
	            wlen += rs ;
		}
	    }

	} /* end for */

	if ((rs >= 0) && ((i % USAGECOLS) != 0)) {
	    rs = shio_printf(pip->efp,"\n") ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{

	if (lip == NULL)
	    return SR_FAULT ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->to = -1 ;
	lip->sid = getsid(0) ;

	return SR_OK ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{

	if (lip == NULL)
	    return SR_FAULT ;

	return SR_OK ;
}
/* end subroutine (locinfo_finish) */


static int locinfo_utfname(LOCINFO *lip,cchar *utfname)
{

	if (lip == NULL)
	    return SR_FAULT ;

	lip->utfname = utfname ;
	return SR_OK ;
}
/* end subroutine (locinfo_utfname) */


static int locinfo_to(LOCINFO *lip,int to)
{

	if (to < 0) to = TO_CACHE ;

	lip->to = to ;
	return SR_OK ;
}
/* end subroutine (locinfo_to) */


#if	CF_LOCUTMPENT
static int locinfo_utmpent(LOCINFO *lip)
{
	int		rs = SR_OK ;

	if (lip == NULL) return SR_FAULT ;

	if (! lip->init.utmpent) {
	    lip->init.utmpent = TRUE ;
	    rs = getutmpent(&lip->ue,lip->sid) ;
	    lip->have.utmpent = (rs >= 0) ;
	}

	return rs ;
}
/* end subroutine (locinfo_utmpent) */
#endif /* CF_LOCUTMPENT */


static int locinfo_username(LOCINFO *lip)
{
	int		rs = SR_OK ;

	if (lip == NULL)
	    return SR_FAULT ;

	if (! lip->init.username) {
	    lip->init.username = TRUE ;
	    rs = getusername(lip->username,USERNAMELEN,-1) ;
	    lip->have.username = (rs >= 0) ;
	}

	return rs ;
}
/* end subroutine (locinfo_username) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *ofn,cchar *afn)
{
	LOCINFO		*lip = pip->lip ;
	SHIO		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

	if ((rs = shio_open(ofp,ofn,"wct",0666)) >= 0) {
	    int		pan = 0 ;
	    int		cl ;
	    cchar	*cp ;

	    if (rs >= 0) {
		int	ai ;
		int	f ;
		for (ai = 1 ; ai < aip->argc ; ai += 1) {

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
		    if (rs >= 0) rs = lib_sigterm() ;
		    if (rs >= 0) rs = lib_sigintr() ;
	            if (rs < 0) break ;
	        } /* end for (handling positional arguments) */
	    } /* end if (ok) */

	    if ((rs >= 0) && (afn!= NULL) && (afn[0] != '\0')) {
	        SHIO	afile, *afp = &afile ;

	        if (strcmp(afn,"-") == 0)
	            afn = STDINFNAME ;

	        if ((rs = shio_open(afp,afn,"r",0666)) >= 0) {
		    const int	llen = LINEBUFLEN ;
	            char	lbuf[LINEBUFLEN + 1] ;

	            while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	                int	len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

			if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
			    if (cp[0] != '#') {
	                	pan += 1 ;
	                	rs = procspecs(pip,ofp,cp,cl) ;
			    	wlen += rs ;
			    }
			}

	                if (rs < 0) break ;
	            } /* end while (reading lines) */

	            rs1 = shio_close(afp) ;
	            if (rs >= 0) rs = rs1 ;
	        } else {
	            if (! pip->f.quiet) {
	                shio_printf(pip->efp,
	                    "%s: inaccessible argument-list (%d)\n",
	                    pip->progname,rs) ;
	                shio_printf(pip->efp,"%s: afile=%s\n",
	                    pip->progname,afn) ;
	            }
	        } /* end if */

	    } /* end if (processing file argument file list) */

	    if ((rs >= 0) && (pan == 0)) {
	        if (pip->verboselevel > 0)
	            shio_printf(ofp,"logged in (%u)\n",lip->sid) ;
	    }

	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    shio_printf(pip->efp,"%s: inaccessible output (%d)\n",
		pip->progname,rs) ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procargs) */


static int procspecs(PROGINFO *pip,void *ofp,cchar *lbuf,int len)
{
	FIELD		fsb ;
	int		rs ;
	int		wlen = 0 ;
	if ((rs = field_start(&fsb,lbuf,len)) >= 0) {
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
static int procspec(PROGINFO *pip,void *ofp,cchar qp[],int ql)
{
	LOCINFO		*lip = pip->lip ;
	GETUTMPENT	*utp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		qi ;			/* query index */
	int		sl = -1 ;
	int		wlen = 0 ;
	cchar		*sp = NULL ;
	char		cvtbuf[CVTBUFLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_logadm/procspec: query=>%t<\n",qp,ql) ;
#endif

	utp = &lip->ue ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_logadm/procspec: q=%t\n",qp,ql) ;
#endif

	cvtbuf[0] = '\0' ;
	qi = matostr(qopts,2,qp,ql) ;

	if (pip->debuglevel > 0)
	    shio_printf(pip->efp,"%s: query=%t (%d)\n",
	        pip->progname,qp,ql,qi) ;

	switch (qi) {
	case qopt_id:
	    sp = cvtbuf ;
	    sl = sncpy1(cvtbuf,CVTBUFLEN,utp->id) ;
	    break ;
	case qopt_line:
	    sp = cvtbuf ;
	    sl = sncpy1(cvtbuf,CVTBUFLEN,utp->line) ;
	    break ;
	case qopt_host:
	    sp = cvtbuf ;
	    sl = sncpy1(cvtbuf,CVTBUFLEN,utp->host) ;
	    break ;
	case qopt_name:
	case qopt_user:
	    sp = cvtbuf ;
	    sl = sncpy1(cvtbuf,CVTBUFLEN,utp->user) ;
	    break ;
	case qopt_session:
	case qopt_ses:
	    sp = cvtbuf ;
	    rs = ctdeci(cvtbuf,CVTBUFLEN,utp->session) ;
	    sl = rs ;
	    break ;
	case qopt_date:
	case qopt_time:
	    sp = cvtbuf ;
	    timestr_log(utp->date,cvtbuf) ;
	    break ;
	case qopt_pid:
	case qopt_sid:
	    sp = cvtbuf ;
	    rs = ctdeci(cvtbuf,CVTBUFLEN,utp->sid) ;
	    sl = rs ;
	    break ;
	default:
	    rs = SR_INVALID ;
	    break ;
	} /* end switch */

	if ((rs >= 0) && (pip->verboselevel > 0)) {
	    rs1 = procout(pip,ofp,sp,sl) ;
	    wlen += rs1 ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_logadm/procspec: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procspec) */


static int procout(PROGINFO *pip,SHIO *ofp,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (pip->verboselevel > 0) {
	    if (sp == NULL) {
	        sp = "*" ;
	        sl = 1 ;
	    }
	    rs = shio_printf(ofp,"%t\n",sp,sl) ;
	    wlen += rs ;
	} /* end if */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procout) */


static int proclogin(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	const int	dlen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		fd ;
	int		ll = -1 ;
	char		dbuf[MAXPATHLEN+1] ;
	char		utmpid[UTMPIDLEN+1] ;
	cchar		*lp ;

/* terminal line */

	if (lip->termline == NULL) {
	    SHIO	ifile, *ifp = &ifile ;
	    if ((rs = shio_open(ifp,STDINFNAME,"rw",0666)) >= 0) {
	        if ((rs = shio_getfd(ifp)) >= 0) {
	            fd = rs ;
	            if (isatty(fd)) {

	                lp = dbuf ;
	                rs = termdevice(dbuf,dlen,fd) ;
	                ll = rs ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(3))
	                    debugprintf("main/proclogin: termdev=%s\n",dbuf) ;
#endif

	                if (strncmp(dbuf,"/dev/",5) == 0) {
	                    lp += 5 ;
	                    ll -= 5 ;
	                }

	            } else
	                rs = SR_ISDIR ;
	        } /* end if (get-fd) */
	        rs1 = shio_close(ifp) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (open) */
	} else
	    lp = lip->termline ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/proclogin: termline=%t\n",lp,ll) ;
#endif

/* UTMP ID */

	if (rs >= 0)
	    rs = mkutmpid(utmpid,UTMPIDLEN,lp,ll) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/proclogin: mkutmpid() rs=%d utmpid=%s\n",
	        rs,utmpid) ;
#endif

/* username (for logname) */

	if (rs >= 0)
	    rs = locinfo_username(lip) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/proclogin: username=%s\n",lip->username) ;
#endif

/* assemble everything */

	if (rs >= 0) {
	    struct utmpx	uc, *up ;

	    strncpy(uc.ut_id,utmpid,TMPX_LID) ;

	    strncpy(uc.ut_line,lp,TMPX_LLINE) ;

	    strncpy(uc.ut_user,lip->username,TMPX_LUSER) ;

	    if (lip->hostname != NULL) {
	        int	cl ;
	        strncpy(uc.ut_host,lip->hostname,TMPX_LHOST) ;

	        cl = strnlen(lip->hostname,TMPX_LHOST) ;

#if	defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)
	        uc.ut_host[cl] = '\0' ;
#endif

	        uc.ut_syslen = (cl+1) ;

	    } else {
	        uc.ut_host[0] = '\0' ;
	        uc.ut_syslen = 0 ;
	    }

	    gettimeofday(&uc.ut_tv,NULL) ;

	    uc.ut_pid = lip->sid ;
	    uc.ut_session = 0 ;
	    uc.ut_exit.e_termination = 0 ;
	    uc.ut_exit.e_exit = 0 ;
	    uc.ut_type = TMPX_TUSERPROC ;

	    up = pututxline(&uc) ;
	    if (up == NULL) rs = SR_PERM ;

	} /* end if */

	return rs ;
}
/* end subroutine (proclogin) */


