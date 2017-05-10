/* main */

/* almost a generic front-end */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1999-03-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/**********************************************************************

	This subroutine forms a program that sends data to a remote
	INET host to its 'echo' service.


***********************************************************************/


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
#include	<ctype.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<bfile.h>
#include	<sockaddress.h>
#include	<exitcodes.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"
#include	"mcmsg.h"



/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#define	NPARG		2	/* number of positional arguments */

#define	TO_READ		30
#undef	BUFLEN
#define	BUFLEN		((10 * 1024) + MAXHOSTNAMELEN)

#define	DENOM		(1000 * 1000)



/* external subroutines */

extern struct tm	*localtime() ;

extern int	cfdeci(const char *,int,int *) ;
extern int	matstr(const char **,const char *,int) ;
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


/* local variables */

static const char *argopts[] = {
	"ROOT",			/* 0 */
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_overlast
} ;

static const char *dialers[] = {
	"tcp",
	"tcpmux",
	"tcpnls",
	"udp",
	"uss",
	"usd",
	"ticotsordnls",
	NULL
} ;

#define	DIALER_TCP		0
#define	DIALER_TCPMUX		1
#define	DIALER_TCPNLS		2
#define	DIALER_UDP		3
#define	DIALER_USS		4
#define	DIALER_USD		5
#define	DIALER_TICOTSORDNLS	6






int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;

	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;

	mode_t	oflags ;

	int	argr, argl, aol, akl, avl, npa, maxai, kwi ;
	int	rs, rs1, i, argnum ;
	int	pan, len, blen ;
	int	ex = EX_INFO ;
	int	dialer, s, fd ;
	int	timeout = -1 ;
	int	cl ;
	int	fd_debug ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_done = FALSE ;
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
	char	*mailuser = NULL ;
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

	if ((rs = bopen(&errfile,BFILE_STDERR,"dwca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	}

/* early initialization */

	pip->verboselevel = 1 ;

/* the arguments */

	rs = SR_OK ;
	for (i = 0 ; i < MAXARGGROUPS ; i += 1) argpresent[i] = 0 ;

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
	i = 0 ;
	argr = argc - 1 ;
	while ((rs >= 0) && (! f_done) && (argr > 0)) {

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

/* do we have a keyword match or should we assume only key letters? */

	                if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

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

	                    } /* end switch (key words) */

	                } else {

	                    while (akl--) {

	                        switch ((int) *akp) {

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

				case 'u':
	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
					mailuser = argp ;

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
				    rs = SR_INVALID ;
	                            f_usage = TRUE ;
	                            bprintf(pip->efp,
	                            "%s: invalid option=%c\n",
	                            pip->progname,*akp) ;

	                        } /* end switch */

	                        akp += 1 ;
				if (rs < 0)
				    break ;

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
	                bprintf(pip->efp,"%s: extra arguments ignored\n",
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

	    bprintf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	    bflush(pip->efp) ;

	}

	if (f_usage)
	    goto usage ;

	ex = EX_INFO ;
	if (f_version) {
		bprintf(pip->efp,"%s: version %s\n",
	                                pip->progname,VERSION) ;
	    goto retearly ;
	}

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
	        debugprintf("main: dialspec=%s portspec=%s\n",
			dialspec,portspec) ;
#endif

	} /* end if */


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
	    dialer = DIALER_UDP ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 1 dialer=%d\n",dialer) ;
#endif


	if (timeout <= 0)
	    timeout = DIALTIME ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 1 debug info\n") ;
#endif

	if (pip->debuglevel > 0) {

	    for (i = 0 ; dialers[i] != NULL ; i += 1)
	        bprintf(pip->efp,"%s: dialer> %s(%d)\n",
	            pip->progname,dialers[i],i) ;

	    bprintf(pip->efp,"%s: got valid arguments, dialer(%d)=%s\n",
	        pip->progname,dialer,dialers[dialer]) ;

	} /* end if (debug info) */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 2 debug info\n") ;
#endif


/* look up some miscellaneous stuff in various databases */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: switching on dialer=%d\n",dialer) ;
#endif

	switch (dialer) {

	case DIALER_TCP:
	case DIALER_UDP:
	case DIALER_TCPMUX:
	case DIALER_TCPNLS:
	    if ((hostname == NULL) || (hostname[0] == '\0'))
	        goto badhost ;

	    if ((portspec == NULL) || (portspec[0] == '\0'))
	        portspec = PORTSPEC_MAILPOLL ;

	    break ;

	} /* end switch */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 2 dialer=%d portspec=%s\n",dialer,portspec) ;
#endif

	switch (dialer) {

	case DIALER_TCPMUX:
	case DIALER_TCPNLS:
	case DIALER_TICOTSORDNLS:
	    if ((svcspec == NULL) || (svcspec[0] == '\0'))
	        svcspec = SVCSPEC_MAILPOLL ;

	    break ;

	} /* end switch */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 3 dialer=%d svcspec=%s\n",dialer,svcspec) ;
#endif

	switch (dialer) {

	case DIALER_TCP:
	    rs = dialtcp(hostname,portspec,AF_INET,timeout,0) ;

	    s = rs ;
	    break ;

	case DIALER_TCPMUX:
	    rs = dialtcpmux(hostname,portspec,AF_INET,svcspec,NULL,timeout,0) ;

	    s = rs ;
	    break ;

	case DIALER_TCPNLS:
	    rs = dialtcpnls(hostname,portspec,AF_INET,svcspec,timeout,0) ;

	    s = rs ;
	    break ;

	case DIALER_UDP:
	    rs = dialudp(hostname,portspec,AF_INET,timeout,0) ;

	    s = rs ;
	    break ;

	case DIALER_TICOTSORDNLS:
	    if ((portspec == NULL) || (portspec[0] == '\0'))
	        portspec = hostname ;

	    rs = dialticotsordnls(portspec,-1,svcspec,timeout,0) ;

	    s = rs ;
	    break ;

	case DIALER_USS:
	    if ((portspec == NULL) || (portspec[0] == '\0'))
	        portspec = hostname ;

	    rs = dialuss(portspec,timeout,0) ;

	    s = rs ;
	    break ;

	case DIALER_USD:
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

	    } /* end if */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: dialusd() rs=%d\n",rs) ;
#endif

	    break ;

	} /* end switch */

	if (rs < 0)
	    goto badconnect ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: connected to the host FD=%d\n",
	        pip->progname,s) ;


/* do it */

	{
		struct mcmsg_report	m1 ;


	memset(&m1,0,sizeof(struct mcmsg_report)) ;

	if (mailuser != NULL)
		strwcpy(m1.mailuser,mailuser,MCMSG_LMAILUSER) ;

	blen = mcmsg_report(buf,MSGBUFLEN,0,&m1) ;

	rs = u_send(s,buf,blen,0) ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: sent message (%d)\n",
	        pip->progname,rs) ;

	}

