/* main (daytime) */

/* main subroutine for the 'daytime' INET server program */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_LOCSETENT	0		/* allow |locinfo_setentry()| */
#define	CF_LOGID	0		/* |procuserinfo_logid()| */
#define	CF_SIGMAN	0		/* use |SIGMAN(3dam)| */


/* revision history:

	= 1988-02-01, David A­D­ Morano
	This subroutine was originally written.

	= 1988-02-01, David A­D­ Morano
        This subroutine was modified to not write out anything to standard
        output if the access time of the associated terminal has not been
        changed in 10 minutes.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We are both a server and client.  The normal action is to be a server.
	Invoked with no positional arguments, we perform the server function
	and print out the current time.  With positional arguments, we go into
	client mode and contact a remove host for a date using the 'daytime'
	service.

	Synopsis:

	$ daytime [<hostname(s)>]


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<sigman.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<ascii.h>
#include	<char.h>
#include	<vecstr.h>
#include	<bfile.h>
#include	<sockaddress.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<opendial.h>
#include	<dialopts.h>
#include	<ucmallreg.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"nistinfo.h"
#include	"config.h"
#include	"defs.h"
#include	"proglog.h"


/* local defines */

#ifndef	AFNAMELEN
#define	AFNAMELEN	12
#endif

#ifndef	ORGLEN
#define	ORGLEN		MAXNAMELEN
#endif

#ifndef	ORGCODELEN
#define	ORGCODELEN	80
#endif

#ifndef	DBUFLEN
#define	DBUFLEN		MAXNAMELEN /* dialer-buffer length */
#endif

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags

#define	COL_HOSTNAME	54
#define	NIOVECS		10


/* external subroutines */

extern int	snsds(char *,int,cchar *,cchar *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpylc(char *,int,const char *) ;
extern int	sncpyuc(char *,int,const char *) ;
extern int	snwcpy(char *,int,cchar *,int) ;
extern int	snwcpylc(char *,int,cchar *,int) ;
extern int	snwcpyuc(char *,int,cchar *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	localgetorg(const char *,char *,int,const char *) ;
extern int	localgetorgcode(const char *,char *,int,const char *) ;
extern int	makedate_get(cchar *,cchar **) ;
extern int	isasocket(int) ;
extern int	isdigitlatin(int) ;

extern int	getaf(const char *,int) ;
extern int	getopendial(cchar *) ;
extern int	isFailOpen(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*strafname(int) ;
extern cchar	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strwcpyuc(char *,const char *,int) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_nist(time_t,struct nistinfo *,char *) ;


/* external variables */

extern const char	makedate[] ;


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		org:1 ;
	uint		anyformat:1 ;
	uint		geekout:1 ;
	uint		dgram:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	vecstr		stores ;
	PROGINFO	*pip ;
	const char	*pspec ;
	const char	*sspec ;
	int		af ;
	int		wl ;
	int		dialer ;
} ;


/* forward references */

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	process(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,VECSTR *,cchar *) ;
static int	procout(PROGINFO *,VECSTR *,cchar *) ;
static int	procorg(PROGINFO *) ;
static int	client(PROGINFO *,bfile *,int,int,int,cchar *,cchar *,cchar *) ;
static int	server(PROGINFO *) ;

static int	procuserinfo_begin(PROGINFO *,USERINFO *) ;
static int	procuserinfo_end(PROGINFO *) ;

#if	CF_LOGID
static int	procuserinfo_logid(PROGINFO *) ;
#endif

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_defs(LOCINFO *) ;
static int	locinfo_dialer(LOCINFO *,cchar *) ;

#if	CF_LOCSETENT
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
#endif

static int	anyformat(bfile *,int,int) ;
static int	isEnd(int) ;

#if	CF_SIGMAN
static void	sighand_int(int) ;
#endif /* CF_SIGMAN */


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

static const char *argopts[] = {
	"ROOT",
	"DEBUG",
	"VERSION",
	"VERBOSE",
	"HELP",
	"LOG",
	"MAKEDATE",
	"dialer",
	"dgram",
	"organization",
	"sn",
	"af",
	"ef",
	"of",
	"wl",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_debug,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_log,
	argopt_makedate,
	argopt_dialer,
	argopt_dgram,
	argopt_organization,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_wl,
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
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;

static const char *akonames[] = {
	"ie",
	"to",
	NULL
} ;

enum akonames {
	akoname_ie,
	akoname_to,
	akoname_overlast
} ;


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	ARGINFO		ainfo ;
	SIGMAN		sm ;
	BITS		pargs ;
	KEYOPT		akopts ;
	bfile		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, avl, akl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs, rs1 ;
	int		cl ;
	int		v ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_makedate = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*ofname = NULL ;
	const char	*efname = NULL ;
	const char	*hfname = NULL ;
	const char	*dialerspec = "TCP" ;
	const char	*addrspec = NULL ;
	const char	*wlspec = NULL ;
	const char	*cp ;


	if_exit = FALSE ;
	if_int = FALSE ;

#if	CF_SIGMAN
	rs = sigman_start(&sm,sigblocks,sigignores,sigints,sighand_int) ;
	if (rs < 0) goto badsigman ;
#endif /* CF_SIGMAN */

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

/* initialization */

	pip->daytime = time(NULL) ;
	pip->verboselevel = 1 ;
	pip->to = -1 ;
	pip->to_open = -1 ;
	pip->to_read = -1 ;

	pip->lip = lip ;
	rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* process program arguments */

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

/* debug level */
	                case argopt_debug:
	                    pip->debuglevel = 1 ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optvalue(avp,avl) ;
	                            pip->debuglevel = rs ;
	                        }
	                    }
	                    break ;

	                case argopt_version:
	                    f_makedate = f_version ;
	                    f_version = TRUE ;
	                    break ;

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

/* help file */
	                case argopt_help:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            hfname = avp ;
	                    }
	                    f_help  = TRUE ;
	                    break ;

