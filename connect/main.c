/* main */

/* program to send echo data to a remote host (and back) */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	1		/* run-time debug print-outs */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */


/* revision history:

	= 1999-03-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine forms a program that sends data to a remote INET host
	to its 'echo' service.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/conf.h>
#include	<sys/socket.h>
#include	<sys/time.h>
#include	<netinet/in.h>
#include	<unistd.h>
#include	<stropts.h>
#include	<time.h>
#include	<signal.h>
#include	<netdb.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<bfile.h>
#include	<logfile.h>
#include	<sockaddress.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	TO_READ		30
#define	TO_PING		20

#undef	BUFLEN
#define	BUFLEN		((10 * 1024) + MAXHOSTNAMELEN)

#define	DENOM		(1000 * 1000)
#define	NFDS		4

#ifndef	SHUT_RD
#define	SHUT_RD		0
#define	SHUT_WR		1
#define	SHUT_RDWR	2
#endif

#ifndef	POLLINTMULT
#define	POLLINTMULT	1000
#endif


/* external subroutines */

extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	getsrcname(char *,int,int) ;
extern int	getaf(const char *,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	opentmpusd(const char *,int,mode_t,char *) ;
extern int	isasocket(int) ;
extern int	isdigitlatin(int) ;

extern int	dialudp(const char *,const char *,int,int,int) ;
extern int	dialtcp(const char *,const char *,int,int,int) ;
extern int	dialtcpnls(const char *,const char *,int,const char *,int,int) ;
extern int	dialtcpmux(cchar *,cchar *,int,cchar *,cchar **,int,int) ;
extern int	dialuss(const char *,int,int) ;
extern int	dialussnls(const char *,const char *,int,int) ;
extern int	dialussmux(const char *,const char *,const char **,int,int) ;
extern int	dialusd(const char *,int,int) ;
extern int	dialticotsord(const char *,int,int,int) ;
extern int	dialticotsordnls(const char *,int,const char *,int,int) ;
extern int	dialpass(const char *,int,int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;
extern int	transfer(PROGINFO *,cchar *,int,int,int,int,int,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif /* CF_DEBUG */

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

struct fpstat {
	uint		in:1 ;
	uint		out:1 ;
	uint		final:1 ;
	uint		hup:1 ;
	uint		eof:1 ;
} ;


/* forward references */

static int usage(PROGINFO *) ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"HELP",
	"ni",
	"no",
	"sanity",
	"sn",
	"af",
	"ef",
	"sa",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_help,
	argopt_ni,
	argopt_no,
	argopt_sanity,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_sa,
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

static const char	*dialers[] = {
	"tcp",
	"tcpmux",
	"tcpnls",
	"udp",
	"uss",
	"ussmux",
	"ussnls",
	"usd",
	"ticotsordnls",
	"fifo",
	"pass",
	"open",
	NULL
} ;

enum dialers {
	dialer_tcp,
	dialer_tcpmux,
	dialer_tcpnls,
	dialer_udp,
	dialer_uss,
	dialer_ussmux,
	dialer_ussnls,
	dialer_usd,
	dialer_ticotsordnls,
	dialer_fifo,
	dialer_pass,
	dialer_open,
	dialer_overlast
} ;


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	BITS		pargs ;
	bfile		errfile ;
	mode_t		operm ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		pan = 0 ;
	int		rs, rs1 ;
	int		v ;
	int		i ;
	int		dialer, af, s, s2 ;
	int		timeout = -1 ;
	int		mxu ;
	int		fd_in, fd_out ;
	int		oflags ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;
	int		f_log = FALSE ;
	int		f_shutdown = FALSE ;
	int		f ;
	const char	*template ;
	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*efname = NULL ;
	const char	*logfname = NULL ;
	const char	*dialspec = NULL ;
	const char	*afspec = NULL ;
	const char	*hostname = NULL ;
	const char	*portspec = NULL ;
	const char	*svcspec = NULL ;
	const char	*fmt ;
	const char	*cp ;
	char		srcpath[MAXPATHLEN + 1] ;
	char		hostnamebuf[MAXHOSTNAMELEN + 1] ;
	char		dialspecbuf[MAXHOSTNAMELEN + 1] ;


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

/* early initialization */

	pip->keeptime = DEFKEEPTIME ;
	pip->verboselevel = 1 ;

	pip->f.ni = FALSE ;
	pip->f.no = FALSE ;

	fd_in = FD_STDIN ;
	fd_out = FD_STDOUT ;

/* the arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	ai_max = 0 ;
	ai_pos = 1 ;
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

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* program root */
	                case argopt_root:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->pr = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->pr = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_ni:
	                    pip->f.ni = TRUE ;
	                    break ;

	                case argopt_no:
	                    pip->f.no = TRUE ;
	                    break ;

	                case argopt_sanity:
	                    pip->f.sanity = TRUE ;
	                    break ;

	                default:
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

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'd':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                dialspec = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'f':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                afspec = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'l':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                logfname = argp ;
	                                f_log = TRUE ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'n':
	                        pip->f.ni = TRUE ;
	                        break ;

	                    case 'p':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                portspec = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 's':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                svcspec = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 't':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = cfdeci(argp,argl,&v) ;
	                                timeout = v ;
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
	                        rs = SR_INVALID ;
	                        break ;

	                    } /* end switch */
	                    akp += 1 ;

	                    if (rs < 0) break ;
	                } /* end while */

	            } /* end if (individual option key letters) */

	        } /* end if (digit or not) */

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
	}

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUGS
	debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;
	}

