/* main */

/* COMSAT server */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* debug print-outs */
#define	CF_DEBUGPROC	0		/* debug 'testproc()' */
#define	CF_DEBUGTN	0		/* debug 'termnote(3dam)' */
#define	CF_DEBUGOT	0		/* debug 'opentermnote(3dam)' */
#define	CF_INET6	0		/* use INET6 */
#define	CF_PRLOCAL	1		/* need PR of LOCAL? */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	This subroutine was originally written.  

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This program is a Comsat server.  It listens for Comsat messages from
	the Comsat network port (UDP 'biff') and attempts to extract the
	necessary mail information for display to one or more of the user's
	(mail user's) logged in terminals.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<sys/timeb.h>		/* for 'struct timeb' */
#include	<netinet/in.h>
#include	<termios.h>
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<syslog.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<sighand.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<paramopt.h>
#include	<bfile.h>
#include	<dater.h>
#include	<userinfo.h>
#include	<paramfile.h>
#include	<logfile.h>
#include	<hostinfo.h>
#include	<sockaddress.h>
#include	<openport.h>
#include	<spawner.h>
#include	<buffer.h>
#include	<ascii.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"proglog.h"


/* type-defs */

#if	defined(IRIX) && (! defined(TYPEDEF_INADDRT))
#define	TYPEDEF_INADDRT	1
typedef unsigned int	in_addr_t ;
#endif


/* local defines */

#ifndef	LOGNAMELEN
#ifdef	TMPX_LUSER
#define	LOGNAMELEN	TMPX_LUSER
#else
#define	LOGNAMELEN	32
#endif
#endif

#ifndef	ENVBUFLEN
#define	ENVBUFLEN	2048
#endif

#ifndef	INET4ADDRLEN
#define	INET4ADDRLEN	sizeof(in_addr_t)
#endif

#ifndef	INET6ADDRLEN
#define	INET6ADDRLEN	16
#endif

#ifndef	INETXADDRLEN
#define	INETXADDRLEN	MAX(INET4ADDRLEN,INET6ADDRLEN)
#endif /* INETXADDRLEN */

#define	O_TERM		(O_WRONLY | O_NDELAY | O_NOCTTY)

#ifndef	ANYHOST
#define	ANYHOST		"anyhost"
#endif

#define	PROTONAME	"udp"


/* external subroutines */

extern uint	inet4int(const void *) ;

extern int	snsds(char *,int,cchar *,cchar *) ;
extern int	sncpy1(cchar *,int,cchar *) ;
extern int	sncpy2(cchar *,int,cchar *,cchar *) ;
extern int	sncpy1w(char *,int,cchar *,int) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mnwcpy(void *,int,const void *,int) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecti(cchar *,int,int *) ;
extern int	ctdeci(char *,int,int) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	bufprintf(char *,int,cchar *,...) ;
extern int	mkpr(char *,int,cchar *,cchar *) ;
extern int	mkplogid(char *,int,cchar *,int) ;
extern int	getportnum(cchar *,cchar *) ;
extern int	getprotofamily(int) ;
extern int	listenudp(int,cchar *,cchar *,int) ;
extern int	opentermnote(cchar *,cchar **,int,int) ;
extern int	pcsgetprogpath(cchar *,char *,cchar *) ;
extern int	initnow(struct timeb *,char *,int) ;
extern int	isasocket(int) ;
extern int	tolc(int) ;
extern int	isdigitlatin(int) ;
extern int	isprintlatin(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

extern int	progerr_begin(PROGINFO *) ;
extern int	progerr_end(PROGINFO *) ;
extern int	progerr_printf(PROGINFO *) ;

extern int	progloglock_begin(PROGINFO *) ;
extern int	progloglock_end(PROGINFO *) ;

extern int	prognote_begin(PROGINFO *) ;
extern int	prognote_end(PROGINFO *) ;

extern int	progcs(PROGINFO *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*strrpbrk(cchar *,cchar *) ;
extern char	*strnrpbrk(cchar *,int,cchar *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local typedefs */


/* local structures */

#ifndef	TYPEDEF_CCHAR
#define	TYPEDEF_CCHAR	1
typedef cchar	cchar ;
#endif


/* forward references */

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procuserinfo_begin(PROGINFO *,USERINFO *) ;
static int	procuserinfo_end(PROGINFO *) ;
static int	procuserinfo_logid(PROGINFO *) ;

#ifdef	COMMENT
static int	procourconf_begin(PROGINFO *,PARAMOPT *,const char *) ;
static int	procourconf_end(PROGINFO *) ;
#endif /* COMMENT */

static int	procdaemonbegin(PROGINFO *) ;
static int	procdaemonend(PROGINFO *) ;
static int	procdaemonaddr(PROGINFO *,void *,int,cchar *) ;

static int	process(PROGINFO *) ;
static int	procback(PROGINFO *) ;
static int	procbacker(PROGINFO *,cchar *,cchar **) ;
static int	procbackenv(PROGINFO *,SPAWNER *) ;
static int	procdaemon(PROGINFO *) ;
static int	procreg(PROGINFO *) ;
static int	openaddr(int,cchar *,int) ;

#if	CF_DEBUG && CF_DEBUGPROC
static int proctest(PROGINFO *) ;
#if	CF_DEBUGTN
static int proctesttn(PROGINFO *) ;
#endif
#if	CF_DEBUGOT
static int proctestot(PROGINFO *) ;
#endif
#endif

static int	hostinfo_findaf(HOSTINFO *,char *,int,int) ;

static void	main_sighand(int,siginfo_t *,void *) ;


/* local variables */

static volatile int	if_int ;
static volatile int	if_exit ;

static const int	sigblocks[] = {
	0
} ;

static const int	sigignores[] = {
	SIGPIPE,
	SIGPOLL,
	SIGHUP,
	0
} ;

static const int	sigints[] = {
	SIGUSR1,
	SIGUSR2,
	SIGINT,
	SIGTERM,
	0
} ;

static cchar *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"sn",
	"ef",
	"cf",
	"lf",
	"md",
	"wto",
	"ra",
	"daemon",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_sn,
	argopt_ef,
	argopt_cf,
	argopt_lf,
	argopt_md,
	argopt_wto,
	argopt_ra,
	argopt_daemon,
	argopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPREXTRA
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
	{ SR_OVERFLOW, EX_SOFTWARE },
	{ SR_NOANODE, EX_SOFTWARE },
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;

static cchar *akonames[] = {
	"ra",
	"cf",
	"lf",
	"daemon",
	"intrun",
	"intidle",
	"hostspec",
	"portspec",
	NULL
} ;

enum akonames {
	akoname_ra,
	akoname_cf,
	akoname_lf,
	akoname_daemon,
	akoname_intrun,
	akoname_intidle,
	akoname_hostspec,
	akoname_portspec,
	akoname_overlast
} ;

#ifdef	COMMENT
static const char	*sched1[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n/%f",
	"%p/%e/%n.%f",
	"%p/%n.%f",
	NULL
} ;
#endif /* COMMENT */

#ifdef	COMMENT
static const char	*cparams[] = {
	"maildir",
	"logsize",
	"logfile",
	NULL
} ;
#endif /* COMMENT */

enum cparams {
	cparam_maildir,
	cparam_logsize,
	cparam_logfile,
	cparam_overlast
} ;

#ifdef	COMMENT
static const char	*varmaildirs[] = {
	VARMAILDNAMESP,
	VARMAILDNAMES,
	VARMAILDNAME,
	NULL
} ;
#endif /* COMMENT */


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	ARGINFO		ainfo ;
	SIGHAND		sm ;
	BITS		pargs ;
	KEYOPT		akopts ;
	PARAMOPT	aparams ;
	bfile		errfile ;
	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs, rs1 ;
	int		cl ;
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
	cchar		*efname = NULL ;
	cchar		*idlespec = NULL ;
	cchar		*cp ;


	if_int = 0 ;
	if_exit = 0 ;

	rs = sighand_start(&sm, sigblocks,sigignores,sigints,main_sighand) ;
	if (rs < 0) goto badsighand ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getenv(VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

/* initialize */

	pip->verboselevel = 1 ;
	pip->intrun = -1 ;
	pip->intidle = -1 ;
	pip->intnote = -1 ;
	pip->notesmax = NOTESMAX ;
	pip->fd_msg = FD_STDIN ;

	pip->f.logprog = TRUE ;

/* start parsing the arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

	if (rs >= 0) {
	    rs = paramopt_start(&aparams) ;
	    pip->open.aparams = (rs >= 0) ;
	}

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

	            argval = NULL ;

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

/* configuration file */
	                case argopt_cf:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->cfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->cfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* log file name */
	                case argopt_lf:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->lfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->lfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* mail directory(s) */
	                case argopt_md:
	                    cp = NULL ;
	                    cl = -1 ;
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
	                        PARAMOPT	*pop = &aparams ;
	                        cchar		*po = PO_MAILDIRS ;
	                        rs = paramopt_loads(pop,po,cp,cl) ;
	                    }
	                    break ;

/* maximum idle time */
	                case argopt_wto:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            idlespec = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                idlespec = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* re-use address */
	                case argopt_ra:
	                    pip->final.reuseaddr = TRUE ;
	                    pip->have.reuseaddr = TRUE ;
	                    pip->f.reuseaddr = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            pip->f.reuseaddr = (rs > 0) ;
	                        }
	                    }
	                    break ;

/* daemon mode */
	                case argopt_daemon:
	                    pip->final.daemon = TRUE ;
	                    pip->have.daemon = TRUE ;
	                    pip->f.daemon = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                                rs = cfdecti(avp,avl,&v) ;
	                                pip->intrun = v ;
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

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* mail directory */
	                    case 'M':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->maildname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* program root */
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

/* daemon mode */
	                    case 'd':
	                        pip->final.background = TRUE ;
	                        pip->f.background = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = cfdecti(avp,avl,&v) ;
	                                pip->intrun = v ;
	                            }
	                        }
	                        break ;

