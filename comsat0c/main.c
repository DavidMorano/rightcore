/* main */

/* COMSAT server */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* debug print-outs */
#define	CF_DEBUGPROC	0		/* debug 'testproc()' */
#define	CF_DEBUGTN	0		/* debug 'termnote(3dam)' */
#define	CF_DEBUGOT	0		/* debug 'opentermnote(3dam)' */
#define	CF_SYSLOG	0		/* use SYSLOG */
#define	CF_PLOGID	1		/* use 'plogid' */
#define	CF_INET6	0		/* use INET6 */


/* revision history:

	= 2004-03-01, David A­D­ Morano

	This subroutine was originally written.  


*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This program is a Comsat server.  It listens for Comsat messages
	from the Comsat network port (UDP 'biff') and attempts to extract
	the necessary mail information for display to one or more of
	the user's (mail user's) logged in terminals.


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
#include	<ctype.h>
#include	<syslog.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<sigman.h>
#include	<keyopt.h>
#include	<bits.h>
#include	<bfile.h>
#include	<dater.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<hostinfo.h>
#include	<sockaddress.h>
#include	<termnote.h>
#include	<openport.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"comsatmsg.h"
#include	"csro.h"

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	PWBUFLEN
#ifdef	PWENTRY_BUFLEN
#define	PWBUFLEN	PWENTRY_BUFLEN
#else
#define	PWBUFLEN	1024
#endif
#endif

#ifndef	LOGNAMELEN
#ifdef	TMPX_LUSER
#define	LOGNAMELEN	TMPX_LUSER
#else
#define	LOGNAMELEN	32
#endif
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

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	45		/* can hold 'int128_t' in decimal */
#endif

#define	O_TERM		(O_WRONLY | O_NDELAY | O_NOCTTY)

#ifndef	TO_READ
#define	TO_READ		5
#endif

#ifndef	ANYHOST
#define	ANYHOST		"anyhost"
#endif

#define	PROTONAME	"udp"


/* external subroutines */

