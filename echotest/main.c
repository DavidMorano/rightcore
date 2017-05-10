/* main */

/* program to send echo data to a remote host (and back) */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine forms a program that sends data to a remote
	INET host to its 'echo' service.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<sys/time.h>
#include	<netinet/in.h>
#include	<time.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<bfile.h>
#include	<sockaddress.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#define	NPARG		2	/* number of positional arguments */

#define	TO_READ		30
#undef	BUFLEN
#define	BUFLEN		((10 * 1024) + MAXHOSTNAMELEN)

#define	DENOM		(1000 * 1000)

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif


/* external subroutines */

extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	dialuss(const char *,int,int) ;
extern int	dialusd(const char *,int,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	opentmpusd(const char *,int,mode_t,char *) ;
extern int	dialudp(const char *,const char *,int,int,int) ;
extern int	dialtcp(const char *,const char *,int,int,int) ;
extern int	dialtcpnls(const char *,const char *,int,const char *,int,int) ;
extern int	dialtcpmux(const char *,const char *,int,const char *,
			const char **,int,int) ;
extern int	dialticotsordnls(const char *,int,const char *,int,int) ;
extern int	getaf(const char *,int) ;
extern int	isdigitlatin(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	usage(PROGINFO *) ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"HELP",
	"proto",
	"sn",
	"af",
	"ef",
	"of",
	"lf",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_help,
	argopt_proto,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_lf,
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

static const char	*dialers[] = {
	"tcp",
	"tcpmux",
	"tcpnls",
	"udp",
	"uss",
	"usd",
	"ticotsordnls",
	NULL
} ;

enum dialers {
	dialer_tcp,
	dialer_tcpmux,
	dialer_tcpnls,
	dialer_udp,
	dialer_uss,
	dialer_usd,
	dialer_ticotsordnls,
	dialer_overlast
} ;


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	PROGINFO	pi, *pip = &pi ;
	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;
	int		argr, argl, aol, akl, avl, kwi, npa ;
	int		ai, ai_max, ai_pos ;
	int		rs, rs1 ;
	int		pan = 0 ;
	int		v ;
	int		i, len ;
	int		operms ;
	int		ex = EX_INFO ;
	int		dialer, s, fd ;
	int		af = AF_UNSPEC ;
	int		timeout = -1 ;
	int		fd_debug = -1 ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_quiet = FALSE ;
	int		f_help = FALSE ;
	int		f_log = FALSE ;
	int		f_shutdown = FALSE ;
	int		f ;
	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	char		argpresent[MAXARGGROUPS + 1] ;
	char		buf[BUFLEN + 1] ;
	char		srcpath[MAXPATHLEN + 1] ;
	char		hostnamebuf[MAXHOSTNAMELEN + 1] ;
	char		dialspecbuf[MAXHOSTNAMELEN + 1] ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*lfname = NULL ;
	const char	*dialspec = NULL ;
	const char	*addrspec = NULL ;
	const char	*hostname = NULL ;
	const char	*portspec = NULL ;
	const char	*svcspec = NULL ;
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
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
	proginfo_setbanner(pip,cp) ;

/* early initialization */

	pip->verboselevel = 1 ;

/* the arguments */

	for (ai = 0 ; ai < MAXARGGROUPS ; ai += 1) 
		argpresent[ai] = 0 ;

	npa = 0 ;			/* number of positional so far */
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
	                    akl = avp - aop ;
	                    avp += 1 ;
	                    avl = aop + argl - 1 - avp ;
	                    aol = akl ;
	                    f_optequal = TRUE ;
	                } else {
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
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pr = argp ;
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

/* argument-list file */
	                case argopt_af:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            afname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            afname = argp ;
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

/* output file */
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

	                        case argopt_lf:
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                lfname = argp ;
	                                f_log = TRUE ;
	                            }
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

	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

	                        case 'd':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                dialspec = argp ;
	                            break ;

	                    case 'f':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            addrspec = argp ;
	                        break ;

	                        case 'l':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                lfname = argp ;
	                                f_log = TRUE ;
	                            }
	                            break ;

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

	                        case 'q':
	                            f_quiet = TRUE ;
	                            break ;

	                        case 's':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                svcspec = argp ;
	                            break ;

	                        case 't':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = cfdecti(argp,argl,&v) ;
					timeout = v ;
	                            }
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

	        if (ai >= MAXARGINDEX)
	            break ;

	        BASET(argpresent,ai) ;
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
	debugprintf("main: f_usage=%d f_vesion=%d\n",f_usage,f_version) ;
	debugprintf("main: f_help=%u\n",f_help) ;
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;
	    bflush(pip->efp) ;
	}

	if (f_version) {
		bprintf(pip->efp,"%s: version %s\n",
			pip->progname,VERSION) ;
	}

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
	} /* end if */

	if (f_usage)
	    usage(pip) ;

	if (f_help) {
	    rs = printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;
#if	CF_DEBUGS
	debugprintf("main: printhelp() rs=%d\n",rs) ;
#endif
	}

	if (f_usage || f_help || f_version)
	    goto retearly ;


	ex = EX_OK ;

