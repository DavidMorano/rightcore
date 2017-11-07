/* main */

/* program to get time from a network time server host */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_SIGHAND	0		/* use "sigmain()| */
#define	CF_NETTIME	1		/* go w/ |nettime()| */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This program will get the time-of-day from a time server specified by a
	hostname given on the command line.  The program tries to connect to a
	TCP listener on the time server and will read 4 bytes out of the
	socket.  These four bytes, when organized as a long word in network
	byte order, represent the time in seconds since Jan 1, 1900.  We will
	subtract the value "86400 * ((365 * 70) + 17)" to get the time in
	seconds since Jan 1, 1970 (which is the UNIX epoch).


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<sys/time.h>
#include	<netinet/in.h>
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>
#include	<time.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<sighand.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<field.h>
#include	<bfile.h>
#include	<userinfo.h>
#include	<getpe.h>
#include	<vecobj.h>
#include	<logsys.h>
#include	<logfile.h>
#include	<ucmallreg.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"nettime.h"


/* local defines */


/* external subroutines */

extern int	snwcpy(char *,int,cchar *,int) ;
extern int	mkpath1w(char *,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	sfshrink(cchar *,int,cchar **) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecti(cchar *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	getaf(cchar *,int) ;
extern int	bufprintf(char *,int,cchar *,...) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

extern int	progout_begin(PROGINFO *,cchar *) ;
extern int	progout_printline(PROGINFO *,cchar *,int) ;
extern int	progout_printf(PROGINFO *,cchar *,...) ;
extern int	progout_end(PROGINFO *) ;

extern int	progadjust(PROGINFO *,struct timeval *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugprinthex(cchar *,int,cchar *,int) ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;

#if	CF_SIGHAND
static void	main_sighand(int,siginfo_t *,void *) ;
#endif


/* external variables */


/* local structures */

struct weights {
	double		w ;		/* calculated weight */
	int64_t		moff ;		/* offset in msecs */
	int64_t		mtrip ;		/* trip interval in msecs */
} ;


/* forward references */

static int	usage(PROGINFO *) ;

static int	loadproto(PROGINFO *,cchar *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,vecobj *,cchar *) ;
static int	procservers(PROGINFO *,vecobj *,cchar *,int) ;
static int	procserver(PROGINFO *,vecobj *,cchar *,int) ;
static int	process(PROGINFO *,vecobj *) ;
static int	procwtab(PROGINFO *,struct timeval *,struct weights *,int) ;

static int	procuserinfo_begin(PROGINFO *,USERINFO *) ;
static int	procuserinfo_end(PROGINFO *) ;

static int	proclogsys_begin(PROGINFO *) ;
static int	proclogsys_end(PROGINFO *) ;

static int	tv_getmsec(struct timeval *,int64_t *) ;
static int	tv_loadmsec(struct timeval *,int64_t) ;

static int	mkoffstr(char *,int,int64_t) ;


/* local variables */

static volatile int	if_exit ;
static volatile int	if_int ;

static const int	sigblocks[] = {
	SIGUSR1,
	SIGUSR2,
	SIGHUP,
	SIGCHLD,
	0
} ;

static const int	sigignores[] = {
	SIGPIPE,
	SIGPOLL,
#if	defined(SIGXFSZ)
	SIGXFSZ,
#endif
	0
} ;

static const int	sigints[] = {
	SIGINT,
	SIGTERM,
	SIGQUIT,
	0
} ;

static cchar	*argopts[] = {
	"ROOT",
	"TMPDIR",
	"VERSION",
	"VERBOSE",
	"CONFIG",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	"lf",
	"svc",
	"port",
	"proto",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_tmpdir,
	argopt_version,
	argopt_verbose,
	argopt_config,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_lf,
	argopt_svc,
	argopt_port,
	argopt_proto,
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

static cchar	*akonames[] = {
	"test",
	"logsys",
	"logfile",
	NULL
} ;

enum akonames {
	akoname_test,
	akoname_logsys,
	akoname_logfile,
	akoname_overlast
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


/* define the configuration keywords */


/* module-scope variables */

#ifdef	COMMENT
static int	if_timeout ;
#endif


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	ARGINFO		ainfo ;
	SIGHAND		sm ;
	BITS		pargs ;
	KEYOPT		akopts ;
	bfile		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, akl, avl, aol, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs ;
	int		rs1 ;
	int		v ;
	int		ex = EX_INFO ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_optplus = FALSE ;
	int		f_optminus = FALSE ;
	int		f_optequal = FALSE ;
	int		f_help = FALSE ;
	int		f_settime = FALSE ;

	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	cchar		*pr = NULL ;
	cchar		*sn = NULL ;
	cchar		*afname = NULL ;
	cchar		*efname = NULL ;
	cchar		*ofname = NULL ;
	cchar		*lfname = NULL ;
	cchar		*protospec = NULL ;
	cchar		*addrspec = NULL ;
	cchar		*cp ;


	if_int = 0 ;
	if_exit = 0 ;

#if	CF_SIGHAND
	rs = sighand_start(&sm,sigblocks,sigignores,sigints,main_sighand) ;
	if (rs < 0) goto badsighand ;
#endif

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
	proginfo_setbanner(pip,cp) ;

	pip->daytime = time(NULL) ;
	pip->verboselevel = 1 ;
	pip->to_open = -1 ;
	pip->to_read = -1 ;

	pip->f.logsys = TRUE ;
	pip->f.logprog = TRUE ;

/* start parsing the arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
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

	                case argopt_tmpdir:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->tmpdname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                pip->tmpdname = argp ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
	                    break ;

/* help */
	                case argopt_help:
	                    f_help = TRUE ;
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

/* search name */
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
	                            if (argl) {
	                                sn = argp ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* argument files */
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

/* error file */
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

/* output file */
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

/* log file */
	                case argopt_lf:
	                    pip->have.lfname = TRUE ;
	                    pip->final.lfname = TRUE ;
	                    cp = NULL ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            cp = avp ;
	                        }
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                cp = argp ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    if ((rs >= 0) && (cp != NULL)) {
	                        pip->f.lfname = TRUE ;
	                        lfname = cp ;
	                    }
	                    break ;

/* service-name */
	                case argopt_svc:
	                case argopt_port:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pip->svc = argp ;
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* protocol */
	                case argopt_proto:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            protospec = argp ;
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* handle all keyword defaults */
	                default:
	                    f_usage = TRUE ;
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

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* quiet mode */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->f.quiet = (rs > 0) ;
	                            }
	                        }
	                        break ;

	                    case 'a':
	                        pip->f.all = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->f.all = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* INET address-family */
	                    case 'f':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                addrspec = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'l':
	                    case 'L':
	                        pip->have.lfname = TRUE ;
	                        pip->final.lfname = TRUE ;
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                lfname = argp ;
	                                pip->f.lfname = TRUE ;
	                            }
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

	                    case 'p':
	                        pip->f.print = TRUE ;
	                        break ;

/* actually set the time! */
	                    case 's':
	                        f_settime = TRUE ;
	                        break ;

/* timeout (connect & read) */
	                    case 't':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = cfdecti(argp,argl,&v) ;
	                                pip->to_read = v ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

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
	                        f_usage = TRUE ;
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
	    bprintf(pip->efp,"%s: version %s\n",pip->progname,VERSION) ;
	}

/* get the program root */

	if (rs >= 0) {
	    if ((rs = proginfo_setpiv(pip,pr,&initvars)) >= 0) {
	        rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;
	    }
	}

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badarg ;
	}

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	} /* end if */

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_usage || f_help)
	    goto retearly ;


	ex = EX_OK ;

