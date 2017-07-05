/* main */

/* program to send echo data to a remote host (and back) */


#define	CF_DEBUGS		1	/* compile-time */
#define	CF_DEBUG		1	/* run-time */
#define	CF_DEBUGCONDTRANS	1


/* revision history:

	= 1992-03-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine forms a program that sends data to a remote INET host to
        its 'echo' service.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<unistd.h>
#include	<stropts.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>
#include	<time.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<bfile.h>
#include	<sockaddress.h>
#include	<getxusername.h>
#include	<userinfo.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"dialopts.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#define	NPARG		2	/* number of positional arguments */

#define	TO_READ		30
#define	TO_PING		20

#undef	BUFLEN
#define	BUFLEN		((10 * 1024) + MAXHOSTNAMELEN)

#undef	MSGBUFLEN
#define	MSGBUFLEN	2048

#define	DENOM		(1000 * 1000)
#define	NFDS		6

#ifndef	SHUT_RD
#define	SHUT_RD		0
#define	SHUT_WR		1
#define	SHUT_RDWR	2
#endif

#ifndef	POLLINTMULT
#define	POLLINTMULT	1000
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	isasocket(int) ;
extern int	inetping(const char *,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	opentmpusd(const char *,int,mode_t,char *) ;
extern int	dialudp(char *,char *,int,int,int) ;
extern int	dialtcp(const char *,const char *,int,int,int) ;
extern int	dialtcpnls(char *,char *,int,char *,int,int) ;
extern int	dialtcpmux(char *,char *,int,char *,char **,int,int) ;
extern int	dialuss(const char *,int,int) ;
extern int	dialusd(const char *,int,int) ;
extern int	dialticotsordnls(const char *,int,const char *,int,int) ;
extern int	dialprog(const char *,int,char **,char **,int *) ;
extern int	dialcprog(const char *,const char *,const char *,
			char **,char **,int,int,int *) ;
extern int	rcmdr(const char *,const char *,const char *,int *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strbasename(char *) ;


/* external variables */


/* local structures */

struct fpstat {
	uint	in : 1 ;
	uint	out : 1 ;
	uint	final : 1 ;
	uint	eof : 1 ;
} ;


/* forward references */

static int	transfer(PROGINFO *,const char *,
			int,int,int,int,int,int) ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"ni",
	"no",
	"sanity",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_ni,
	argopt_no,
	argopt_sanity,
	argopt_overlast
} ;

static const char	*dialers[] = {
	"tcp",
	"tcpmux",
	"tcpnls",
	"udp",
	"uss",
	"usd",
	"ticotsordnls",
	"prog",
	"cprog",
	"rcmdr",
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
	dialer_prog,
	dialer_cprog,
	dialer_rcmdr,
	dialer_overlast
} ;


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	PROGINFO	pi, *pip = &pi ;

	bfile	errfile, *efp = &errfile ;

	mode_t	operms ;

	int	argr, argl, aol, akl, avl, npa, maxai, kwi ;
	int	ai = -1 ;
	int	argnum = -1 ;
	int	rs = SR_OK ;
	int	pan ;
	int	i, len ;
	int	dialer, s, s2, fd ;
	int	timeout = -1 ;
	int	mxu ;
	int	fd_debug = -1 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_exitargs = FALSE ;
	int	f_ignore = FALSE ;
	int	f_log = FALSE ;
	int	f_shutdown = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	char	argpresent[MAXARGGROUPS + 1] ;
	char	buf[BUFLEN + 1] ;
	char	srcpath[MAXPATHLEN + 1] ;
	char	hostnamebuf[MAXHOSTNAMELEN + 1] ;
	char	dialspecbuf[MAXHOSTNAMELEN + 1] ;
	char	userinfobuf[USERINFO_LEN + 1] ;
	const char	*logfname = NULL ;
	const char	*dialspec = NULL ;
	const char	*hostname = NULL ;
	const char	*portspec = NULL ;
	const char	*svcspec = NULL ;
	const char	*template ;
	const char	*cp ;


	if ((cp = getenv(VARDEBUGFD1)) == NULL)
	    cp = getenv(VARDEBUGFD2) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


#if	CF_DEBUGS
	debugprintf("main: started\n") ;
#endif

	memset(pip,0,sizeof(PROGINFO)) ;

	pip->progname = strbasename(argv[0]) ;

	if ((rs = bopen(efp,BFILE_STDERR,"dwca",0666)) >= 0) {
	    pip->f.errfile = TRUE ;
	    pip->efp = &errfile ;
	    bcontrol(efp,BC_LINEBUF,0) ;
	}

/* early initialization */

	pip->pr = NULL ;

	pip->keeptime = DEFKEEPTIME ;

	pip->debuglevel = 0 ;
	pip->verboselevel = 1 ;

	pip->f.ni = FALSE ;
	pip->f.no = FALSE ;

/* the arguments */

	for (i = 0 ; i < MAXARGGROUPS ; i += 1) argpresent[i] = 0 ;

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
	i = 0 ;
	argr = argc - 1 ;
	while ((! f_exitargs) && (argr > 0)) {

	    argp = argv[++i] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 0) && (f_optminus || f_optplus)) {

	        if (argl > 1) {

	            if (isdigit(argp[1])) {

	                if (cfdeci(argp + 1,argl - 1,&argnum))
	                    goto badargval ;

	            } else {

#if	CF_DEBUGS
	                debugprintf("main: got an option\n") ;
#endif

	                aop = argp + 1 ;
	                akp = aop ;
	                aol = argl - 1 ;
	                f_optequal = FALSE ;
	                if ((avp = strchr(aop,'=')) != NULL) {

#if	CF_DEBUGS
	                    debugprintf("main: got an option key w/ a value\n") ;
#endif

	                    akl = avp - aop ;
	                    avp += 1 ;
	                    avl = aop + argl - 1 - avp ;
	                    aol = akl ;
	                    f_optequal = TRUE ;

	                } else {

	                    avl = 0 ;
	                    akl = aol ;

	                }

/* do we have a keyword or only key letters ? */

#if	CF_DEBUGS
	                debugprintf("main: key word match ? >%W<\n",
	                    aop,aol) ;
#endif

	                if ((kwi = matstr3(argopts,aop,aol)) >= 0) {

#if	CF_DEBUGS
	                    debugprintf("main: option keyword=%t kwi=%d\n",
	                        aop,aol,kwi) ;
#endif

	                    switch (kwi) {

/* program root */
	                    case argopt_root:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                pip->pr = avp ;

	                        } else {

	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                pip->pr = argp ;

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

	                    } /* end switch (key words) */

	                } else {

#if	CF_DEBUGS
	                    debugprintf("main: got an option key letter\n") ;
#endif

	                    while (akl--) {

#if	CF_DEBUGS
	                        debugprintf("main: option key letter=%c\n",*aop) ;
#endif

	                        switch (*aop) {

	                        case 'D':
	                            pip->debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl) {

	                                    rs = cfdeci(avp,avl, 
	                                        &pip->debuglevel) ;

	                                    if (rs < 0)
	                                        goto badargval ;

	                                }
	                            }

	                            break ;

	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

	                        case 'd':
	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

#if	CF_DEBUGS
	                            debugprintf("main: A dialspec=%s\n",argp) ;
#endif

	                            if (argl)
	                                dialspec = argp ;

	                            break ;

	                        case 'l':
	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) {

	                                logfname = argp ;
	                                f_log = TRUE ;
	                            }

	                            break ;

	                        case 'n':
	                            pip->f.ni = TRUE ;
	                            break ;

	                        case 'p':
	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                portspec = argp ;

	                            break ;

	                        case 'q':
	                            pip->f.quiet = TRUE ;
	                            break ;

	                        case 's':
	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

#if	CF_DEBUGS
	                            debugprintf("main: A svcspec=%s\n",argp) ;
#endif

	                            if (argl)
	                                svcspec = argp ;

	                            break ;

	                        case 't':
	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) {

	                                rs = cfdeci(argp,argl,&timeout) ;

	                                if (rs < 0)
	                                    goto badargval ;

	                            }

	                            break ;

	                        case 'v':
	                            pip->verboselevel = 2 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl) {

	                                    rs = cfdeci(avp,avl, 
	                                        &pip->verboselevel) ;

	                                    if (rs < 0)
	                                        goto badargval ;

	                                }
	                            }

	                            break ;

	                        case '?':
	                            ex = EX_USAGE ;
	                            f_usage = TRUE ;
	                            break ;

	                        default:
	                            ex = EX_USAGE ;
	                            f_usage = TRUE ;
	                            bprintf(efp,"%s: unknown option - %c\n",
	                                pip->progname,*aop) ;

	                        } /* end switch */

	                        akp += 1 ;

	                    } /* end while */

	                } /* end if (individual option key letters) */

	            } /* end if (digit or not) */

	        } else {

	            if (i < MAXARGINDEX) {

#if	CF_DEBUGS
	                debugprintf("main: positional=%s\n",argv[i]) ;
#endif

	                BASET(argpresent,i) ;
	                maxai = i ;
	                npa += 1 ;	/* increment position count */
	                if (npa >= 2)
	                    f_exitargs = TRUE ;

	            }

	        } /* end if */

	    } else {

	        if (i < MAXARGGROUPS) {

#if	CF_DEBUGS
	            debugprintf("main: positional=%s\n",argv[i]) ;
#endif

	            BASET(argpresent,i) ;
	            maxai = i ;
	            npa += 1 ;
	            if (npa >= 2)
	                f_exitargs = TRUE ;

	        } else {

	            if (! f_extra) {

	                ex = EX_USAGE ;
	                f_extra = TRUE ;
	                bprintf(efp,"%s: extra arguments specified\n",
	                    pip->progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */


#if	CF_DEBUGS
	debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
	debugprintf("main: past getting arguments f_usage=%d f_vesion=%d\n",
	    f_usage,f_version) ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	if (f_version)
	    bprintf(efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage)
	    goto usage ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* check arguments */

#if	CF_DEBUGS
	debugprintf("main: checking arguments\n") ;
#endif

	pan = 0 ;			/* number of positional so far */
	if (npa > 0) {

	    for (i = 0 ; i <= maxai ; i += 1) {

	        if ((! BATST(argpresent,i)) && (argv[i][0] != '\0'))
	            continue ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: arg=\"%s\"\n",argv[i]) ;
#endif

	        switch (pan) {

	        case 0:
	            hostname = argv[i] ;
	            break ;

	        case 1:
	            portspec = argv[i] ;
	            ai = i + 1 ;
	            break ;

	        } /* end switch */

	        pan += 1 ;

	    } /* end for (looping through requested circuits) */

	} /* end if */

	if (f_log && ((logfname == NULL) || (logfname[0] == '\0'))) {

	    f_log = FALSE ;

	}

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: checking hostname=%s portspec=%s\n",
	        hostname,portspec) ;
#endif

/* other initialization */

	pip->username = userinfobuf ;
	getusername(userinfobuf,USERNAMELEN,-1) ;

	srcpath[0] = '\0' ;

/* parse some stuff */

	if (hostname != NULL) {

	    if ((cp = strchr(hostname,':')) != NULL) {

	        i = cp - hostname ;
	        strwcpy(hostnamebuf,hostname,(MAXHOSTNAMELEN - 1)) ;

	        hostnamebuf[i] = '\0' ;
	        hostname = hostnamebuf ;
	        if ((portspec == NULL) || (portspec[0] == '\0'))
	            portspec = hostnamebuf + i + 1 ;

	    } /* end if */

	} /* end if */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: checking dialspec=%s\n",dialspec) ;
#endif

	if ((dialspec != NULL) && (dialspec[0] != '\0')) {

	    if ((cp = strchr(dialspec,':')) != NULL) {

	        i = cp - dialspec ;
	        strwcpy(dialspecbuf,dialspec,MAXHOSTNAMELEN) ;

	        dialspecbuf[i] = '\0' ;
	        dialspec = dialspecbuf ;
	        if ((portspec == NULL) || (portspec[0] == '\0'))
	            portspec = dialspecbuf + i + 1 ;

	    } /* end if */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: dialspec=%s portspec=%s\n",
		dialspec,portspec) ;
#endif

	} /* end if */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: checking portspec\n") ;
#endif

/* find the dialer */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: find the dialer, dialspec=%s\n",dialspec) ;
#endif

	if (dialspec != NULL) {

	    dialer = matstr(dialers,dialspec,-1) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
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
	if (pip->debuglevel > 1)
	    debugprintf("main: 1 dialer=%d\n",dialer) ;
#endif

	if (pip->debuglevel > 0) {
	    for (i = 0 ; dialers[i] != NULL ; i += 1)
	        bprintf(efp,"%s: dialer> %s(%d)\n",
	            pip->progname,dialers[i],i) ;
	    bprintf(efp,"%s: got valid arguments, dialer(%d)=%s\n",
	        pip->progname,dialer,dialers[dialer]) ;
	}

/* look up some miscellaneous stuff in various databases */

	switch (dialer) {

	case dialer_tcp:
	case dialer_udp:
	    if ((hostname == NULL) || (hostname[0] == '\0'))
	        goto badhost ;

	    if ((portspec == NULL) || (portspec[0] == '\0')) {

	        if ((svcspec != NULL) && (svcspec[0] != '\0'))
	            portspec = svcspec ;

	        if (portspec == NULL)
	            portspec = PORTSPEC_ECHO ;

	    }

	    break ;

	case dialer_tcpmux:
	case dialer_tcpnls:
	case dialer_cprog:
	    if ((hostname == NULL) || (hostname[0] == '\0'))
	        goto badhost ;

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

/* set the maximum transmit/receive unit size */

	mxu = BUFLEN ;
	s2 = -1 ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 2 dialer=%s(%d) portspec=%s\n",
	        dialers[dialer],dialer,portspec) ;
#endif

	switch (dialer) {

	case dialer_tcp:
	    f_shutdown = TRUE ;
	    rs = dialtcp(hostname,portspec,AF_INET,timeout,0) ;
	    s = rs ;
	    break ;

	case dialer_tcpmux:
	    f_shutdown = TRUE ;
	    rs = dialtcpmux(hostname,portspec,AF_INET,svcspec,NULL,timeout,0) ;
	    s = rs ;
	    break ;

	case dialer_tcpnls:
	    f_shutdown = TRUE ;
	    rs = dialtcpnls(hostname,portspec,AF_INET,svcspec,timeout,0) ;
	    s = rs ;
	    break ;

	case dialer_udp:
	    rs = dialudp(hostname,portspec,AF_INET,timeout,0) ;
	    s = rs ;
	    mxu = MSGBUFLEN ;
	    break ;

	case dialer_ticotsordnls:
	    if ((portspec == NULL) || (portspec[0] == '\0'))
	        portspec = hostname ;

	    rs = dialticotsordnls(portspec,-1,svcspec,timeout,0) ;
	    s = rs ;
	    rs = u_ioctl(s,I_PUSH,"tirdwr") ;

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
	    if (pip->debuglevel > 1)
	        debugprintf("main: DIALUSD portspec=%s\n",portspec) ;
#endif

	    operms = (S_IFSOCK | 0600) ;
	    template = "/tmp/echoXXXXXXXXXX" ;
	    if ((rs = opentmpfile(template,0,operms,srcpath)) >= 0) {
	        SOCKADDRESS	dst ;
	        int	alen ;
	        s = rs ;

	            sockaddress_start(&dst,AF_UNIX,portspec,0,0) ;

	            alen = sockaddress_getlen(&dst) ;

	            rs = u_connect(s,(struct sockaddr *) &dst,alen) ;

	            sockaddress_finish(&dst) ;

	            if (rs < 0)
	                u_close(s) ;
	    } /* end if (got a socket) */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: dialusd() rs=%d\n",rs) ;
#endif

	    mxu = MSGBUFLEN ;
	    break ;

	case dialer_prog:
	    f_shutdown = TRUE ;
	    {
	        int	an, size ;
	        char	**av ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: DIALPROG portspec=%s\n",portspec) ;
#endif

	        if ((portspec == NULL) || (portspec[0] == '\0'))
	            portspec = hostname ;

	        if (portspec == NULL)
	            portspec = "cat" ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("main: argc=%d ai=%d\n",argc,ai) ;
	            if (ai > 0)
	                debugprintf("main: argv[%d]=>%s<\n",ai,argv[ai]) ;
	        }
#endif

	        an = 1 ;
	        if (ai > 0)
	            an += (argc - ai) ;

	        size = (an + 2) * sizeof(char *) ;
	        if ((uc_malloc(size,&av)) >= 0) {
		    const int	of = O_NOCTTY ;

	        av[0] = strbasename(portspec) ;

	        for (i = 1 ; i < an ; i += 1)
	            av[i] = argv[ai + i - 1] ;

	        av[i] = NULL ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("main: an=%d\n",an) ;
	            for (i = 0 ; av[i] != NULL ; i += 1)
	                debugprintf("main: arg[%d]=%s\n",
	                    i,av[i]) ;
	        }
#endif /* CF_DEBUG */

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: dialprog() \n") ;
#endif

	        rs = dialprog(portspec,of,av,NULL,&s2) ;
	        s = rs ;

	        uc_free(av) ;
		} /* end if (memory-allocation) */

	    } /* end block */
	    break ;

	case dialer_cprog:
	    f_shutdown = TRUE ;
	    {
	        int	an, size ;
	        int	to = timeout ;
	        int	opts = 0 ;

	        char	**av ;


#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: dialcprog() portspec=%s\n",portspec) ;
#endif

	        opts |= DIALOPTS_MPWD ;

	        if (portspec == NULL)
	            portspec = "cat" ;

	        an = 1 ;
	        if (ai > 0)
	            an += (argc - ai) ;

	        size = (an + 2) * sizeof(char *) ;
	        uc_malloc(size,&av) ;

	        av[0] = strbasename(portspec) ;

	        for (i = 1 ; i < an ; i += 1)
	            av[i] = argv[ai + i - 1] ;

	        av[i] = NULL ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("main: an=%d\n",an) ;
	            for (i = 0 ; av[i] != NULL ; i += 1)
	                debugprintf("main: arg[%d]=%s\n",
	                    i,av[i]) ;
	        }
#endif /* CF_DEBUG */

	        rs = dialcprog(pip->pr,hostname,portspec,av,NULL,to,opts,&s2) ;
	        s = rs ;

	        uc_free(av) ;

	    } /* end block */

	    break ;

	case dialer_rcmdr:
	    f_shutdown = TRUE ;
	    if (portspec == NULL)
	        portspec = "cat" ;

	    rs = rcmdr(hostname,pip->username,portspec,&s2) ;
	    s = rs ;
	    break ;

	} /* end switch (dialer selection) */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 3 dialer rs=%d s=%d s2=%d\n",rs,s,s2) ;
