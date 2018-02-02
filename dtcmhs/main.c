/* main (DTCMHS) */

/* DTCM Have Server */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* debug print-outs */
#define	CF_GETEXECNAME	1		/* use 'getexecname(3c)' */
#define	CF_TMPX		1		/* use TMPX (Solaris is broken) */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This program is a DTCM_HAVE server. It listens for requests and returns
        the present-status on a DTCM calendar.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<sys/uio.h>
#include	<netinet/in.h>
#include	<termios.h>
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#include	<stdio.h>
#include	<syslog.h>
#include	<netdb.h>
#include	<pwd.h>
#include	<grp.h>
#include	<utmpx.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<bfile.h>
#include	<userinfo.h>
#include	<vecstr.h>
#include	<vecobj.h>
#include	<sockaddress.h>
#include	<sbuf.h>
#include	<msg.h>
#include	<exitcodes.h>
#include	<tmpx.h>
#include	<localmisc.h>

#include	"havemsg.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	LOGNAMELEN
#ifdef	TMPX_LUSER
#define	LOGNAMELEN	TMPX_LUSER
#else
#define	LOGNAMELEN	32
#endif
#endif

#define	TO_READ		5

#define	PROTONAME	"udp"


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	headkeymat(const char *,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	matmsgstart(const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	getportnum(cchar *,cchar *) ;
extern int	listenudp(int,const char *,const char *,int) ;
extern int	openport(int,int,int,SOCKADDRESS *) ;
extern int	logfile_userinfo(LOGFILE *,USERINFO *,time_t,
			const char *,const char *) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;

extern cchar	*getourenv(cchar **,cchar *) ;

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

static int	usage(PROGINFO *) ;

static void	sighand_int(int) ;

static int	procmsg(PROGINFO *,int,char *,int,struct sockaddr *,int) ;


/* local variables */

static volatile int	if_int ;

static const int	sigblocks[] = {
	0
} ;

static const int	sigignores[] = {
	SIGHUP,
	SIGPIPE,
	SIGPOLL,
	0
} ;

static const int	sigints[] = {
	SIGUSR1,
	SIGUSR2,
	SIGINT,
	SIGTERM,
	0
} ;

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"of",
	"wto",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_of,
	argopt_wto,
	argopt_overlast
} ;


/* exported subroutines */


int main(argc, argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct sigaction	san ;
	struct sigaction	sao[nelem(sigints) + nelem(sigignores)] ;
	struct sockaddr	*sap ;
	struct ustat	sb ;
	PROGINFO	pi, *pip = &pi ;
	USERINFO	u ;
	SOCKADDRESS	sa ;
	sigset_t	oldsigmask, newsigmask ;
	time_t		lastlogtime ;
	time_t		lastmsgtime ;

	bfile	errfile ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan ;
	int	rs, rs1, n, size, len, loglen, i, j ;
	int	mflags, opts, childstat, cc, timeout ;
	int	salen, ci ;
	int	sl, cl, ml ;
	int	fd_msg, fd_debug = -1 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_entok = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*pr = NULL ;
	const char	*caldname = NULL ;
	const char	*hostspec = NULL ;
	const char	*portspec = NULL ;
	const char	*ofname = NULL ;
	const char	*tp, *sp, *cp ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 1] ;
	char	userbuf[USERINFO_LEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	logfname[MAXPATHLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	proginfo_start(pip,envv,argv[0],VERSION) ;

	if ((cp = getourenv(envv,VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

	if ((cp = getenv(VARERRORFNAME)) != NULL) {
	    rs = bopen(&errfile,cp,"wca",0666) ;
	} else {
	    rs = bopen(&errfile,BFILE_STDERR,"dwca",0666) ;
	}
	if (rs >= 0) {
	    pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	}

/* initialize some basic stuff */

	pip->verboselevel = 1 ;
	pip->runint = -2 ;
	pip->maxidle = -1 ;

/* start parsing the arguments */

	for (ai = 0 ; ai < MAXARGGROUPS ; ai += 1)
	    argpresent[ai] = 0 ;

	ai = 0 ;
	ai_max = 0 ;
	ai_pos = 0 ;
	argr = argc - 1 ;
	while ((rs >= 0) && (argr > 0)) {

	    argp = argv[++ai] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optminus || f_optplus)) {
		const int	ach = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ach)) {

		    rs = cfdeci((argp + 1),(argl - 1),&argvalue) ;

	        } else if (ach == '-') {

	            ai_pos = ai ;
	            break ;

	        } else {

	            aop = argp + 1 ;
	            aol = argl - 1 ;
	            akp = aop ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {

	                akl = avp - aop ;
	                avp += 1 ;
	                avl = aop + aol - avp ;
	                f_optequal = TRUE ;

	            } else {

	                akl = aol ;
	                avl = 0 ;

	            }

	            if ((kwi = matstr(argopts,akp,akl)) >= 0) {

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

	                            rs = cfdeci(avp,avl,
	                                &pip->verboselevel) ;

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

/* maximum idle time */
	                case argopt_wto:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl) {
				    const int	ch = MKCHAR(avp[0]) ;

	                            if (isdigitlatin(ch)) {
	                                rs = cfdecti(avp,avl,&pip->maxidle) ;

	                            } else if ((tolower(avp[0]) == 'i') ||
	                                (avp[0] == '-')) {
	                                pip->maxidle = INT_MAX ;

	                            } else
	                                rs = SR_INVALID ;

	                        }

			    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl) {
				    const int	ch = MKCHAR(argp[0]) ;

	                            if (isdigitlatin(ch)) {
	                                rs = cfdecti(argp,argl,&pip->maxidle) ;

	                            } else if ((tolower(argp[0]) == 'i') ||
	                                (argp[0] == '-')) {
	                                pip->maxidle = INT_MAX ;

	                            } else
	                                rs = SR_INVALID ;

	                        }

			    }

	                        break ;

/* handle all keyword defaults */
	                default:
	                    rs = SR_INVALID ;
	                    bprintf(pip->efp,
	                        "%s: option (%s) not supported\n",
	                        pip->progname,akp) ;

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
	                            if (avl)
	                                rs = cfdeci(avp,avl,
	                                    &pip->debuglevel) ;

	                        }

	                        break ;

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* calendar spool directory */
	                    case 'S':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            caldname = argp ;

	                        break ;

	                    case 'd':
	                        pip->f.daemon = TRUE ;
	                        pip->runint = -1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                rs = cfdecti(avp,avl,
	                                    &pip->runint) ;

	                        }

	                        break ;

/* host specifications */
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

	                        if (argl) {
				    const int	ch = MKCHAR(argp[0]) ;

	                            if (isdigitlatin(ch)) {
	                                rs = cfdecti(argp,argl,&pip->maxidle) ;

	                            } else if ((tolower(argp[0]) == 'i') ||
	                                (argp[0] == '-')) {
	                                pip->maxidle = INT_MAX ;

	                            } else
	                                rs = SR_INVALID ;

	                        }

	                        break ;

/* port specifications */
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

/* quiet mode */
	                    case 'q':
	                        pip->f.quiet = TRUE ;
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
	                        bprintf(pip->efp,
	                            "%s: unknown option - %c\n",
	                            pip->progname,*aop) ;

	                    } /* end switch */

	                    akp += 1 ;
	                    if (rs < 0)
	                        break ;

	                } /* end while */

	            } /* end if (individual option key letters) */

	        } /* end if (digits as argument or not) */

	    } else {

	        if (ai >= MAXARGINDEX)
	            break ;

	        BASET(argpresent,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (rs < 0)
	    goto badarg ;

	pip->daytime = time(NULL) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(1)) {
	    debugprintf("main: %s debuglevel=%u\n",
	        timestr_logz(pip->daytime,timebuf),
	        pip->debuglevel) ;
	}
#endif

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: finished parsing arguments\n") ;
#endif

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
	if (DEBUGLEVEL(2))
	    debugprintf("main: final pr=%s\n", pip->pr) ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: pr=%s\n",
	        pip->progname,pip->pr) ;