/* check arguments */

	if ((pip->to_read < 0) && (argval != NULL)) {
	    rs = cfdecti(argval,-1,&v) ;
	    pip->to_read = v ;
	}

	if (afname == NULL) afname = getenv(VARAFNAME) ;

	if (lfname == NULL) lfname = getenv(VARLFNAME) ;
	if (lfname == NULL) lfname = getenv(VARLOGFNAME) ;

	if ((rs >= 0) && (lfname != NULL)) {
	    cchar	**vpp = &pip->lfname ;
	    rs = proginfo_setentry(pip,vpp,lfname,-1) ;
	}

/* program options */

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	    if (rs < 0) {
	        ex = EX_USAGE ;
	    }
	}

/* timeouts */

	if (pip->to_open <= 0) pip->to_open = TO_OPEN ;

	if (pip->to_read < 0) pip->to_read = TO_READ ;

/* protocol? */

	pip->proto = -1 ;
	if ((rs >= 0) && (protospec != NULL) && (protospec[0] != '\0')) {
	    rs = loadproto(pip,protospec) ;
	    if (rs < 0) {
	        ex = EX_USAGE ;
	        bprintf(pip->efp,"%s: bad protocol specified (%d)\n",
	            pip->progname,rs) ;
	    }
	}

/* INET address family */

	pip->af = AF_UNSPEC ;
	if ((rs >= 0) && (addrspec != NULL) && (addrspec[0] != '\0')) {
	    rs = getaf(addrspec,-1) ;
	    pip->af = rs ;
	    if (rs < 0) {
	        ex = EX_USAGE ;
	        bprintf(pip->efp,"%s: bad addr-family specified (%d)\n",
	            pip->progname,rs) ;
	    }
	}