#endif

	if (rs < 0)
	    goto badconnect ;

	if (pip->debuglevel > 0) {

	    bprintf(efp,"%s: connected to the host FD=%d\n",
	        pip->progname,s) ;

	    bflush(efp) ;

	}


	if (rs >= 0)
	rs = transfer(pip,hostname,s,s2,FD_STDIN,FD_STDOUT,FD_STDERR,mxu) ;

	u_close(s) ;

	if (s2 >= 0)
	    u_close(s2) ;

	ex = EX_OK ;
	if (rs < 0) {
	    ex = EX_DATAERR ;
	    bprintf(efp,"%s: exiting bad rs=%d\n", pip->progname,rs) ;
	}

/* and exit */
done:
ret4:
	if (f_shutdown)
	    u_shutdown(s,2) ;

ret3:
	u_close(s) ;

ret2:
	if (srcpath[0] != '\0')
	    u_unlink(srcpath) ;

badret:
retearly:
ret1:
	bclose(efp) ;

ret0:
	return ex ;

/* handle some other stuff */
usage:
	bprintf(efp,
	    "%s: USAGE> %s [host|prog] [-d dialer[:info]] [-n]\n",
	    pip->progname,pip->progname) ;

	bprintf(efp,
	    "%s: \t[-q] [-v[=n]] [-D[=n]]\n",
	    pip->progname) ;

	goto retearly ;

