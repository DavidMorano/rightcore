/* main */

/* test listening on a USS portal */


#define	CF_DEBUGS	1		/* compile-time debugging */


/* revision history:

	= 1992-03-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1992 David A­D­ Morano.  All rights reserved. */

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
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	NPARG		2	/* number of positional arguments */

#define	TO_READ		30
#undef	BUFLEN
#define	BUFLEN		((10 * 1024) + MAXHOSTNAMELEN)


/* external variables */

extern int	cfdeci(const char *,int,int *) ;
extern int	matstr(char * const *,const char *,int) ;
extern int	dialusd(const char *,int,int) ;
extern int	dialudp(char *,char *,int,int,int) ;
extern int	dialtcp(const char *,const char *,int,int,int) ;
extern int	dialtcpnls(char *,char *,int,char *,int,int) ;
extern int	dialtcpmux(char *,char *,int,char *,char **,int,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strbasename(char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */

static cchar	*dialers[] = {
	"tcp",
	"tcpmux",
	"tcpnls",
	"udp",
	"uss",
	"usd",
	NULL
} ;

#define	DIALER_TCP	0
#define	DIALER_TCPMUX	1
#define	DIALER_TCPNLS	2
#define	DIALER_UDP	3
#define	DIALER_USS	4
#define	DIALER_USD	5


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	struct sockaddr_in	server ;
	struct sockaddr_in	from ;
	struct servent		*sp ;
	struct protoent		*pp ;
	struct hostent		*hp ;

	bfile	errfile, *efp = &errfile ;
	bfile	outfile, *ofp = &outfile ;

	uid_t	uid ;

	ulong	lw1, lw2 ;

	uint	port ;

	int	argr, argl, aol, pan ;
	int	rs ;
	int	i, len,;
	int	dialer, s ;
	int	timeout = -1 ;
	int	err_fd = -1 ;
	int	ex = EX_INFO ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_debug = FALSE ;
	int	f_verbose = FALSE ;
	int	f_quiet = FALSE ;
	int	f_ignore = FALSE ;
	int	f_log = FALSE ;

	cchar	*progname, *argp, *aop ;
	cchar	*logfname = NULL ;
	cchar	*dialspec = NULL ;
	cchar	*hostname = NULL ;
	cchar	*portspec = NULL ;
	cchar	*svcspec = NULL ;
	cchar	*cp ;
	char	buf[BUFLEN + 1] ;
	char	hostnamebuf[MAXHOSTNAMELEN + 1] ;
	char	dialspecbuf[MAXHOSTNAMELEN + 1] ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif

	progname = strbasename(argv[0]) ;

	if ((rs = bopen(efp,BFILE_STDERR,"dwca",0666)) >= 0)
	    bcontrol(efp,BC_LINEBUF,0) ;

	pan = 0 ;			/* number of positional so far */
	i = 1 ;
	argr = argc - 1 ;

	while (argr > 0) {

	    argp = argv[i++] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    if ((argl > 0) && (*argp == '-')) {

	        if (argl > 1) {

	            aop = argp ;
	            aol = argl ;
	            while (--aol) {

	                akp += 1 ;
	                switch (*aop) {

	                case 'D':
	                    f_debug = TRUE ;
	                    break ;

	                case 'V':
	                    f_version = TRUE ;
	                    bprintf(efp,"%s: version %s\n",
	                        progname,VERSION) ;

	                    break ;

	                case 'd':
	                    if (argr <= 0)
	                        goto badargnum ;

	                    argp = argv[i++] ;
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

	                    argp = argv[i++] ;
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

	                    argp = argv[i++] ;
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

	                    argp = argv[i++] ;
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

	                    argp = argv[i++] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;

	                    if (argl) {

	                        rs = cfdeci(argp,argl,&timeout) ;

	                        if (rs < 0)
	                            goto badargval ;

	                    }

	                    break ;

	                case 'v':
	                    f_verbose = TRUE ;
	                    break ;

	                default:
	                    f_usage = TRUE ;
	                    bprintf(efp,"%s: unknown option - %c\n",
	                        progname,*aop) ;

	                case '?':
	                    f_usage = TRUE ;

	                } /* end switch */

	            } /* end while */

	        } else {

	            pan += 1 ;	/* increment position count */

	        } /* end if */

	    } else {

	        if (pan < NPARG) {

	            switch (pan) {

	            case 0:
	                if (argl > 0)
	                    hostname = argp ;

	                break ;

	            case 1:
	                if (argl > 0)
	                    portspec = argp ;

	                break ;

	            } /* end switch */

	            pan += 1 ;

	        } else
	            goto badextra ;

	    } /* end if */

	} /* end while (processing command invocation arguments) */


#if	CF_DEBUGS
	debugprintf("main: past getting arguments\n") ;
#endif

	if (f_debug) {

	    bprintf(efp,"%s: about to check arguments\n",
	        progname) ;

	    bflush(efp) ;

	}

	if (f_usage)
	    goto usage ;

	if (f_version)
	    goto retearly ;


/* check arguments */

#if	CF_DEBUGS
	debugprintf("main: checking arguments\n") ;
#endif

	if (pan < 1) {

	    ex = EX_USAGE ;
	    bprintf(efp,"%s: no host specified\n",
	        progname) ;

	    goto retearly ;
	}

	if (f_log && ((logfname == NULL) || (logfname[0] == '\0'))) {

	    f_log = FALSE ;

	}


#if	CF_DEBUGS
	debugprintf("main: checking hostname and stuff\n") ;
#endif

	if ((hostname == NULL) || (hostname[0] == '\0'))
	    goto badhost ;

	if ((cp = strchr(hostname,':')) != NULL) {

	    i = cp - hostname ;
	    strwcpy(hostnamebuf,hostname,MAXHOSTNAMELEN) ;

	    hostnamebuf[i] = '\0' ;
	    hostname = hostnamebuf ;
	    if ((portspec == NULL) || (portspec[0] == '\0'))
	        portspec = hostnamebuf + i + 1 ;

	} /* end if */

#if	CF_DEBUGS
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

#if	CF_DEBUGS
	debugprintf("main: dialspec=%s portspec=%s\n",dialspec,portspec) ;
#endif

	} /* end if */

#if	CF_DEBUGS
	debugprintf("main: checking portspec\n") ;
#endif

	if ((portspec == NULL) || (portspec[0] == '\0'))
	    portspec = PORTSPEC_ECHO ;

	if ((svcspec == NULL) || (svcspec[0] == '\0'))
	    svcspec = SVCSPEC_ECHO ;

/* find the dialer */

#if	CF_DEBUGS
	debugprintf("main: find the dialer, dislspec=%s\n",dialspec) ;
#endif

	if (dialspec != NULL) {

	    dialer = matstr(dialers,dialspec,-1) ;

	    if (dialer < 0)
	        goto baddial ;

	} else
	    dialer = DIALER_UDP ;

	if (timeout <= 0)
	    timeout = DIALTIME ;


/* miscellaneous */

	if (f_debug) {

	    for (i = 0 ; dialers[i] != NULL ; i += 1)
	    bprintf(efp,"%s: dialer> %s(%d)\n",
		progname,dialers[i],i) ;

	    bprintf(efp,"%s: got valid arguments, dialer(%d)=%s\n",
	        progname,dialer,dialers[dialer]) ;

	}

/* look up some miscellaneous stuff in various databases */

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

	case DIALER_USS:
	    if ((svcspec == NULL) || (svcspec[0] == '\0'))
	        svcspec = portspec ;

	    rs = dialuss(hostname,svcspec,timeout,0) ;
	    s = rs ;
	    break ;

	case DIALER_USD:
	    if ((svcspec == NULL) || (svcspec[0] == '\0'))
	        svcspec = portspec ;

	    rs = dialusd(svcspec,timeout,0) ;
	    s = rs ;
	    break ;

	} /* end switch */

	if (rs < 0)
	    goto badconnect ;

	if (f_debug) {

	    bprintf(efp,"%s: connected to the host FD=%d\n",
	        progname,s) ;

	    bflush(efp) ;

	}


	rs = SR_OK ;
	while (rs >= 0) {

#if	CF_DEBUGS
	bflush(efp) ;
#endif

	    rs = u_read(FD_STDIN,buf,BUFLEN) ;

	    if (rs < 0)
	        break ;

	    if (f_debug)
	        bprintf(efp,"%s: STDIN read rs=%d\n",
	            progname,rs) ;

	    len = rs ;
	    if (len == 0)
	        break ;

	    rs = u_write(s,buf,len) ;

	    if (f_debug)
	        bprintf(efp,"%s: network write rs=%d\n",
	            progname,rs) ;

	    if (rs < 0) {

	        bprintf(efp,"%s: bad network write rs=%d\n",
	            progname,rs) ;

	        goto done ;
	    }

	    rs = uc_reade(s,buf,BUFLEN,TO_READ,0) ;

	    if (f_debug)
	        bprintf(efp,"%s: network read len=%d\n",
	            progname,len) ;

	    if (rs < 0) {

	        bprintf(efp,"%s: bad network read rs=%d\n",
	            progname,rs) ;

	        goto done ;
	    }

	    len = rs ;
	if (len > 0)
	    rs = u_write(FD_STDOUT,buf,len) ;

	    if (f_debug)
	        bprintf(efp,"%s: STDOUT write rs=%d\n",
	            progname,rs) ;

	} /* end while */


	ex = EX_OK ;
	if (rs < 0) {

	    ex = EX_DATAERR ;
	    bprintf(efp,"%s: exiting bad rs=%d\n",
	        progname,rs) ;

	}