extern int	sncpy1(const char *,int,const char *) ;
extern int	sncpy2(const char *,int,const char *,const char *) ;
extern int	sncpy1w(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mnwcpy(void *,int,const void *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	headkeymat(const char *,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	ctdeci(char *,int,int) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	mkpr(char *,int,const char *,const char *) ;
extern int	mkplogid(char *,int,const char *,int) ;
extern int	getportnum(const char *,const char *) ;
extern int	listenudp(int,const char *,const char *,int) ;
extern int	opentermnote(const char *,const char **,int,int) ;
extern int	logfile_userinfo(LOGFILE *,USERINFO *,time_t,
			const char *,const char *) ;
extern int	initnow(struct timeb *,char *,int) ;
extern int	isprintlatin(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;
extern int	proglogfname(struct proginfo *,char *,
			const char *,const char *) ;
extern int	progreader(struct proginfo *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

#if	CF_DEBUG && CF_DEBUGPROC
static int proctest(struct proginfo *) ;
#if	CF_DEBUGTN
static int proctesttn(struct proginfo *) ;
#endif
#if	CF_DEBUGOT
static int proctestot(struct proginfo *) ;
#endif
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local typedefs */

#if	defined(IRIX) && (! defined(TYPEDEF_INADDRT))
#define	TYPEDEF_INADDRT	1
typedef unsigned int	in_addr_t ;
#endif


/* local structures */


/* forward references */

static int	usage(struct proginfo *) ;

static int	procopts(struct proginfo *,KEYOPT *) ;
static int	procdaemonbegin(struct proginfo *,const char *,const char *) ;
static int	procdaemonend(struct proginfo *) ;
static int	procdaemonaddr(struct proginfo *,void *,int,const char *) ;

static void	sighand_int(int) ;

static int	hostinfo_findaf(HOSTINFO *,char *,int,int) ;

#ifdef	COMMENT
static int	storemsgs(struct proginfo *,CSRO *,const char *,int) ;
#endif


/* local variables */

static volatile int	if_int ;
static volatile int	if_exit ;

static const int	sigblocks[] = {
	0
} ;

static const int	sigignores[] = {
	SIGPIPE,
	SIGPOLL,
	0
} ;

static const int	sigints[] = {
	SIGHUP,
	SIGUSR1,
	SIGUSR2,
	SIGINT,
	SIGTERM,
	0
} ;

static const char *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"sn",
	"of",
	"ef",
	"lf",
	"wto",
	"ra",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_sn,
	argopt_ef,
	argopt_of,
	argopt_lf,
	argopt_wto,
	argopt_ra,
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
	{ SR_OVERFLOW, EX_SOFTWARE },
	{ SR_NOANODE, EX_SOFTWARE },
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;

static const char *akonames[] = {
	"ra",
	NULL
} ;

enum akonames {
	akoname_ra,
	akoname_overlast
} ;


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;
	struct timeb	now ;
	SIGMAN		sm ;
	BITS		pargs ;
	KEYOPT		akopts ;
	USERINFO	u ;
	sigset_t	oldsigmask, newsigmask ;
	bfile		errfile ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	rs, rs1 ;
	int	n, i, j ;
	int	size ;
	int	cl ;
	int	v ;
	int	logsize = -1 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	char	userbuf[USERINFO_LEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*hostspec = NULL ;
	const char	*portspec = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*lfname = NULL ;
	const char	*idlespec = NULL ;
	const char	*cp ;


	if_int = 0 ;
	if_exit = 0 ;

	rs = sigman_start(&sm, sigblocks,sigignores,sigints,sighand_int) ;
	if (rs < 0) goto ret0 ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getenv(VARDEBUGFNAME)) == NULL) {
	    if ((cp = getenv(VARDEBUGFD1)) == NULL)
	        cp = getenv(VARDEBUGFD2) ;
	}
	if (cp != NULL)
	    debugopen(cp) ;
	debugprintf("main: starting\n") ;
#endif /* CF_DEBUGS */

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

/* initialize */

	pip->verboselevel = 1 ;
	pip->intrun = -2 ;
	pip->intidle = -1 ;
	pip->inttermnote = -1 ;
	pip->fd_msg = FD_STDIN ;
	pip->daytime = time(NULL) ;

/* start parsing the arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

	ai = 0 ;
	ai_max = 0 ;
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

	        if (isdigit(argp[1])) {

	            argval = NULL ;

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

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

	                case argopt_root:
	                    if (argr <= 0) {
	                        rs = SR_INVALID ;
	                        break ;
	                    }
	                    argp = argv[++ai] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;
	                    if (argl)
	                        pr = argp ;
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
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            sn = argp ;
	                    }
	                    break ;

/* output file name */
	                case argopt_of:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            ofname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            ofname = argp ;
	                    }
	                    break ;

/* error file name */
	                case argopt_ef:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            efname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            efname = argp ;
	                    }
	                    break ;

/* log file name */
	                case argopt_lf:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            lfname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            lfname = argp ;
	                    }
	                    break ;

/* maximum idle time */
	                case argopt_wto:
			    cp = NULL ;
			    cl = -1 ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
				    idlespec = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            idlespec = argp ;
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

/* handle all keyword defaults */
	                default:
	                    rs = SR_INVALID ;
	                    break ;

	                } /* end switch */

	            } else {

	                while (akl--) {
	                    int	kc = (*akp & 0xff) ;

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
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pip->maildname = argp ;
	                        break ;

/* program root */
	                    case 'R':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pr = argp ;
	                        break ;

/* daemon mode */
	                    case 'd':
	                        pip->f.daemon = TRUE ;
	                        pip->intrun = -1 ;
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
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            hostspec = argp ;
	                        break ;

/* maximum idle time */
	                    case 'i':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
				    idlespec = argp ;
	                        break ;

/* n-parallism */
	                    case 'n':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                                rs = cfdecti(argp,argl,&v) ;
	                                pip->npar = v ;
	                        }
	                        break ;

/* options */
	                    case 'o':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            rs = keyopt_loads(&akopts,argp,argl) ;
	                        break ;

/* port specification */
	                    case 'p':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            portspec = argp ;
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
	}

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUGS
	    debugprintf("main: apr=%s\n",pr) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(1)) {
	    debugprintf("main: debuglevel=%u\n",
	        pip->debuglevel) ;
	}