/* host specification */
	                    case 'h':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->hostspec = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* maximum idle time */
	                    case 'i':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                idlespec = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* n-parallism */
	                    case 'n':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = cfdecti(argp,argl,&v) ;
	                                pip->npar = v ;
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

/* port specification */
	                    case 'p':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->portspec = argp ;
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

	if (efname == NULL) efname = getenv(VAREFNAME) ;
	if (efname == NULL) efname = getenv(VARERRORFNAME) ;
	if (efname == NULL) efname = BFILE_STDERR ;
	if ((rs1 = bopen(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    bcontrol(&errfile,BC_SETBUFLINE,TRUE) ;
	} else if (! isFailOpen(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0) goto badarg ;

#if	CF_DEBUGS
	debugprintf("main: apr=%s\n",pr) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(1)) {
	    debugprintf("main: debuglevel=%u\n", pip->debuglevel) ;
	}
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
	    goto retearly ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: pr=%s\n", pip->pr) ;
	    debugprintf("main: sn=%s\n", pip->searchname) ;
	}
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	}

	if (f_usage)
	    usage(pip) ;

/* get help if requested */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: pn=%s\n",pip->progname) ;
#endif

/* process program options */

	if (pip->hostspec == NULL) pip->hostspec = getenv(VARHOSTSPEC) ;
	if (pip->portspec == NULL) pip->portspec = getenv(VARPORTSPEC) ;

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: background=%u\n",pip->f.background) ;
	    debugprintf("main: daemon=%u\n",pip->f.daemon) ;
	    debugprintf("main: hostspec=%s\n",pip->hostspec) ;
	    debugprintf("main: portspec=%s\n",pip->portspec) ;
	}
#endif /* CF_DEBUG */

/* some initialization */

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

	if (pip->maildname == NULL) pip->maildname = getenv(VARMAILDNAME) ;
	if (pip->maildname == NULL) pip->maildname = MAILDNAME ;

/* any idle specification? */

	if ((rs >= 0) && (idlespec != NULL)) {
	    int		ch = MKCHAR(*idlespec) ;
	    cp = idlespec ;
	    cl = -1 ;
	    if (isdigitlatin(ch)) {
	        rs = cfdecti(cp,cl,&v) ;
	        pip->intidle = v ;
	    } else if ((tolc(ch) == 'i') || (ch == '-')) {
	        pip->intidle = INT_MAX ;
	    } else {
	        rs = SR_INVALID ;
	    }
	} /* end if (idle-spec processing) */

#ifdef	COMMENT
	if (pip->intrun < 0)
	    pip->intrun = DEFINTEUN ;
#endif

	if ((rs >= 0) && (pip->intidle < 0) && (argval != NULL)) {
	    rs = cfdecti(argval,-1,&v) ;
	    pip->intidle = v ;
	}

	if (pip->intidle < 0)
	    pip->intidle = DEFINTIDLE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: intidle=%d\n",pip->intidle) ;
	    debugprintf("main: intrun=%d\n",pip->intrun) ;
	}
#endif

	if (pip->intnote < 0)
	    pip->intnote = DEFINTNOTE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: pn=%s\n",pip->progname) ;
#endif