/* check arguments */

#if	CF_DEBUGS
	debugprintf("main: checking arguments\n") ;
#endif

	if (addrspec != NULL) {
	    af = getaf(addrspec,-1) ;
	    if (af < 0)
		af = AF_UNSPEC ;
	} /* end if (AF hint) */

	if (f_log && ((lfname == NULL) || (lfname[0] == '\0'))) {
	    f_log = FALSE ;
	}

/* process positional arguments */

	pan = 0 ;			/* number of positional so far */

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (f) {
	        switch (pan) {
	        case 0:
	            hostname = argv[ai] ;
	            break ;
	        case 1:
	            portspec = argv[ai] ;
	            break ;
	        } /* end switch */
	        pan += 1 ;
	    }

	    } /* end for (looping through requested circuits) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: checking hostname=%s portspec=%s\n",
	        hostname,portspec) ;
#endif

/* other initialization */

	srcpath[0] = '\0' ;

/* parse some stuff */

	if (hostname != NULL) {
	    if ((cp = strchr(hostname,':')) != NULL) {
	        i = (cp - hostname) ;
	        strwcpy(hostnamebuf,hostname,MAXHOSTNAMELEN) ;
	        hostnamebuf[i] = '\0' ;
	        hostname = hostnamebuf ;
	        if ((portspec == NULL) || (portspec[0] == '\0'))
	            portspec = hostnamebuf + i + 1 ;
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
	        if ((portspec == NULL) || (portspec[0] == '\0'))
	            portspec = dialspecbuf + i + 1 ;
	    } /* end if */
#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	        debugprintf("main: dialspec=%s portspec=%s\n",
		dialspec,portspec) ;
#endif
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: checking portspec\n") ;
#endif

/* find the dialer */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: find the dialer, dialspec=%s\n",dialspec) ;
#endif

	if (dialspec != NULL) {
	    dialer = matostr(dialers,2,dialspec,-1) ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: dialer=%d\n",dialer) ;
#endif
	    if (dialer < 0)
	        goto baddial ;
	} else
	    dialer = dialer_udp ;

	if (timeout <= 0)
	    timeout = DIALTIME ;

/* miscellaneous */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: 1 dialer=%d\n",dialer) ;
#endif

	if (pip->debuglevel > 0) {
	    for (i = 0 ; dialers[i] != NULL ; i += 1) {
	        bprintf(pip->efp,"%s: dialer> %s(%d)\n",
	            pip->progname,dialers[i],i) ;
	    }
	    bprintf(pip->efp,"%s: got valid arguments, dialer(%d)=%s\n",
	        pip->progname,dialer,dialers[dialer]) ;
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main: portspec=%s\n",portspec) ;
	    debugprintf("main: svcspec=%s\n",svcspec) ;
	}
#endif /* CF_DEBUG */

/* look up some miscellaneous stuff in various databases */

	switch (dialer) {

	case dialer_tcp:
	case dialer_udp:
	    if ((hostname == NULL) || (hostname[0] == '\0')) {
		rs = SR_INVALID ;
	        goto badhost ;
	    }
	    if ((portspec == NULL) || (portspec[0] == '\0')) {
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
	        goto badhost ;
	    }
		break ;

	} /* end switch */

	switch (dialer) {

	case dialer_tcpmux:
	case dialer_tcpnls:
	case dialer_ticotsordnls:
	    if ((svcspec == NULL) || (svcspec[0] == '\0'))
	        svcspec = SVCSPEC_ECHO ;
	    break ;

	} /* end switch */

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

	case dialer_tcpmux:
		f_shutdown = TRUE ;
	    rs = dialtcpmux(hostname,portspec,af,svcspec,NULL,timeout,0) ;
	    s = rs ;
	    break ;

	case dialer_tcpnls:
		f_shutdown = TRUE ;
	    rs = dialtcpnls(hostname,portspec,af,svcspec,timeout,0) ;
	    s = rs ;
	    break ;

	case dialer_udp:
	    rs = dialudp(hostname,portspec,af,timeout,0) ;
	    s = rs ;
	    break ;

	case dialer_ticotsordnls:
	    if ((portspec == NULL) || (portspec[0] == '\0')) {
		rs = SR_INVALID ;
	        portspec = hostname ;
	    }
	    rs = dialticotsordnls(portspec,-1,svcspec,timeout,0) ;
	    s = rs ;
	    break ;

	case dialer_uss:
		f_shutdown = TRUE ;
	    if ((portspec == NULL) || (portspec[0] == '\0'))
	        portspec = hostname ;
	    rs = dialuss(portspec,timeout,0) ;
	    s = rs ;
	    break ;

	case dialer_usd:
	    if ((portspec == NULL) || (portspec[0] == '\0'))
	        portspec = hostname ;