/* log file */
	                case argopt_log:
	                    pip->f.logprog = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            pip->lfname = avp ;
	                        }
	                    }
	                    break ;

/* display the time this program was last "made" */
	                case argopt_makedate:
	                    f_makedate = TRUE ;
	                    break ;

	                case argopt_dgram:
	                    lip->f.dgram = TRUE ;
	                    break ;

	                case argopt_dialer:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            dialerspec = argp ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

	                case argopt_organization:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            pip->org = argp ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
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
	                            if (argl) {
	                                sn = argp ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* argument list file */
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
	                            if (argl) {
	                                afname = argp ;
	                            }
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
	                            if (argl) {
	                                ofname = argp ;
	                            }
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
	                            if (argl) {
	                                efname = argp ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* write-lenght (for UDP requests) */
	                case argopt_wl:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            wlspec = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                wlspec = argp ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                default:
	                    f_usage = TRUE ;
	                    rs = SR_INVALID ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {
	                    const int	kc = MKCHAR(*akp) ;

	                    switch (kc) {

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

	                    case 'V':
	                        f_makedate = f_version ;
	                        f_version = TRUE ;
	                        break ;

/* service name */
	                    case 'd':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                dialerspec = argp ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'f':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                addrspec = argp ;
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

	                    case 'q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* service name */
	                    case 's':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                lip->sspec = argp ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* timeout */
	                    case 't':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = cfdecti(argp,argl,&v) ;
	                                pip->to = v ;
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

	                    case 'x':
	                        lip->final.anyformat = TRUE ;
	                        lip->f.anyformat = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.anyformat = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* fall through from above */
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

	        } /* end if (digits or progopts) */

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
	} else if (! isFailOpen(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0)
	    goto badarg ;

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;
	}

	if (f_makedate) {
	    cl = makedate_get(makedate,&cp) ;
	    bprintf(pip->efp,"%s: makedate %t\n",
	        pip->progname,cp,cl) ;
	} /* end if */

/* get our program root (if we have one) */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	}

	if (f_usage)
	    usage(pip) ;

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,hfname) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* check arguments */

	if (afname == NULL) afname = getenv(VARAFNAME) ;

	if (wlspec == NULL) wlspec = argval ;

	if ((rs >= 0) && (lip->wl < 0) && (wlspec != NULL)) {
	    rs = optvalue(wlspec,-1) ;
	    lip->wl = rs ;
	}

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

	if (rs >= 0) {
	    if ((dialerspec == NULL) || (dialerspec[0] == '\0')) {
	        rs = SR_INVALID ;
	    }
	}

	if ((rs >= 0) && (pip->debuglevel > 0) && (lip->wl >= 0)) {
	    bprintf(pip->efp,"%s: wl=%u\n",pip->progname,lip->wl) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: dialerspec=%s\n",dialerspec) ;
#endif

/* does this dialer specification have a port-like part */

	if (rs >= 0) {
	    rs = locinfo_dialer(lip,dialerspec) ;
	}

	if (pip->to <= 0)
	    pip->to = TO_READ ;

	if ((rs >= 0) && (addrspec != NULL)) {
	    rs = getaf(addrspec,-1) ;
	    lip->af = rs ;
	} /* end if (AF hint) */

	if (pip->debuglevel > 0) {
	    const int	af = lip->af ;
	    cchar	*pn = pip->progname ;
	    bprintf(pip->efp,"%s: dialer=%u\n",pn,lip->dialer) ;
	    bprintf(pip->efp,"%s: af=%s(%d)\n",pn,strafname(af),af) ;
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    const int	af = lip->af ;
	    debugprintf("main: af=%s(%d)\n",strafname(af),af) ;
	}
#endif

/* start processing */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: go\n") ;
#endif

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
	                if ((rs = procorg(pip)) >= 0) {
	                    cchar	*afn = afname ;
	                    cchar	*ofn = ofname ;
	                    rs = process(pip,&ainfo,&pargs,afn,ofn) ;
	                } /* end if (procorg) */
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
	        char	*fmt = "%s: userinfo failure (%d)\n" ;
	        ex = EX_NOUSER ;
	        bprintf(pip->efp,fmt,pn,rs) ;
	    } /* end if (userinfo) */
	} else {
	    cchar	*pn = pip->progname ;
	    char	*fmt = "%s: invalid argument (%d)\n" ;
	    ex = EX_USAGE ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	} /* end if (ok) */

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    ex = mapex(mapexs,rs) ;
	} else if (if_exit) {
	    ex = EX_TERM ;
	} else if (if_int)
	    ex = EX_INTR ;

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
	locinfo_finish(lip) ;