/* handle bad arguments */
badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",
	    pip->progname) ;

	goto badarg ;

badextra:
	bprintf(efp,"%s: extra command line arguments ignored\n",
	    pip->progname) ;

	goto badarg ;

badargval:
	bprintf(efp,"%s: bad argument value specified\n",
	    pip->progname) ;

	goto badarg ;

badhost:
	bprintf(efp,"%s: no host was specified\n",
	    pip->progname) ;

	goto badarg ;

baddial:
	bprintf(efp,"%s: unknown dialer specified\n",
	    pip->progname) ;

	goto badarg ;

badarg:
	ex = EX_USAGE ;
	goto retearly ;

/* handle other things */
badconnect:
	ex = EX_UNAVAILABLE ;
	bprintf(efp,"%s: could not connect to host (%d)\n",
	    pip->progname,rs) ;

	goto ret2 ;

}
/* end subroutine (main) */


/* local subroutines */


static int transfer(pip,hostname,rfd,r2fd,ifd,ofd,efd,mxu)
PROGINFO	*pip ;
const char	hostname[] ;
int	rfd, r2fd ;
int	ifd, ofd, efd ;
int	mxu ;
{
	struct pollfd	fds[NFDS] ;
	struct fpstat	fp[NFDS] ;
	struct ustat	sb ;
	time_t	t_pollsanity ;
	time_t	t_sanity ;
	int	rs ;
	int	i, nfds, len, sanityfailures = 0 ;
	int	fdi = 0 ;
	int	pollint = (10 * POLLINTMULT) ;
	int	pollinput = POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI ;
	int	polloutput = POLLWRNORM | POLLWRBAND ;
	int	c_already = 0 ;
	int	f_exit ;
	int	f_issock = FALSE ;

	char	buf[BUFLEN + 1] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/transfer: rfd=%d r2fd=%d\n",rfd,r2fd) ;
#endif

	for (i = 0 ; i < 6 ; i += 1) {

	    fds[i].fd = -1 ;
	    fds[i].events = 0 ;
	    fds[i].revents = 0 ;
	    memset(fp + i,0,sizeof(struct fpstat)) ;

	}

	f_issock = isasocket(rfd) ;

/* standard input */

	fds[0].fd = -1 ;
	if (! pip->f.ni) {

	    fds[0].fd = ifd ;
	    fds[0].events = POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI ;

	} else
	    fp[0].eof = TRUE ;

/* standard output */

	fds[1].fd = -1 ;
	if ((rs = u_fstat(ofd,&sb)) >= 0) {

	    fds[1].fd = ofd ;
	    fds[1].events = POLLWRNORM | POLLWRBAND ;

	}

/* standard error */

	fds[2].fd = -1 ;
	if ((r2fd >= 0) && (u_fstat(efd,&sb) >= 0)) {

	    fds[2].fd = efd ;
	    fds[2].events = POLLWRNORM | POLLWRBAND ;
	    fds[2].events != POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI ;

	}

/* remote socket */

	fds[3].fd = rfd ;
	fds[3].events = POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI ;

	if (pip->f.ni) {

	    if (f_issock)
	        u_shutdown(rfd,SHUT_WR) ;

	    else
	        u_write(rfd,buf,0) ;

	} else
	    fds[3].events |= (POLLWRNORM | POLLWRBAND) ;

/* secondary connection */

	fdi = 4 ;
	if (r2fd >= 0) {

	    fds[fdi].fd = r2fd ;
	    fds[fdi].events = POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI ;
	    fds[fdi].events |= (POLLWRNORM | POLLWRBAND) ;

	    fdi += 1 ;
	}

/* what about sanity checking */

	if (pip->f.sanity) {

	    t_pollsanity = 0 ;
	    t_sanity = 1 ;
	    sanityfailures = 0 ;

	}

/* do the copy data function */

	f_exit = FALSE ;
	while (! f_exit) {

	    rs = u_poll(fds,fdi,pollint) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main/transfer: back from POLL w/ rs=%d\n",
	            rs) ;
#endif

	    if (rs < 0) {

	        if (rs == SR_AGAIN) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("main/transfer: from POLL w/ EAGAIN\n") ;
#endif

	            sleep(1) ;

	            continue ;

	        } else if (rs == SR_INTR) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("main/transfer: from POLL w/ EINTR\n") ;
#endif

	            continue ;

	        } else {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("main/transfer: from POLL w/ BADPOLL\n") ;
#endif

	            break ;
	        }

	    } /* end if (poll got an error) */

	    nfds = rs ;