/* get help if requested */

	if (f_help)
	    goto help ;

/* some initialization */

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* get some host/user information */

	rs = userinfo(&u,userbuf,USERINFO_LEN,NULL) ;

	pip->nodename = u.nodename ;
	pip->domainname = u.domainname ;
	pip->username = u.username ;

/* do we have an activity log file? */

	rs = SR_NOENT ;
	if ((logfname == NULL) || (logfname[0] == '\0'))
	    strwcpy(logfname,LOGFNAME,(MAXPATHLEN - 1)) ;

	if ((logfname[0] == '/') || (u_access(logfname,W_OK) >= 0))
	    rs = logfile_open(&pip->lh,logfname,0,0666,u.logid) ;

	if ((rs < 0) && (logfname[0] != '/')) {
	    mkpath2(tmpfname, pip->pr,logfname) ;
	    rs = logfile_open(&pip->lh,tmpfname,0,0666,u.logid) ;
	} /* end if (we tried to open another log file) */

	if (rs >= 0) {

	    pip->f.log = TRUE ;

/* we opened it, maintenance this log file if we have to */

	    if (loglen < 0)
	        loglen = LOGSIZE ;

	    lastlogtime = pip->daytime ;
	    logfile_checksize(&pip->lh,loglen) ;

	    logfile_userinfo(&pip->lh,&u,
	        pip->daytime,pip->progname,pip->version) ;

	} /* end if (we have a log file or not) */

