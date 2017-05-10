/* main */

/* program to send echo data to a remote host (and back) */


#define	CF_DEBUGS	0
#define	CF_DEBUG	1


/* revision history:

	= 92/03/01, David A­D­ Morano

	This program was originally written.


*/


/**********************************************************************

	This subroutine forms a program that sends data to a remote
	INET host to its 'echo' service.


***********************************************************************/



#include	<sys/types.h>
#include	<sys/socket.h>
#include	<sys/time.h>
#include	<netinet/in.h>
#include	<time.h>
#include	<signal.h>
#include	<netdb.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<bfile.h>
#include	<sockaddress.h>
#include	<exitcodes.h>

#include	"localmisc.h"
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



/* external variables */

extern struct tm	*localtime() ;

extern int	cfdeci(const char *,int,int *) ;
extern int	matstr(char * const *,const char *,int) ;
extern int	dialudp(char *,char *,int,int,int) ;
extern int	dialtcp(const char *,const char *,int,int,int) ;
extern int	dialtcpnls(char *,char *,int,char *,int,int) ;
extern int	dialtcpmux(char *,char *,int,char *,char **,int,int) ;
extern int	dialuss(const char *,int,int) ;
extern int	dialusd(const char *,int,int) ;
extern int	opentmpusd(const char *,int,mode_t,char *) ;
extern int	dialticotsordnls(const char *,int,const char *,int,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strbasename(char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local data */

static char *const argopts[] = {
	"ROOT",			/* 0 */
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_overlast
} ;

static char	*const dialers[] = {
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





int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	struct proginfo	pi, *pip = &pi ;

	bfile	errfile, *efp = &errfile ;
	bfile	outfile, *ofp = &outfile ;

	mode_t	operms ;

	int	argr, argl, aol, akl, avl, npa, maxai, kwi ;
	int	rs = SR_OK ;
	int	argnum ;
	int	pan, i, len ;
	int	dialer, s, fd ;
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

	    bprintf(efp,"%s: about to check arguments\n",
	        pip->progname) ;

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


/* look up some miscellaneous stuff in various databases */

	switch (dialer) {

	case dialer_tcp:
	case dialer_udp:
	case dialer_tcpmux:
	case dialer_tcpnls:
	    if ((hostname == NULL) || (hostname[0] == '\0'))
	        goto badhost ;

	    if ((portspec == NULL) || (portspec[0] == '\0'))
	        portspec = PORTSPEC_ECHO ;

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
	    debugprintf("main: 2 dialer=%d\n",dialer) ;
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
	    rs = opentmpusd("/tmp/echoXXXXXXXXXX",0,operms,srcpath) ;
	    s = rs ;
	    if (rs >= 0) {

	            SOCKADDRESS	dst ;

	            int	alen ;


	            sockaddress_start(&dst,AF_UNIX,portspec,0,0) ;

	            alen = sockaddress_getlen(&dst) ;

	            rs = u_connect(s,(struct sockaddr *) &dst,alen) ;

	            sockaddress_finish(&dst) ;

	            if (rs < 0)
	                u_close(s) ;

	    } /* end if (got a socket) */

	    break ;

	} /* end switch */

	if (rs < 0)
	    goto badconnect ;

	if (pip->debuglevel > 0) {

	    bprintf(efp,"%s: connected to the host FD=%d\n",
	        pip->progname,s) ;

	    bflush(efp) ;

	}


	bopen(ofp,BFILE_STDOUT,"wct",0666) ;


/* do it */

	{
		hrtime_t	sum, ave ;

		int	ntime = 0 ;


		sum = 0 ;

	rs = SR_OK ;
	while (rs >= 0) {

		hrtime_t	start, end, diff ;


#if	CF_DEBUGS
	    bflush(efp) ;
#endif

	    rs = u_read(FD_STDIN,buf,BUFLEN) ;

	    if (rs < 0)
	        break ;

	    if (pip->debuglevel > 0)
	        bprintf(efp,"%s: STDIN read rs=%d\n",
	            pip->progname,rs) ;

	    len = rs ;
	    if (len == 0)
	        break ;

		start = gethrtime() ;

	    rs = u_write(s,buf,len) ;

	    if (pip->debuglevel > 0)
	        bprintf(efp,"%s: network write rs=%d\n",
	            pip->progname,rs) ;

	    if (rs < 0) {

	        bprintf(efp,"%s: bad network write rs=%d\n",
	            pip->progname,rs) ;

	        goto done ;
	    }

	    rs = uc_reade(s,buf,BUFLEN,TO_READ,0) ;

		end = gethrtime() ;

		diff = end - start ;
		sum += diff ;

	    if (pip->debuglevel > 0)
	        bprintf(efp,"%s: network read len=%d\n",
	            pip->progname,len) ;

	    if (rs < 0) {

	        bprintf(efp,"%s: bad network read rs=%d\n",
	            pip->progname,rs) ;

	        goto done ;
	    }

	    len = rs ;
	    if (len > 0)
	        rs = bwrite(ofp,buf,len) ;

	    if (pip->debuglevel > 0)
	        bprintf(efp,"%s: STDOUT write rs=%d\n",
	            pip->progname,rs) ;

		if (pip->verboselevel > 1)
	        bprintf(ofp,"delay=%lld msec\n",(diff / DENOM)) ;

		ntime += 1 ;
		bflush(ofp) ;

	} /* end while */

		if ((pip->verboselevel > 1) && (ntime > 0))
	        bprintf(ofp,"average delay=%lld\n",((sum / DENOM) / ntime)) ;

	} /* end block */

	bclose(ofp) ;


	ex = EX_OK ;
	if (rs < 0) {

	    ex = EX_DATAERR ;
	    bprintf(efp,"%s: exiting bad rs=%d\n",
	        pip->progname,rs) ;

	}


/* and exit */
done:
ret4:
	u_shutdown(s,2) ;

ret3:
	u_close(s) ;

ret2:
	if (srcpath[0] != '\0')
	    u_unlink(srcpath) ;

#ifdef	COMMENT
	bclose(ofp) ;
#endif

badret:
earlyret:
ret1:
	bclose(efp) ;

ret0:
	return ex ;

/* handle some other stuff */
usage:
	bprintf(efp,
	    "%s: USAGE> %s [-V] [-s] [-q] [-i] [-v] timehost(s)",
	    pip->progname,pip->progname) ;

	bprintf(efp,
	    " [-l logfile] [-D]\n") ;

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