/* program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto done ;
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

	if (f_help || f_version || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* check arguments */

	if (f_log && ((logfname == NULL) || (logfname[0] == '\0'))) {
	    f_log = FALSE ;
	}

/* process */

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (f) {
	        cp = argv[ai] ;
	        switch (pan) {
	        case 0:
	            hostname = cp ;
	            break ;
	        case 1:
	            svcspec = cp ;
	            break ;
	        } /* end switch */
	    }

	    pan += 1 ;
	} /* end for (looping through requested circuits) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: checking hostname=%s svctspec=%s\n",
	        hostname,svcspec) ;
#endif

/* other initialization */

	srcpath[0] = '\0' ;

/* parse some stuff */

	if (hostname != NULL) {

	    if ((cp = strchr(hostname,':')) != NULL) {

	        i = cp - hostname ;
	        strwcpy(hostnamebuf,hostname,MAXHOSTNAMELEN) ;

	        hostnamebuf[i] = '\0' ;
	        hostname = hostnamebuf ;
	        if ((portspec == NULL) || (portspec[0] == '\0')) {
	            portspec = hostnamebuf + i + 1 ;
		}

	    } /* end if */

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: checking dialspec=%s\n",dialspec) ;
#endif

	if ((dialspec != NULL) && (dialspec[0] != '\0')) {

	    if ((cp = strchr(dialspec,':')) != NULL) {

	        i = (cp - dialspec) ;
	        strwcpy(dialspecbuf,dialspec,MAXHOSTNAMELEN) ;

	        dialspecbuf[i] = '\0' ;
	        dialspec = dialspecbuf ;
	        if ((portspec == NULL) || (portspec[0] == '\0')) {
	            portspec = dialspecbuf + i + 1 ;
		}

	    } /* end if */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: dialspec=%s portspec=%s\n",
	            dialspec,portspec) ;
#endif

	} /* end if */

/* find the dialer */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: find the dialer, dialspec=%s\n",dialspec) ;
#endif

	if (dialspec != NULL) {

	    dialer = matostr(dialers,1,dialspec,-1) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: dialer=%d\n",dialer) ;
#endif

	} else
	    dialer = dialer_tcp ;

	if (dialer < 0) {
	    rs = SR_INVALID ;
	    ex = EX_USAGE ;
	    bprintf(pip->efp,"%s: unknown dialer specified\n",
	        pip->progname) ;
	    goto baddial ;
	}

/* address family */

	af = AF_UNSPEC ;
	if ((afspec != NULL) && (afspec[0] != '\0')) {
	    rs1 = getaf(afspec,-1) ;
	    if (rs1 >= 0)
	        af = rs1 ;
	} /* end if */

	if (timeout <= 0)
	    timeout = DIALTIME ;