#if	CF_DEBUG
	    if (pip->debuglevel > 2) {

	        for (i = 0 ; i < NFDS ; i += 1) {

	            debugprintf("main/transfer: fds%d %s\n",i,
	                d_reventstr(fds[i].revents,buf,BUFLEN)) ;

	        }
	    }
#endif /* CF_DEBUG */

/* check the actual low level events */

	    if (fds[0].revents != 0) {

	        if (fds[0].revents & pollinput) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("main/transfer: IN0\n") ;
#endif

	            fp[0].in = TRUE ;
	            fds[0].events &= (~ pollinput) ;

	        }

	        if (fds[0].revents & POLLHUP) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("main/transfer: HUP0\n") ;
#endif

	            fp[0].final = TRUE ;
	            fp[0].out = FALSE ;
	            fds[0].events &= (~ polloutput) ;

	        }

	    } /* end if */

	    if (fds[1].revents != 0) {

	        if (fds[1].revents & polloutput) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("main/transfer: OUT1\n") ;
#endif

	            fp[1].out = TRUE ;
	            fds[1].events &= (~ polloutput) ;

	        }

	        if (fds[1].revents & POLLHUP) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("main/transfer: HUP1\n") ;
#endif

	            fp[1].final = TRUE ;
	            fp[1].out = FALSE ;
	            fds[1].events &= (~ polloutput) ;

	        }

	    } /* end if */

	    if (fds[2].revents != 0) {

	        if (fds[2].revents & pollinput) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("main/transfer: IN2\n") ;
#endif

	            fp[2].in = TRUE ;
	            fds[2].events &= (~ pollinput) ;

	        }

	        if (fds[2].revents & polloutput) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("main/transfer: OUT2\n") ;