#endif

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;
	}

/* get the program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;

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

	rs = procopts(pip,&akopts) ;
	if (rs < 0) {
	    ex = EX_USAGE ;
	    goto retearly ;
	}

/* some initialization */

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* get some host/user information */

	rs = userinfo(&u,userbuf,USERINFO_LEN,NULL) ;
	if (rs < 0) {
	    ex = EX_NOUSER ;
	    goto retearly ;
	}

	pip->nodename = u.nodename ;
	pip->domainname = u.domainname ;
	pip->username = u.username ;
	pip->logid = u.logid ;

	pip->pid = u.pid ;
	pip->euid = u.euid ;
	pip->uid = u.uid ;

/* check some parametes */

	if (pip->maildname == NULL) pip->maildname = getenv(VARMAILDNAME) ;
	if (pip->maildname == NULL) pip->maildname = MAILDNAME ;

/* any idle specification? */

	if (idlespec != NULL) {
	    cp = idlespec ;
	    cl = -1 ;
	                            if (isdigit(cp[0])) {
	                                rs = cfdecti(cp,cl,&v) ;
	                                pip->intidle = v ;
	                            } else if ((tolower(cp[0]) == 'i') ||
	                                (cp[0] == '-')) {
	                                pip->intidle = INT_MAX ;
	                            } else
	                                rs = SR_INVALID ;
	} /* end if (idle-spec processing) */

	if (rs < 0) {
	    ex = EX_USAGE ;
	    goto retearly ;
	}

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

	if (pip->inttermnote < 0)
	    pip->inttermnote = DEFINTTERMNOTE ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
		debugprintf("main: pn=%s\n",pip->progname) ;
#endif

#if	CF_PLOGID
	if (rs >= 0) {
	    char	logidbuf[LOGIDLEN+1] ;
	    v = pip->pid ;
	    if ((rs = mkplogid(logidbuf,LOGIDLEN,pip->nodename,v)) >= 0) {
		const char	**vpp = &pip->logid ;
	        cl = rs ;
		rs = proginfo_setentry(pip,vpp,logidbuf,cl) ;
	    } /* end if (mkplogid) */
	}
#endif /* CF_PLOGID */

/* do we have an activity log file? */

	if (pip->logsize <= 0) pip->logsize = LOGSIZE ;

	if (rs >= 0) {
	    const char	*logcname = LOGCNAME ;
	    const char	*logid = pip->logid ;

	    rs = proglogfname(pip,tmpfname,logcname,lfname) ;
	    if (rs > 0) lfname = tmpfname ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: logfname=%s logid=%s\n",
	            lfname,pip->logid) ;
#endif

	    if ((rs >= 0) && (lfname != NULL)) {
		const mode_t	om = 0666 ;
		const char	*logid = pip->logid ;

	    if ((rs1 = logfile_open(&pip->lh,lfname,0,om,logid)) >= 0) {
	        pip->open.logfile = TRUE ;

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: logfile=%s\n",
	            pip->progname,lfname) ;

/* we opened it, maintenance this log file if we have to */

	    if (pip->logsize > 0)
	        logfile_checksize(&pip->lh,pip->logsize) ;

	    logfile_userinfo(&pip->lh,&u,
	        pip->daytime,pip->searchname,pip->version) ;

	    } /* end if (logfile opened) */

	    if (pip->debuglevel > 0)
		bprintf(pip->efp,"%s: logfile=%s (%d)\n",
			pip->progname,lfname,rs1) ;

	} /* end if (we have a log file or not) */

	} /* end block */

	if ((rs >= 0) && pip->open.logfile) {
	    if ((pip->intidle >= 0) && (pip->intidle != INT_MAX))
	        logfile_printf(&pip->lh,"intidle=%u\n",pip->intidle) ;
	}

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
		debugprintf("main: pn=%s\n",pip->progname) ;