#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	        debugprintf("main: DIALUSD portspec=%s\n",portspec) ;
#endif
	    cp = "/tmp/echoXXXXXXXXXX" ;
	    operms = (S_IFSOCK | 0600) ;
	    rs = opentmpfile(cp,0,operms,srcpath) ;
	    s = rs ;
	    if (rs >= 0) {

#ifdef	COMMENT
	        rs = dialusd(portspec,timeout,1) ;
#else
	        {
	            SOCKADDRESS	dst ;
	            int		alen ;

	            rs = sockaddress_start(&dst,AF_UNIX,portspec,0,0) ;
		    if (rs >= 0) {

	            alen = sockaddress_getlen(&dst) ;

	            rs = u_connect(s,(struct sockaddr *) &dst,alen) ;

	            sockaddress_finish(&dst) ;
		    }

	            if (rs < 0)
	                u_close(s) ;

	        } /* end block */
#endif /* COMMENT */

	    } /* end if (got a socket) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: dialusd() rs=%d\n",rs) ;
#endif

	    break ;

	} /* end switch (dialer selection) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: 3 dialer rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto badconnect ;

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: connected to the host FD=%d\n",
	        pip->progname,s) ;
	    bflush(pip->efp) ;
	}

	rs = bopen(ofp,BFILE_STDOUT,"wct",0666) ;

/* do it */

	if (rs >= 0) {
		hrtime_t	sum, ave ;
		int	ntime = 0 ;

		sum = 0 ;

	while (rs >= 0) {
		hrtime_t	start, end, diff ;

#if	CF_DEBUGS
	    bflush(pip->efp) ;
#endif

	    rs = u_read(FD_STDIN,buf,BUFLEN) ;
	    len = rs ;
	    if (rs < 0)
	        break ;

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: STDIN read rs=%d\n",
	            pip->progname,rs) ;

	    if (len == 0)
	        break ;

		start = gethrtime() ;

	    rs = u_write(s,buf,len) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: u_write() rs=%d\n",rs) ;
#endif

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: network write rs=%d\n",
	            pip->progname,rs) ;

	    if (rs < 0) {

	        bprintf(pip->efp,"%s: bad network write rs=%d\n",
	            pip->progname,rs) ;

	        goto done ;
	    }

	    rs = uc_reade(s,buf,BUFLEN,TO_READ,0) ;
	    len = rs ;

		end = gethrtime() ;

		diff = end - start ;
		sum += diff ;

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: network read len=%d\n",
	            pip->progname,len) ;

	    if (rs < 0) {

	        bprintf(pip->efp,"%s: bad network read rs=%d\n",
	            pip->progname,rs) ;

	        goto done ;
	    }

	    if (len > 0)
	        rs = bwrite(ofp,buf,len) ;

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: STDOUT write rs=%d\n",
	            pip->progname,rs) ;

		if (pip->verboselevel > 1)
	        bprintf(ofp,"delay=%lld msec\n",(diff / DENOM)) ;

		ntime += 1 ;
		bflush(ofp) ;

	} /* end while */

		if ((pip->verboselevel > 1) && (ntime > 0))
	        bprintf(ofp,"average delay=%lld\n",((sum / DENOM) / ntime)) ;

	bclose(ofp) ;
	} /* end block */

	if ((rs < 0) && (ex == EX_OK)) {
	    ex = EX_DATAERR ;
	    bprintf(pip->efp,"%s: exiting bad (%d)\n",
	        pip->progname,rs) ;
	}

/* and exit */
done:
	if (f_shutdown)
		u_shutdown(s,SHUT_RDWR) ;

ret5:
	if (s >= 0)
	    u_close(s) ;

ret4:
	if (srcpath[0] != '\0')
	    u_unlink(srcpath) ;

retearly:
ret3:
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

ret2:
	if (pip->efp != NULL) {
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

ret1:
	proginfo_finish(pip) ;

badprogstart:

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

badhost:
	ex = EX_NOHOST ;
	bprintf(pip->efp,"%s: no host was specified\n",
	    pip->progname) ;

	goto retearly ;

baddial:
	ex = EX_UNAVAILABLE ;
	bprintf(pip->efp,"%s: unknown dialer specified\n",
	    pip->progname) ;

	goto retearly ;

/* handle other things */
badconnect:
	ex = EX_UNAVAILABLE ;
	bprintf(pip->efp,"%s: could not connect to host (%d)\n",
	    pip->progname,rs) ;

	goto ret2 ;

}
/* end subroutine (main) */


/* local subroutines */


static int usage(pip)
PROGINFO	*pip ;
{
	int		rs ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [-d <dialer>] <host> <svc>\n" ;
	rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-lf <logfile>]\n" ;
	rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