/* user-information */

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
	            if ((rs = proclogsys_begin(pip)) >= 0) {
	                vecobj		r ;
	                const int	size = sizeof(struct nettime) ;
	                if ((rs = vecobj_start(&r,size,5,0)) >= 0) {
	                    if ((rs = progout_begin(pip,ofname)) >= 0) {
	                        ARGINFO	*aip = &ainfo ;
	                        BITS	*bop = &pargs ;
	                        cchar	*afn = afname ;
	                        if ((rs = procargs(pip,aip,bop,&r,afn)) >= 0) {
	                            int		c = rs ;
	                            cchar	*pn = pip->progname ;
	                            cchar	*fmt ;
	                            if (c > 0) {
	                                if (f_settime) {
	                                    rs = process(pip,&r) ;
	                                }
	                            } else if ((c == 0) && (! f_settime)) {
	                                if (! pip->f.quiet) {
	                                    fmt = "%s: no servers available\n" ;
	                                    bprintf(pip->efp,fmt,pn) ;
	                                }
	                            }
	                        } /* end if (progargs) */
	                        rs1 = progout_end(pip) ;
	                        if (rs >= 0) rs = rs1 ;
	                    } /* end if (progout) */
	                    rs1 = vecobj_finish(&r) ;
	                    if (rs >= 0) rs = rs1 ;
	                } /* end if (results) */
	                rs1 = proclogsys_end(pip) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (proclogsys) */
	            rs1 = procuserinfo_end(pip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (procuserinfo) */
	        rs1 = userinfo_finish(&u) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt = "%s: userinfo failure (%d)\n" ;
	        ex = EX_NOUSER ;
	        bprintf(pip->efp,fmt,pn,rs) ;
	    }
	} /* end if (ok) */

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
	} else if (if_exit) {
	    ex = EX_TERM ;
	} else if (if_int)
	    ex = EX_INTR ;

/* early return thing */
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

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

	bits_finish(&pargs) ;

badpargs:
	proginfo_finish(pip) ;

badprogstart:
#if	CF_SIGHAND
	sighand_finish(&sm) ;
badsighand:
#endif

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mi[12] ;
	    uint	mo ;
	    uint	mdiff ;
	    uc_mallout(&mo) ;
	    mdiff = (mo-mo_start) ;
	    debugprintf("main: final mallout=%u\n",mdiff) ;
	    if (mdiff > 0) {
	        UCMALLREG_CUR	cur ;
	        UCMALLREG_REG	reg ;
	        const int	size = (10*sizeof(uint)) ;
	        const char	*ids = "main" ;
	        uc_mallinfo(mi,size) ;
	        debugprintf("main: MIoutnum=%u\n",mi[ucmallreg_outnum]) ;
	        debugprintf("main: MIoutnummax=%u\n",mi[ucmallreg_outnummax]) ;
	        debugprintf("main: MIoutsize=%u\n",mi[ucmallreg_outsize]) ;
	        debugprintf("main: MIoutsizemax=%u\n",
	            mi[ucmallreg_outsizemax]) ;
	        debugprintf("main: MIused=%u\n",mi[ucmallreg_used]) ;
	        debugprintf("main: MIusedmax=%u\n",mi[ucmallreg_usedmax]) ;
	        debugprintf("main: MIunder=%u\n",mi[ucmallreg_under]) ;
	        debugprintf("main: MIover=%u\n",mi[ucmallreg_over]) ;
	        debugprintf("main: MInotalloc=%u\n",mi[ucmallreg_notalloc]) ;
	        debugprintf("main: MInotfree=%u\n",mi[ucmallreg_notfree]) ;
	        ucmallreg_curbegin(&cur) ;
	        while (ucmallreg_enum(&cur,&reg) >= 0) {
	            debugprintf("main: MIreg.addr=%p\n",reg.addr) ;
	            debugprintf("main: MIreg.size=%u\n",reg.size) ;
	            debugprinthexblock(ids,80,reg.addr,reg.size) ;
	        } /* end while */
	        ucmallreg_curend(&cur) ;
	    } /* end if (positive) */
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* bad stuff */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