#endif

	if (rs >= 0) {
	    const int	dlen = MAXPATHLEN ;
	    int		dl ;
	    char	dbuf[MAXPATHLEN+1] ;
	    rs = mkpr(dbuf,dlen,VARPRLOCAL,u.domainname) ;
	    dl = rs ;
	    if (rs >= 0) {
	        const char	**vpp = &pip->prlocal ;
	        rs = proginfo_setentry(pip,vpp,dbuf,dl) ;
	    }
	} /* end if (ok) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
		debugprintf("main: pn=%s\n",pip->progname) ;
#endif

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGPROC
	if ((rs >= 0) && DEBUGLEVEL(2)) {
	    rs = proctest(pip) ;
	}
#endif /* CF_DEBUG */

	if (rs >= 0) {
	    rs = initnow(&now,pip->zname,DATER_ZNAMESIZE) ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
		char	timebuf[TIMEBUFLEN+1] ;
	        debugprintf("main: initnow timestr=%s\n",
			timestr_logz(now.time,timebuf)) ;
	        debugprintf("main: initnow milli=%u\n",now.millitm) ;
	        debugprintf("main: initnow timezone=%u\n",now.timezone) ;
	        debugprintf("main: initnow dstflag=%u\n",now.dstflag) ;
	        debugprintf("main: initnow zname=%s\n",pip->zname) ;
	    }
#endif /* CF_DEBUG */
	        pip->daytime = now.time ;
	} /* end block (getting some current time stuff) */

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    bprintf(pip->efp,"%s: daemon mode=%u\n",
		pip->progname,pip->f.daemon) ;
	}

	if (rs >= 0) {
	    if ((rs = dater_start(&pip->d,&now,pip->zname,-1)) >= 0) {

	        if (pip->f.daemon) {
	            rs = procdaemonbegin(pip,hostspec,portspec) ;
	            if (rs < 0) {
	                ex = EX_NOINPUT ;
	                bprintf(pip->efp,
	                    "%s: cannot listen on specified port (%d)\n",
	                    pip->progname,rs) ;
	            }
	        } /* end if (daemon mode) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: mid rs=%d pid=%u\n",rs,pip->pid) ;
	    debugprintf("main: pn=%s\n",pip->progname) ;
	}
#endif

	        if ((rs >= 0) && ((! pip->f.daemon) || (pip->pid == 0)))
	            rs = progreader(pip) ;

	        if (pip->f.daemon) {
	            rs1 = procdaemonend(pip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (daemon mode) */

	        dater_finish(&pip->d) ;
	    } /* end if (dater) */
	} /* end if (ok) */

/* we are done */
done:
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
	} else if (if_int)
	    ex = EX_INTR ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: open.logfile=%u pid=%u\n",
		pip->open.logfile,pip->pid) ;
#endif

	if (pip->open.logfile && ((! pip->f.daemon) || (pip->pid == 0))) {
	    char	timebuf[TIMEBUFLEN + 1] ;
	    if (rs == SR_TIMEDOUT) {
	        rs1 = logfile_printf(&pip->lh,"%s exiting %u ­ on work timeout",
	            timestr_logz(pip->daytime,timebuf),ex) ;
	    } else
	        rs1 = logfile_printf(&pip->lh,"%s exiting %u (%d)",
	            timestr_logz(pip->daytime,timebuf),ex,rs) ;
#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: logfile_printf() rs=%d\n",rs1) ;
#endif
	} /* end if */

	if (pip->open.logfile) {
	    pip->open.logfile = FALSE ;
	    logfile_close(&pip->lh) ;
	} /* end if */

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

	sigman_finish(&sm) ;