/* check some parametes */

	if (pip->runint < 0)
	    pip->runint = RUNINT ;

	if ((pip->maxidle < 0) && (argvalue >= 0))
	    pip->maxidle = argvalue ;

	if (pip->maxidle < 0)
	    pip->maxidle = MAXIDLE ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: maxidle=%u\n",pip->maxidle) ;
#endif

	if ((portspec == NULL) || (portspec[0] == '\0'))
	    portspec = SVCSPEC_DTCMHAVE ;

	if (pip->f.log) {
	    logfile_printf(&pip->lh,"port=%s\n",portspec) ;
	    if ((pip->maxidle >= 0) && (pip->maxidle != INT_MAX)) {
	        logfile_printf(&pip->lh,"maxidle=%u\n",pip->maxidle) ;
	    }
	}

/* verify proper invocation */

#if	F_SYSLOG
	openlog("comsat", LOG_PID, LOG_DAEMON) ;
#endif

	lastmsgtime = pip->daytime ;

/* should we go into daemon mode? */

	fd_msg = FD_STDIN ;
	if (pip->f.daemon) {
	    uint	port ;
	    char	*hostname, *portname ;

	    for (i = 0 ; i < 3 ; i += 1)
	        u_close(i) ;

	    u_setsid() ;

	    hostname = hostspec ;
	    portname = portspec ;
	    if ((tp = strchr(portspec,':')) != NULL) {
	        hostname = buf ;
	        ml = MIN(BUFLEN,(tp - portspec)) ;
	        strwcpy(buf,portspec,ml) ;
	        portname = (tp + 1) ;
	    }

	    if ((portname == NULL) || (portname[0] == '\0'))
	        portname = SVCSPEC_DTCMHAVE ;

	    if ((rs = getportnum(PROTONAME,portname)) >= 0) {
		port = rs ;
	    } else if (isNotPresent(rs)) {
	        port = IPPORT_DTCMHAVE ;
		rs = SR_OK ;
	    }

	    if (rs >= 0) {
		const int	af = AF_INET ;

/* can we bind our port ourselves? */

	    if ((u.euid == 0) && (port < IPPORT_RESERVED)) {

	        portname = buf ;
	        ctdeci(buf,BUFLEN,port) ;

	        rs = listenudp(af,hostname,portname,0) ;
	        fd_msg = rs ;

	    } else {
	        SOCKADDRESS	sa ;
	        in_addr_t	addr = htonl(INADDR_ANY) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: port=%u\n",port) ;
#endif

	        if ((rs = sockaddress_start(&sa,af,&addr,port,0)) >= 0) {
	            rs = openport(PF_INET,SOCK_DGRAM,0,&sa) ;
	            fd_msg = rs ;
	            sockaddress_finish(&sa) ;
	        } /* end if (sockaddress) */

	    } /* end if (how to open the network port) */

	    } /* end if (ok) */

	} /* end if (daemon mode) */
	    if (rs < 0) goto badlisten ;

/* OK, loop as appropriate */

	if_int = 0 ;

	n = nelem(sigints) + nelem(sigignores) ;
	size = n * sizeof(struct sigaction) ;
	memset(sao,0,size) ;