/* miscellaneous */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: dialer=%d\n",dialer) ;
#endif

	if (pip->debuglevel > 0) {
	    const char	*pn = pip->progname ;
	    bprintf(pip->efp,"%s: af=%u\n",pn,af) ;
	    bprintf(pip->efp,"%s: host=%s\n",pn,hostname) ;
	    bprintf(pip->efp,"%s: portspec=%s\n",pn,portspec) ;
	    bprintf(pip->efp,"%s: svcspec=%s\n",pn,svcspec) ;
	    bprintf(pip->efp,"%s: dialer=%s(%d)\n",pn,dialers[dialer],dialer) ;
	    bprintf(pip->efp,"%s: timeout=%d\n",pn,timeout) ;
	} /* end if (debugging information) */

/* look up some miscellaneous stuff in various databases */

	switch (dialer) {

	case dialer_tcp:
	case dialer_udp:
	    if ((hostname == NULL) || (hostname[0] == '\0')) {
	        rs = SR_INVALID ;
	    }
	    if ((rs >= 0) && ((portspec == NULL) || (portspec[0] == '\0'))) {
	        if ((svcspec != NULL) && (svcspec[0] != '\0'))
	            portspec = svcspec ;
	        if (portspec == NULL)
	            portspec = PORTSPEC_ECHO ;
	    }
	    break ;

	case dialer_tcpmux:
	case dialer_tcpnls:
	    if ((hostname == NULL) || (hostname[0] == '\0')) {
	        rs = SR_INVALID ;
	    }
	    break ;

	} /* end switch */

	if (rs >= 0) {
	    switch (dialer) {
	    case dialer_tcpmux:
	    case dialer_tcpnls:
	    case dialer_ticotsordnls:
	        if ((svcspec == NULL) || (svcspec[0] == '\0')) {
	            svcspec = SVCSPEC_ECHO ;
	        }
	        break ;
	    } /* end switch */
	} /* end if */

	if (rs < 0) {
	    ex = EX_USAGE ;
	    bprintf(pip->efp,"%s: no host was specified\n",
	        pip->progname) ;
	    goto badhost ;
	}

/* set the maximum transmit/receive unit size */

	mxu = BUFLEN ;
	s2 = -1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: 2 dialer=%s(%d)\n",dialers[dialer],dialer) ;
#endif

	switch (dialer) {

	case dialer_tcp:
	    f_shutdown = TRUE ;
	    rs = dialtcp(hostname,portspec,af,timeout,0) ;
	    s = rs ;
	    break ;

	case dialer_tcpnls:
	    f_shutdown = TRUE ;
	    rs = dialtcpnls(hostname,portspec,af,svcspec,timeout,0) ;
	    s = rs ;
	    break ;

	case dialer_tcpmux:
	    f_shutdown = TRUE ;
	    rs = dialtcpmux(hostname,portspec,af,svcspec,NULL,timeout,0) ;
	    s = rs ;
	    break ;

	case dialer_udp:
	    rs = dialudp(hostname,portspec,af,timeout,0) ;
	    s = rs ;
	    mxu = 1024 ;
	    break ;

	case dialer_ticotsordnls:
	    if ((portspec == NULL) || (portspec[0] == '\0')) {
	        portspec = hostname ;
	    }
#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	        debugprintf("main: ps=%s\n",portspec) ;
	        debugprintf("main: ss=%s\n",svcspec) ;
	    }
#endif
	    rs = dialticotsordnls(portspec,-1,svcspec,timeout,0) ;
	    s = rs ;
	    break ;

	case dialer_uss:
	    f_shutdown = TRUE ;
	    if ((portspec == NULL) || (portspec[0] == '\0')) {
	        portspec = hostname ;
	    }
	    rs = dialuss(portspec,timeout,0) ;
	    s = rs ;
	    break ;

	case dialer_ussnls:
	    f_shutdown = TRUE ;
	    if ((portspec == NULL) || (portspec[0] == '\0')) {
	        portspec = hostname ;
	    }
	    rs = dialussnls(portspec,svcspec,timeout,0) ;
	    s = rs ;
	    break ;

	case dialer_ussmux:
	    f_shutdown = TRUE ;
	    if ((portspec == NULL) || (portspec[0] == '\0')) {
	        portspec = hostname ;
	    }
	    rs = dialussmux(portspec,svcspec,NULL,timeout,0) ;
	    s = rs ;
	    break ;

	case dialer_usd:
	    if ((portspec == NULL) || (portspec[0] == '\0')) {
	        portspec = hostname ;
	    }