ret0:
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


int progexit(pip)
struct proginfo	*pip ;
{
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;

	if (if_exit) rs = SR_EXIT ;
	else if (if_int) rs = SR_INTR ;

	return rs ;
}
/* end subroutine (progexit) */


/* local subroutines */


static void sighand_int(sn)
int	sn ;
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
/* end subroutine (sighand_int) */


static int usage(pip)
struct proginfo	*pip ;
{
	int		rs ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [-i <idle>] [-d=<intrun>] [-p <port>]\n" ;
	rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-h <host>] [-wto <idle>]\n" ;
	rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* process the program ako-options */
static int procopts(pip,kop)
struct proginfo	*pip ;
KEYOPT		*kop ;
{
	int		rs = SR_OK ;
	int		c = 0 ;
	const char	*cp ;

	if ((cp = getenv(VAROPTS)) != NULL)
	    rs = keyopt_loads(kop,cp,-1) ;

	if (rs >= 0) {
	    KEYOPT_CUR	kcur ;
	    if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
	        int		kl, vl ;
	        int		oi ;
	        const char	*kp, *vp ;

	    while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	        vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	        if ((oi = matostr(akonames,2,kp,kl)) >= 0) {

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

	                } /* end switch */

	                c += 1 ;
	            } /* end if (valid option) */

		    if (rs < 0) break ;
	        } /* end while (looping through key options) */

	        keyopt_curend(kop,&kcur) ;
	    } /* end if (keyopt-cursor) */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int procdaemonbegin(pip,hostspec,portspec)
struct proginfo	*pip ;
const char	*hostspec ;
const char	*portspec ;
{
	struct servent	se ;
	uint		port = 0 ;
	const int	selen = SEBUFLEN ;
	const int	proto = IPPROTO_UDP ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		af = AF_UNSPEC ;
	const char	*protoname = PROTONAME ;
	const char	*hostname = NULL ;
	const char	*portname = NULL ;
	const char	*tp ;
	char		hostbuf[MAXHOSTNAMELEN+1] ;
	char		sebuf[SEBUFLEN + 1] ;

#ifdef	COMMENT
	if ((hostspec == NULL) || (hostspec[0] == '\0'))
	    hostspec = ANYHOST ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("main/procdaemonbegin: hostspec=%s\n",hostspec) ;
	    debugprintf("main/procdaemonbegin: portspec=%s\n",portspec) ;
	}
#endif

	if ((hostspec != NULL) && (hostspec[0] != '\0')) {
	    hostname = hostspec ;
	    if ((tp = strchr(hostspec,':')) != NULL) {
		int	ml = (tp - hostspec) ;
	        portname = (tp + 1) ;
	        hostname = hostbuf ;
	        rs = sncpy1w(hostbuf,MAXHOSTNAMELEN,hostspec,ml) ;
	    }
	} /* end if */

	if (rs < 0) goto ret0 ;

	if ((portspec != NULL) && (portspec[0] != '\0')) {
	    portname = portspec ;
	}

/* defaults */

#ifdef	COMMENT /* not needed */
	if ((hostname == NULL) || (hostname[0] == '\0'))
	    hostname = ANYHOST ;
#endif

	if ((portname == NULL) || (portname[0] == '\0'))
	    portname = SVCSPEC_COMSAT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("main/procdaemonbegin: hostname=%s\n",hostname) ;
	    debugprintf("main/procdaemonbegin: portname=%s\n",portname) ;
	}
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: host=%s\n",pip->progname,hostname) ;
	    bprintf(pip->efp,"%s: port=%s\n",pip->progname,portname) ;
	}

	if (pip->open.logfile) {
	    logfile_printf(&pip->lh,"host=%s",hostname) ;
	    logfile_printf(&pip->lh,"port=%s",portname) ;
	}

	port = IPPORT_BIFFUDP ;