/* do we have an activity log file? */

	if (pip->logsize <= 0) pip->logsize = LOGSIZE ;

	if (rs >= 0) {
	    if ((pip->intidle >= 0) && (pip->intidle != INT_MAX)) {
	        proglog_printf(pip,"intidle=%u\n",pip->intidle) ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: pn=%s\n",pip->progname) ;
#endif

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    cchar	*pn = pip->progname ;
	    bprintf(pip->efp,"%s: daemon mode=%u\n",pn,pip->f.daemon) ;
	}

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    USERINFO	u ;
	    cchar	*pn = pip->progname ;
	    if ((rs = userinfo_start(&u,NULL)) >= 0) {
	        if ((rs = procuserinfo_begin(pip,&u)) >= 0) {
	            if (pip->cfname != NULL) {
	                if (pip->euid != pip->uid) u_seteuid(pip->uid) ;
	                if (pip->egid != pip->gid) u_setegid(pip->gid) ;
	            }
	            if ((rs = proglog_begin(pip,&u)) >= 0) {
	                {
	                    rs = process(pip) ;
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
	        ex = EX_NOUSER ;
	        bprintf(pip->efp,"%s: userinfo failure (%d)\n",pn,rs) ;
	    } /* end if */
	} else {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	} /* end if (ok) */

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_TIMEDOUT:
	        ex = EX_OK ;
	        break ;
	    case SR_NOMEM:
	        ex = EX_OSERR ;
	        break ;
	    default:
	        ex = mapex(mapexs,rs) ;
	        break ;
	    } /* end switch */
	} else if (if_exit) {
	    ex = EX_TERM ;
	} else if (if_int) {
	    ex = EX_INTR ;
	}

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

	if (pip->open.aparams) {
	    pip->open.aparams = FALSE ;
	    paramopt_finish(&aparams) ;
	}

	if (pip->open.akopts) {
	    keyopt_finish(&akopts) ;
	    pip->open.akopts = FALSE ;
	}

	bits_finish(&pargs) ;

badpargs:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	sighand_finish(&sm) ;

badsighand:
	return ex ;

/* the bad things */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument(s) specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (main) */


int progexit(PROGINFO *pip)
{
	int		rs = SR_OK ;
	if (pip == NULL) return SR_FAULT ;
	if (if_exit) {
	    rs = SR_EXIT ;
	} else if (if_int) {
	    rs = SR_INTR ;
	}
	return rs ;
}
/* end subroutine (progexit) */


/* local subroutines */


/* ARGSUSED */
static void main_sighand(int sn,siginfo_t *sip,void *vcp)
{
	switch (sn) {
	case SIGINT:
	    if_int = TRUE ;
	    break ;
	default:
	    if_exit = TRUE ;
	    break ;
	} /* end switch */
}
/* end subroutine (main_sighand) */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s [-i <idle>] [-d=<intrun>] [-p <port>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-h <host>] [-wto <idle>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* process the program ako-options */
static int procopts(PROGINFO *pip,KEYOPT *kop)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	cchar		*cp ;

	if ((cp = getenv(VAROPTS)) != NULL) {
	    rs = keyopt_loads(kop,cp,-1) ;
	}

	if (rs >= 0) {
	    KEYOPT_CUR	kcur ;
	    if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
	        int	kl, vl ;
	        cchar	*kp, *vp ;

	        while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {
	            int	oi ;
	            if ((oi = matostr(akonames,2,kp,kl)) >= 0) {

	                vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	                switch (oi) {
	                case akoname_ra:
	                    if (! pip->final.reuseaddr) {
	                        pip->have.reuseaddr = TRUE ;
	                        pip->final.reuseaddr = TRUE ;
	                        pip->f.reuseaddr = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.reuseaddr = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_cf:
	                    if (pip->cfname == NULL) {
	                        if (vl > 0) {
	                            cchar	**vpp = &pip->cfname ;
	                            rs = proginfo_setentry(pip,vpp,vp,vl) ;
	                        }
	                    }
	                    break ;
	                case akoname_lf:
	                    if (pip->lfname == NULL) {
	                        if (vl > 0) {
	                            cchar	**vpp = &pip->lfname ;
	                            rs = proginfo_setentry(pip,vpp,vp,vl) ;
	                        }
	                    }
	                    break ;
	                case akoname_daemon:
	                    if (! pip->final.daemon) {
	                        pip->have.daemon= TRUE ;
	                        pip->final.daemon = TRUE ;
	                        pip->f.daemon = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.daemon = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_intrun:
	                    if (! pip->final.intrun) {
	                        pip->have.intrun = TRUE ;
	                        pip->final.intrun = TRUE ;
	                        if (vl > 0) {
	                            int	v ;
	                            rs = cfdecti(vp,vl,&v) ;
	                            pip->intrun = v ;
	                        }
	                    }
	                    break ;
	                case akoname_intidle:
	                    if (! pip->final.intidle) {
	                        pip->have.intidle = TRUE ;
	                        pip->final.intidle = TRUE ;
	                        if (vl > 0) {
	                            int	v ;
	                            rs = cfdecti(vp,vl,&v) ;
	                            pip->intidle = v ;
	                        }
	                    }
	                    break ;
	                case akoname_hostspec:
	                    if (pip->hostspec == NULL) {
	                        if (vl > 0) {
	                            cchar	**vpp = &pip->hostspec ;
	                            rs = proginfo_setentry(pip,vpp,vp,vl) ;
	                        }
	                    }
	                    break ;
	                case akoname_portspec:
	                    if (pip->portspec == NULL) {
	                        if (vl > 0) {
	                            cchar	**vpp = &pip->portspec ;
	                            rs = proginfo_setentry(pip,vpp,vp,vl) ;
	                        }
	                    }
	                    break ;
	                } /* end switch */

	                c += 1 ;
	            } else {
	                rs = SR_INVALID ;
	            }
	            if (rs < 0) break ;
	        } /* end while (looping through key options) */

	        keyopt_curend(kop,&kcur) ;
	    } /* end if (keyopt-cur) */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


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

#if	CF_PRLOCAL
	if (rs >= 0) {
	    const int	dlen = MAXPATHLEN ;
	    cchar	*un = pip->username ;
	    char	dbuf[MAXPATHLEN+1] ;
	    if ((rs = mkpr(dbuf,dlen,VARPRLOCAL,un)) >= 0) {
	        cchar		**vpp = &pip->prlocal ;
	        const int	dl = rs ;
	        rs = proginfo_setentry(pip,vpp,dbuf,dl) ;
	    }
	} /* end if (ok) */
#endif /* CF_PRLOCAL */

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

	return rs ;
}
/* end subroutine (procuserinfo_end) */


static int procuserinfo_logid(PROGINFO *pip)
{
	const int	plen = LOGIDLEN ;
	const int	pv = pip->pid ;
	int		rs ;
	cchar		*nn = pip->nodename ;
	char		pbuf[LOGIDLEN+1] ;
	if ((rs = mkplogid(pbuf,plen,nn,pv)) >= 0) {
	    cchar	**vpp = &pip->logid ;
	    rs = proginfo_setentry(pip,vpp,pbuf,rs) ;
	}
	return rs ;
}
/* end subroutine (procuserinfo_logid) */


static int process(PROGINFO *pip)
{
	struct timeb	now ;
	const int	zlen = DATER_ZNAMESIZE ;
	int		rs ;
	char		*zbuf = pip->zname ;
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/process: ent\n") ;
#endif
	if ((rs = initnow(&now,zbuf,zlen)) >= 0) {
	    pip->daytime = now.time ;
	    if ((rs = dater_start(&pip->d,&now,zbuf,-1)) >= 0) {

	        if (pip->f.background) {
	            rs = procback(pip) ;
	        } else if (pip->f.daemon) {
	            rs = procdaemon(pip) ;
	        } else {
	            rs = procreg(pip) ;
	        }
	        if (pip->open.logprog) {
	            char	tbuf[TIMEBUFLEN+1] ;
	            timestr_logz(pip->daytime,tbuf) ;
	            proglog_printf(pip,"%s exiting",tbuf) ;
	        }

	        dater_finish(&pip->d) ;
	    } /* end if (dater) */
	} /* end if (initnow) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/process: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (process) */


static int procback(PROGINFO *pip)
{
	const int	elen = MAXPATHLEN ;
	int		rs ;
	char		ebuf[MAXPATHLEN+1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procback: ent\n") ;
#endif

	if (pip->open.logprog) {
	    proglog_printf(pip,"mode=background") ;
	    proglog_flush(pip) ;
	}

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: mode=background\n",pip->progname) ;
	    bflush(pip->efp) ;
	}

/* do the spawn */

	if ((rs = proginfo_getename(pip,ebuf,elen)) >= 0) {
	    const int	el = rs ;
	    cchar	*pf = ebuf ;
	    cchar	*tp ;
	    char	pbuf[MAXPATHLEN+1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procback: mid2 rs=%d\n",rs) ;
#endif

	    if ((tp = strnrpbrk(ebuf,el,"/.")) != NULL) {
	        if (tp[0] == '.') ebuf[tp-ebuf] = '\0' ;
	    }

	    if ((rs = pcsgetprogpath(pip->pr,pbuf,ebuf)) > 0) {
	        pf = pbuf ;
	    }

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procback: mid3 rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {
	        int	i = 0 ;
	        cchar	*av[5] ;
	        char	dbuf[10+1] ;
	        av[i++] = pip->progname ;
	        av[i++] = "-daemon" ;
	        if (pip->debuglevel > 0) {
	            bufprintf(dbuf,10,"-D=%u",pip->debuglevel) ;
	            av[i++] = dbuf ;
	        }
	        av[i++] = NULL ;
	        rs = procbacker(pip,pf,av) ;
	    } /* end if (ok) */
	} /* end if (proginfo_getename) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procback: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procback) */


static int procbacker(PROGINFO *pip,cchar *pf,cchar **av)
{
	SPAWNER		s ;
	int		rs ;
	int		rs1 ;
	int		pid = 0 ;
	cchar		**ev = pip->envv ;
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procbacker: ent\n") ;
#endif
	if ((rs = spawner_start(&s,pf,av,ev)) >= 0) {
	    if ((rs = procbackenv(pip,&s)) >= 0) {
	        int	i ;
	        for (i = 0 ; sigignores[i] > 0 ; i += 1) {
	            spawner_sigignore(&s,sigignores[i]) ;
	        }
	        spawner_setsid(&s) ;
	        if (pip->uid != pip->euid)
	            spawner_seteuid(&s,pip->uid) ;
	        if (pip->gid != pip->egid)
	            spawner_setegid(&s,pip->gid) ;
	        for (i = 0 ; i < 2 ; i += 1) {
	            spawner_fdclose(&s,i) ;
	        }
	        if ((rs = spawner_run(&s)) >= 0) {
	            cchar	*fmt ;
	            pid = rs ;
	            if (pip->open.logprog) {
	                fmt = "backgrounded (%u)" ;
	                proglog_printf(pip,fmt,pid) ;
	            }
	        }
	    } /* end if (procbackenv) */
	    rs1 = spawner_finish(&s) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (spawner) */
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procbacker: ret rs=%d pid=%u\n",rs,pid) ;
#endif
	return (rs >= 0) ? pid : rs ;
}
/* end subroutine (procbacker) */


static int procbackenv(PROGINFO *pip,SPAWNER *srp)
{
	BUFFER		b ;
	int		rs ;
	int		rs1 ;
	cchar		*varopts = VAROPTS ;
	if ((rs = buffer_start(&b,ENVBUFLEN)) >= 0) {
	    cchar	*np ;
	    int		v ;
	    int		i ;
	    int		c = 0 ;

	    for (i = 0 ; i < 3 ; i += 1) {
	        np = NULL ;
	        switch (i) {
	        case 0:
	            v = pip->intrun ;
	            if (v > 0) np = "intrun" ;
	            break ;
	        case 1:
	            v = pip->intidle ;
	            if (v > 0) np = "intidle" ;
	            break ;
	        case 2:
	            v = (pip->f.reuseaddr&1) ;
	            if (v > 0) np = "resueaddr" ;
	            break ;
	        } /* end switch */
	        if (np != NULL) {
	            if (c++ > 0) {
	                buffer_char(&b,CH_COMMA) ;
	            }
	            rs = buffer_printf(&b,"%s=%d",np,v) ;
	        }
	        if (rs < 0) break ;
	    } /* end for */

	    if (rs >= 0) {
	        cchar	*vp ;
	        for (i = 0 ; i < 2 ; i += 1) {
	            np = NULL ;
	            switch (i) {
	            case 0:
	                if (pip->hostspec != NULL) {
	                    np = "hostspec" ;
	                    vp = pip->hostspec ;
	                }
	                break ;
	            case 1:
	                if (pip->portspec != NULL) {
	                    np = "portspec" ;
	                    vp = pip->portspec ;
	                }
	                break ;
	            } /* end switch */
	            if (np != NULL) {
	                if (c++ > 0) {
	                    buffer_char(&b,CH_COMMA) ;
	                }
	                rs = buffer_printf(&b,"%s=%s",np,vp) ;
	            } /* end if (non-null) */
	            if (rs < 0) break ;
	        } /* end for */
	    } /* end if (ok) */

	    if ((rs >= 0) && (c > 0)) {
	        if ((rs = buffer_get(&b,&np)) >= 0) {
	            rs = spawner_envset(srp,varopts,np,rs) ;
	        }
	    }

	    rs1 = buffer_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (buffer) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procbackenv: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procbackenv) */


static int procdaemon(PROGINFO *pip)
{
	int		rs ;
	int		rs1 ;
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procdaemon: ent\n") ;
#endif
	if (pip->open.logprog) {
	    proglog_printf(pip,"mode=daemon") ;
	}
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: mode=daemon\n",pip->progname) ;
	}
	if ((rs = procdaemonbegin(pip)) >= 0) {
	    {
	        rs = procreg(pip) ;
	    }
	    rs1 = procdaemonend(pip) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: cannot listen on specified port (%d)\n" ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procdaemon: procdaemon failed rs=%d\n",rs) ;
#endif
	    bprintf(pip->efp,fmt,pn,rs) ;
	} /* end if (procdaemon) */
	return rs ;
}
/* end subroutine (procdaemon) */


static int procdaemonbegin(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		af = AF_UNSPEC ;
	cchar		*pn = pip->progname ;
	cchar		*protoname = PROTONAME ;
	cchar		*hostname = NULL ;
	cchar		*portname = NULL ;
	cchar		*tp ;
	char		hostbuf[MAXHOSTNAMELEN+1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("main/procdaemonbegin: hostspec=%s\n",pip->hostspec) ;
	    debugprintf("main/procdaemonbegin: portspec=%s\n",pip->portspec) ;
	}
#endif

	if ((pip->hostspec != NULL) && (pip->hostspec[0] != '\0')) {
	    hostname = pip->hostspec ;
	    if ((tp = strchr(pip->hostspec,':')) != NULL) {
	        int	ml = (tp - pip->hostspec) ;
	        portname = (tp + 1) ;
	        hostname = hostbuf ;
	        rs = sncpy1w(hostbuf,MAXHOSTNAMELEN,pip->hostspec,ml) ;
	    }
	} /* end if */

	if (rs >= 0) {

	    if ((pip->portspec != NULL) && (pip->portspec[0] != '\0')) {
	        portname = pip->portspec ;
	    }

/* defaults */

	    if ((portname == NULL) || (portname[0] == '\0')) {
	        portname = SVCSPEC_COMSAT ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3)) {
	        debugprintf("main/procdaemonbegin: hostname=%s\n",hostname) ;
	        debugprintf("main/procdaemonbegin: portname=%s\n",portname) ;
	    }
#endif

	    if (pip->debuglevel > 0) {
	        bprintf(pip->efp,"%s: host=%s\n",pn,hostname) ;
	        bprintf(pip->efp,"%s: port=%s\n",pn,portname) ;
	        bflush(pip->efp) ;
	    }

	    if (pip->open.logprog) {
	        proglog_printf(pip,"host=%s",hostname) ;
	        proglog_printf(pip,"port=%s",portname) ;
	        proglog_flush(pip) ;
	    }

	    if ((rs = getportnum(protoname,portname)) >= 0) {
	        const int	port = rs ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3)) {
	            debugprintf("main/procdaemonbegin: port=%d\n",port) ;
	        }
#endif

	        if ((pip->euid == 0) || (port >= IPPORT_RESERVED)) {
	            char	digbuf[DIGBUFLEN+1] ;
	            int		opts = 0 ;

	            if ((rs = ctdeci(digbuf,DIGBUFLEN,port)) >= 0) {
	                if (pip->f.reuseaddr) opts |= 1 ;
	                rs = listenudp(af,hostname,digbuf,opts) ;
	                pip->fd_msg = rs ;
	                pip->open.listen = (rs >= 0) ;
#if	CF_DEBUG
	                if (DEBUGLEVEL(3))
	                    debugprintf("main/procdaemonbegin: "
	                        "listenudp() rs=%d\n",rs) ;
#endif
	            }

	        } else {
	            char	addr[INETXADDRLEN+1] ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3)) {
	                debugprintf("main/procdaemonbegin: af=%u\n",af) ;
	                debugprintf("main/procdaemonbegin: hostname=%s\n",
	                    hostname) ;
	            }
#endif

	            memset(addr,0,INETXADDRLEN) ;
	            if (hostname != NULL) {
	                rs = procdaemonaddr(pip,addr,af,hostname) ;
	                af = rs ;
	                if (rs == SR_NOTFOUND) rs = SR_HOSTUNREACH ;
	            }

#if	CF_DEBUG
	            if (DEBUGLEVEL(3)) {
	                uint	iv = inet4int(addr) ;
	                debugprintf("main/procdaemonbegin: rs=%d af=%u\n",
	                    rs,af) ;
	                debugprintf("main/procdaemonbegin: "
	                    "port=%u addr=\\x%08x\n",port,iv) ;
	            }
#endif /* CF_DEBUG */

	            if (rs >= 0) {
	                if ((rs = openaddr(af,addr,port)) >= 0) {
	                    pip->fd_msg = rs ;
	                    pip->open.listen = TRUE ;
	                }
	            } /* end if (ok) */

	        } /* end if (how to open the network port) */

	    } /* end if (getportnum) */
	} /* end if (ok) */

	if ((rs >= 0) && (pip->pid > 0)) {
	    if ((pip->efp != NULL) && (pip->debuglevel > 0)) {
	        bprintf(pip->efp,"%s: daemon pid=%u\n",pn,pip->pid) ;
	    }
	    if (pip->open.logprog) {
	        proglog_printf(pip,"daemon pid=%u",pip->pid) ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("main/procdaemonbegin: done rs=%d fd_msg=%d\n",
	        rs,pip->fd_msg) ;
	    debugprintf("main/procdaemonbegin: ret rs=%d\n",rs) ;
	}
#endif /* CF_DEBUG */

	return rs ;
}
/* end subroutine (procdaemonbegin) */


static int procdaemonend(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip->open.listen && (pip->fd_msg >= 0)) {
	    pip->open.listen = FALSE ;
	    rs1 = u_close(pip->fd_msg) ;
	    if (rs >= 0) rs = rs1 ;
	    pip->fd_msg = -1 ;
	}

	return rs ;
}
/* end subroutine (procdaemonend) */


static int procdaemonaddr(PROGINFO *pip,void *hap,int af,cchar *hn)
{
	HOSTINFO	hi ;
	int		rs ;
	int		raf = 0 ;

	if (pip == NULL) return SR_FAULT ;
	memset(hap,0,INETXADDRLEN) ;

	if ((rs = hostinfo_start(&hi,af,hn)) >= 0) {

	    rs = 0 ;
	    if (rs == 0) {
	        raf = AF_INET6 ;
	        if ((af == raf) || (af == AF_UNSPEC)) {
	            rs = hostinfo_findaf(&hi,hap,INETXADDRLEN,raf) ;
	        }
	    }
	    if (rs == 0) {
	        raf = AF_INET4 ;
	        if ((af == raf) || (af == AF_UNSPEC)) {
	            rs = hostinfo_findaf(&hi,hap,INETXADDRLEN,raf) ;
	        }
	    }

	    hostinfo_finish(&hi) ;
	} /* end if (hostinfo) */

	if (rs == 0) rs = SR_HOSTUNREACH ;
	return (rs >= 0) ? raf : rs ;
}
/* end subroutine (procdaemonaddr) */


static int procreg(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;

	if (isasocket(pip->fd_msg)) {
	    pip->f.issocket = TRUE ;
	    if ((rs = uc_getsocktype(pip->fd_msg)) >= 0) {
		pip->f.isstream = (rs == SOCK_STREAM) ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("main/procreg: ent fd_msg=%d\n",
	        pip->fd_msg) ;
	    debugprintf("main/procreg: f_issocket=%u f_isstream=%d\n",
	        pip->f.issocket,pip->f.isstream) ;
	}
#endif /* CF_DEBUG */

	if (rs >= 0) {
	if ((rs = progerr_begin(pip)) >= 0) {
	    if ((rs = progloglock_begin(pip)) >= 0) {
	        if ((rs = prognote_begin(pip)) >= 0) {
	            {
	                rs = progcs(pip) ;
			c = rs ;
	            }
	            rs1 = prognote_end(pip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (prognote) */
	        rs1 = progloglock_end(pip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (progloglock) */
	    rs1 = progerr_end(pip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (progerr) */
	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procreg: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procreg) */


static int openaddr(int af,cchar *addr,int port)
{
	int		rs ;
	int		rs1 ;
	int		s = -1 ;
#if	CF_INET6
	if (af == AF_UNSPEC) af = AF_INET6 ;
#else
	if (af == AF_UNSPEC) af = AF_INET4 ;
#endif /* CF_INET6 */
	if ((rs = getprotofamily(af)) >= 0) {
	    SOCKADDRESS	sa ;
	    const int	pf = rs ;
	    if ((rs = sockaddress_start(&sa,af,addr,port,0)) >= 0) {
	        const int	st = SOCK_DGRAM ;
	        const int	proto = IPPROTO_UDP ;
	        {
	            rs = openport(pf,st,proto,&sa) ;
	            s = rs ;
	        }
	        rs1 = sockaddress_finish(&sa) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (sockaddress) */
	} /* end if (getprotofamily) */
	if ((rs < 0) && (s >= 0)) u_close(s) ;
	return (rs >= 0) ? s : rs ;
}
/* end subroutine (openaddr) */


static int hostinfo_findaf(HOSTINFO *hip,char *abuf,int alen,int af)
{
	HOSTINFO_CUR	cur ;
	int		rs ;
	int		al = 0 ;

	if ((rs = hostinfo_curbegin(hip,&cur)) >= 0) {
	    int		f = FALSE ;
	    const uchar	*ap ;

	    while ((rs = hostinfo_enumaddr(hip,&cur,&ap)) >= 0) {
	        al = rs ;

	        switch (al) {
	        case INET4ADDRLEN:
	            f = (af == AF_INET4) || (af == AF_UNSPEC) ;
	            break ;
	        case INET6ADDRLEN:
	            f = (af == AF_INET6) || (af == AF_UNSPEC) ;
	            break ;
	        } /* end switch */

	        if (f) {
	            rs = mnwcpy(abuf,alen,ap,al) ;
	            break ;
	        } else
	            al = 0 ;

	        if (rs < 0) break ;
	    } /* end while */

	    hostinfo_curend(hip,&cur) ;
	} /* end if (cursor) */

	return (rs >= 0) ? al : rs ;
}
/* end subroutine (hostinfo_findaf) */


#if	CF_DEBUG && CF_DEBUGPROC
static int proctest(PROGINFO *pip)
{
	int		rs = SR_OK ;

#if	CF_DEBUGTN
	if (rs >= 0)
	    rs = proctesttn(pip) ;
#endif

#if	CF_DEBUGOT
	if (rs >= 0)
	    rs = proctestot(pip) ;
#endif

	debugprintf("main/proctest: ret rs=%d\n",rs) ;
	return rs ;
}
/* end subroutine (proctest) */
#endif /* CF_DEBUG && CF_DEBUGPROC */


#if	CF_DEBUG && CF_DEBUGTN
static int proctesttn(PROGINFO *pip)
{
	TERMNOTE	tn ;
	const int	max = 3 ;
	const int	opts = 0 ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i = 0 ;
	int		sl ;
	cchar		*sp = "hello world!" ;
	cchar		*recips[3] ;
	recips[i++] = "dam" ;
	recips[i] = NULL ;
	rs = termnote_open(&tn,pip->pr) ;
	debugprintf("main/proctesttn: termnote_open() rs=%d\n",rs) ;
	if (rs >= 0) {
	    sl = strlen(sp) ;
	    rs = termnote_write(&tn,recips,max,opts,sp,sl) ;
	    debugprintf("main/proctesttn: termnote_write() rs=%d\n",rs) ;
	    rs1 = termnote_close(&tn) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (termnote) */
	debugprintf("main/proctestrn: ret rs=%d\n",rs) ;
	return rs ;
}
/* end subroutine (proctesttn) */
#endif /* CF_DEBUG && CF_DEBUGTN */


#if	CF_DEBUG && CF_DEBUGOT
static int proctestot(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		max = 3 ;
	int		opts = 0 ;
	int		i = 0 ;
	int		sl ;
	cchar		*recips[3] ;
	cchar		*sp = "from the underworld!" ;
	recips[i++] = "dam" ;
	recips[i] = NULL ;
	opts |= TERMNOTE_OBIFF ;
	rs = opentermnote(pip->pr,recips,max,opts) ;
	debugprintf("main/proctestot: opentermnote() rs=%d\n",rs) ;
	if (rs >= 0) {
	    int	fd = rs ;
	    sl = strlen(sp) ;
	    rs = u_write(fd,sp,sl) ;
	    u_close(fd) ;
	} /* end if (opentermnote) */
	debugprintf("main/proctestot: ret rs=%d\n",rs) ;
	return rs ;
}
/* end subroutine (proctestot) */
#endif /* CF_DEBUG && CF_DEBUGOT */