/* is there a reply ? */

	if (rs >= 0) {

		rs1 = uc_recve(s,buf,BUFLEN,0,TO_READ,0) ;

		if ((rs1 == 0) || ((rs1 > 0) && (buf[0] != '\0')))
			rs1 = SR_INVALID ;

		if (pip->debuglevel > 0)
	    	bprintf(pip->efp,"%s: reply (%d)\n",
	        	pip->progname,rs1) ;

	}

/* get out */

	ex = EX_OK ;
	if (rs < 0) {

	    ex = EX_DATAERR ;
	    if (! pip->f.quiet)
	    	bprintf(pip->efp,"%s: exiting (%d)\n",
	        	pip->progname,rs) ;

	}


/* and exit */
done:
ret4:

ret3:
	u_close(s) ;

ret2:
	if (srcpath[0] != '\0')
	    u_unlink(srcpath) ;

#ifdef	COMMENT
	bclose(ofp) ;
#endif

badret:
retearly:
ret1:
	bclose(pip->efp) ;

ret0:
	return ex ;

/* handle some other stuff */
usage:
	bprintf(pip->efp,
	    "%s: USAGE> %s host [-d dialer] [-u username] [-V] [-q]\n",
	    pip->progname,pip->progname) ;

	goto retearly ;

/* handle bad arguments */
badargnum:
	bprintf(pip->efp,"%s: not enough arguments specified\n",
	    pip->progname) ;

	goto badarg ;

badextra:
	bprintf(pip->efp,"%s: extra command line arguments ignored\n",
	    pip->progname) ;

	goto badarg ;

badargval:
	bprintf(pip->efp,"%s: bad argument value specified\n",
	    pip->progname) ;

	goto badarg ;

badhost:
	bprintf(pip->efp,"%s: no host was specified\n",
	    pip->progname) ;

	goto badarg ;

baddial:
	bprintf(pip->efp,"%s: unknown dialer specified >%s<\n",
	    pip->progname,dialspec) ;

	goto badarg ;

badarg:
	ex = EX_USAGE ;
	goto retearly ;

/* handle other things */
badconnect:
	ex = EX_UNAVAILABLE ;
	if (! pip->f.quiet)
		bprintf(pip->efp,"%s: could not connect to host (%d)\n",
	    		pip->progname,rs) ;

	goto ret2 ;

}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */





