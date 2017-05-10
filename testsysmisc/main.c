/* main */

/* program to send echo data to a remote host (and back) */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	1		/* run-time debugging */
#define	CF_REP		1		/* ask for repetition */


/* revision history:

	= 1998-03-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine forms a program that sends data to a remote INET host to
        its 'echo' service.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<sys/time.h>
#include	<netinet/in.h>
#include	<time.h>
#include	<signal.h>
#include	<netdb.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<bfile.h>
#include	<sockaddress.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"sysmisc.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#define	NPARG		2	/* number of positional arguments */

#define	TO_READ		30
#undef	BUFLEN
#define	BUFLEN		((10 * 1024) + MAXHOSTNAMELEN)
#define	MAXLEN		BUFLEN

#define	TAG		23
#define	DURATION	15

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	DENOM		(1000 * 1000)

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif


/* external variables */

extern int	cfdeci(const char *,int,int *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	listenudp(int,const char *,const char *,int) ;
extern int	dialudp(char *,char *,int,int,int) ;
extern int	dialtcp(const char *,const char *,int,int,int) ;
extern int	dialtcpnls(char *,char *,int,char *,int,int) ;
extern int	dialtcpmux(char *,char *,int,char *,char **,int,int) ;
extern int	dialuss(const char *,int,int) ;
extern int	dialusd(const char *,int,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	opentmpusd(const char *,int,mode_t,char *) ;
extern int	dialticotsordnls(const char *,int,const char *,int,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strbasename(char *) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int bprintmsg(bfile *,char *,int) ;

#if	CF_DEBUGS
extern int	mkhexstr(char *,int,void *,int) ;
#endif


/* local variables */

static const char *argopts[] = {
	"ROOT",
	NULL
} ;

enum argopts {
	argopt_root,
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
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;

	bfile	errfile, *efp = &errfile ;
	bfile	outfile, *ofp = &outfile ;

	mode_t	operms ;

	int	argr, argl, aol, akl, avl, npa, maxai, kwi ;
	int	rs = SR_OK ;
	int	argnum ;
	int	pan, i, len ;
	int	dialer ;
	int	s, fd_listen = -1 ;
	int	timeout = -1 ;
	int	fd_debug ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_done = FALSE ;
	int	f_quiet = FALSE ;
	int	f_ignore = FALSE ;
	int	f_log = FALSE ;

	char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS + 1] ;
	char	buf[BUFLEN + 1] ;
	char	srcpath[MAXPATHLEN + 1] ;
	char	hostnamebuf[MAXHOSTNAMELEN + 1] ;
	char	dialspecbuf[MAXHOSTNAMELEN + 1] ;
	char	*logfname = NULL ;
	char	*dialspec = NULL ;
	char	*hostname = NULL ;
	char	*portspec = NULL ;
	char	*svcspec = NULL ;
	char	*cp ;


	if ((cp = getenv(VARDEBUGFD1)) == NULL)
	    cp = getenv(VARDEBUGFD2) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


#if	CF_DEBUGS
	debugprintf("main: started\n") ;
#endif

	memset(pip,0,sizeof(struct proginfo)) ;

	pip->progname = strbasename(argv[0]) ;

	if ((rs = bopen(efp,BFILE_STDERR,"dwca",0666)) >= 0) {
		pip->f.errfile = TRUE ;
	    pip->efp = &errfile ;
	    bcontrol(efp,BC_LINEBUF,0) ;
	}

/* early initialization */

	pip->verboselevel = 1 ;

/* the arguments */

	for (i = 0 ; i < MAXARGGROUPS ; i += 1) argpresent[i] = 0 ;

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
	i = 0 ;
	argr = argc - 1 ;
	while ((! f_done) && (argr > 0)) {

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

/* do we have a keyword match or should we assume only key letters ? */

#if	CF_DEBUGS
	                debugprintf("main: key word match ? >%W<\n",
	                    aop,aol) ;
#endif

	                if ((kwi = matstr3(argopts,aop,aol)) >= 0) {

#if	CF_DEBUGS
	                    debugprintf("main: got an option keyword, kwi=%d\n",
	                        kwi) ;
#endif

	                    switch (kwi) {

/* program root */
	                    case argopt_root:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                pip->programroot = avp ;

	                        } else {

	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                pip->programroot = argp ;

	                        }

	                        break ;

	                    } /* end switch (key words) */

	                } else {

#if	CF_DEBUGS
	                    debugprintf("main: got an option key letter\n") ;
#endif

	                    while (akl--) {

#if	CF_DEBUGS
	                        debugprintf("main: option key letters\n") ;
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
	                            bprintf(efp,"%s: version %s\n",
	                                pip->progname,VERSION) ;

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
	                            f_quiet = TRUE ;
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

	                BASET(argpresent,i) ;
	                maxai = i ;
	                npa += 1 ;	/* increment position count */

	            }

	        } /* end if */

	    } else {

	        if (i < MAXARGGROUPS) {

	            BASET(argpresent,i) ;
	            maxai = i ;
	            npa += 1 ;

	        } else {

	            if (! f_extra) {

	                f_extra = TRUE ;
	                bprintf(efp,"%s: extra arguments ignored\n",
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

	if (pip->debuglevel > 0) {

	    bprintf(efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	    bflush(efp) ;

	}

	if (f_usage)
	    goto usage ;

	if (f_version || f_help || f_usage)
	    goto earlyret ;


	ex = EX_OK ;

/* check arguments */

#if	CF_DEBUGS
	debugprintf("main: checking arguments\n") ;
#endif

	if (f_log && ((logfname == NULL) || (logfname[0] == '\0'))) {

	    f_log = FALSE ;

	}

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
	            break ;

	        } /* end switch */

	        pan += 1 ;

	    } /* end for (looping through requested circuits) */

	} /* end if */


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: checking hostname=%s portspec=%s\n",
	        hostname,portspec) ;
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
	        debugprintf("main: dialspec=%s portspec=%s\n",dialspec,portspec) ;
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

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 2 dialer=%s(%d)\n",dialers[dialer],dialer) ;
#endif

	switch (dialer) {

	case dialer_tcp:
	    rs = dialtcp(hostname,portspec,AF_INET,timeout,0) ;

	    s = rs ;
	    break ;

	case dialer_tcpmux:
	    rs = dialtcpmux(hostname,portspec,AF_INET,svcspec,NULL,timeout,0) ;

	    s = rs ;
	    break ;

	case dialer_tcpnls:
	    rs = dialtcpnls(hostname,portspec,AF_INET,svcspec,timeout,0) ;

	    s = rs ;
	    break ;

	case dialer_udp:
	    rs = dialudp(hostname,portspec,AF_INET,timeout,0) ;

	    s = rs ;
	    break ;

	case dialer_ticotsordnls:
	    if ((portspec == NULL) || (portspec[0] == '\0'))
	        portspec = hostname ;

	    rs = dialticotsordnls(portspec,-1,svcspec,timeout,0) ;

	    s = rs ;
	    break ;

	case dialer_uss:
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
	    rs = opentmpfile("/tmp/echoXXXXXXXXXX",0,operms,srcpath) ;
	    s = rs ;
	    if (rs >= 0) {

	            SOCKADDRESS	dst ;

	            int	salen ;


	            rs = sockaddress_start(&dst,AF_UNIX,portspec,0,0) ;

		    if (rs >= 0) {

	                salen = sockaddress_getlen(&dst) ;

	                rs = u_connect(s,(struct sockaddr *) &dst,salen) ;

	                sockaddress_finish(&dst) ;

		    } /* end if */

	            if (rs < 0)
	                u_close(s) ;

	    } /* end if (got a socket) */

	    break ;

	} /* end switch (dialer selection) */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 3 dialer rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto badconnect ;

	if (pip->debuglevel > 0) {

	    bprintf(efp,"%s: connected to the host FD=%d\n",
	        pip->progname,s) ;

	    bflush(efp) ;

	}

/* listen on a local UDP port */

	{
	    const int	af = AF_INET ;
	    rs = listenudp(af,"anyhost",NULL,0) ;
	    fd_listen = rs ;
	}
	if (rs < 0)
		goto ret5 ;

	bopen(ofp,BFILE_STDOUT,"wct",0666) ;

	bcontrol(ofp,BC_LINEBUF,0) ;

/* do it */

	{
		struct sysmisc_request	m0 ;

		struct sysmisc_loadave	m1 ;

		struct sysmisc_hostinfo	m3 ;

		SOCKADDRESS	sa ;

		uint	addrhost[4] ;
		uint	addrfamily, addrport ;

		int	type, blen ;
		int	salen ;
		int	size ;

		char	inbuf[BUFLEN + 1] ;
		char	timebuf[TIMEBUFLEN + 1] ;


/* get our socket "name" */

		salen = sizeof(SOCKADDRESS) ;
		u_getsockname(fd_listen,&sa,&salen) ;

/* get the individual information elements of it */

		size = 4 * sizeof(uint) ;
		sockaddress_getaddr(&sa,addrhost,size) ;

		addrfamily = sockaddress_getaf(&sa) ;

		addrport = sockaddress_getport(&sa) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: addrfamily=%04x addrport=%04x\n",
		addrfamily,addrport) ;
#endif

/* fill in the message fields */

		memset(&m0,0,sizeof(struct sysmisc_request)) ;

		m0.tag = TAG ;
#if	CF_REP
		m0.duration = DURATION ;
		m0.interval = 5 ;
		m0.opts |= (SYSMISC_MLOADAVE | SYSMISC_MUDP) ;
#else
		m0.duration = 0 ;
		m0.interval = 0 ;
		m0.opts |= (SYSMISC_MLOADAVE | SYSMISC_MTCP) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: m0.opts=%04x\n",m0.opts) ;
#endif

		m0.addrfamily = addrfamily ;
		m0.addrport = addrport ;

#ifdef	OPTIONAL
		m0.addrhost[0] = addrhost[0] ;
		m0.addrhost[1] = addrhost[1] ;
		m0.addrhost[2] = addrhost[2] ;
		m0.addrhost[3] = addrhost[3] ;
#endif

		blen = sysmisc_request(buf,BUFLEN,0,&m0) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {

		int	cl ;

		char	hexbuf[100 + 1] ;


		cl = mkhexstr(hexbuf,100,buf,16) ;

		debugprintf("main: %t\n",hexbuf,cl) ;

		cl = mkhexstr(hexbuf,100,(buf + 16),MIN((blen - 16),16)) ;

		debugprintf("main: %t\n",hexbuf,cl) ;

		if ((blen - 32) > 0) {

		cl = mkhexstr(hexbuf,100,(buf + 32),(blen - 32)) ;

		debugprintf("main: %t\n",hexbuf,cl) ;

		}
		}
#endif /* CF_DEBUG */

		bprintf(ofp,"transmitted message\n") ;

		bprintmsg(ofp,buf,blen) ;

	    rs = u_write(s,buf,blen) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: u_write() blen=%d rs=%d\n",blen,rs) ;
#endif

		memset(buf,0,BUFLEN) ;

		if (rs >= 0)
		rs = uc_reade(s,inbuf,SYSMISC_SHOSTINFO,TO_READ,FM_EXACT) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: u_read() rs=%d\n",rs) ;
#endif

		len = rs ;
		if (rs >= 0) {

		bprintf(ofp,"initial received message\n") ;

		bprintmsg(ofp,inbuf,len) ;

		} /* end if */

#if	CF_REP
		{
			time_t	daytime, endtime ;


		daytime = time(NULL) ;

		endtime = daytime + DURATION + 5 ;
		while ((rs >= 0) && (daytime < endtime)) {

			rs = uc_recve(fd_listen,inbuf,BUFLEN,0,TO_READ,0) ;

				len = rs ;
			if (rs > 0) {

				bprintf(ofp,"periodic message\n") ;

				bprintmsg(ofp,inbuf,len) ;

			}

			daytime = time(NULL) ;

		} /* end while */

		rs = SR_OK ;

		} /* end block */
#endif /* CF_REP */

		sockaddress_finish(&sa) ;

	} /* end block */

	bclose(ofp) ;

	if (fd_listen >= 0)
	    u_close(fd_listen) ;

ret5:
	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

	if (rs < 0)
	    bprintf(efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

/* and exit */
done:
ret4:
	u_shutdown(s,2) ;

ret3:
	u_close(s) ;

ret2:
	if (srcpath[0] != '\0')
	    u_unlink(srcpath) ;

badret:
earlyret:
ret1:
	bclose(efp) ;

ret0:
	return ex ;

/* handle some other stuff */
usage:
	bprintf(efp,
	    "%s: USAGE> %s [-V] [-s] [-q] [-i] [-v] <timehost(s)>",
	    pip->progname,pip->progname) ;

	bprintf(efp,
	    " [-l <logfile>] [-D]\n") ;

	goto earlyret ;

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
	goto earlyret ;

/* handle other things */
badconnect:
	ex = EX_UNAVAILABLE ;
	bprintf(efp,"%s: could not connect to host (%d)\n",
	    pip->progname,rs) ;

	goto ret2 ;

}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



static int bprintmsg(ofp,buf,buflen)
bfile	*ofp ;
char	buf[] ;
int	buflen ;
{
	struct sysmisc_request	m0 ;

	struct sysmisc_loadave	m1 ;

	struct sysmisc_extra	m2 ;

	struct sysmisc_hostinfo	m3 ;

	int	rs = SR_OK ;
	int	type ;

	char	timebuf[TIMEBUFLEN + 1] ;


	type = (int) ((uint) buf[0]) ;

	switch (type) {

	case sysmisctype_request:
		rs = sysmisc_request(buf,buflen,1,&m0) ;

		bprintf(ofp,"msglen=%d\n",rs) ;

		bprintf(ofp,"type=%d\n",m0.type) ;

		bprintf(ofp,"tag=%d\n",m0.tag) ;

		bprintf(ofp,"duration=%d\n",m0.duration) ;

		bprintf(ofp,"interval=%d\n",m0.interval) ;

		bprintf(ofp,"addrfamily=%04x\n",m0.addrfamily) ;

		bprintf(ofp,"addrport=%04x\n",m0.addrport) ;

		bprintf(ofp,"addrhost0=%04x\n",m0.addrhost[0]) ;

		bprintf(ofp,"addrhost1=%04x\n",m0.addrhost[1]) ;

		bprintf(ofp,"addrhost2=%04x\n",m0.addrhost[2]) ;

		bprintf(ofp,"addrhost3=%04x\n",m0.addrhost[3]) ;

		bprintf(ofp,"opts=%04x\n",m0.opts) ;

		break ;

	case sysmisctype_loadave:
		rs = sysmisc_loadave(buf,buflen,1,&m1) ;

		bprintf(ofp,"msglen=%d\n",rs) ;

		type = m1.type ;
		bprintf(ofp,"type=%d\n",type) ;

		bprintf(ofp,"tag=%d\n",m1.tag) ;

		bprintf(ofp,"rc=%d\n",m1.rc) ;

		bprintf(ofp,"timestamp=%s\n",
			timestr_log(m1.timestamp,timebuf)) ;

		bprintf(ofp,"providerid=%d\n",m1.providerid) ;

		bprintf(ofp,"hostid=%08x\n",m1.hostid) ;

		bprintf(ofp,"la_1min=%d\n",m1.la_1min) ;

		bprintf(ofp,"la_5min=%d\n",m1.la_5min) ;

		bprintf(ofp,"la_15min=%d\n",m1.la_15min) ;

		break ;

	case sysmisctype_extra:
		rs = sysmisc_extra(buf,buflen,1,&m2) ;

		bprintf(ofp,"msglen=%d\n",rs) ;

		bprintf(ofp,"type=%d\n",m2.type) ;

		bprintf(ofp,"tag=%d\n",m2.tag) ;

		bprintf(ofp,"rc=%d\n",m2.rc) ;

		bprintf(ofp,"timestamp=%s\n",
			timestr_log(m2.timestamp,timebuf)) ;

		bprintf(ofp,"providerid=%d\n",m2.providerid) ;

		bprintf(ofp,"hostid=%08x\n",m2.hostid) ;

		bprintf(ofp,"la_1min=%d\n",m2.la_1min) ;

		bprintf(ofp,"la_5min=%d\n",m2.la_5min) ;

		bprintf(ofp,"la_15min=%d\n",m2.la_15min) ;

		bprintf(ofp,"boottime=%s\n",
			timestr_log(m2.boottime,timebuf)) ;

		bprintf(ofp,"nproc=%d\n",m2.nproc) ;

		break ;

	case sysmisctype_hostinfo:
		rs = sysmisc_hostinfo(buf,buflen,1,&m3) ;

		bprintf(ofp,"msglen=%d\n",rs) ;

		bprintf(ofp,"type=%d\n",m3.type) ;

		bprintf(ofp,"tag=%d\n",m3.tag) ;

		bprintf(ofp,"rc=%d\n",m3.rc) ;

		bprintf(ofp,"timestamp=%s\n",
			timestr_log(m3.timestamp,timebuf)) ;

		bprintf(ofp,"providerid=%d\n",m3.providerid) ;

		bprintf(ofp,"hostid=%08x\n",m3.hostid) ;

		bprintf(ofp,"la_1min=%d\n",m3.la_1min) ;

		bprintf(ofp,"la_5min=%d\n",m3.la_5min) ;

		bprintf(ofp,"la_15min=%d\n",m3.la_15min) ;

		bprintf(ofp,"boottime=%s\n",
			timestr_log(m3.boottime,timebuf)) ;

		bprintf(ofp,"nproc=%d\n",m3.nproc) ;

		bprintf(ofp,"provider=%s\n",m3.provider) ;

		bprintf(ofp,"hostnamelen=%d\n",m3.hostnamelen) ;

		bprintf(ofp,"hostname=%s\n",m3.hostname) ;

		break ;

	default:
		bprintf(ofp,"len=%d type=%d\n",buflen,m0.type) ;

		bprintf(ofp,"unknown message\n") ;

	} /* end switch */

	return rs ;
}
/* end subroutine (bprintmsg) */



