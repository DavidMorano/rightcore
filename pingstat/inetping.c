/* inetping */

/* PING an INET machine */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUGPING	0
#define	CF_BROKEN	1		/* PING program broken */
#define	CF_GETLINE	1		/* use 'uc_readlinetimed()' */
#define	CF_PING		0		/* expose a 'ping' entry point */
#define	CF_EXECVP	CF_DEBUGPING


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine performs an INET ICMP 'ping' of the specified host. An
        optional timeout can be specified as well.

	Synopsis:

	int inetping(host,timeout)
	const char	host[] ;
	int		timeout ;

	Arguments:

	host		character string of host name to 'ping'
	timeout		number of secnods to wait for a 'ping' response

	Returns:

	>=0		success
	<0		failure


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<sys/wait.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<getbufsize.h>
#include	<hostent.h>
#include	<inetaddr.h>
#include	<exitcodes.h>
#include	<localmisc.h>


/* local defines */

#ifndef	ADDR_NOT
#define	ADDR_NOT	((uint) (~ 0))
#endif

#ifndef	NOFILE
#define	NOFILE		64		/* modern value (old was 20) */
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#ifndef	DOTBUFLEN
#define	DOTBUFLEN	40		/* INET dotted-decimal length */
#endif

#undef	BUFLEN
#define	BUFLEN		(2 * MAXHOSTNAMELEN)

#undef	PROGNAME_PING
#define	PROGNAME_PING	"INETPING"

#if	CF_DEBUGPING
#define	TO_PING		10
#define	TO_MORETIME	2
#define	TO_READ		2
#else
#define	TO_PING		(5*60)
#define	TO_MORETIME	10
#define	TO_READ		30
#endif

#define	TO_CHILDEXIT	5		/* time to wait for child exit */


/* external subroutines */