#endif

	            fp[2].out = TRUE ;
	            fds[2].events &= (~ polloutput) ;

	        }

	        if (fds[2].revents & POLLHUP) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("main/transfer: HUP2\n") ;
#endif

	            fp[2].final = TRUE ;
	            fp[2].out = FALSE ;
	            fds[2].events &= (~ polloutput) ;

	        }

	    } /* end if */

/* the remote promary connection */

	    if (fds[3].revents != 0) {

	        if (fds[3].revents & pollinput) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("main/transfer: IN3\n") ;
#endif

	            fp[3].in = TRUE ;
	            fds[3].events &= (~ pollinput) ;

	        }

	        if (fds[3].revents & polloutput) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("main/transfer: OUT3\n") ;
#endif

	            fp[3].out = TRUE ;
	            fds[3].events &= (~ polloutput) ;

	        }

	        if (fds[3].revents & POLLHUP) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("main/transfer: HUP3\n") ;
#endif

	            fp[3].out = FALSE ;
	            fp[3].final = TRUE ;
	            fds[3].events &= (~ polloutput) ;

	        }

	        if ((fds[3].revents & POLLERR) ||
	            (fds[3].revents & POLLNVAL)) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2) {

	                if (fds[3].revents & POLLERR)
	                    debugprintf("main/transfer: ERR3\n") ;

	                if (fds[3].revents & POLLNVAL)
	                    debugprintf("main/transfer: NVAL3\n") ;

	            }