/* and exit */
done:
	u_shutdown(s,2) ;

	u_close(s) ;

#ifdef	COMMENT
	bclose(ofp) ;
#endif

badret:
retearly:
	bclose(efp) ;

	return ex ;

usage:
	bprintf(efp,
	    "%s: USAGE> %s [-V] [-s] [-q] [-i] [-v] timehost(s)",
	    progname,progname) ;

	bprintf(efp,
	    " [-l logfile] [-D]\n") ;

	goto retearly ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",
	    progname) ;

	goto badarg ;

badextra:
	bprintf(efp,"%s: extra command line arguments ignored\n",
	    progname) ;

	goto badarg ;

badargval:
	bprintf(efp,"%s: bad argument value specified\n",
	    progname) ;

	goto badarg ;

badhost:
	bprintf(efp,"%s: no host was specified\n",
	    progname) ;

	goto badarg ;

baddial:
	bprintf(efp,"%s: unknown dialer specified\n",
	    progname) ;

	goto badarg ;

badarg:
	ex = EX_USAGE ;
	goto retearly ;

badconnect:
	ex = EX_UNAVAILABLE ;
	bprintf(efp,"%s: could not connect to host (%d)\n",
	    progname,rs) ;

	goto retearly ;

}
/* end subroutine (main) */


