/* dialtcp */

/* subroutine to dial out to a machine using TCP */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_PROTO	0		/* need dynamic protocol number */
#define	CF_TRYINET	0		/* try_inet() ? */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will dial out to an INET host using the TCP protocol
	and the optional TCP port specified.

	Synopsis:

	int dialtcp(hostname,portspec,af,to,options)
	const char	hostname[] ;
	const char	portspec[] ;
	int		af ;
	int		to ;
	int		options ;

	Arguments:

	hostname	host to dial to
	portspec	port specification to use
	af		address family
	to		to ('>0' means use one, '-1' means don't)
	options		any dial options

	Returns:

	<0		error in dialing
	>=0		file descriptor


	What is up with the 'to' argument?

	<0              use the system default to (xx minutes --
			whatever)
	==0             asynchronously span the connect and do not wait
			for it
	>0		use the to as it is


	Notes:

	Note that Sun-Solaris has a number (at least two) problems with their
	INET-TCP protocol stack code.  Their version of an asynchronous connect
	does not always work properly unless one attempts the connect a second
	time AFTER the socket is already writable (a 'poll(2)' return for
	output).  This problem is handled inside the call to
	'uc_connecte(3uc)'.  The other major problem with Sun-Solaris is that a
	connect attempt will fail to even be made for some reason and no bad
	error is returned from the 'u_connect(3xnet)' other than INPROGRESS
	(which is due to the connection being attempted asynchronously).  This
	latter problem is harder to handle.  We handle it by simply trying the
	whole shebang process again when we get an ultimate failure of
	TIMEDOUT.  However, this generally also fails on Sun-Solaris since the
	internal state of the Solaris INET-TCP stack is somehow hosed.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<hostinfo.h>
#include	<hostaddr.h>
#include	<hostent.h>
#include	<sockaddress.h>
#include	<localmisc.h>

#if	CF_DEBUGS
#include	<inetaddr.h>
#endif

#include	"dialopts.h"


/* local defines */

#ifndef	ADDRINFO
#define	ADDRINFO	struct addrinfo
#endif

#define	PROTONAME	"tcp"

#ifndef	ADDRBUFLEN
#define	ADDRBUFLEN	64
#endif

#define	HEXBUFLEN	100

#define	RETRIES		1		/* Sun-Solaris problem */

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	inetpton(uchar *,int,int,const char *,int) ;
extern int	getprotofamily(int) ;
extern int	getportnum(cchar *,cchar *) ;
extern int	getproto_name(cchar *,int) ;
extern int	opensockaddr(int,int,int,struct sockaddr *,int) ;
extern int	openaddrinfo(ADDRINFO *,int) ;
extern int	hasalldig(cchar *,int) ;
extern int	isFailConn(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif


/* external variables */


/* local typedefs */


/* local structures */

struct subinfo_flags {
	uint		address:1 ;
} ;

struct subinfo {
	const char	*hostname ;
	const char	*portspec ;
	const char	*protoname ;
	SUBINFO_FL	f ;
	int		count ;
	int		type ;
	int		proto ;
	int		af ;
	int		port ;
	int		to ;
	uchar		addrbuf[ADDRBUFLEN + 1] ;
} ;


/* forward references */

static int	subinfo_start(SUBINFO *,const char *,const char *,int) ;
static int	subinfo_proto(SUBINFO *) ;
static int	subinfo_svc(SUBINFO *) ;
static int	subinfo_finish(SUBINFO *) ;

static int	subinfo_addr(SUBINFO *,int) ;
static int	subinfo_try(SUBINFO *) ;
static int	subinfo_tryone(SUBINFO *) ;

static int	try_inet4(SUBINFO *) ;
static int	try_inet6(SUBINFO *) ;
static int	try_addr(SUBINFO *) ;

#if	CF_TRYINET
static int	try_inet(SUBINFO *) ;
#endif

static int	subinfo_makeconn(SUBINFO *,int,struct sockaddr *) ;

#if	CF_DEBUGS
extern int	mkhexstr(char *,int,const void *,int) ;
static int	makeint(const void *) ;
#endif


/* local variables */


/* exported subroutines */


/* ARGSUSED */
int dialtcp(cchar *hostname,cchar *portspec,int af,int to,int opts)
{
	SUBINFO		si, *sip = &si ;
	int		rs ;
	int		rs1 ;
	int		fd = -1 ;

#if	CF_DEBUGS
	debugprintf("dialtcp: hostname=%s portspec=%s af=%d\n",
	    hostname,portspec,af) ;
	debugprintf("dialtcp: to=%d\n",to) ;
#endif

	if (hostname == NULL) return SR_FAULT ;
	if (portspec == NULL) return SR_FAULT ;

	if (hostname[0] == '\0') rs = SR_INVALID ;
	if (portspec[0] == '\0') rs = SR_INVALID ;

	if (to == 0) to = 1 ;

	if ((rs = subinfo_start(sip,hostname,portspec,to)) >= 0) {
	    if ((rs = subinfo_proto(sip)) >= 0) {
	        if ((rs = subinfo_svc(sip)) >= 0) {
	            if ((rs = subinfo_addr(sip,af)) >= 0) {
	                if ((rs = subinfo_try(sip)) >= 0) {
	                    fd = rs ;
	                    rs = dialopts(fd,opts) ;
#if	CF_DEBUGS
			    debugprintf("dialtcp: dialopts() rs=%d\n",rs) ;
#endif
	                }
	                if ((rs < 0) && (sip->count == 0)) {
	                    rs = SR_HOSTUNREACH ;
			}
	            } /* end if (subinfo_addr) */
	        } /* end if (subinfo_svc) */
	    } /* end if (subinfo_proto) */
	    rs1 = subinfo_finish(sip) ;
	    if (rs >= 0) rs = rs1 ;
	    if ((rs < 0) && (fd >= 0)) u_close(fd) ;
	} /* end if (subinfo) */

#if	CF_DEBUGS
	debugprintf("dialtcp: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (dialtcp) */


/* local subroutines */


static int subinfo_start(SUBINFO *sip,cchar *hostname,cchar *portspec,int to)
{

	memset(sip,0,sizeof(SUBINFO)) ;
	sip->hostname = hostname ;
	sip->portspec = portspec ;
	sip->protoname = PROTONAME ;
	sip->type = SOCK_STREAM ;
	sip->to = to ;

	return SR_OK ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{
	if (sip == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_proto(SUBINFO *sip)
{
	int		rs = SR_OK ;

#if	CF_PROTO
	{
	    int		rs ;
	    cchar	*pn = sip->protoname ;
	    if ((rs = getproto_name(pn,-1)) >= 0) {
	        sip->proto = rs ;
	    }
	}
#else
	sip->proto = IPPROTO_TCP ;
#endif /* CF_PROTO */

	return rs ;
}
/* end subroutine (subinfo_proto) */


static int subinfo_svc(SUBINFO *sip)
{
	int		rs ;
	cchar		*pn = sip->protoname ;
	cchar		*ps = sip->portspec ;

	if ((rs = getportnum(pn,ps)) >= 0) {
	    sip->port = rs ;
	}

#if	CF_DEBUGS
	debugprintf("dialtcp/_svc: ret rs=%d port=%u\n",rs,sip->port) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_svc) */


static int subinfo_addr(SUBINFO *sip,int af)
{
	const int	addrlen = ADDRBUFLEN ;
	int		rs ;
	cchar		*hn = sip->hostname ;

#if	CF_DEBUGS
	debugprintf("dialtcp/_addr: ent af=%d\n",af) ;
	debugprintf("dialtcp/_addr: ent host=%s\n",hn) ;
#endif

	if ((rs = inetpton(sip->addrbuf,addrlen,af,hn,-1)) >= 0) {
	    af = rs ;
	    sip->f.address = TRUE ;
	} else if (rs == SR_INVALID) {
#if	CF_DEBUGS
	    debugprintf("dialtcp/_addr: inetpton() rs=%d\n",rs) ;
#endif
	    rs = SR_OK ;
	}

#if	CF_DEBUGS
	debugprintf("dialtcp/_addr: ret rs=%d af=%d f_addr=%u\n",
	    rs,af,sip->f.address) ;
#endif

	sip->af = af ;
	return (rs >= 0) ? af : rs ;
}
/* end subroutine (subinfo_addr) */


static int subinfo_try(SUBINFO *sip)
{
	int		rs ;

	if ((rs = subinfo_tryone(sip)) == SR_TIMEDOUT) {
	    int		c = RETRIES ;
	    while ((c-- > 0) && (rs == SR_TIMEDOUT)) {
	        rs = subinfo_tryone(sip) ;
	    } /* end while */
	}

#if	CF_DEBUGS
	debugprintf("dialtcp/_try: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_try) */


static int subinfo_tryone(SUBINFO *sip)
{
	int		rs = SR_HOSTUNREACH ;
	int		rs1 ;
	int		af = sip->af ;
	int		fd = 0 ;

#if	CF_DEBUGS
	debugprintf("dialtcp/_tryone: ent\n") ;
#endif

	if (sip->f.address) {
	    rs = try_addr(sip) ;
	    fd = rs ;
	} else {

/* first try IPv4 addresses */

	    if ((isFailConn(rs) || (sip->count == 0)) && 
	        ((af == AF_UNSPEC) || (af == AF_INET4))) {

	        rs1 = try_inet4(sip) ;
	        fd = rs1 ;
	        if (rs1 != SR_NOTFOUND) rs = rs1 ;

#if	CF_DEBUGS
	        debugprintf("dialtcp/_tryone: try_inet4() rs=%d\n",rs) ;
#endif

	    } /* end if (IPv4) */

/* now try IPv6 addresses */

	    if ((isFailConn(rs) || (sip->count == 0)) && 
	        ((af == AF_UNSPEC) || (af == AF_INET6))) {

	        rs1 = try_inet6(sip) ;
	        fd = rs1 ;
	        if (rs1 != SR_NOTFOUND) rs = rs1 ;

#if	CF_DEBUGS
	        debugprintf("dialtcp/_tryone: try_inet6() rs1=%d\n",rs1) ;
#endif

	    } /* end if (IPv6) */

#if	CF_TRYINET
	    if (isFailConn(rs) || (sip->count == 0)) {

	        rs1 = try_inet(sip) ;
	        fd = rs1 ;
	        if (rs1 != SR_NOTFOUND) rs = rs1 ;

#if	CF_DEBUGS
	        debugprintf("dialtcp/_tryone: try_inet() rs=%d \n",rs) ;
#endif

	    } /* end if (any address family) */
#endif /* CF_TRYINET */

#if	CF_DEBUGS
	    debugprintf("dialtcp/_tryone: fin rs=%d\n",rs) ;
#endif

	} /* end if */

#if	CF_DEBUGS
	debugprintf("dialtcp/_tryone: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (subinfo_tryone) */


static int try_inet4(SUBINFO *sip)
{
	HOSTINFO	hi ;
	HOSTINFO_CUR	hc ;
	const int	af = AF_INET4 ;
	int		rs ;
	int		rs1 ;
	int		fd = -1 ;

#if	CF_DEBUGS
	debugprintf("dialtcp/try_inet4: ent\n") ;
#endif

	if ((rs = hostinfo_start(&hi,af,sip->hostname)) >= 0) {
	    SOCKADDRESS	server ;
	    const int	pf = PF_INET4 ;
	    uchar	da[2] ; /* dummy address */

	    da[0] = '\0' ;
	    if ((rs = sockaddress_start(&server,af,da,sip->port,0)) >= 0) {

	        if ((rs = hostinfo_curbegin(&hi,&hc)) >= 0) {
		    struct sockaddr	*sap ;
	            int			c = 0 ;
		    int			al ;
		    const uchar		*ap ;

	            while (rs >= 0) {
	                al = hostinfo_enumaddr(&hi,&hc,&ap) ;
	                if (al == SR_NOTFOUND) break ;
	                rs = al ;
	                if (rs < 0) break ;

#if	CF_DEBUGS
	                {
	                    inetaddr	ia ;
	                    char	abuf[ADDRBUFLEN + 1] ;
	                    inetaddr_start(&ia,ap) ;
	                    inetaddr_getdotaddr(&ia,abuf,ADDRBUFLEN) ;
	                    debugprintf("dialtcp/try_inet4: addr=%s "
	                        "(\\x%08x)\n",
	                        abuf,makeint(ap)) ;
	                    inetaddr_finish(&ia) ;
	                }
#endif /* CF_DEBUGS */

	                sockaddress_putaddr(&server,ap) ;

	                c += 1 ;
	                sip->count += 1 ;
	                sap = (struct sockaddr *) &server ;
	                rs = subinfo_makeconn(sip,pf,sap) ;
	                fd = rs ;

	                if (fd >= 0) break ;
	                if ((rs < 0) && (rs != SR_PFNOSUPPORT)) break ;
	            } /* end while */

	            rs1 = hostinfo_curend(&hi,&hc) ;
		    if (rs >= 0) rs = rs1 ;
	            if ((rs >= 0) && (c == 0)) rs = SR_HOSTUNREACH ;
	        } /* end if (cursor) */

	        sockaddress_finish(&server) ;
	    } /* end if (sockaddress) */

	    rs1 = hostinfo_finish(&hi) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (hostinfo) */
	if ((rs < 0) && (fd >= 0)) u_close(fd) ;

#if	CF_DEBUGS
	debugprintf("dialtcp/try_inet4: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (try_inet4) */


static int try_inet6(SUBINFO *sip)
{
	struct hostent	*hep ;
	const int	af = AF_INET6 ;
	int		rs ;
	int		rs1 ;
	int		flags ;
	int		fd = -1 ;

#if	CF_DEBUGS
	debugprintf("dialtcp/try_inet6: ent\n") ;
#endif

	flags = AI_ADDRCONFIG ;
	if ((rs = uc_getipnodebyname(sip->hostname,af,flags,&hep)) >= 0) {
	    SOCKADDRESS	server ;
	    const int	pf = PF_INET6 ;
	    uchar	dummy[2] ;

#if	CF_DEBUGS
	    debugprintf("dialtcp/try_inet6: name=%s\n",hep->h_name) ;
	    debugprintf("dialtcp/try_inet6: addrtype=%d\n",
	        hep->h_addrtype) ;
#endif /* CF_DEBUGS */

	    dummy[0] = '\0' ;
	    if ((rs = sockaddress_start(&server,af,dummy,sip->port,0)) >= 0) {
	        hostent_cur	hc ;

	        if ((rs = hostent_curbegin(hep,&hc)) >= 0) {
	    	    struct sockaddr	*sap ;
	            int			c = 0 ;
	    	    const uchar		*ap ;

	            while ((rs1 = hostent_enumaddr(hep,&hc,&ap)) >= 0) {

#if	CF_DEBUGS
	                {
	                    char	hexbuf[HEXBUFLEN + 1] ;
	                    mkhexstr(hexbuf,HEXBUFLEN,ap,16) ;
	                    debugprintf("dialtcp/try_inet6: a0= %s\n",
				hexbuf) ;
	                }
#endif /* CF_DEBUGS */

	                sockaddress_putaddr(&server,ap) ;

	                c += 1 ;
	                sip->count += 1 ;
	                sap = (struct sockaddr *) &server ;
	                rs = subinfo_makeconn(sip,pf,sap) ;
	                fd = rs ;

	                if (fd >= 0) break ;
	                if ((rs < 0) && (rs != SR_PFNOSUPPORT)) break ;
	            } /* end while */
	            if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;

	            rs1 = hostent_curend(hep,&hc) ;
		    if (rs >= 0) rs = rs1 ;
	            if ((rs >= 0) && (c == 0)) rs = SR_HOSTUNREACH ;
	        } /* end if (cursor) */

	        sockaddress_finish(&server) ;
	    } /* end if (sockaddress) */

	    rs1 = uc_freehostent(hep) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (uc_getipnodebyname) */
	if ((rs < 0) && (fd >= 0)) u_close(fd) ;

#if	CF_DEBUGS
	debugprintf("dialtcp/try_inet6: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (try_inet6) */


static int try_addr(SUBINFO *sip)
{
	int		rs ;
	int		fd = 0 ;

#if	CF_DEBUGS
	debugprintf("dialtcp/try_addr: ent\n") ;
#endif

	if ((rs = getprotofamily(sip->af)) >= 0) {
	    SOCKADDRESS		server ;
	    const int		pf = rs ;
	    const int		port = sip->port ;
	    uchar		*ap = (uchar *) sip->addrbuf ;

#if	CF_DEBUGS
	    debugprintf("dialtcp/try_addr: af=%u pf=%u\n",sip->af,pf) ;
#endif

	    if ((rs = sockaddress_start(&server,sip->af,ap,port,0)) >= 0) {
	        struct sockaddr	*sap ;

	        sip->count += 1 ;
	        sap = (struct sockaddr *) &server ;
	        rs = subinfo_makeconn(sip,pf,sap) ;
	        fd = rs ;

#if	CF_DEBUGS
	        debugprintf("dialtcp/try_addr: makeconn() rs=%d\n",rs) ;
#endif

	        sockaddress_finish(&server) ;
	    } /* end if (sockaddress) */

	} /* end if (getprotofamily) */

#if	CF_DEBUGS
	debugprintf("dialtcp/try_addr: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (try_addr) */


#if	CF_TRYINET
static int try_inet(SUBINFO *sip)
{
	ADDRINFO	hint, *aip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		fd = -1 ;

#if	CF_DEBUGS
	debugprintf("dialtcp/try_inet: ent\n") ;
#endif

	memset(&hint,0,sizeof(ADDRINFO)) ;
	hint.ai_protocol = sip->proto ;

	if (sip->af >= 0) {
	    if ((rs = getprotofamily(sip->af)) >= 0) {
	        hint.ai_family = rs ;	/* documentation says use PF! */
	    }
	}

/* do the spin */

	if (rs >= 0) {
	    HOSTADDR	ha ;
	    cchar	*hn = sip->hostname ;
	    cchar	*ps = sip->portspec ;
	    if ((rs = hostaddr_start(&ha,hn,ps,&hint)) >= 0) {
	        HOSTADDR_CUR	cur ;

	        if ((rs = hostaddr_curbegin(&ha,&cur)) >= 0) {
	            int		c = 0 ;

	            while ((rs1 = hostaddr_enum(&ha,&cur,&aip)) >= 0) {
	                const int	aif = aip->ai_family ; /* PF */
		        int		f = FALSE ;

	                f = f || (hint.ai_family == 0) ;
	                f = f || (hint.ai_family == aif) ;
	                f = f || (aif == 0) ;
		        if (f) {
			    const int	to = sip->to ;
	                    c += 1 ;
	                    sip->count += 1 ;
			    rs = openaddrinfo(aip,to) ;
	                    fd = rs ;
	                } /* end if (protocol match) */

	                if (fd >= 0) break ;
	                if ((rs < 0) && (rs != SR_PFNOSUPPORT)) break ;
	            } /* end while */
	            if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;

	            rs1 = hostaddr_curend(&ha,&cur) ;
		    if (rs >= 0) rs = rs1 ;
	            if ((rs >= 0) && (c == 0)) rs = SR_HOSTUNREACH ;
	        } /* end if (cursor) */

	        rs1 = hostaddr_finish(&ha) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (initialized host addresses) */
	    if ((rs < 0) && (fd >= 0)) u_close(fd) ;
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("dialtcp/try_inet: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (try_inet) */
#endif /* CF_TRYINET */


/* make a connection (with protocol family and socket-address) */
static int subinfo_makeconn(SUBINFO *sip,int pf,struct sockaddr *sap)
{
	const int	st = sip->type ;
	const int	proto = sip->proto ;
	const int	to = sip->to ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("dialtcp/makeconn: pf=%u type=%u proto=%u\n",
	    pf,sip->type,sip->proto) ;
#endif

	rs = opensockaddr(pf,st,proto,sap,to) ;

#if	CF_DEBUGS
	debugprintf("dialtcp/makeconn: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_makeconn) */


#if	CF_DEBUGS
static int makeint(const void *addr)
{
	int		hi = 0 ;
	uchar		*us = (uchar *) addr ;
	hi |= (MKCHAR(us[0]) << 24) ;
	hi |= (MKCHAR(us[1]) << 16) ;
	hi |= (MKCHAR(us[2]) << 8) ;
	hi |= (MKCHAR(us[3]) << 0) ;
	return hi ;
}
#endif /* CF_DEBUGS */


