/* inetping */

/* PING an INET machine */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUGPING	0		/* debug PING */
#define	CF_DEBUGINT	0		/* compile |makeint()| */
#define	CF_BROKEN	1		/* PING program broken */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine performs an INET ICMP 'ping' of the specified host. An
        optional timeout can be specified as well.

	Synopsis:

	int inetping(host,to)
	const char	host[] ;
	int		to ;

	Arguments:

	host		character string of host name to 'ping'
	to		number of secnods to wait for a 'ping' response

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
#include	<spawnproc.h>
#include	<exitcodes.h>
#include	<localmisc.h>


/* local defines */

#ifndef	ADDR_NOT
#define	ADDR_NOT	((uint) (~ 0))
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#ifndef	DOTBUFLEN
#define	DOTBUFLEN	40		/* INET dotted-decimal length */
#endif

#define	TOBUFLEN	DIGBUFLEN

#define	RBUFLEN		MAXNAMELEN

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
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;

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
static int	pingoneresp(int,int) ;
static int	pingoneparse(cchar *,int) ;

#if	CF_DEBUGS & CF_DEBUGINT
static int	makeint(void *) ;
#endif


/* local variables */

static cchar	*pings[] = {
	"/usr/sbin/ping",
	"/usr/bin/ping",
	"/sbin/ping",
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
	debugprintf("inetping: ent host=%s to=%d\n",rhost,timeout) ;
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
	            cchar	*pingprog = pings[i] ;
#if	CF_DEBUGS
	            debugprintf("inetping: ping i=%d prog=%s\n",
			i,pingprog) ;
#endif
/* do the dirty deed! */
	            if (f_numeric) {
	                iap = (in_addr_t *) &addr ;
	                rs = pingone(pingprog,iap,timeout) ;
	            } else {
	                hostent_cur	hc ;
	                if ((rs = hostent_curbegin(&he,&hc)) >= 0) {
	            	    cuchar	*ap ;
	                    while (hostent_enumaddr(&he,&hc,&ap) >= 0) {
	                        iap = (in_addr_t *) ap ;
	                        rs = pingone(pingprog,iap,timeout) ;
	                        if (rs >= 0) break ;
	                    } /* end while */
	                    hostent_curend(&he,&hc) ;
	                } /* end if (cursor) */
	            }
	        } else {
	            rs = SR_NOTSUP ;
	        }
	    } /* end if (ok) */
	    uc_free(hebuf) ;
	} /* end if (m-a-f) */