badlocstart:
	proginfo_finish(pip) ;

badprogstart:

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
	        }
	        ucmallreg_curend(&cur) ;
	    }
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

#if	CF_SIGMAN
	sigman_finish(&sm) ;
badsigman:
#endif /* CF_SIGMAN */

	return ex ;

/* argument errors */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,
	    "%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [<host(s)>] [-d <dialer>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-s <service>] [-f <af>] [-x] [-v]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


#if	CF_SIGMAN
void sighand_int(int sn)
{
	switch (sn) {
	case SIGINT:
	    if_int = TRUE ;
	    break ;
	case SIGKILL:
	    if_exit = TRUE ;
	    break ;
	default:
	    if_exit = TRUE ;
	    break ;
	} /* end switch */
}
/* end subroutine (sighand_int) */
#endif /* CF_SIGMAN */


/* process the program ako-options */
static int procopts(PROGINFO *pip,KEYOPT *kop)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		c = 0 ;
	const char	*cp ;

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

	                case akoname_ie:
	                    if (! lip->final.geekout) {
	                        lip->have.geekout = TRUE ;
	                        lip->final.geekout = TRUE ;
	                        lip->f.geekout = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.geekout = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_to:
	                    if (pip->to < 0) {
	                        if (vl > 0) {
	                            int	v ;
	                            rs = cfdecti(vp,vl,&v) ;
	                            pip->to = v ;
	                        }
	                    }
	                    break ;

	                } /* end switch */

	                c += 1 ;
	            } /* end if (valid option) */

	            if (rs < 0) break ;
	        } /* end while (looping through key options) */

	        keyopt_curend(kop,&kcur) ;
	    } /* end if (keyopt-cur) */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int process(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *afn,cchar *ofn)
{
	vecstr		names ;
	const int	vo = 0 ;
	int		rs ;
	int		rs1 ;
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/process: ent\n") ;
#endif
	if ((rs = vecstr_start(&names,10,vo)) >= 0) {
	    if ((rs = procargs(pip,aip,bop,&names,afn)) > 0) {
	        LOCINFO	*lip = pip->lip ;
	        if ((rs = locinfo_defs(lip)) >= 0) {
	            rs = procout(pip,&names,ofn) ;
	        } /* end if (locinfo_defs) */
	    } else {
	        rs = server(pip) ;
	    }
	    rs1 = vecstr_finish(&names) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecstr) */
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/process: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (process) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,VECSTR *nlp,cchar *afn)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		pan = 0 ;
	int		cl ;
	const char	*cp ;

	if (rs >= 0) {
	    int		ai ;
	    int		f ;
	    for (ai = 1 ; ai < aip->argc ; ai += 1) {

	        f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	        f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	        if (f) {
	            cp = aip->argv[ai] ;
	            if (cp[0] != '\0') {
	                pan += 1 ;
	                rs = vecstr_add(nlp,cp,-1) ;
	            }
	        }

	        if (rs < 0) break ;
	    } /* end for */
	} /* end if (ok) */