#if	CF_SIGHAND
/* ARGSUSED */
static void main_sighand(int sn,siginfo_t *sip,void *vcp)
{
	switch (sn) {
	case SIGINT:
	    if_int = TRUE ;
	    break ;
	case SIGKILL:
	    if_exit = TRUE ;
	    break ;
	} /* end switch */
}
/* end subroutine (main_sighand) */
#endif /* CF_SIGHAND */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s [-s] [-p] <timehost(s)>\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-proto <proto>] [-f <af>] [-t <to>] [-lf <logfile>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int loadproto(PROGINFO *pip,cchar *protospec)
{
	struct protoent	pe ;
	const int	pelen = getbufsize(getbufsize_pe) ;
	int		rs ;
	char		*pebuf ;
	if ((rs = uc_malloc((pelen+1),&pebuf)) >= 0) {
	    if ((rs = getpe_name(&pe,pebuf,pelen,protospec)) >= 0) {
	        pip->proto = pe.p_proto ;
	    }
	    uc_free(pebuf) ;
	} /* end if (m-a) */
	return rs ;
}
/* end subroutine (loadproto) */


/* process the program ako-options */
static int procopts(PROGINFO *pip,KEYOPT *kop)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;
	cchar		*cp ;

	if ((cp = getenv(VAROPTS)) != NULL) {
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
	                case akoname_test:
	                    if (! pip->final.test) {
	                        pip->have.test = TRUE ;
	                        pip->f.test = TRUE ;
	                        pip->final.test = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.test = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_logsys:
	                    if (! pip->final.logsys) {
	                        pip->have.logsys = TRUE ;
	                        pip->f.logsys = TRUE ;
	                        pip->final.logsys = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.logsys = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_logfile:
	                    if (! pip->final.lfname) {
	                        pip->have.lfname = TRUE ;
	                        pip->final.lfname = TRUE ;
	                        pip->f.lfname = TRUE ;
	                        if (vl > 0) {
	                            rs1 = optbool(vp,vl) ;
	                            if (rs1 >= 0) {
	                                pip->f.lfname = (rs1 > 0) ;
	                            } else if (rs1 == SR_INVALID) {
	                                cchar	**vpp = &pip->lfname ;
	                                rs = proginfo_setentry(pip,vpp,vp,vl) ;
	                            } else
	                                rs = rs1 ;
	                        }
	                    }
	                    break ;
	                } /* end switch */

	                c += 1 ;
	            } /* end if (valid option) */

	            if (rs < 0) break ;
	        } /* end while (looping through key options) */

	        keyopt_curend(kop,&kcur) ;
	    } /* end if (keyop-cur) */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,vecobj *rp,cchar *afn)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		pan = 0 ;
	int		c = 0 ;
	int		cl ;
	cchar		*cp ;

	if (rs >= 0) {
	    int		ai ;
	    int		f ;
	    cchar	**argv = aip->argv ;
	    for (ai = 1 ; ai < aip->argc ; ai += 1) {
	        f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	        f = f || ((ai > aip->ai_pos) && (argv[ai] != NULL)) ;
	        if (f) {
	            cp = argv[ai] ;
	            if (cp[0] != '\0') {
	                pan += 1 ;
	                rs = procserver(pip,rp,cp,-1) ;
	                c += rs ;
	            }
	        }

	        if ((! pip->f.all) && (c > 0)) break ;
	        if (rs < 0) break ;
	    } /* end for */
	} /* end if (ok) */

	if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	    if (pip->f.all || (c == 0)) {
	        bfile	afile, *afp = &afile ;

	        if (strcmp(afn,"-") == 0) afn = BFILE_STDIN ;

	        if ((rs = bopen(afp,afn,"r",0666)) >= 0) {
	            const int	llen = LINEBUFLEN ;
	            int		len ;
	            char	lbuf[LINEBUFLEN + 1] ;

	            while ((rs = breadline(afp,lbuf,llen)) > 0) {
	                len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

	                if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                    if (cp[0] != '#') {
	                        pan += 1 ;
	                        rs = procservers(pip,rp,cp,cl) ;
	                        c += rs ;
	                    }
	                }

	                if ((! pip->f.all) && (c > 0)) break ;
	                if (rs < 0) break ;
	            } /* end while (reading lines) */

	            rs1 = bclose(afp) ;
	            if (rs >= 0) rs = rs1 ;
	        } else {
	            if (! pip->f.quiet) {
	                cchar	*pn = pip->progname ;
	                cchar	*fmt ;
	                fmt = "%s: inaccessible argument-list (%d)\n" ;
	                bprintf(pip->efp,fmt,pn,rs) ;
	                bprintf(pip->efp,"%s: argfile=%s\n",pn,afn) ;
	            }
	        } /* end if */
	    } /* end if (needed) */
	} /* end if (processing file argument file list) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procargs) */