#ifdef	COMMENT
	rs1 = uc_getservbyname(portname,protoname,&se,sebuf,selen) ;
	if (rs1 >= 0) port = (uint) ntohs(se.s_port) ;
#else
	rs = getportnum(protoname,portname) ;
	port = rs ;
#endif /* COMMENT */
	if (rs < 0) goto ret0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("main/procdaemonbegin: port=%d\n",port) ;
	}
#endif

/* prepare for the fork */

	if (pip->open.logfile)
	    logfile_flush(&pip->lh) ;

	if (pip->efp)
	    bflush(pip->efp) ;

/* do the fork */

	pip->pid = 0 ;
	if (pip->debuglevel == 0) {
	    rs = uc_fork() ;
	    pip->pid = rs ;
	}

	if (rs < 0) goto ret0 ;

/* conitnue with the child process (or parent if no child) */

	if (pip->pid == 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
		debugprintf("main: child\n") ;
		if (pip->open.logfile) {
	    		logfile_printf(&pip->lh,"hello there child") ;
	    		logfile_flush(&pip->lh) ;
		}
	    }
#endif

	    if (pip->debuglevel == 0) {
#ifdef	COMMENT
	        for (i = 0 ; i < 3 ; i += 1)
	            u_close(i) ;
#endif
	        u_setsid() ;
	    }

/* can we bind our port ourselves? */

	    if ((pip->euid == 0) || (port >= IPPORT_RESERVED)) {
	        char	digbuf[DIGBUFLEN+1] ;
		int	opts = 0 ;

	        rs = ctdeci(digbuf,DIGBUFLEN,port) ;
	        if (rs >= 0) {
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
		char		addr[INETXADDRLEN+1] ;

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
		    uint	*uip = (uint *) addr ;
		    debugprintf("main/procdaemonbegin: rs=%d af=%u\n",
		    rs,af) ;
		    debugprintf("main/procdaemonbegin: port=%u addr=\\x%08x\n",
		    port,(*uip)) ;
		}
#endif

	        if (rs >= 0) {
	            SOCKADDRESS	sa ;
#if	CF_INET6
		    if (af == AF_UNSPEC) af = AF_INET6 ;
#else
		    if (af == AF_UNSPEC) af = AF_INET4 ;
#endif /* CF_INET6 */
	            if ((rs = sockaddress_start(&sa,af,addr,port,0)) >= 0) {

	                rs = openport(af,SOCK_DGRAM,proto,&sa) ;
	                pip->fd_msg = rs ;
	                pip->open.listen = (rs >= 0) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/procdaemonbegin: openport() rs=%d\n",rs) ;
#endif

	                sockaddress_finish(&sa) ;
	            } /* end if (sockaddress) */
	        } /* end if */

	    } /* end if (how to open the network port) */

	} /* end if (child) */

	if ((rs >= 0) && (pip->pid > 0)) {
	    if ((pip->efp != NULL) && (pip->debuglevel > 0))
	        bprintf(pip->efp,"%s: daemon pid=%u\n",pip->progname,pip->pid) ;
	    if (pip->open.logfile)
	        logfile_printf(&pip->lh,"daemon pid=%u",pip->pid) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/procdaemonbegin: done rs=%d fd_msg=%d\n",
		rs,pip->fd_msg) ;
#endif

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/procdaemonbegin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procdaemonbegin) */


static int procdaemonend(pip)
struct proginfo	*pip ;
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