#endif /* CF_DEBUG */

	            fp[3].in = FALSE ;
	            fp[3].out = FALSE ;
	            fp[3].final = TRUE ;
	            fds[3].fd = -1 ;
	            fds[3].events = 0 ;

	        }

	    } /* end if */

/* the secondard connection */

	    if ((fdi > 4) && (fds[4].revents != 0)) {

	        if (fds[4].revents & pollinput) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("main/transfer: IN4\n") ;
#endif

	            fp[4].in = TRUE ;
	            fds[4].events &= (~ pollinput) ;

	        }

	        if (fds[4].revents & polloutput) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("main/transfer: OUT4\n") ;
#endif

	            fp[4].out = TRUE ;
	            fds[4].events &= (~ polloutput) ;

	        }

	        if (fds[4].revents & POLLHUP) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("main/transfer: HUP4\n") ;
#endif

	            fp[4].out = FALSE ;
	            fp[4].final = TRUE ;
	            fds[4].events &= (~ polloutput) ;

	        }

	        if ((fds[4].revents & POLLERR) ||
	            (fds[4].revents & POLLNVAL)) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2) {

	                if (fds[4].revents & POLLERR)
	                    debugprintf("main/transfer: ERR4\n") ;

	                if (fds[4].revents & POLLNVAL)
	                    debugprintf("main/transfer: NVAL4\n") ;

	            }