#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: DIALUSD portspec=%s\n",portspec) ;
#endif
	    template = "/tmp/echoXXXXXXXXXX" ;
	    operm = (S_IFSOCK | 0600) ;
	    if ((rs = opentmpfile(template,0,operm,srcpath)) >= 0) {
	        SOCKADDRESS	dst ;
	        const int	af = AF_UNIX ;
	        int		sal ;
	        s = rs ;
	        if ((rs = sockaddress_start(&dst,af,portspec,0,0)) >= 0) {
	            struct sockaddr	*sap = (struct sockaddr *) &dst ;
	            sal = sockaddress_getlen(&dst) ;
	            rs = u_connect(s,sap,sal) ;
	            sockaddress_finish(&dst) ;
	        } /* end if */
	    } /* end if (got a socket) */
#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: dialusd() rs=%d\n",rs) ;
#endif
	    mxu = 1024 ;
	    break ;

	case dialer_fifo:
	    rs = SR_NOENT ;
	    if ((hostname != NULL) && (hostname[0] != '\0')) {
	        rs = uc_open(hostname,O_RDWR,0666) ;
	        s = rs ;
	    }
	    if (rs >= 0) {
	        int	flags = 0 ;
	        flags |= RMSGD ;
	        u_ioctl(s,I_SRDOPT,flags) ;
	        flags = 0 ;
	        flags |= SNDZERO ;
	        u_ioctl(s,I_SWROPT,flags) ;
	    } /* end if */
	    break ;

	case dialer_pass:
	    rs = dialpass(hostname,timeout,0) ;
	    s = rs ;
	    break ;

	case dialer_open:
	    oflags = O_RDWR ;
	    operm = 0666 ;
	    rs = uc_opene(hostname,oflags,operm,timeout) ;
	    s = rs ;
	    break ;

	default:
	    rs = SR_NOTFOUND ;
	    break ;

	} /* end switch (dialer selection) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: 3 dialer rs=%d\n",rs) ;
#endif

	if (rs < 0) {
	    ex = EX_UNAVAILABLE ;
	    if (! pip->f.quiet) {
	        fmt = "%s: could not connect to host (%d)\n" ;
	        bprintf(pip->efp,fmt,pip->progname,rs) ;
	    }
	    goto badconnect ;
	}

	if (pip->debuglevel > 0) {
	    const int	slen = MAXPATHLEN ;
	    cchar	*pn = pip->progname ;
	    char	sbuf[MAXPATHLEN+1] ;
	    if ((rs = getsrcname(sbuf,slen,s)) >= 0) {
	        bprintf(pip->efp,"%s: srcname=%s\n",pn,sbuf) ;
	        bprintf(pip->efp,"%s: connected\n",pn) ;
	        bflush(pip->efp) ;
	    }
	} /* end if */

/* do the transferring */

	rs = transfer(pip,hostname,s,s2,fd_in,fd_out,-1,mxu) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: transfer() rs=%d\n",rs) ;
#endif

#ifdef	COMMENT
	if (f_shutdown)
	    u_shutdown(s,SHUT_RDWR) ;
#endif

	u_close(s) ;

badconnect:
baddial:
badhost:
done:
	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

retearly:
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

	if (srcpath[0] != '\0') {
	    u_unlink(srcpath) ;
	    srcpath[0] = '\0' ;
	}

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	bits_finish(&pargs) ;

badpargs:
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

/* handle bad arguments */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
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

	fmt = "%s: USAGE> %s [-d <dialer>] <addr> [<svc>] [-s <subsvc>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-t <timeout>] [-f <af>] [-sa <srcaddr>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