static int procservers(PROGINFO *pip,vecobj *rp,cchar *lbuf,int llen)
{
	FIELD		fsb ;
	int		rs ;
	int		c = 0 ;
	if ((rs = field_start(&fsb,lbuf,llen)) >= 0) {
	    int		fl ;
	    cchar	*fp ;
	    while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
	        if (fl > 0) {
	            rs = procserver(pip,rp,fp,fl) ;
	            c += rs ;
	        }
	        if (fsb.term == '#') break ;
	        if (rs < 0) break ;
	    } /* end while */
	    field_finish(&fsb) ;
	} /* end if (field) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procservers) */


static int procuserinfo_begin(PROGINFO *pip,USERINFO *uip)
{
	int		rs = SR_OK ;

	pip->nodename = uip->nodename ;
	pip->domainname = uip->domainname ;
	pip->username = uip->username ;
	pip->homedname = uip->homedname ;
	pip->gecosname = uip->gecosname ;
	pip->name = uip->name ;
	pip->mailname = uip->mailname ;
	pip->shell = uip->shell ;
	pip->org = uip->organization ;
	pip->logid = uip->logid ;
	pip->pid = uip->pid ;
	pip->uid = uip->uid ;
	pip->gid = uip->gid ;

	pip->uip = uip ;

	return rs ;
}
/* end subroutine (procuserinfo_begin) */


static int procuserinfo_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	if (pip == NULL) return SR_FAULT ;
	pip->uip = NULL ;
	return rs ;
}
/* end subroutine (procuserinfo_end) */


static int proclogsys_begin(PROGINFO *pip)
{
	int		rs = SR_OK ;
	if (pip->f.logsys) {
	    const int	fac = LOG_DAEMON ;
	    int		lo = 0 ;
	    cchar	*logid = pip->logid ;
	    cchar	*logtag = pip->searchname ;
	    if ((rs = logsys_open(&pip->ls,fac,logtag,logid,lo)) >= 0) {
	        pip->open.logsys = TRUE ;
	    }
	}
	return rs ;
}
/* end subroutine (proclogsys_begin) */