static int procdaemonaddr(pip,hap,af,hn)
struct proginfo	*pip ;
void		*hap ;
int		af ;
const char	*hn ;
{
	HOSTINFO	hi ;
	HOSTINFO_CUR	cur ;
	int		rs ;
	int		raf = 0 ;
	int		c = 0 ;

	memset(hap,0,INETXADDRLEN) ;

	if ((rs = hostinfo_start(&hi,af,hn)) >= 0) {
	    rs = 0 ;

	    if (rs == 0) {
	        raf = AF_INET6 ;
	        if ((af == raf) || (af == AF_UNSPEC))
		    rs = hostinfo_findaf(&hi,hap,INETXADDRLEN,raf) ;
	    }
	    if (rs == 0) {
	        raf = AF_INET4 ;
	        if ((af == raf) || (af == AF_UNSPEC))
		    rs = hostinfo_findaf(&hi,hap,INETXADDRLEN,raf) ;
	    }

	    hostinfo_finish(&hi) ;
	} /* end if (hostinfo) */

	if (rs == 0) rs = SR_HOSTUNREACH ;
	return (rs >= 0) ? raf : rs ;
}
/* end subroutine (procdaemonaddr) */


static int hostinfo_findaf(hip,abuf,alen,af)
HOSTINFO	*hip ;
char		abuf[] ;
int		alen ;
int		af ;
{
	HOSTINFO_CUR	cur ;
	int		rs ;
	int		al = 0 ;
	int		f = FALSE ;

	if ((rs = hostinfo_curbegin(hip,&cur)) >= 0) {
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
static int proctest(pip)
struct proginfo	*pip ;
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
static int proctesttn(pip)
struct proginfo	*pip ;
{
	TERMNOTE	tn ;
	const int	max = 3 ;
	const int	opts = 0 ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		sl ;
	const char	*sp = "hello world!" ;
	const char	*recips[3] ;

	    i = 0 ;
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
static int proctestot(pip)
struct proginfo	*pip ;
{
	int		rs = SR_OK ;
	int		max = 3 ;
	int		opts = 0 ;
	int		i ;
	int		sl ;
	const char	*recips[3] ;
	const char	*sp = "from the underworld!" ;

	    i = 0 ;
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


#ifdef	COMMENT

static int process(pip)
struct proginfo	*pip ;
{
	int		rs = SR_OK ;

#if	CF_SYSLOG
	openlog("comsat", LOG_PID, LOG_DAEMON) ;
#endif

	rs = u_chdir(maildname) ;
	if (rs < 0) {

#if	CF_SYSLOG
	    syslog(LOG_ERR, "chdir: %s: %m", maildname) ;
#endif

	    if (! pip->f.daemon) {
	        opts = FM_TIMED ;
	        uc_recve(fd_msg,msgbuf,MSGBUFLEN,0,5,opts) ;
	    }

	    goto done ;
	}

	lastmsgtime = pip->daytime ;

/* OK, loop as appropriate */

	pip->ti_start = time(NULL) ;

	signal(SIGTTOU, SIG_IGN) ;

	signal(SIGCHLD, reapchildren) ;

	for (;;) {
	    CSRO	cmsgs ;

	    size = sizeof(struct comsatmsg_mailoff) ;
	    opts = 0 ;

	    rs = csro_start(&cmsgs,20) ;
	    if (rs < 0) break ;

	    if (pip->open.logfile)
	        logfile_flush(&pip->lh) ;

	    opts = FM_TIMED ;
	    timeout = (f_child) ? 1 : TO_READ ;
	    rs = uc_recve(fd_msg,msgbuf,MSGBUFLEN,0,timeout,opts) ;
	    cc = rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main: uc_recve() rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {

	        msgbuf[cc] = '\0' ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("main: msgbuf=>%t<\n",msgbuf,cc) ;
#endif

	        rs1 = 0 ;
	        if (cc > 0) {

	            rs1 = storemsgs(pip,&cmsgs,msgbuf,cc) ;
	        }

	        while (rs1 >= 0) {

	            rs1 = uc_recve(fd_msg,msgbuf,MSGBUFLEN,0,1,opts) ;
	            cc = rs1 ;
	            if (rs1 < 0)
	                break ;

	            msgbuf[cc] = '\0' ;
	            if (cc > 0) {

	                rs1 = storemsgs(pip,&cmsgs,msgbuf,cc) ;
	            }

	        } /* end while (additional messages) */

	        pip->daytime = time(NULL) ;

	        {
	            struct comsatmsg_mailoff	m ;

	            CSRO_NCURSOR	ncur ;

	            CSRO_VCURSOR	vcur ;

	            CSRO_VALUE	*vp ;

	            const char	*np ;


	            csro_ncurbegin(&cmsgs,&ncur) ;

	            while (csro_getname(&cmsgs,&ncur,&np) >= 0) {

	                csro_vcurbegin(&cmsgs,&vcur) ;

	                while (csro_getvalue(&cmsgs,np,&vcur,&vp) >= 0) {

	                    m.offset = vp->mailoff ;
	                    sncpy1(m.username,LOGNAMELEN,vp->mailname) ;

	                    sncpy1(m.fname,MAXNAMELEN,vp->fname) ;

	                    sighold(SIGALRM) ;

	                    f_child = TRUE ;
	                    mailfor(pip,maildname,&m) ;

	                    sigrelse(SIGALRM) ;

	                } /* end while (looping on messages) */

	                csro_vcurend(&cmsgs,&vcur) ;
	            } /* end while */

	            csro_ncurend(&cmsgs,&ncur) ;
	        } /* end if */

	        pip->daytime = time(NULL) ;

	        lastmsgtime = pip->daytime ;

	    } else
	        pip->daytime = time(NULL) ;

	    csro_finish(&cmsgs) ;

	    if ((rs < 0) && (rs != SR_TIMEDOUT) && (rs != SR_INTR))
	        break ;

	    if (if_int) {
	        rs = SR_INTR ;
	        break ;
	    }

	    if ((pip->intidle > 0) &&
	        ((pip->daytime - lastmsgtime) >= pip->intidle))
	        break ;

	    rs1 = 1 ;
	    while (rs1 > 0) {

	        rs1 = u_waitpid(((pid_t) -1),&childstat,WNOHANG) ;

	    }

	    if ((rs1 <= 0) && (rs == SR_TIMEDOUT))
	        f_child = FALSE ;

	    if ((pip->daytime - pip->ti_lastlog) > pip->intlogcheck) {

	        lastlogtime = pip->daytime ;
	        logfile_checksize(&pip->lh,logsize) ;

	    }

	    if (pip->intrun > 0) {
	        if ((pip->daytime - pip->ti_start) >= pip->intrun)
	            break ;
	    }

	} /* end for (looping) */

	return rs ;
}
/* end subroutine (process) */

#endif /* COMMENT */


#ifdef	COMMENT

/* parse out and store COMSAT mail-offset messages */
static int storemsgs(pip,mlp,msgbuf,msglen)
struct proginfo	*pip ;
CSRO		*mlp ;
const char	msgbuf[] ;
int		msglen ;
{
	struct comsatmsg_mailoff	m0 ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		sl, cl ;
	int		c = 0 ;
	const char	*tp, *sp, *cp ;

	sp = msgbuf ;
	sl = msglen ;
	while ((rs >= 0) && ((tp = strnchr(sp,sl,'\n')) != NULL)) {

	    cp = sp ;
	    cl = (tp - sp) ;
	    if (cl > 0) {

	        rs1 = comsatmsg_mailoff(cp,cl,1,&m0) ;

	        if (rs1 >= 0) {
	            c += 1 ;
	            rs = csro_add(mlp,m0.username,m0.fname,m0.offset) ;
	        }

	    } /* end if */

	    sl -= ((tp + 1) - sp) ;
	    sp = (tp + 1) ;

	} /* end while */

	if ((rs >= 0) && (sl > 0)) {

	    rs1 = comsatmsg_mailoff(sp,sl,1,&m0) ;

	    if (rs1 >= 0) {
	        c += 1 ;
	        rs = csro_add(mlp,m0.username,m0.fname,m0.offset) ;
	    }
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (storemsgs) */

#endif /* COMMENT */


