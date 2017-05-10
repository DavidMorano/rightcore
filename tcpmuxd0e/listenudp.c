/* listenudp */

/* subroutine to listen on a UDP port */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_PROTOLOOKUP	0		/* lookup protocol spec? */


/* revision history:

	= 1998-04-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine listens on a UDP socket that it finds and binds to
        itself.

	Synopsis:

	int listenudp(af,hostname,portspec,opts)
	int		af ;
	const char	hostname[] ;
	const char	portspec[] ;
	int		opts ;

	Arguments:

	af			address family
	hostname		address to bind to
	portspec		port to bind to
	opts			opts to use

	Return:

	>=0			FD of socket
	<0			error


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
#include	<sockaddress.h>
#include	<inetaddr.h>
#include	<hostent.h>
#include	<hostinfo.h>
#include	<hostaddr.h>
#include	<localmisc.h>


/* local defines */

#ifndef	INET4ADDRLEN
#define	INET4ADDRLEN	sizeof(in_addr_t)
#endif

#ifndef	INET6ADDRLEN
#define	INET6ADDRLEN	16
#endif

#ifndef	INETXADDRLEN
#define	INETXADDRLEN	MAX(INET4ADDRLEN,INET6ADDRLEN)
#endif /* INETXADDRLEN */

#ifndef	PROTONAME_UDP
#define	PROTONAME_UDP	"udp"
#endif

#ifndef	ANYHOST
#define	ANYHOST		"anyhost"
#endif

#undef	PRNAME
#define	PRNAME		"LOCAL"

#ifndef	INETADDRBAD
#define	INETADDRBAD	((unsigned int) (~ 0))
#endif

#ifndef	INET4_ADDRSTRLEN
#define	INET4_ADDRSTRLEN	16
#endif

#ifndef	INET6_ADDRSTRLEN
#define	INET6_ADDRSTRLEN	46	/* Solaris® says this is 46! */
#endif

#ifndef	INETX_ADDRSTRLEN
#define	INETX_ADDRSTRLEN	MAX(INET4_ADDRSTRLEN,INET6_ADDRSTRLEN)
#endif

#ifndef	INETOPT_REUSEADDR
#define	INETOPT_REUSEADDR	(1<<0)
#endif


/* external subroutines */

extern int	snwcpy(char *,int,cchar *,int) ;
extern int	mnwcpy(char *,int,const void *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	getportnum(cchar *,cchar *) ;
extern int	getprotofamily(int) ;
extern int	getproto_name(cchar *,int) ;


/* external variables */


/* local typedefs */

#if	defined(IRIX) && (! defined(TYPEDEF_INADDRT))
#define	TYPEDEF_INADDRT	1
typedef unsigned int	in_addr_t ;
#endif /* SGI IRIX problem */


/* local structures */


/* forward references */

static int	getaddr(char *,int,const char *) ;

static int	hostinfo_findaf(HOSTINFO *,char *,int,int) ;


/* local variables */


/* exported subroutines */


int listenudp(int af,cchar *hostname,cchar *portspec,int opts)
{
	SOCKADDRESS	sa ;
	in_addr_t	inet4addr ;
	int		rs = SR_OK ;
	int		proto = IPPROTO_UDP ;
	int		pf ;
	int		port = 0 ;
	int		s = -1 ;
	int		f_anyhost = FALSE ;
	const char	*protoname = PROTONAME_UDP ;
	char		addr[INETXADDRLEN+1] ;


#if	CF_DEBUGS
	debugprintf("listenudp: ent af=%d\n",af) ;
	debugprintf("listenudp: hostname=%s\n",hostname) ;
	debugprintf("listenudp: portspec=%s\n",portspec) ;
	debugprintf("listenudp: opts=%08x\n",opts) ;
#endif

	if (portspec == NULL) return SR_FAULT ;
	if (portspec[0] == '\0') return SR_INVALID ;

/* host */

	if ((hostname != NULL) && (hostname[0] != '\0') && 
	    (hostname[0] != '*')) {

	    if ((inet4addr = inet_addr(hostname)) != INETADDRBAD) {
		f_anyhost = (inet4addr == INADDR_ANY) ;
	        memcpy(addr,&inet4addr,INET4ADDRLEN) ;
		if ((af == AF_UNSPEC) || (af == AF_INET4)) {
		    af = AF_INET4 ;
		} else {
		    rs = SR_AFNOSUPPORT ;
		}
	    } else {
		int	f ;

		f = (strcmp(hostname,ANYHOST) == 0) ;
		f = f || (strcmp(hostname,"*") == 0) ;
	        if (! f) {

		    rs = getaddr(addr,af,hostname) ;
		    af = rs ;

#if	CF_DEBUGS
		    {
			char	addrstr[INETX_ADDRSTRLEN+1] ;
	        	debugprintf("listenudp: getaddr() rs=%d\n",rs) ;
			sninetaddr(addrstr,INETX_ADDRSTRLEN,af,addr) ;
	        	debugprintf("listenudp: addr=%s\n",addrstr) ;
		    }
#endif

	        } else {
	            f_anyhost = TRUE ;
	            memset(addr,0,INETXADDRLEN) ; /* any-host */
	    	    if (af == AF_UNSPEC) af = AF_INET6 ;
		}

	    } /* end if (inet_addr) */

	} else {
	    f_anyhost = TRUE ;
	    memset(addr,0,INETXADDRLEN) ; /* any-host */
	    if (af == AF_UNSPEC) af = AF_INET6 ;
	}

/* port */

	if ((rs >= 0) && (portspec[0] != '*')) {
	    rs = getportnum(protoname,portspec) ;
	    port = rs ;
	} /* end if (getting a port) */

#if	CF_DEBUGS
	debugprintf("listenudp: port=%d\n",port) ;
#endif

/* get protocol family (from address-family) */

	if (rs >= 0) {
	    if (af == AF_UNSPEC) af = AF_INET6 ;
	    rs = getprotofamily(af) ;
	    pf = rs ;
	}

/* get the protocol number */

#if	CF_PROTOLOOKUP
	if (rs >= 0) {
	    if ((rs = getproto_name(protoname,-1)) >= 0) {
	        proto = pe.p_proto ;
	    }
	}
#endif /* CF_PROTOLOOKUP */

/* continue */

	if (rs >= 0) {
	    rs = u_socket(pf,SOCK_DGRAM,proto) ;
	    s = rs ;
	    if ((rs == SR_PFNOSUPPORT) && f_anyhost && (pf == PF_INET6)) {
	        pf = PF_INET4 ;
	        af = AF_INET4 ;
	        rs = u_socket(pf,SOCK_DGRAM,proto) ;
	        s = rs ;
	    }
	}

/* opts */

	if ((rs >= 0) && (opts & INETOPT_REUSEADDR)) {
	    const int	onesize = sizeof(int) ;
	    int		one = 1 ;
	    rs = u_setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&one,onesize) ;
	}

/* bind */

	if (rs >= 0) {
	    if ((rs = sockaddress_start(&sa,af,addr,port,0)) >= 0) {
	        struct sockaddr	*sap = (struct sockaddr *) &sa ;
	        const int	sal = rs ;

	        rs = u_bind(s,sap,sal) ;

	        sockaddress_finish(&sa) ;
	    } /* end if (socketet) */
	} /* end if */

/* done */

	if ((rs < 0) && (s >= 0)) {
	    u_close(s) ;
	}

#if	CF_DEBUGS
	debugprintf("listenudp: ret rs=%d fd=%u\n",rs,s) ;
#endif

	return (rs >= 0) ? s : rs ;
}
/* end subroutine (listenudp) */