static int proclogsys_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (pip->open.logsys) {
	    pip->open.logsys = FALSE ;
	    rs1 = logsys_close(&pip->ls) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (proclogsys_end) */


static int procserver(PROGINFO *pip,vecobj *rlp,cchar *np,int nl)
{
	const int	hlen = MAXHOSTNAMELEN ;
	int		rs ;
	int		rs1 ;
	int		rs2 ;
	int		c = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	char		hbuf[MAXHOSTNAMELEN+1] ;

	if (np == NULL) return SR_FAULT ;

	if (np[0] == '\0') return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/procserver: ent name=%t\n",np,nl) ;
#endif

	if ((rs = snwcpy(hbuf,hlen,np,nl)) >= 0) {
	    struct nettime	nte ;
	    time_t		ntime ;
	    int64_t		moff = 0 ;
	    const int		to = pip->to_read ;
	    const int		proto = pip->proto ;
	    const int		af = pip->af ;
	    cchar		*svc = pip->svc ;
	    char		buf[BUFLEN + 1] ;
	    char		timebuf[TIMEBUFLEN + 1] ;
	    char		moffbuf[TIMEBUFLEN + 1] ;

	    if (pip->debuglevel > 0) {
	        bprintf(pip->efp,"%s: host=%s\n",pn,hbuf) ;
	        bprintf(pip->efp,"%s: to=%d\n",pn,to) ;
	    }

#if	CF_NETTIME
	    if ((rs1 = nettime(&nte,proto,af,hbuf,svc,to)) >= 0) {
	        c += 1 ;
	        rs = vecobj_add(rlp,&nte) ;
	    }
#else
		rs1 = SR_HOSTUNREACH ;
#endif /* CF_NETTIME */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main/procserver: nettime() rs=%d\n",rs1) ;
#endif

	    if (pip->debuglevel > 0) {
	        bprintf(pip->efp,"%s: server_response (%d)\n",
	            pip->progname,rs1) ;
	    }

	    if ((rs >= 0) && pip->f.print) {

	        moff = 0 ;
	        timebuf[0] = '-' ;
	        timebuf[1] = '\0' ;
	        if (rs1 >= 0) {
	            rs2 = tv_getmsec(&nte.off,&moff) ;

	            if (rs2 >= 0) {
	                ntime = pip->daytime + nte.off.tv_sec ;
	                timestr_logz(ntime,timebuf) ;
	            } else {
	                strwcpy(timebuf,"OV",TIMEBUFLEN) ;
	            }
	        }

#if	CF_DEBUG
	        if (DEBUGLEVEL(3)) {
	            int64_t	mtrip ;
	            tv_getmsec(&nte.trip,&mtrip) ;
	            debugprintf("procserver: mtrip=%lld\n",mtrip) ;
	            debugprintf("procserver: moff=%lld\n",moff) ;
	        }
#endif

	        if ((rs = mkoffstr(moffbuf,TIMEBUFLEN,moff)) >= 0) {
	            const int	osl = rs ;
	            int		bl ;

	            fmt = "%-23s %s %2u %2u %t (%d)" ;
	            if (rs1 >= 0)
	                fmt = "%-23s %s %2u %2u %t"  ;

	            rs = bufprintf(buf,BUFLEN,fmt,timebuf,moffbuf,
	                nte.proto,nte.pf, 
	                hbuf,strnlen(hbuf,(80-24-osl)), rs1) ;
	            bl = rs ;

	            if (rs >= 0) {
	                rs = progout_printline(pip,buf,bl) ;
#if	CF_DEBUG
	                if (DEBUGLEVEL(2))
	                    debugprintf("main/procserver: "
	                        "progout_printline() rs=%d\n",rs) ;
#endif
	            }

	        } /* end if ( mkoffstr) */

	    } /* end if (print) */

	} /* end if (snwcpy) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/procserver: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procserver) */


static int process(PROGINFO *pip,vecobj *rlp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/process: entered\n") ;
#endif

	if ((rs = vecobj_count(rlp)) >= 0) {
	    struct weights	*wtab = NULL ;
	    const int		n = rs ;
	    int			size ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/process: n=%u\n",n) ;
#endif

	    size = (n + 1) * sizeof(struct weights) ;
	    if ((rs = uc_malloc(size,&wtab)) >= 0) {
	        struct nettime	*ep ;
	        struct timeval	adj ;
	        int64_t		mtrip, moff ;
	        int		i ;

	        for (i = 0 ; vecobj_get(rlp,i,&ep) >= 0 ; i += 1) {
	            if (ep == NULL) continue ;

	            rs1 = tv_getmsec(&ep->off,&moff) ;
	            if (rs1 >= 0)
	                rs1 = tv_getmsec(&ep->trip,&mtrip) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3)) {
	                debugprintf("main/process: rs1=%d \n",rs1) ;
	                debugprintf("main/process: c=%u moff=%lld mtrip=%lld\n",
	                    c,moff,mtrip) ;
	            }
#endif

	            if ((rs1 >= 0) && (mtrip >= 0)) {
	                wtab[c].moff = moff ;
	                wtab[c].mtrip = mtrip ;
	                c += 1 ;
	            }

	            if (c >= n) break ;
	        } /* end for */

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("main/process: c=%u\n",c) ;
#endif

	        if (c > 0) {

	            rs = procwtab(pip,&adj,wtab,c) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3)) {
	                debugprintf("main/process: procwtab() rs=%d\n",rs) ;
	                debugprintf("main/process: time-off sec=%ld usec=%ld\n",
	                    adj.tv_sec,adj.tv_usec) ;
	            }
#endif /* CF_DEBUG */

	            if ((rs > 0) && (c > 0)) {
	                rs = progadjust(pip,&adj) ;
	            }

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("main/process: progadjust() rs=%d\n",rs) ;
#endif

	        } /* end if */

	        uc_free(wtab) ;
	    } /* end if (memory-allocation) */
	} /* end if (vecstr_count) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/process: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (process) */