#endif /* CF_DEBUG */

	            fp[4].in = FALSE ;
	            fp[4].out = FALSE ;
	            fp[4].final = TRUE ;
	            fds[4].fd = -1 ;
	            fds[4].events = 0 ;

	        }

	    } /* end if (secondary connection) */


/* now we are ready to check for the events that we really want */

/* output from remote connection to our standard output */

	    if (fp[3].in && fp[1].out) {

	        len = u_read(rfd,buf,mxu) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 2)
	            debugprintf("main/transfer: IN3 -> OUT1 len=%d\n",
	                len) ;
#endif

	        if (len <= 0) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("main/transfer: IN3 EOF\n") ;
#endif

	            fp[3].eof = TRUE ;
	            fp[3].in = FALSE ;
	            fds[3].events &= (~ pollinput) ;

	        } else
	            uc_writen(fds[1].fd,buf,len) ;

	        if (! fp[3].eof) {

	            fp[3].in = FALSE ;
	            fds[3].events |= pollinput ;

	        } else if (fds[3].events == 0)
	            fds[3].fd = -1 ;

	        fp[1].out = FALSE ;
	        fds[1].events |= polloutput ;

	    } /* end if (remote connection to standard output) */

/* input from our standard input out to the remote connection */

	    if (fp[0].in && fp[3].out) {

	        len = u_read(ifd,buf,mxu) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 2)
	            debugprintf("main/transfer: IN0 -> OUT3 len=%d\n",len) ;
