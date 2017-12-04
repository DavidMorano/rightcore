/* listentcp */

/* subroutine to listen on a TCP port */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_PROTOLOOKUP	0		/* lookup protocol spec? */


/* revision history:

	= 1998-04-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine will bind to a port (any free one) and start to listen
        on that port for incoming connections.

	Synopsis:

	int listentcp(af,hostname,portspec,opts)
	int		af ;
	const char	hostname[] ;
	const char	portspec[] ;
	int		opts ;

	Arguments:

	af		address family (numeric)
	hostname	hostname
	portspec	port (alpa or numeric)
	opts		opts

	Returns:

	<0		error
	>=0		listen FD


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
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

#ifndef	ADDRINFO
#define	ADDRINFO	struct addrinfo
#endif

#ifndef	PROTONAME_TCP
#define	PROTONAME_TCP	"tcp"
#endif

#ifndef	ANYHOST
#define	ANYHOST		"anyhost"
#endif

#undef	PRNAME
#define	PRNAME		"LOCAL"


/* external subroutines */

extern int	getprotofamily(int) ;
extern int	getproto_name(cchar *,int) ;
extern int	getnodedomain(const char *,const char *) ;
extern int	getdomainname(const char *,int,const char *) ;
extern int	isOneOf(const int *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local typedefs */


/* local structures */


/* forward references */

static int	listentcp_lookup(int,int,cchar *,cchar *,int) ;
static int	listentcp_try(ADDRINFO *,int) ;

static int	isFailListen(int) ;


/* local variables */

static const int	rsfails[] = {
	SR_ADDRINUSE,
	SR_ADDRNOTAVAIL,
	SR_AFNOSUPPORT,
	SR_ACCESS,
	0
} ;


/* exported subroutines */


int listentcp(int af,cchar *hostname,cchar *portspec,int opts)
{
	int		rs = SR_OK ;
	int		proto = IPPROTO_TCP ;
	int		s = -1 ;
	const char	*hp = hostname ;

#if	CF_DEBUGS
	{
	    int	f_ra = (opts&1)?1:0 ;
	    debugprintf("listentcp: ent ra=%u\n",f_ra) ;
	}
#endif

	if (portspec == NULL) return SR_FAULT ;

	if (af < 0) return SR_INVALID ;
	if (portspec[0] == '\0') return SR_INVALID ;

/* get the protocol number */

#if	CF_PROTOLOOKUP
	rs = getproto_name(PROTONAME_TCP,-1) ;
	proto = rs ;
#endif /* CF_PROTOLOOKUP */

/* host */

	if (hp != NULL) {
	    int	f = FALSE ;
	    f = f || (hp[0] == '\0') ;
	    f = f || (strcmp(hp,"*") == 0) ;
	    if (f)
	        hp = ANYHOST ;
	}

#if	CF_DEBUGS
	debugprintf("listentcp: hp=%s\n",hp) ;
#endif

	if (rs >= 0) {
	    if ((rs = getprotofamily(af)) >= 0) {
	        const int	pf = rs ;
	        const char	*ps = portspec ;
	        rs = listentcp_lookup(pf,proto,hp,ps,opts) ;
	        s = rs ;
	    } /* end if (getprotofamily) */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("listentcp: ret rs=%d s=%u\n",rs,s) ;
#endif

	return (rs >= 0) ? s : rs ;
}
/* end subroutine (listentcp) */


/* local subroutines */


static int listentcp_lookup(pf,proto,hs,ps,opts)
int		pf ;
int		proto ;
const char	hs[] ;
const char	ps[] ;
int		opts ;
{
	struct addrinfo	hint ;
	HOSTADDR	ha ;
	HOSTADDR_CUR	hacur ;
	int		rs ;
	int		rs_last = 0 ;
	int		rs1 ;
	int		s = -1 ;

#if	CF_DEBUGS
	debugprintf("listentcp_lookup: ent\n") ;
#endif

	memset(&hint,0,sizeof(struct addrinfo)) ;
	hint.ai_flags = AI_PASSIVE ;
	hint.ai_family = pf ;
	hint.ai_socktype = SOCK_STREAM ;
	hint.ai_protocol = proto ;

	if ((rs = hostaddr_start(&ha,hs,ps,&hint)) >= 0) {
	    if ((rs = hostaddr_curbegin(&ha,&hacur)) >= 0) {
	        while ((rs >= 0) && (s < 0)) {
	            struct addrinfo	*aip ;

	            while ((rs = hostaddr_enum(&ha,&hacur,&aip)) >= 0) {
	                if ((aip->ai_family == pf) || (pf == PF_UNSPEC)) break ;
	            } /* end while */

#if	CF_DEBUGS
	            debugprintf("listentcp: hostaddr_enum() rs=%d\n",rs) ;
#endif

		    if (rs >= 0) {
	                if ((rs = listentcp_try(aip,opts)) >= 0) {
	                    s = rs ;
	                } else if (isFailListen(rs)) {
			    rs_last = rs ;
	                    rs = SR_OK ;
			}
	            } /* end if (ok) */

	        } /* end while */
	        rs1 = hostaddr_curend(&ha,&hacur) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (cursor) */
	    rs1 = hostaddr_finish(&ha) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (hostaddr) */

	if ((rs < 0) && (s >= 0)) u_close(s) ;

	if ((rs == SR_NOENT) && (rs_last < 0)) rs = rs_last ;

#if	CF_DEBUGS
	debugprintf("listentcp_lookup: ret rs=%d s=%u\n",rs,s) ;
#endif

	return (rs >= 0) ? s : rs ;
}
/* end subroutine (listentcp_lookup) */


static int listentcp_try(ADDRINFO *aip,int opts)
{
	const int	proto = aip->ai_protocol ;
	const int	st = aip->ai_socktype ;
	const int	af = aip->ai_family ;
	int		rs ;
	int		s = -1 ;

#if	CF_DEBUGS
	debugprintf("listentcp_try: ent\n") ;
#endif

	if ((rs = getprotofamily(af)) >= 0) {
	    const int	spf = rs ;
	    if ((rs = u_socket(spf,st,proto)) >= 0) {
	        s = rs ;

	        if (opts != 0) {
	            const int	size = sizeof(int) ;
	            const int	opt = SO_REUSEADDR ;
	            int		one = 1 ;
	            rs = u_setsockopt(s,SOL_SOCKET,opt,&one,size) ;
	        }

#if	defined(IPPROTO_IPV6) && defined(IPV6_V6ONLY)
	        if ((rs >= 0) && (pf == PF_INET6)) {
	            const int	cmd = IPPROTO_IPV6 ;
	            const int	opt = IPV6_V6ONLY ;
	            const int	size = sizeof(int) ;
	            int		one = 1 ;
	            rs = u_setsockopt(s,cmd,opt,&one,size) ;
	        }
#endif /* defined(IPV6_V6ONLY) */

	        if (rs >= 0) {
	            struct sockaddr	*sap = aip->ai_addr ;
	            const int		sal = aip->ai_addrlen ;
	            if ((rs = u_bind(s,sap,sal)) >= 0) {

#if	CF_DEBUGS
	                debugprintf("listentcp: u_bind() rs=%d\n",
	                    rs) ;
#endif

	                rs = u_listen(s,10) ;
#if	CF_DEBUGS
	                debugprintf("listentcp_lookup: "
	                    "u_listen() rs=%d s=%u\n",rs) ;
#endif
	            } /* end if (bind) */
	        } /* end if (ok) */

	        if ((rs < 0) && (s >= 0)) u_close(s) ;
	    } /* end if (socket) */
	} /* end if (getprotofamily) */

#if	CF_DEBUGS
	debugprintf("listentcp_try: ret rs=%d s=%u\n",rs,s) ;
#endif

	return (rs >= 0) ? s : rs ;
}
/* end subroutine (listentcp_try) */


static int isFailListen(int rs)
{
	return isOneOf(rsfails,rs) ;
}
/* end subroutine (isFailListen) */