/* local subroutines */


static int getaddr(char *hap,int af,cchar *hn)
{
	HOSTINFO	hi ;
	int		rs ;
	int		raf = 0 ;

	memset(hap,0,INETXADDRLEN) ;
	if ((rs = hostinfo_start(&hi,af,hn)) >= 0) {
	    rs = 0 ;

	    if ((rs == 0) || (rs == SR_NOTFOUND)) {
	        raf = AF_INET6 ;
	        if ((af == raf) || (af == AF_UNSPEC)) {
		    rs = hostinfo_findaf(&hi,hap,INETXADDRLEN,raf) ;
		}
	    }
	    if ((rs == 0) || (rs == SR_NOTFOUND)) {
	        raf = AF_INET4 ;
	        if ((af == raf) || (af == AF_UNSPEC)) {
		    rs = hostinfo_findaf(&hi,hap,INETXADDRLEN,raf) ;
	        }
	    }

	    hostinfo_finish(&hi) ;
	} /* end if (hostinfo) */

#ifdef	COMMENT
	if ((rs == 0) || (rs == SR_NOTFOUND)) rs = SR_HOSTUNREACH ;
#endif

	return (rs >= 0) ? raf : rs ;
}
/* end subroutine (getaddr) */


static int hostinfo_findaf(HOSTINFO *hip,char *abuf,int alen,int af)
{
	HOSTINFO_CUR	cur ;
	int		rs ;
	int		al = 0 ;
	int		f = FALSE ;

	if ((rs = hostinfo_curbegin(hip,&cur)) >= 0) {
	    const uchar	*ap ;

	    while ((rs = hostinfo_enumaddr(hip,&cur,&ap)) >= 0) {
		al = rs ;

		switch (al) {
		case INET4ADDRLEN:
		    f = (af == AF_INET4) || (af == AF_UNSPEC) ;
		    break ;
		case INET6ADDRLEN:
		    f = (af == AF_INET6) || (af == AF_UNSPEC) ;
		    break ;
		} /* end switch */

		if (f) {
		    rs = mnwcpy(abuf,alen,ap,al) ;
		} else {
		    al = 0 ;
		}

		if (f) break ;
		if (rs < 0) break ;
	    } /* end while */

	    hostinfo_curend(hip,&cur) ;
	} /* end if (cursor) */

	return (rs >= 0) ? al : rs ;
}
/* end subroutine (hostinfo_findaf) */