/* block some signals and catch the others */

	uc_sigsetempty(&newsigmask) ;
	for (i = 0 ; sigblocks[i] != 0 ; i += 1) {
	    uc_sigsetadd(&newsigmask,sigblocks[i]) ;
	}
	u_sigprocmask(SIG_BLOCK,&newsigmask,&oldsigmask) ;
	uc_sigsetempty(&newsigmask) ;

/* ignore these signals */

	j = 0 ;
	for (i = 0 ; sigignores[i] != 0 ; i += 1) {
	    memset(&san,0,sizeof(struct sigaction)) ;
	    san.sa_handler = SIG_IGN ;
	    san.sa_mask = newsigmask ;
	    san.sa_flags = 0 ;
	    u_sigaction(sigignores[i],&san,(sao + j)) ;
	    j += 1 ;
	} /* end for */

/* ignore these signals */

	for (i = 0 ; sigints[i] != 0 ; i += 1) {
	    memset(&san,0,sizeof(struct sigaction)) ;
	    san.sa_handler = sighand_int ;
	    san.sa_mask = newsigmask ;
	    san.sa_flags = 0 ;
	    u_sigaction(sigints[i],&san,(sao + j)) ;
	    j += 1 ;
	} /* end for */

/* continue with looping */

	for (;;) {
	    const int	msglen = MSGBUFLEN ;
	    char	msgbuf[MSGBUFLEN + 1] ;

	    if (pip->f.log)
	        logfile_flush(&pip->lh) ;

	    mflags = 0 ;
	    sap = (struct sockaddr *) &sa ;
	    salen = sizeof(SOCKADDRESS) ;
	    timeout = TO_READ ;
	    opts = FM_TIMED ;
	    rs = uc_recvfrome(fd_msg,msgbuf,msglen,mflags,
	        sap,&salen,timeout,opts) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main: uc_recvfrome() rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {
	        len = rs ;
	        rs = procmsg(pip,fd_msg,msgbuf,len,sap,salen) ;
	        pip->daytime = time(NULL) ;
	        lastmsgtime = pip->daytime ;
	    } else
	        pip->daytime = time(NULL) ;

	    if ((rs < 0) && (rs != SR_TIMEDOUT) && (rs != SR_INTR))
	        break ;

	    if (if_int) {
	        rs = SR_INTR ;
	        break ;
	    }

	    if ((pip->maxidle >= 0) &&
	        ((pip->daytime - lastmsgtime) >= pip->maxidle))
	        break ;

	    if ((pip->daytime - lastlogtime) > CHECKLOGINT) {
	        lastlogtime = pip->daytime ;
	        logfile_checksize(&pip->lh,loglen) ;
	    }

	} /* end for (looping) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: done rs=%d\n",rs) ;
#endif

/* what should we return? */

	switch (rs) {
	case SR_OK:
	case SR_TIMEDOUT:
	    ex = EX_OK ;
	    break ;
	case SR_INTR:
	    ex = EX_INTR ;
	    break ;
	case SR_NOMEM:
	    ex = EX_OSERR ;
	    break ;
	default:
	    rs = EX_DATAERR ;
	    break ;
	} /* end switch */

/* get out */

	j = 0 ;
	for (i = 0 ; sigints[i] != 0 ; i += 1) {
	    u_sigaction(sigints[i],(sao + j++),NULL) ;
	}
	for (i = 0 ; sigignores[i] != 0 ; i += 1) {
	    u_sigaction(sigignores[i],(sao + j++),NULL) ;
	}
	u_sigprocmask(SIG_SETMASK,&oldsigmask,NULL) ;

	u_close(fd_msg) ;

/* we are done */
done:
ret2:
	if (pip->f.log) {
	    if (rs == SR_TIMEDOUT) {
	        logfile_printf(&pip->lh,"%s exiting on work timeout",
	            timestr_logz(pip->daytime,timebuf)) ;
	    } else {
	        logfile_printf(&pip->lh,"%s exiting (%d)",
	            timestr_logz(pip->daytime,timebuf),rs) ;
	    }
	    logfile_close(&pip->lh) ;
	}

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: program exiting ex=%u\n",
	        pip->progname,ex) ;
	}

/* early return thing */
retearly:
ret1:
	bclose(pip->efp) ;