/* process any files in the argument filename list file */

	if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	    bfile	afile, *afp = &afile ;

	    if (strcmp(afn,"-") == 0) afn = BFILE_STDIN ;

	    if ((rs = bopen(afp,afn,"r",0666)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        int		len ;
	        char		lbuf[LINEBUFLEN + 1] ;

	        while ((rs = breadline(afp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	            if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                if (cp[0] != '#') {
	                    pan += 1 ;
	                    rs = vecstr_add(nlp,cp,cl) ;
	                }
	            }

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
	            bprintf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	        } /* end if */
	    } /* end if */

	} /* end if (processing file argument file list) */

	return (rs >= 0) ? pan : rs ;
}
/* end subroutine (procargs) */


static int procout(PROGINFO *pip,VECSTR *nlp,const char *ofn)
{
	LOCINFO		*lip = pip->lip ;
	bfile		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofn,"wct",0666)) >= 0) {
	    const int	to = pip->to ;
	    const int	wl = lip->wl ;
	    const int	af = lip->af ;
	    int		i ;
	    const char	*pspec = lip->pspec ;
	    const char	*sspec = lip->sspec ;
	    const char	*cp ;

	    for (i = 0 ; vecstr_get(nlp,i,&cp) >= 0 ; i += 1) {
	        if (cp != NULL) {
	            rs = client(pip,ofp,wl,af,to,cp,pspec,sspec) ;
	        }
	        if (rs < 0) break ;
	    } /* end for */

	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    fmt = "%s: inaccessible output (%d)\n" ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    bprintf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	} /* end if (output-file) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procout) */


