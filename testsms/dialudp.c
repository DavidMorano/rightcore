/* dialudp */

/* subroutine to dial out to a machine using UDP */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_PROTO	0		/* lookup protocol name */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will dial out to an INET host using the UDP protocol
	and the optional port specified.

	Synopsis:

	int dialudp(hostname,portspec,af,to,opts)
	const char	hostname[] ;
	const char	portspec[] ;
	int		af, to ;
	int		opts ;

	Arguments:

	hostname	host to dial to
	portspec	port specification to use
	af		address family
	to		to ('>=0' mean use one, '-1' means don't)
	opts		any dial options

	Returns:

	>=0		file descriptor
	<0		error in dialing

	Notes:

	If the given address family is unspecified, we cycle through:
		INET4
		INET6
	looking fo a connection.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<hostaddr.h>
#include	<localmisc.h>


/* local defines */

#ifndef	COLUMNS
#define	COLUMNS		80
#endif

#ifndef	ADDRINFO
#define	ADDRINFO	struct addrinfo
#endif

#define	PROTONAME	"udp"


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	inetpton(uchar *,int,int,const char *,int) ;
extern int	getprotofamily(int) ;
extern int	getproto_name(cchar *,int) ;
extern int	openaddrinfo(ADDRINFO *,int) ;
extern int	isFailConn(int) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif /* CF_DEBUGS */


/* external variables */


/* local typedefs */


/* local structures */


/* forward references */

static int	opendialudp(int,cchar *,cchar *,int) ;
static int	opendialudp_hint(ADDRINFO *,cchar *,cchar *,int) ;


/* local variables */

static const int	pfs[] = {
	PF_INET4,
	PF_INET6,
	0
} ;


/* exported subroutines */


/* ARGSUSED */
int dialudp(cchar *hostname,cchar *portspec,int af,int to,int opts)
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("dialudp: af=%d hostname=%s portspec=%s\n",
	    af,hostname,portspec) ;
#endif

	if (hostname == NULL) return SR_FAULT ;
	if (portspec == NULL) return SR_FAULT ;

	if (hostname[0] == '\0') return SR_INVALID ;
	if (portspec[0] == '\0') return SR_INVALID ;

	rs = opendialudp(af,hostname,portspec,to) ;

#if	CF_DEBUGS
	debugprintf("dialudp: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (dialudp) */


/* local subroutines */


static int opendialudp(int af,cchar *hostname,cchar *portspec,int to)
{
	ADDRINFO	hint ;
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("opendialudp: ent af=%d\n",af) ;
#endif

	memset(&hint,0,sizeof(ADDRINFO)) ;

#if	CF_PROTO
	{
	    int		rs ;
	    cchar	*pn = PROTONAME ;
	    if ((rs = getproto_name(pn,-1)) >= 0) {
	        hint.ai_protocol = rs ;
	    }
	}
#else
	hint.ai_protocol = IPPROTO_UDP ;
#endif /* CF_PROTO */

	if ((rs >= 0) && (af > 0)) {
	    if ((rs = getprotofamily(af)) >= 0) {
	        hint.ai_family = rs ; /* PF */
	    }
	}

#if	CF_DEBUGS
	debugprintf("opendialudp: before rs=%d\n",rs) ;
	debugprintf("opendialudp: af=%d\n",af)  ;
	debugprintf("opendialudp: hint aiproto=%d\n",hint.ai_protocol) ;
	debugprintf("opendialudp: hint aifamily=%d\n",hint.ai_family) ;
#endif

/* do the spin */

	if (rs >= 0) {
	    if (hint.ai_family == PF_UNSPEC) {
	        int	i ;
	        rs = SR_NOTCONN ;
	        for (i = 0 ; (pfs[i] > 0) && isFailConn(rs) ; i += 1) {
	            hint.ai_family = pfs[i] ;
	            rs = opendialudp_hint(&hint,hostname,portspec,to) ;
	            if (rs >= 0) break ;
	        } /* end for */
	    } else {
	        rs = opendialudp_hint(&hint,hostname,portspec,to) ;
	    }
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("opendialudp: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (opendialudp) */


static int opendialudp_hint(ADDRINFO *hip,cchar *hs,cchar *ps,int to)
{
	HOSTADDR	ha ;
	int		rs ;
	int		rs1 ;
	int		fd = -1 ;
#if	CF_DEBUGS
	debugprintf("opendialudp_hint: ent hs=%s ps=%s\n",hs,ps) ;
#endif
	if ((rs = hostaddr_start(&ha,hs,ps,hip)) >= 0) {
	    HOSTADDR_CUR	cur ;

	    if ((rs = hostaddr_curbegin(&ha,&cur)) >= 0) {
	        ADDRINFO	*aip ;
	        int		c = 0 ;

	        while ((rs1 = hostaddr_enum(&ha,&cur,&aip)) >= 0) {
	            const int	aif = aip->ai_family ; /* PF */
	            int		f = FALSE ;

#if	CF_DEBUGS
	            {
	                const int	cols = COLUMNS ;
	                const int	al = aip->ai_addrlen ;
	                cchar	*ap = (cchar *) aip->ai_addr ;
	                debugprintf("opendialudp: enum aifamily=%u\n",
	                    aip->ai_family) ;
	                debugprintf("opendialudp: enum aisocktype=%u\n",
	                    aip->ai_socktype) ;
	                debugprinthex("opendialudp:",cols,ap,al) ;
	            }
#endif /* CF_DEBUGS */

	            f = f || (hip->ai_family == 0) ;
	            f = f || (hip->ai_family == aif) ;
	            f = f || (aif == 0) ;
	            if (f) {
	                c += 1 ;
	                rs = openaddrinfo(aip,to) ;
	                fd = rs ;
#if	CF_DEBUGS
	        debugprintf("dialudp: openaddrinfo() rs=%d\n",rs) ;
#endif
	            } /* end if (protocol match) */

	            if (fd >= 0) break ;
	            if ((rs < 0) && (rs != SR_PFNOSUPPORT)) break ;
	        } /* end while */
	        if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;

#if	CF_DEBUGS
	        debugprintf("dialudp: done rs=%d c=%u\n",rs,c) ;
#endif

	        rs1 = hostaddr_curend(&ha,&cur) ;
	        if (rs >= 0) rs = rs1 ;
	        if ((rs >= 0) && (c == 0)) rs = SR_HOSTUNREACH ;
	    } /* end if (cursor) */

	    rs1 = hostaddr_finish(&ha) ;
	    if (rs >= 0) rs = rs1 ;
	    if ((rs < 0) && (fd >= 0)) u_close(fd) ;
	} /* end if (hostaddr) */
#if	CF_DEBUGS
	debugprintf("opendialudp_hint: ret rs=%d fd=%u\n",rs,fd) ;
#endif
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opendialudp_hint) */