ret0:
	proginfo_finish(pip) ;

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* the information type thing */
usage:
	usage(pip) ;
	goto retearly ;

/* print out some help */
help:
	printhelp(NULL,pip->pr,SEARCHNAME,HELPFNAME) ;
	goto retearly ;

/* the bad things */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

badlisten:
	ex = EX_NOINPUT ;
	bprintf(pip->efp,"%s: cannot listen on specified port (%d)\n",
	    pip->progname,rs) ;
	goto ret2 ;

}
/* end subroutine (main) */


/* local subroutines */


static void sighand_int(int sn)
{

	if_int = TRUE ;
}
/* end subroutine (sighand_int) */


static int usage(pip)
PROGINFO	*pip ;
{
	int	rs = SR_OK ;
	int	wlen = 0 ;

	rs = bprintf(pip->efp,
	    "%s: USAGE> % [-d] [-i maxidletime] [-h hostname] [-p port]\n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = bprintf(pip->efp,"%s: \t[-?V] [-Dv]\n",
	    pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procmsg(pip,fd,msgbuf,msglen,sap,salen)
PROGINFO	*pip ;
int		fd ;
char		msgbuf[] ;
int		msglen ;
struct sockaddr	*sap ;
int		salen ;
{
	struct havemsg_request	m0 ;
	struct havemsg_report	m1 ;
	struct ustat		sb ;
	int	rs, rs1, mbl, cl ;
	int	msgtype ;
	int	mflags = 0 ;

	char	calfname[MAXPATHLEN + 1] ;
	char	calname[MAXNAMELEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	char	*tp, *cp ;
	char	*mp, *up ;


#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procmsg: msglen=%u\n",msglen) ;
#endif

	msgtype = msgbuf[0] & 255 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procmsg: msgtype=%u\n",msgtype) ;
#endif

	switch (msgtype) {

	case havemsgtype_request:
	    {

	        memset(&m1,0,sizeof(struct havemsg_report)) ;

	        m1.type = havemsgtype_report ;
	        m1.tag = m0.tag ;
	        m1.seq = m0.seq ;
	        m1.timestamp = pip->daytime ;

	        strcpy(m1.calendar,m0.calendar) ;

	        rs1 = havemsg_request(msgbuf,msglen,1,&m0) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procmsg: havemsg_request() rs=%d\n",rs1) ;
#endif

	        if (rs1 > 0) {

	            up = m0.calendar ;
	            mp = NULL ;
	            if ((tp = strchr(m0.calendar,'@')) != NULL) {

	                *tp = '\0' ;
	                mp = (tp + 1) ;
	            }

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procmsg: user=%s\n",up) ;
#endif

	            sncpy3(calname,MAXNAMELEN,CALLOG,".",up) ;

	            mkpath2(calfname, CALDNAME,calname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procmsg: calfname=%s\n",calfname) ;
#endif

	            rs1 = u_stat(calfname,&sb) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procmsg: u_stat() rs=%d\n",rs1) ;
#endif

	            m1.rc = (rs1 >= 0) ? havemsgrc_ok : havemsgrc_notavail ;

	        } else if (rs1 == 0)
	            rs1 = SR_INVALID ;

	        if (rs1 < 0)
	            m1.rc = havemsgrc_invalid ;

	        if ((rs = havemsg_report(msgbuf,msglen,0,&m1)) >= 0) {
	            mbl = rs ;
	            rs = u_sendto(fd,msgbuf,mbl,mflags,sap,salen) ;
	        }

	    }

	    break ;

	default:
	    {
	        memset(&m1,0,sizeof(struct havemsg_report)) ;
	        m1.type = havemsgtype_report ;
	        m1.timestamp = pip->daytime ;
	        m1.rc = havemsgrc_invalid ;
	        if ((rs = havemsg_report(msgbuf,msglen,0,&m1)) >= 0) {
	            mbl = rs ;
	            rs = u_sendto(fd,msgbuf,mbl,mflags,sap,salen) ;
	        }
	    }
	    break ;

	} /* end switch */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procmsg: ret rs=%d mbl=%u\n",rs,mbl) ;
#endif

	return (rs >= 0) ? mbl : rs ;
}
/* end subroutine (procmsg) */