static int client(pip,ofp,wl,af,to,name,pspec,svc)
PROGINFO	*pip ;
bfile		*ofp ;
int		wl ;
int		af ;
int		to ;
const char	name[] ;
const char	pspec[] ;
const char	svc[] ;
{
	LOCINFO		*lip = pip->lip ;
	const int	opts = 0 ;
	const int	dlen = DBUFLEN ;
	int		rs ;
	int		di ;
	int		len ;
	char		dbuf[DBUFLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("client: dialer=%u\n",lip->dialer) ;
	    debugprintf("client: af=%d\n",af) ;
	    debugprintf("client: host=%s\n",name) ;
	    debugprintf("client: port=%s\n",pspec) ;
	    debugprintf("client: svc=%s\n",svc) ;
	}
#endif /* CF_DEBUG */

	if (name == NULL) return SR_FAULT ;

	if (name[0] == '\0') return SR_INVALID ;

	di = lip->dialer ;
	if (name[0] == '-') name = pip->nodename ;

	if (pip->debuglevel > 0) {
	    const char	*pn = pip->progname ;
	    bprintf(pip->efp,"%s: af=%u\n",pn,af) ;
	    bprintf(pip->efp,"%s: host=%s\n",pn,name) ;
	    bprintf(pip->efp,"%s: port=%s\n",pn,pspec) ;
	    bprintf(pip->efp,"%s: svc=%s\n",pn,svc) ;
	}

	if ((rs = opendial(di,af,name,pspec,svc,NULL,NULL,to,opts)) >= 0) {
	    const int	s = rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf( "client: dial rs=%d\n", rs) ;
#endif /* CF_DEBUG */

	    if (lip->dialer == opendialer_udp) {
	        int	bl = 0 ;

	        dbuf[0] = '\0' ;
	        if (wl < 0) {
	            rs = sncpy2(dbuf,dlen,INETSVC_DAYTIME,"\n") ;
	            bl = rs ;
	        }
	        if (rs >= 0) {
	            if ((wl >= 0) && (bl > wl)) bl = wl ;
	            rs = uc_writen(s,dbuf,bl) ;
	        }

#if	CF_DEBUG
	        if (DEBUGLEVEL(2)) {
	            debugprintf( "client: DGRAM write len=%u\n",bl) ;
	            debugprintf( "client: uc_writen() rs=%d\n",rs) ;
	        }
#endif

	    } /* end if (sending packet for DGRAM transports) */

	    if (rs >= 0) {

	        if (! lip->f.anyformat) {
	            const int	ropts = FM_TIMED ;
	            int		llen ;
	            char	*bp ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("client: to_read=%d\n",to) ;
#endif /* CF_DEBUG */

	            if ((rs = uc_reade(s,dbuf,dlen,to,ropts)) >= 0) {
	                len = rs ;

	                if (len > 1) {
	                    int	ch ;

	                    bp = dbuf ;
	                    while (len > 0) {
	                        ch = MKCHAR(*bp) ;
	                        if (! isEnd(ch)) break ;
	                        bp += 1 ;
	                        len -= 1 ;
	                    }

	                    for (llen = 0 ; llen < len ; llen += 1) {
	                        if (bp[llen] == '\n') break ;
	                    } /* end for */

	                    while (llen > 0) {
	                        ch = MKCHAR(bp[llen-1]) ;
	                        if (! isEnd(ch)) break ;
	                        llen -= 1 ;
	                    }

	                    while (llen < COL_HOSTNAME) {
	                        bp[llen++] = ' ' ;
	                    }

	                    if (llen > 76)
	                        llen = 76 ;

	                    bp[llen] = '\0' ;
	                    bprintf(ofp,"%t %s\n",bp,llen,name) ;

	                } else 
	                    rs = SR_BADMSG ;

	            } /* end if */

	            if (rs < 0)
	                bprintf(ofp,
	                    "rs=%4d                  %s\n",
	                    rs,name) ;

	            if ((rs < 0) && lip->f.geekout) rs = SR_OK ;

	        } else
	            rs = anyformat(ofp,s,to) ;

	    } /* end if */

	    u_close(s) ;
	} else
	    bprintf(ofp,"** no connection (%d) **\n",rs) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("client: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (client) */


/* perform the server function */
static int server(PROGINFO *pip)
{
	const int	olen = ORGCODELEN ;
	int		rs ;
	cchar		*un = pip->username ;
	char		obuf[ORGCODELEN+1] ;

	if ((rs = localgetorgcode(pip->pr,obuf,olen,un)) >= 0) {
	    struct nistinfo	nist ;
	    int			fd_portal = FD_STDOUT ;
	    int			cl ;
	    const char		*cp ;
	    char		timebuf[TIMEBUFLEN + 1] ;

/* handle case of data-gram transports */

	    if (isasocket(fd_portal)) {
	        const int	sol = SOL_SOCKET ;
	        const int	cmd = SO_TYPE ;
	        int		optv = 0 ;
	        int		optl = sizeof(int) ;
	        if ((rs = u_getsockopt(fd_portal,sol,cmd,&optv,&optl)) >= 0) {
	            LOCINFO		*lip = pip->lip ;
	            struct msghdr	msg ;
	            struct iovec	vecs[NIOVECS] ;
	            SOCKADDRESS		from ;
	            const int 		size = NIOVECS * sizeof(struct iovec) ;

	            if (optl == sizeof(int)) {
	                if (optv == SOCK_DGRAM) lip->f.dgram = TRUE ;
	            } /* end if (get socket option) */

	            memset(&vecs,0,size) ;
	            vecs[0].iov_base = timebuf ;
	            vecs[0].iov_len = TIMEBUFLEN ;

	            memset(&msg,0,sizeof(struct msghdr)) ;
	            msg.msg_name = &from ;
	            msg.msg_namelen = sizeof(SOCKADDRESS) ;
	            msg.msg_iov = vecs ;
	            msg.msg_iovlen = NIOVECS ;

	            if (lip->f.dgram) {
	                fd_portal = FD_STDIN ;
	                msg.msg_namelen = sizeof(SOCKADDRESS) ;
	                vecs[0].iov_len = TIMEBUFLEN ;
	                rs = u_recvmsg(fd_portal,&msg,0) ;
	            } /* end if (read the data-gram off of the socket) */

/* prepare response to send */

	            if (rs >= 0) {
	                memset(&nist,0,sizeof(struct nistinfo)) ;
	                sncpy1(nist.org,NISTINFO_ORGSIZE,obuf) ;
	                cp = timestr_nist(pip->daytime,&nist,timebuf) ;
	                cl = strlen(timebuf) ;
	                timebuf[cl++] = '\n' ;
	                if (lip->f.dgram) {
	                    vecs[0].iov_len = cl ;
	                    rs = u_sendmsg(fd_portal,&msg,0) ;
	                } else {
	                    rs = uc_writen(fd_portal,cp,cl) ;
	                }
	            } /* end if (ok) */

	        } /* end if (u_getsockopt) */
	    } else {
	        memset(&nist,0,sizeof(struct nistinfo)) ;
	        sncpy1(nist.org,NISTINFO_ORGSIZE,obuf) ;
	        cp = timestr_nist(pip->daytime,&nist,timebuf) ;
	        cl = strlen(timebuf) ;
	        timebuf[cl++] = '\n' ;
	        rs = uc_writen(fd_portal,timebuf,cl) ;
	    } /* end if (socket or other) */

	} /* end if (localgetorgcode) */

#if	CF_DEBUGS
	debugprintf("server: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (server) */


static int procorg(PROGINFO *pip)
{
	int		rs = SR_OK ;
	cchar		*org = pip->org ;
	if ((org == NULL) || (org[0] == '\0')) {
	    const int	olen = ORGLEN ;
	    cchar	*un = pip->username ;
	    char	obuf[ORGLEN+1] ;
	    if ((rs = localgetorg(pip->pr,obuf,olen,un)) > 0) {
	        cchar	**vpp = &pip->org ;
	        rs = proginfo_setentry(pip,vpp,obuf,rs) ;
	    }
	}
	return rs ;
}
/* end subroutine (procorg) */


static int procuserinfo_begin(PROGINFO *pip,USERINFO *uip)
{
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procuserinfo_begin: ent\n") ;
#endif

	pip->nodename = uip->nodename ;
	pip->domainname = uip->domainname ;
	pip->username = uip->username ;
	pip->gecosname = uip->gecosname ;
	pip->realname = uip->realname ;
	pip->name = uip->name ;
	pip->fullname = uip->fullname ;
	pip->mailname = uip->mailname ;
	if (pip->org == NULL) {
	    pip->org = uip->organization ;
	}
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

#if	CF_LOGID
	if (rs >= 0) {
	    rs = procuserinfo_logid(pip) ;
	} /* end if (ok) */
#endif /* CF_LOGID */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procuserinfo_begin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procuserinfo_begin) */


static int procuserinfo_end(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procuserinfo_end: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procuserinfo_end) */


#if	CF_LOGID
static int procuserinfo_logid(PROGINFO *pip)
{
	int		rs ;
	if ((rs = lib_runmode()) >= 0) {
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
#endif /* CF_LOGID */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (lip == NULL) return SR_FAULT ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->af = -1 ;
	lip->wl = -1 ;
	lip->dialer = -1 ;

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
int locinfo_setentry(LOCINFO *lip,cchar **epp,cchar vp[],int vl)
{
	int		rs = SR_OK ;
	int		len = 0 ;

	if (lip == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_FAULT ;

	if (! lip->open.stores) {
	    rs = vecstr_start(&lip->stores,4,0) ;
	    lip->open.stores = (rs >= 0) ;
	}

	if (rs >= 0) {
	    int	oi = -1 ;
	    if (*epp != NULL) {
	        oi = vecstr_findaddr(&lip->stores,*epp) ;
	    }
	    if (vp != NULL) {
	        len = strnlen(vp,vl) ;
	        rs = vecstr_store(&lip->stores,vp,len,epp) ;
	    } else {
	        *epp = NULL ;
	    }
	    if ((rs >= 0) && (oi >= 0)) {
	        vecstr_del(&lip->stores,oi) ;
	    }
	} /* end if */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_setentry) */
#endif /* CF_LOCSETENT */


static int locinfo_defs(LOCINFO *lip)
{
	if (lip->af < 0) lip->af = AF_UNSPEC ;
	if ((lip->sspec == NULL) || (lip->sspec[0] == '\0')) {
	    lip->sspec = INETSVC_DAYTIME ;
	}
	return SR_OK ;
}
/* end subroutine (locinfo_defs) */


static int locinfo_dialer(LOCINFO *lip,cchar *dialerspec)
{
	const int	dlen = DBUFLEN ;
	int		rs ;
	cchar		*tp ;
	char		dbuf[DBUFLEN+1] ;
	if ((tp = strchr(dialerspec,':')) != NULL) {
	    lip->pspec = (tp + 1) ;
	    rs = snwcpylc(dbuf,dlen,dialerspec,(tp - dialerspec)) ;
	} else {
	    rs = sncpylc(dbuf,dlen,dialerspec) ;
	}
	if (rs >= 0) {
	    rs = getopendial(dbuf) ;
	    lip->dialer = rs ;
	}
	return rs ;
} 
/* end if (locinfo_dialer) */


static int anyformat(bfile *ofp,int s,int to)
{
	const int	llen = LINEBUFLEN ;
	int		rs ;
	int		len ;
	int		wlen = 0 ;
	char		lbuf[LINEBUFLEN + 1] ;

	while ((rs = uc_readlinetimed(s,lbuf,llen,to)) > 0) {
	    len = rs ;

	    rs = bwrite(ofp,lbuf,len) ;
	    wlen += rs ;

	    if (rs < 0) break ;
	} /* end while */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (anyformat) */


static int isEnd(int ch)
{
	int		f = FALSE ;
	f = f || CHAR_ISWHITE(ch) ;
	f = f || (ch == '\n') ;
	return f ;
}
/* end subroutine (isEnd) */