extern int	ctdeci(char *,int,int) ;
extern int	getourhe(const char *,char *,struct hostent *,char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strnchr(char *,int,int) ;


/* external variables */


/* local typedefs */

#if	defined(IRIX) && (! defined(TYPEDECF_INADDRT))
#define	TYPEDECF_INADDRT	1
typedef unsigned int	in_addr_t ;
#endif


/* local structures */


/* forward reference */

int		inetping(const char *,int) ;

static int	pingone(const char *,in_addr_t *,int) ;

#if	CF_DEBUGS
static int	makeint(void *) ;
#endif


/* local variables */

static cchar	*pings[] = {
	"/usr/sbin/ping",
	"/usr/etc/ping",
	"/usr/bin/ping",
	"/bin/ping",
	NULL
} ;


/* exported subroutines */


int inetping(cchar *rhost,int timeout)
{
	struct hostent	he, *hep ;
	const int	helen = getbufsize(getbufsize_he) ;
	int		rs ;
	char		*hebuf ;

#if	CF_DEBUGS
	debugprintf("inetping: host=%s to=%d\n",rhost,timeout) ;
#endif

	if (rhost == NULL) return SR_FAULT ;

	if (rhost[0] == '\0') return SR_INVALID ;

/* get an addressable ("E"ntry) hostname */

	if ((rs = uc_malloc((helen+1),&hebuf)) >= 0) {
	    in_addr_t	addr ;
	    int		f_numeric = TRUE ;
	    if ((addr = inet_addr(rhost)) == ADDR_NOT) {
	        f_numeric = FALSE ;
	        hep = &he ;
	        if ((rs = getourhe(rhost,NULL,hep,hebuf,helen)) >= 0) {
	            if (hep->h_addrtype != AF_INET) {
	                rs = SR_HOSTUNREACH ;
	            }
	        }
	    } /* end (non-numeric addresses) */
/* find a "ping" program */
	    if (rs >= 0) {
	        int	i ;
	        for (i = 0 ; pings[i] != NULL ; i += 1) {
	            if (u_access(pings[i],X_OK) >= 0) break ;
	        }
	        if (pings[i] != NULL) {
	            in_addr_t	*iap ;
	            const char	*pingprog = NULL ;
#if	CF_DEBUGS
	            debugprintf("inetping: ping i=%d program=%s\n",
			i,pings[i]) ;
#endif
/* do the dirty deed! */
	            if (! f_numeric) {
	                hostent_cur	hc ;
#if	CF_DEBUGPING
	                pingprog = "ourping" ;
#else
	                pingprog = pings[i] ;
#endif
	                if ((rs = hostent_curbegin(&he,&hc)) >= 0) {
	            	    const uchar	*ap ;
	                    while (hostent_enumaddr(&he,&hc,&ap) >= 0) {
	                        iap = (in_addr_t *) ap ;
	                        rs = pingone(pingprog,iap,timeout) ;
	                        if (rs >= 0) break ;
	                    } /* end while */
	                    hostent_curend(&he,&hc) ;
	                } /* end if (cursor) */
	            } else {
	                iap = (in_addr_t *) &addr ;
	                rs = pingone(pingprog,iap,timeout) ;
	            }
	        } else {
	            rs = SR_NOTSUP ;
	        }
	    } /* end if (ok) */
	    uc_free(hebuf) ;
	} /* end if (m-a-f) */

	return rs ;
}
/* end subroutine (inetping) */


#if	CF_PING

int ping(cchar *rhost,int timeout)
{

	return inetping(rhost,timeout) ;
}

#endif /* CF_PING */


/* local subroutines */


int pingone(cchar *pingprog,in_addr_t *ap,int timeout)
{
	struct ustat	sb ;
	inetaddr	ia ;
	pid_t		pid ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		child_stat ;
	int		i, len ;
	int		to ;
	int		cl ;
	int		bfd[4] ;
	int		pfd[2] ;
	const char	*nullfname = NULLFNAME ;
	const char	*args[4] ;
	const char	*cp ;
	char		buf[BUFLEN + 1] ;
	char		dotaddr[DOTBUFLEN + 1] ;
	char		pa_timeout[DIGBUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("inetping/pingone: ent, timeout=%d\n",timeout) ;
#endif

	inetaddr_start(&ia,ap) ;

	inetaddr_getdotaddr(&ia,dotaddr,DOTBUFLEN) ;

#if	CF_DEBUGS
	debugprintf("inetping/pingone: addr=%s (\\x%08x)\n",
	    dotaddr,makeint(ap)) ;
#endif


/* put the program invocation together */

#if	CF_DEBUGS
	debugprintf("inetping/pingone: putting invocation together \n") ;
#endif

#if	CF_BROKEN
	if (timeout < 0)
	    timeout = TO_PING ;
#endif /* CF_BROKEN */

	args[0] = PROGNAME_PING ;
	args[1] = dotaddr ;

	i = 2 ;
	pa_timeout[0] = '\0' ;
	if (timeout >= 0) {
	    ctdeci(pa_timeout,DIGBUFLEN,timeout) ;
#if	CF_DEBUGS
	    debugprintf("inetping/pingone: timeout=%s\n",pa_timeout) ;
#endif
	    args[i++] = pa_timeout ;
	}

	args[i] = NULL ;

/* open some low FDs if some are not already occupied */

	for (i = 0 ; i < 3 ; i += 1) {

	    bfd[i] = -1 ;
	    if (u_fstat(i,&sb) >= 0) continue ;

	    if (i == 0) {
	        rs = u_open(nullfname,O_RDONLY,0600) ;
	    } else {
	        rs = u_open(nullfname,O_WRONLY,0600) ;
	    }

	    if (rs >= 0) {
	        bfd[i] = i ;
	    }

	} /* end for */

/* this is now guaranteed to come up at or above 3! */

	rs = u_pipe(pfd) ;
	if (rs < 0)
	    goto done ;

#if	CF_DEBUGS
	debugprintf("inetping/pingone: got our pipe\n") ;
#endif

	if ((rs = uc_fork()) == 0) {

	    u_close(pfd[0]) ;

	    for (i = 0 ; i < 3 ; i += 1) {
	        u_close(i) ;
	    }

	    u_open(nullfname,O_RDONLY,0666) ;

	    u_dup(pfd[1]) ;

	    u_open(nullfname,O_WRONLY,0666) ;

	    u_close(pfd[1]) ;

	    for (i = 3 ; i < NOFILE ; i += 1) {
	        u_close(i) ;
	    }

#if	CF_EXECVP
	    rs = u_execvp(pingprog,args) ;
#else
	    rs = u_execv(pingprog,args) ;
#endif

	    uc_exit(EX_NOEXEC) ;
	} else if (rs > 0) {
	    pid = rs ;
	} else
	    goto badfork ;

#if	CF_DEBUGS
	debugprintf("inetping/pingone: forked pid=%d\n",rs) ;
#endif

	u_close(pfd[1]) ;

#if	CF_DEBUGS
	debugprintf("inetping/pingone: opened command OK\n") ;
#endif

/* read the response from the PING program */

	if (timeout < 0)
	    timeout = TO_PING ;

	timeout += TO_MORETIME ;

#if	CF_DEBUGS
	debugprintf("inetping/pingone: BUFLEN=%d\n",BUFLEN) ;
	debugprintf("inetping/pingone: entering loop, timeout=%d\n",
		timeout) ;
#endif

	i = 0 ;
	to = 0 ;
	while ((i < BUFLEN) && (to < timeout)) {
	    int		bl ;
	    int		f_found ;
	    char	*bp ;

#if	CF_DEBUGS
	    debugprintf("inetping/pingone: about to read\n") ;
#endif

	    bp = buf + i ;
	    bl = BUFLEN - i ;

#if	CF_GETLINE
	    rs = uc_readlinetimed(pfd[0],bp,bl,TO_READ) ;
#else
	    rs = uc_reade(pfd[0],bp,bl,TO_READ,0) ;
#endif

#if	CF_DEBUGS
	    debugprintf("inetping/pingone: read rs=%d\n",rs) ;
#endif

	    len = rs ;
	    if (rs < 0) break ;

	    if (len > 0) {
#if	CF_DEBUGS
	        debugprintf("inetping/pingone: buf=>%t<\n",bp,len) ;
#endif
	        f_found = (strnchr(bp,len,'\n') != NULL) ;
	        i += len ;
	        if (f_found) break ;
	    } else {
	        to += TO_READ ;
	    }

#if	CF_DEBUGS
	    debugprintf("inetping/pingone: bottom loop, to=%d\n",to) ;
#endif

	} /* end while (reading response from PING program) */

	len = i ;

#if	CF_DEBUGS
	debugprintf("inetping/pingone: out loop read rs=%d\n",rs) ;
#endif

/* we have enough output from the program, let it die if it wants to! */

	u_close(pfd[0]) ;

/* find the part of the output string that we like */

	if ((rs >= 0) && (len >= 0)) {

#if	CF_DEBUGS
	    debugprintf("inetping/pingone: read len=%d\n",len) ;
#endif

	    cl = nextfield(buf,len,&cp) ;

#if	CF_DEBUGS
	    debugprintf("inetping/pingone: read host=%t\n",cp,cl) ;
#endif

	    rs = SR_HOSTDOWN ;
	    if (sfsub((buf + cl),(len - cl),"is alive",&cp) >= 0)
	        rs = SR_OK ;

#if	CF_DEBUGS
	    debugprintf("inetping/pingone: past getting a response rs=%d\n",
	        rs) ;
#endif

	} /* end if */

#if	CF_DEBUGS
	debugprintf("inetping/pingone: to=%d timeout=%d\n",to,timeout) ;
#endif

	if (to >= timeout) {
#if	CF_DEBUGS
	    debugprintf("inetping/pingone: killing pid=%d sig=%u\n",
	        pid,SIGTERM) ;
#endif
	    u_kill(pid,SIGTERM) ;
	    sleep(1) ;
	}

#if	CF_DEBUGS
	debugprintf("inetping/pingone: TO_CHILDEXIT=%d\n",
	    TO_CHILDEXIT) ;
#endif

	to = 0 ;
	while (to < TO_CHILDEXIT) {

#if	CF_DEBUGS
	    debugprintf("inetping/pingone: wait polling, to=%d\n",
	        to) ;
#endif

	    rs1 = u_waitpid(pid,&child_stat,WNOHANG) ;

#if	CF_DEBUGS
	    debugprintf("inetping/pingone: waitpid rs=%d\n",rs1) ;
#endif

	    if (rs1 != 0)
	        break ;

	    to += 1 ;
	    if (to >= TO_CHILDEXIT) {
#if	CF_DEBUGS
	        debugprintf("inetping/pingone: wait killing pid=%d sig=%u\n",
	            pid,SIGTERM) ;
#endif
	        to = MAX((to - 3),0) ;
	        u_kill(pid,SIGTERM) ;
	    }

	    sleep(1) ;

	} /* end while */

#if	CF_DEBUGS
	debugprintf("inetping/pingone: out of wait loop, to=%d\n",
	    to) ;
#endif

/* we're out of here */
done:
	for (i = 0 ; i < 3 ; i += 1) {
	    if (bfd[i] >= 0) u_close(i) ;
	} /* end for */

	inetaddr_finish(&ia) ;

#if	CF_DEBUGS
	debugprintf("inetping/pingone: ret rs=%d\n",
	    rs) ;
#endif

	return rs ;

/* bad stuff */
badfork:
	u_close(pfd[0]) ;

	u_close(pfd[1]) ;

	goto done ;
}
/* end subroutine (pingone) */


#if	CF_DEBUGS
static int makeint(void *addr)
{
	int		hi = 0 ;
	uchar		*us = (uchar *) addr ;
	hi |= ((us[3] & 0xFF) << 24) ;
	hi |= ((us[2] & 0xFF) << 16) ;
	hi |= ((us[1] & 0xFF) << 8) ;
	hi |= (us[0] & 0xFF)  ;
	return hi ;
}
#endif /* CF_DEBUGS */