static int procwtab(PROGINFO *pip,struct timeval *tvp,
		struct weights *wtab,int c)
{
	int64_t		moff_avg = 0 ;
	int64_t		mtrip_min = LLONG_MAX ;
	int64_t		mtrip_max = 0 ;
	int64_t		mtrip_span ;
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;
	if (tvp == NULL) return SR_FAULT ;
	if (wtab == NULL) return SR_FAULT ;

	if (c > 0) {
	    double	avg ;
	    double	sumw, sumwv ;
	    int		i ;

/* calculate minimum and maximum round-trip times */

	for (i = 0 ; i < c ; i += 1) {
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main/procwtab: w[%u].mtrip=%llu\n",
	            i,wtab[i].mtrip) ;
#endif
	    if (wtab[i].mtrip < mtrip_min) {
	        mtrip_min = wtab[i].mtrip ;
	    }
	    if (wtab[i].mtrip > mtrip_max) {
	        mtrip_max = wtab[i].mtrip ;
	    }
	}

/* calculate weights */

	mtrip_span = (mtrip_max - mtrip_min) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main/procwtab: mtrip_max=%llu\n",mtrip_max) ;
	    debugprintf("main/procwtab: mtrip_min=%llu\n",mtrip_min) ;
	    debugprintf("main/procwtab: mtrip_span=%llu\n",mtrip_span) ;
	}
#endif

	for (i = 0 ; i < c ; i += 1) {
	    double	vn = (mtrip_max - wtab[i].mtrip) ;
	    double	vd = mtrip_span ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main/procwtab: vn=%7.2f vd=%7.2f\n",vn,vd) ;
#endif

	    wtab[i].w = (mtrip_span > 0) ? (vn / vd) : 1.0 ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	           debugprintf("main/procwtab: i=%u mt=%llu mo=%lld w=%7.2f\n",
	            i,wtab[i].mtrip,wtab[i].moff,wtab[i].w) ;
	    }
#endif

	} /* end for */

/* calulate weighted average */

	sumw = 0 ;
	sumwv = 0 ;
	for (i = 0 ; i < c ; i += 1) {
	    sumw += wtab[i].w ;
	    sumwv += (wtab[i].moff * wtab[i].w) ;
	}
	avg = (c > 0) ? (sumwv / sumw) : 0.0 ;

	moff_avg = avg ;

	} else {
	    c = 0 ;
	}

	if ((rs >= 0) && (c > 0)) {
	    rs = tv_loadmsec(tvp,moff_avg) ;
	} else {
	    memset(tvp,0,sizeof(struct timeval)) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procwtab: ret rs=%d c=%u moff_avg=%lld\n",
	        rs,c,moff_avg) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procwtab) */


static int tv_loadmsec(struct timeval *tvp,int64_t msecs)
{
	if (tvp == NULL) return SR_FAULT ;
	tvp->tv_sec = (msecs / 1000) ;
	tvp->tv_usec = ((msecs % 1000) * 1000) ;
	return SR_OK ;
}
/* end subroutine (tv_loadmsec) */


static int tv_getmsec(struct timeval *tvp,int64_t *rp)
{
	int64_t		sec ;
	int64_t		usec ;
	int64_t		r ;
	int		rs = SR_OK ;
	if (tvp == NULL) return SR_FAULT ;
	if (rp == NULL) return SR_FAULT ;
	sec = tvp->tv_sec ;
	usec = tvp->tv_usec ;
	r = (sec * 1000000) + usec ;
	r = (r / 1000) ;
	*rp = r ;
	return rs ;
}
/* end subroutine (tv_getmsec) */


static int mkoffstr(char offbuf[],int offlen,int64_t moff)
{
	int64_t		tm ;
	int		rs = SR_OK ;
	int		secs, msecs ;
	int		ch ;
	int		f_sign = (moff >= 0) ;
	cchar		*fmt ;

	if (offbuf == NULL) return SR_FAULT ;

	if (offlen <= 0) return SR_OVERFLOW ;

	tm = llabs(moff) ;

	msecs = (tm % 1000) ;

	tm /= 1000 ;
	secs = (int) (MIN(tm,999) & INT_MAX) ;

	ch = ((f_sign) ? '+' : '-') ;
	fmt = "%c%3.3u.%3.3u" ;
	rs = bufprintf(offbuf,offlen,fmt,ch, secs, msecs) ;

	return rs ;
}
/* end subroutine (mkoffstr) */