#if	CF_DEBUGS
	debugprintf("inetping: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (inetping) */


/* local subroutines */


int pingone(cchar *pingprog,in_addr_t *ap,int to)
{
	inetaddr	ia ;
	int		rs ;
	int		rs1 ;
	int		rv = 0 ;

#if	CF_DEBUGS
	{
	    const int	af = AF_INET4 ;
	    cchar	*as = (cchar *) ap ;
	    char	abuf[INETX_ADDRSTRLEN+1] ;
	    debugprintf("inetping/pingone: ent to=%d\n",to) ;
	    sninetaddr(abuf,INETX_ADDRSTRLEN,af,as) ;
	    debugprintf("inetping/pingone: a=%s\n",abuf) ;
	    debugprintf("inetping/pingone: pingprog=%s\n",pingprog) ;
	}
#endif /* CF_DEBUGS */

#if	CF_BROKEN
	if (to < 0) to = TO_PING ;
#endif /* CF_BROKEN */

	if ((rs = inetaddr_start(&ia,ap)) >= 0) {
	    const int	dotlen = DOTBUFLEN ;
	    char	dotbuf[DOTBUFLEN + 1] ;
	    if ((rs = inetaddr_getdotaddr(&ia,dotbuf,dotlen)) >= 0) {
		SPAWNPROC	ps ;
		const int	tolen = TOBUFLEN ;
		int		ai = 0 ;
		cchar		*args[4] ;
		char		tobuf[TOBUFLEN+1] ;

		args[ai++] = PROGNAME_PING ;
		args[ai++] = dotbuf ;
	        if (to >= 0) {
	    	    ctdeci(tobuf,tolen,to) ;
	    	    args[ai++] = tobuf ;
	        }
		args[ai] = NULL ;

#if	CF_DEBUGS
	debugprintf("inetping/pingone: mid2 rs=%d\n",rs) ;
#endif

		memset(&ps,0,sizeof(SPAWNPROC)) ;
		ps.disp[0] = SPAWNPROC_DNULL ;
		ps.disp[1] = SPAWNPROC_DCREATE ;
		ps.disp[2] = SPAWNPROC_DNULL ;
		if ((rs = spawnproc(&ps,pingprog,args,NULL)) >= 0) {
		    const pid_t	pid = rs ;
		    const int	fd = ps.fd[1] ;
		    int		cs = 0 ;
#if	CF_DEBUGS
	debugprintf("inetping/pingone: mid3 rs=%d\n",rs) ;
#endif
		    if (to < 0) to = TO_PING ;
		    to += TO_MORETIME ;
		    if ((rs = pingoneresp(fd,to)) >= 0) {
	        	rs = u_waitpid(pid,&cs,WNOHANG) ;
	    	    } else {
			rv = rs ;
	        	u_kill(pid,SIGTERM) ;
	        	rs = u_waitpid(pid,&cs,WNOHANG) ;
		    }
#if	CF_DEBUGS
	debugprintf("inetping/pingone: mid5 rs=%d\n",rs) ;
#endif
		    u_close(fd) ;
		} /* end if (spawnproc) */
#if	CF_DEBUGS
	debugprintf("inetping/pingone: spawnproc-out rs=%d\n",rs) ;
#endif
	    } /* end if (inetaddr_getdotaddr) */
	    rs1 = inetaddr_finish(&ia) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (inetaddr) */
#if	CF_DEBUGS
	debugprintf("inetping/pingone: ret rs=%d rv=%d\n",rs,rv) ;
#endif
	return (rs >= 0) ? rv : rs ;
}
/* end subroutine (pingone) */


static int pingoneresp(int fd,int to)
{
	const int	rlen = RBUFLEN ;
	int		rs = SR_OK ;
	int		tl = 0 ;
	int		t = 0 ;
	int		len = 0 ;
	char		rbuf[RBUFLEN+1] ;
	while ((rs >= 0) && (tl < rlen) && (t < to)) {
	    int		f_found ;
	    int		rl = (rlen-tl) ;
	    char	*rp = (rbuf+tl) ;
	    if ((rs = uc_readlinetimed(fd,rp,rl,TO_READ)) > 0) {
	        len = rs ;
	        if (len > 0) {
#if	CF_DEBUGS
	            debugprintf("inetping/pingone: rbuf=>%t<\n",rp,len) ;
#endif
	            f_found = (strnchr(rp,len,'\n') != NULL) ;
	            tl += len ;
	            if (f_found) break ;
	        } else {
	            t += TO_READ ;
	        }
#if	CF_DEBUGS
	        debugprintf("inetping/pingone: bottom loop to=%d\n",to) ;
#endif
	    } /* end if (uc_readlinetimed) */
	    if (f_found || (len == 0)) break ;
	} /* end while (reading response from PING program) */
	if (rs >= 0) {
	    rs = pingoneparse(rbuf,tl) ;
	}
#if	CF_DEBUGS
	debugprintf("inetping/pingoneresp: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (pingoneresp) */


static int pingoneparse(cchar *rbuf,int rlen)
{
	int		rs = SR_OK ;
	int		cl ;
	cchar		*cp ;
	if ((cl = nextfield(rbuf,rlen,&cp)) > 0) {
	    if (sfsub((rbuf+cl),(rlen-cl),"is alive",&cp) < 0) {
	        rs = SR_HOSTDOWN ;
	    }
	} else {
	    rs = SR_HOSTDOWN ;
	}
	return rs ;
}
/* end subroutine (pingoneparse) */


#if	CF_DEBUGS & CF_DEBUGINT
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