#endif

	        if (len <= 0) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("main/transfer: IN0 EOF\n") ;
#endif

	            fp[0].eof = TRUE ;
	            fp[0].in = FALSE ;
	            fds[0].fd = -1 ;

#if	CF_DEBUG
	            if (pip->debuglevel > 2) {
	                rs = u_fstat(rfd,&sb) ;

	                debugprintf("main/transfer: RFD stat rs=%d mode=%08X\n",
	                    rs,sb.st_mode) ;
	            }
#endif /* CF_DEBUG */

	            if (f_issock)
	                u_shutdown(rfd,SHUT_WR) ;

	            else
	                u_write(rfd,buf,0) ;

	        } else
	            uc_writen(fds[3].fd,buf,len) ;

	        if (! fp[0].eof) {

	            fp[0].in = FALSE ;
	            fds[0].events |= pollinput ;

	        }

	        fp[3].out = FALSE ;
	        fds[3].events |= polloutput ;

	    } /* end if (standard input to primary socket) */

/* standard error to secondary socket */

	    if ((r2fd >= 0) && (fp[2].in && fp[4].out)) {

	        len = u_read(efd,buf,mxu) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 2)
	            debugprintf("main/transfer: IN2 -> OUT4 len=%d\n",len) ;
#endif

	        if (len <= 0) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("main/transfer: IN2 EOF\n") ;
#endif

	            fp[2].eof = TRUE ;
	            fp[2].in = FALSE ;
	            fds[2].events &= (~ pollinput) ;

	            if (f_issock)
	                u_shutdown(r2fd,SHUT_WR) ;

	            else
	                u_write(r2fd,buf,0) ;

	        } else
	            uc_writen(fds[4].fd,buf,len) ;

	        if (! fp[2].eof) {

	            fp[2].in = FALSE ;
	            fds[2].events |= pollinput ;

	        } else if (fds[2].events == 0)
	            fds[2].fd = -1 ;

	        fp[4].out = FALSE ;
	        fds[4].events |= polloutput ;

	    } /* end if (standard error to secondary connection) */

/* secondary socket to standard error */

	    if ((r2fd >= 0) && (fp[4].in && fp[2].out)) {

	        len = u_read(r2fd,buf,mxu) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 2)
	            debugprintf("main/transfer: IN4 -> OUT2 len=%d\n",
	                len) ;
#endif

	        if (len <= 0) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("main/transfer: IN4 EOF\n") ;
#endif

	            fp[4].eof = TRUE ;
	            fp[4].in = FALSE ;
	            fds[4].events &= (~ pollinput) ;

	        } else
	            uc_writen(fds[2].fd,buf,len) ;

	        if (! fp[4].eof) {

	            fp[4].in = FALSE ;
	            fds[4].events |= pollinput ;

	        } else if (fds[4].events == 0)
	            fds[4].fd = -1 ;

	        fp[2].out = FALSE ;
	        fds[2].events |= polloutput ;

	    } /* end if (secondary connection to standard error) */

/* should we break out ? */

#ifdef	COMMENT
	    if (fp[0].eof && fp[3].eof)
	        break ;
#else
	    if ((c_already > 0) && (nfds == 0)) {

	        if (c_already > 1)
	            break ;

	        c_already += 1 ;
	    }

	    if (fp[3].eof && (c_already == 0)) {

	        pollint = POLLINTMULT / 2 ;
	        c_already = 1 ;
	    }
#endif /* COMMENT */

/* miscellaneous functions */

	    if (pip->f.sanity) {

	        u_time(&pip->daytime) ;

	        if ((pip->daytime - t_pollsanity) > 
	            (pip->keeptime / SANITYFAILURES)) {

	            t_pollsanity = pip->daytime ;
	            if (inetping(hostname,TO_PING) >= 0) {

	                sanityfailures = 0 ;
	                t_sanity = pip->daytime ;

	            } else
	                sanityfailures += 1 ;

	            if (((pip->daytime - t_sanity) > pip->keeptime) &&
	                (sanityfailures >= SANITYFAILURES) &&
	                ((rs = inetping(hostname,TO_PING)) < 0))
	                break ;

	        } /* end if (sanity poll) */

	    } /* end if (sanity check) */

	} /* end while (transferring data) */

ret0:
	return rs ;
}
/* end subroutine (transfer) */



