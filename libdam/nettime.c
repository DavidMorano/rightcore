/* nettime */

/* program to get time from a network time server host */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_FETCHPROTO	0		/* fetch protocol (no!) */
#define	CF_USEUDP	1		/* use UDP */
#define	CF_UDPMUX	1		/* pretend using UDPMUX */
#define	CF_SOLARIS	1		/* broken Solaris */
#define	CF_CONNECTUDP	0		/* make UDP connection */
#define	CF_SLEEP	0		/* debug-sleep */


/* revision history:

	= 2009-04-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 2009 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will get the time-of-day from a time server specified
	by a hostname given on the command line.  The subroutine tries to
	connect to a TCP listener on the time server and will read 4 bytes out
	of the socket.  These four bytes, when organized as a long word in
	network byte order, represent the time in seconds since Jan 1, 1900.
	We will subtract the value "86400 * ((365 * 70) + 17)" to get the time
	in seconds since Jan 1, 1970 (the UNIX epoch).

	Synopsis:

	int nettime(ntp,proto,af,hostname,svc,to)
	NETTIME		*ntp ;
	int		proto ;
	int		af ;
	cchar		hostname[] ;
	cchar		svc[] ;
	int		to ;

	Arguments:

	ntp		pointer to result structure
	proto		protocol number ( UDP, TCP )
	af		address family
	hostname	host-name
	svc		service-name
	to		timeout

	Returns:

	<0		error
	>=0		OK


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/time.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<hostaddr.h>
#include	<sockaddress.h>
#include	<vechand.h>
#include	<sbuf.h>
#include	<cthex.h>
#include	<localmisc.h>

#include	"nettime.h"


/* local defines */

#ifndef	ADDRBUFLEN
#define	ADDRBUFLEN	(MAXPATHLEN + 20)
#endif

#define	NETBUFLEN	1024
#define	HEXBUFLEN	100

#define	PRINTADDRLEN	100

#define	NETTIMEROLL	3576712516UL

#define	LARGETIME	(60 * 22)
#define	SMALLTIME	(50)

#define	EPOCHDIFF	(86400 * ((365 * 70) + 17))

#define	NETTIMELEN	4

#ifndef	NYEARS_CENTURY
#define	NYEARS_CENTURY	100
#endif

#define	NETTIME		struct nettime

#ifndef	INETSVC_TIME
#define	INETSVC_TIME	"time"
#endif

#define	PROTONAME_UDP	"udp"

#define	NTRIES		2


/* external subroutines */

extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy2(char *,int,cchar *,cchar *) ;
extern int	mkpath1w(char *,cchar *,int) ;
extern int	matostr(cchar **,cchar *,int) ;
extern int	getprotofamily(int) ;
extern int	dialtcp(cchar *,cchar *,int,int,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */

struct udpargs {
	cchar		*hostname ;
	cchar		*svc ;
	struct nettime	*ntp ;
	struct timeval	*nsp, *nep ;
	int		proto ;
	int		pf ;
	int		af ;
	int		fd ;
	int		to ;
	int		c ;
} ;


/* forward references */

static int	nettime_tcp(NETTIME *,int,cchar *,cchar *,int) ;
static int	nettime_udp(NETTIME *,int,cchar *,cchar *,int) ;
static int	nettime_udptrythem(struct udpargs *,char *) ;
static int	nettime_udptrysome(struct udpargs *,char *,
			VECHAND *,HOSTADDR *,int) ;
static int	nettime_udptryone(struct udpargs *,char *,
			struct addrinfo *) ;
static int	nettime_udptryoner(struct udpargs *,char *,
			struct addrinfo *,int) ;

static uint64_t	gettime_inet(cchar *) ;

static uint64_t	utime_timeval(struct timeval *) ;
static uint64_t	utime_tcpcalc(uint64_t,uint64_t) ;
static uint64_t	utime_udpcalc(uint64_t,uint64_t) ;

static int	tv_loadusec(struct timeval *,int64_t) ;

static int	vechand_already(VECHAND *,void *) ;

static int	connagain(int) ;

static int	isaddrsame(const void *,const void *) ;

#if	CF_DEBUGS
static int	mkprintaddr(char *,int,struct sockaddr *) ;
static int	mkprintscope(char *,int,struct sockaddr *) ;
#endif


/* local variables */

static const int	connagains[] = {
	SR_NOENT,
	SR_HOSTUNREACH,
	SR_HOSTDOWN,
	SR_CONNREFUSED,
	SR_NOPROTOOPT,
	SR_PROTONOSUPPORT,
	SR_AFNOSUPPORT,
	SR_TIMEDOUT,
	0
} ;


/* exported subroutines */


int nettime(ntp,proto,af,hostname,svc,to)
NETTIME		*ntp ;
int		proto ;
int		af ;
cchar		hostname[] ;
cchar		svc[] ;
int		to ;
{
	int		rs = SR_OK ;
	int		f ;
	int		f_got = FALSE ;
	cchar		*inetsvc = INETSVC_TIME ;

	if (ntp == NULL) return SR_FAULT ;
	if (hostname == NULL) return SR_FAULT ;

	if (hostname[0] == '\0') return SR_INVALID ;

	if ((svc == NULL) || (svc[0] == '\0'))
	    svc = inetsvc ;

	if (af < 0)
	    af = AF_UNSPEC ;

#if	CF_DEBUGS
	debugprintf("nettime: proto=%d\n",proto) ;
	debugprintf("nettime: af=%d\n",af) ;
	debugprintf("nettime: host=%s\n",hostname) ;
	debugprintf("nettime: svc=%s\n",svc) ;
	debugprintf("nettime: to=%d\n",to) ;
#endif

	memset(ntp,0,sizeof(NETTIME)) ;

	f = ((proto == IPPROTO_UDP) || (proto <= 0)) ;
	if ((! f_got) && f) {

	    rs = nettime_udp(ntp,af,hostname,svc,to) ;
	    f_got = (rs > 0) ;

#if	CF_DEBUGS
	    debugprintf("nettime: nettime_udp() rs=%d\n",rs) ;
#endif

	} /* end if */

	f = ((proto == IPPROTO_TCP) || (proto <= 0)) ;
	if ((! f_got) && f && ((rs >= 0) || connagain(rs))) {

	    rs = nettime_tcp(ntp,af,hostname,svc,to) ;
	    f_got = (rs > 0) ;

#if	CF_DEBUGS
	    debugprintf("nettime: nettime_tcp() rs=%d\n",rs) ;
#endif

	} /* end if */

#if	CF_DEBUGS
	debugprintf("nettime: ret rs=%d f_got=%u\n",rs,f_got) ;
#endif

	return (rs >= 0) ? f_got : rs ;
}
/* end subroutine (nettime) */


/* local subroutines */


static int nettime_udp(ntp,af,hostname,svc,to)
NETTIME		*ntp ;
int		af ;
cchar		hostname[] ;
cchar		svc[] ;
int		to ;
{
	struct timeval	netstart, netend ;
	int		rs = SR_OK ;
	int		f_got = FALSE ;
	char		timebuf[TIMEBUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("nettime_udp: af=%d host=%s svc=%s\n",af,hostname,svc) ;
#endif

	ntp->proto = IPPROTO_UDP ;

#if	CF_USEUDP
	{
	    struct udpargs	ua ;

	    memset(&ua,0,sizeof(struct udpargs)) ;
	    ua.ntp = ntp ;
	    ua.proto = ntp->proto ;
	    ua.af = af ;
	    ua.hostname = hostname ;
	    ua.svc = svc ;
	    ua.to = to ;
	    ua.nsp = &netstart ;
	    ua.nep = &netend ;
	    ua.pf = -1 ;
	    rs = nettime_udptrythem(&ua,timebuf) ;
	    f_got = (rs > 0) ;
	}
#else
	rs = SR_PROTONOSUPPORT ;
#endif /* CF_USEUDP */

#if	CF_DEBUGS
	debugprintf("nettime_udp: mid rs=%d\n",rs) ;
#endif

	if ((rs >= 0) && f_got) {
	    uint64_t	uti_start, uti_end ;
	    uint64_t	uti_inet, uti_local ;
	    uint64_t	uti_trip ;
	    int64_t	uti_off ;

	    uti_start = utime_timeval(&netstart) ;

	    uti_end = utime_timeval(&netend) ;

	    uti_local = utime_udpcalc(uti_end,uti_start) ;

	    {
	        uint64_t	t = gettime_inet(timebuf) ;

#if	CF_DEBUGS
	        debugprintf("nettime_udp: t=%llu\n",t) ;
#endif

	        uti_inet = t * 1000000 ;
	        uti_off = uti_inet - uti_local ;
	        tv_loadusec(&ntp->off,uti_off) ;
	    }

	    uti_trip = uti_end - uti_start ;
	    tv_loadusec(&ntp->trip,uti_trip) ;

#if	CF_DEBUGS
	    debugprintf("nettime_udp: s-usec=%llu\n",uti_start) ;
	    debugprintf("nettime_udp: e-usec=%llu\n",uti_end) ;
	    debugprintf("nettime_udp: ustrip=%llu\n",uti_trip) ;
	    debugprintf("nettime_udp: e-inet=%llu\n",uti_inet) ;
	    debugprintf("nettime_udp: e-loca=%llu\n",uti_local) ;
	    debugprintf("nettime_udp: usoffe=%lld\n",uti_off) ;
#endif

	} /* end if */

#if	CF_DEBUGS
	debugprintf("nettime_udp: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? f_got : rs ;
}
/* end subroutine (nettime_udp) */


static int nettime_tcp(ntp,af,hostname,svc,to)
NETTIME		*ntp ;
int		af ;
cchar		hostname[] ;
cchar		svc[] ;
int		to ;
{
	struct timeval	netstart, netend ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		s = -1 ;
	int		pf ;
	int		raf ;
	int		len ;
	int		f_got = FALSE ;
	char		timebuf[TIMEBUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("nettime_tcp: af=%d host=%s svc=%s\n",af,hostname,svc) ;
	debugprintf("nettime_tcp: to=%d\n",to) ;
#endif

	ntp->proto = IPPROTO_TCP ;

/* retrieve network data */

	gettimeofday(&netstart,NULL) ;

	if ((af == AF_UNSPEC) || (af == AF_INET4)) {
#if	CF_DEBUGS
	debugprintf("nettime_tcp: af=INET4\n") ;
#endif
	    raf = AF_INET4 ;
	    pf = PF_UNSPEC ;
	    if ((rs1 = getprotofamily(raf)) >= 0) pf = rs1 ;
	    ntp->pf = pf ;
	    rs = dialtcp(hostname,svc,raf,to,0) ;
	    s = rs ;
#if	CF_DEBUGS
	debugprintf("nettime_tcp: INET4 dialtcp() rs=%d\n",rs) ;
#endif
	}

	if ((((s < 0) && (rs >= 0)) || ((rs < 0) && connagain(rs))) &&
	    ((af == AF_UNSPEC) || (af == AF_INET6))) {
#if	CF_DEBUGS
	debugprintf("nettime_tcp: af=INET6\n") ;
#endif
	    raf = AF_INET6 ;
	    pf = PF_UNSPEC ;
	    if ((rs1 = getprotofamily(raf)) >= 0) pf = rs1 ;
	    ntp->pf = pf ;
	    rs = dialtcp(hostname,svc,raf,to,0) ;
	    s = rs ;
#if	CF_DEBUGS
	debugprintf("nettime_tcp: INET6 dialtcp() rs=%d\n",rs) ;
#endif
	}

	if (rs >= 0) {

	    if ((rs = uc_reade(s,timebuf,TIMEBUFLEN,to,FM_EXACT)) >= 0) {
	        len = rs ;
	        if (len == NETTIMELEN) {
	            f_got = TRUE ;
	            gettimeofday(&netend,NULL) ;
	        } else {
	            rs = SR_BADMSG ;
		}
	    }

	    u_close(s) ;
	} /* end if */

/* process network data */

	if ((rs >= 0) && f_got) {
	    uint64_t	uti_start, uti_end ;
	    uint64_t	uti_inet, uti_local ;
	    uint64_t	uti_trip ;
	    int64_t	uti_off ;

	    uti_start = utime_timeval(&netstart) ;

	    uti_end = utime_timeval(&netend) ;

	    uti_local = utime_tcpcalc(uti_end,uti_start) ;

	    {
	        uint64_t	t = gettime_inet(timebuf) ;

#if	CF_DEBUGS
	        debugprintf("nettime_tcp: t=%llu\n",t) ;
#endif

	        uti_inet = t * 1000000 ;
	        uti_off = uti_inet - uti_local ;
	        tv_loadusec(&ntp->off,uti_off) ;
	    }

	    uti_trip = uti_end - uti_start ;
	    tv_loadusec(&ntp->trip,uti_trip) ;

	} /* end if */

#if	CF_DEBUGS
	debugprintf("nettime_tcp: ret rs=%d f_got=%u\n",rs,f_got) ;
#endif

	return (rs >= 0) ? f_got : rs ;
}
/* end subroutine (nettime_tcp) */


#if	CF_USEUDP

static int nettime_udptrythem(uap,timebuf)
struct udpargs	*uap ;
char		timebuf[] ;
{
	struct addrinfo	hint ;
	VECHAND		alist ;
	HOSTADDR	ha ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		proto = 0 ;
	int		pf ;
	int		f_got = FALSE ;

/* get the protocol number */

#if	CF_FETCHPROTO
	{
	    cchar	*pn = PROTONAME_UDP ;
	    if ((rs = getproto_name(pn,-1)) >= 0) {
	        proto = rs ;
	    }
	}
#else
	proto = IPPROTO_UDP ;
#endif /* CF_FETCHPROTO */

	uap->proto = proto ;

#if	CF_DEBUGS
	debugprintf("nettime_udptrythem: proto=%u\n",proto) ;
#endif

	if (rs < 0) goto ret0 ;

/* setup search restrictions */

	memset(&hint,0,sizeof(struct addrinfo)) ;
	hint.ai_protocol = proto ;

	if (uap->af >= 0) {
	    int pf = getprotofamily(uap->af) ;
	    if (pf >= 0) {
	        uap->pf = pf ;
	        hint.ai_family = pf ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("nettime_udptrythem: hint_pf=%u\n",hint.ai_family) ;
#endif

	if ((rs = vechand_start(&alist,2,0)) >= 0) {
	    cchar	*hn = uap->hostname ;
	    if ((rs = hostaddr_start(&ha,hn,uap->svc,&hint)) >= 0) {

	        if (((! f_got) && (rs >= 0)) || ((rs < 0) && connagain(rs))) {
	            pf = PF_INET4 ;
	            if ((uap->pf <= 0) || (uap->pf == pf)) {
	                rs1 = nettime_udptrysome(uap,timebuf,&alist,&ha,pf) ;
	                f_got = (rs1 > 0) ;
	                if ((rs >= 0) || (rs1 != SR_NOTFOUND)) rs = rs1 ;
	            }
	        }

	        if (((! f_got) && (rs >= 0)) || ((rs < 0) && connagain(rs))) {
	            pf = PF_INET6 ;
	            if ((uap->pf <= 0) || (uap->pf == pf)) {
	                rs1 = nettime_udptrysome(uap,timebuf,&alist,&ha,pf) ;
	                f_got = (rs1 > 0) ;
	                if ((rs >= 0) || (rs1 != SR_NOTFOUND)) rs = rs1 ;
	            }
	        }

	        if (((! f_got) && (rs >= 0)) || ((rs < 0) && connagain(rs))) {
	            pf = PF_UNSPEC ;
	            if (uap->pf <= 0) {
	                rs1 = nettime_udptrysome(uap,timebuf,&alist,&ha,pf) ;
	                f_got = (rs1 > 0) ;
	                if ((rs >= 0) || (rs1 != SR_NOTFOUND)) rs = rs1 ;

	            }
	        }

	        hostaddr_finish(&ha) ;
	        if ((rs >= 0) && (uap->c == 0)) rs = SR_PROTONOSUPPORT ;
	    } /* end if (initialized host addresses) */
	    vechand_finish(&alist) ;
	} /* end if (vechand) */

ret0:

#if	CF_DEBUGS
	debugprintf("nettime_udptrythem: ret rs=%d f_got=%u\n",rs,f_got) ;
#endif

	return (rs >= 0) ? f_got : rs ;
}
/* end subroutine (nettime_udptrythem) */


static int nettime_udptrysome(uap,timebuf,alp,hap,pf)
struct udpargs	*uap ;
char		timebuf[] ;
VECHAND		*alp ;
HOSTADDR	*hap ;
int		pf ;
{
	HOSTADDR_CUR	cur ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		f_got = FALSE ;

#if	CF_DEBUGS
	debugprintf("nettime_udptrysome: ent pf=%d\n",pf) ;
#endif

	if ((rs = hostaddr_curbegin(hap,&cur)) >= 0) {
	    struct addrinfo	*aip ;
	    int			proto ;
	    int			f ;

	    while (hostaddr_enum(hap,&cur,&aip) >= 0) {

#if	CF_DEBUGS
	        debugprintf("nettime_udptrysome: aip=%p\n",aip) ;
	        debugprintf("nettime_udptrysome: pf=%d\n",aip->ai_family) ;
	        debugprintf("nettime_udptrysome: proto=%u\n",
	            aip->ai_protocol) ;
#endif

	        proto = aip->ai_protocol ;
	        f = ((proto == uap->proto) || (proto <= 0)) ;

	        f = f && ((pf == PF_UNSPEC) || (pf == aip->ai_family)) ;

	        if (f) {
	            rs1 = vechand_already(alp,aip) ;
	            f = (rs1 == SR_NOTFOUND) ;
	        }

	        if (f) {

	            uap->c += 1 ;
	            rs = vechand_add(alp,aip) ;
	            if (rs >= 0) {
	                rs = nettime_udptryone(uap,timebuf,aip) ;
	                f_got = (rs > 0) ;
	            }

#if	CF_DEBUGS
	            debugprintf("nettime_udptrysome: _udptryone() rs=%d\n",
	                rs) ;
#endif

	            if (f_got) break ;
	        } /* end if (match) */

	    } /* end while */

	    hostaddr_curend(hap,&cur) ;
	} /* end if (cursor) */

#if	CF_DEBUGS
	debugprintf("nettime_udptrysome: ret rs=%d f_got=%u\n",rs,f_got) ;
#endif

	return (rs >= 0) ? f_got : rs ;
}
/* end subroutine (nettime_udptrysome) */


static int nettime_udptryone(uap,timebuf,aip)
struct udpargs	*uap ;
char		timebuf[] ;
struct addrinfo	*aip ;
{
	int		rs = SR_OK ;
	int		pf ;
	int		st ;
	int		proto ;
	int		i ;
	int		f = FALSE ;

#if	CF_DEBUGS
	debugprintf("nettime_udptryone: enter\n") ;
	debugprintf("nettime_udptryone: rproto=%d\n",uap->proto) ;
	debugprintf("nettime_udptryone: pf=%d\n",aip->ai_family) ;
	debugprintf("nettime_udptryone: st=%d\n",aip->ai_socktype) ;
	debugprintf("nettime_udptryone: proto=%d\n",aip->ai_protocol) ;
	{
	    int		rs1 ;
	    char	printaddr[PRINTADDRLEN + 1] ;
	    rs1 = mkprintaddr(printaddr,PRINTADDRLEN,aip->ai_addr) ;
	    debugprintf("nettime_udptryone: rs1=%d addrlen=%u\n",
	        rs1,aip->ai_addrlen) ;
	    debugprintf("nettime_udptryone: addr=%s\n",printaddr) ;
	    rs1 = mkprintscope(printaddr,PRINTADDRLEN,aip->ai_addr) ;
	    if (rs1 >= 0)
	        debugprintf("nettime_udptryone: scope=%s\n",printaddr) ;
	}
#endif /* CF_DEBUGS */

	uap->ntp->pf = aip->ai_family ;
	pf = aip->ai_family ;
	st = aip->ai_socktype ;
	proto = uap->proto ;
	if ((rs = u_socket(pf,st,proto)) >= 0) {
	    const int	fd = rs ;

#if	CF_CONNECTUDP
	    rs = u_connect(fd,aip->ai_addr,aip->ai_addrlen) ;
#endif

	    for (i = 0 ; (rs >= 0) && (i < NTRIES) ; i += 1) {

#if	CF_DEBUGS
	        debugprintf("nettime_udptryone: i=%u\n",i) ;
#endif

	        rs = nettime_udptryoner(uap,timebuf,aip,fd) ;
	        f = (rs > 0) ;

#if	CF_DEBUGS
	        debugprintf("nettime_udptryone: _udptryoner() rs=%d\n",rs) ;
#endif

#if	CF_SOLARIS
	        if (f || ((rs < 0) && (! connagain(rs))))
	            break ;
#else
	        if (f || ((rs < 0) && (rs != SR_TIMEDOUT)))
	            break ;
#endif

	    } /* end for */

	    u_close(fd) ;
	} /* end if (opened socket) */

#if	CF_DEBUGS
	debugprintf("nettime_udptryone: ret rs=%d f=%u i=%u\n",
	    rs,f,i) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (nettime_udptryone) */


static int nettime_udptryoner(uap,timebuf,aip,fd)
struct udpargs	*uap ;
char		timebuf[] ;
struct addrinfo	*aip ;
int		fd ;
{
	SOCKADDRESS	from ;
	int		rs = SR_OK ;
	int		to = uap->to ;
	int		fromlen ;
	int		netlen ;
	int		c ;
	int		f = FALSE ;
	char		netbuf[NETBUFLEN + 1] ;

	timebuf[0] = '\0' ;
	gettimeofday(uap->nsp,NULL) ;

#if	CF_DEBUGS
	{
	    struct timeval	*tvp = uap->nsp ;
	    debugprintf("nettime_udptryoner: s-sec=%ld\n",
	        tvp->tv_sec) ;
	    debugprintf("nettime_udptryoner: s-usec=%ld\n",
	        tvp->tv_usec) ;
	}
#endif /* CF_DEBUGS */

#if	CF_UDPMUX
	rs = sncpy2(netbuf,NETBUFLEN,INETSVC_TIME,"\n") ;
	netlen = rs ;
#else
	netbuf[0] ;
	netlen = 0 ;
#endif /* CF_UDPMUX */

#if	CF_DEBUGS
	debugprintf("nettime_udptryoner: netlen=%u\n",netlen) ;
	debugprintf("nettime_udptryoner: addrlen=%u\n", aip->ai_addrlen) ;
#endif

	if (rs >= 0) {
	    int		addrlen = aip->ai_addrlen ;

#if	CF_CONNECTUDP
	    rs = u_send(fd,netbuf,netlen,0) ;
#else
	    rs = u_sendto(fd,netbuf,netlen,0,aip->ai_addr,addrlen) ;
#endif

#if	CF_DEBUGS
	    debugprintf("nettime_udptryoner: u_sendto() rs=%d\n",rs) ;
#if	CF_SLEEP
	    sleep(10) ;
#endif
#endif

	} /* end if (send message) */

	if (rs >= 0) {

	    c = 0 ;
	    while (rs >= 0) {

#if	CF_DEBUGS
	        debugprintf("nettime_udptryoner: c=%u\n",c) ;
#endif

	        fromlen = sizeof(SOCKADDRESS) ;
	        rs = uc_recvfrome(fd,netbuf,NETBUFLEN,0,&from,&fromlen,to,0) ;
	        netlen = rs ;

#if	CF_DEBUGS
	        debugprintf("nettime_udptryoner: "
	            "uc_recvfrome() rs=%d\n",rs) ;
#endif

	        if (rs < 0)
	            break ;

#if	CF_DEBUGS
	        {
	            struct sockaddr	*sap = (struct sockaddr *) &from ;
	            int		rs1 ;
	            char	printaddr[PRINTADDRLEN + 1] ;
	            rs1 = mkprintaddr(printaddr,PRINTADDRLEN,sap) ;
	            debugprintf("nettime_udptryone: FROM rs1=%d\n", rs1) ;
	            debugprintf("nettime_udptryone: addr=%s\n",printaddr) ;
	            rs1 = mkprintscope(printaddr,PRINTADDRLEN,sap) ;
	            if (rs1 >= 0)
	                debugprintf("nettime_udptryone: scope=%s\n",printaddr) ;
	        }
#endif /* CF_DEBUGS */

	        f = isaddrsame(&from,aip->ai_addr) ;

#if	CF_DEBUGS
	        debugprintf("nettime_udptryone: isaddrsame=%u\n",f) ;
#endif

	        if (f) {
	            if (netlen == NETTIMELEN) {
	                f = TRUE ;
	            } else
	                rs = SR_BADMSG ;
	            break ;
	        }

	        c += 1 ;

	    } /* end while */

	} /* end if */

	if ((rs >= 0) && f) {
	    gettimeofday(uap->nep,NULL) ;
	    memcpy(timebuf,netbuf,NETTIMELEN) ;
	}

#if	CF_DEBUGS
	if (rs >= 0) {
	    struct timeval	*tvp = uap->nep ;
	    debugprintf("nettime_udptryoner: e-sec=%ld\n",
	        tvp->tv_sec) ;
	    debugprintf("nettime_udptryoner: e-usec=%ld\n",
	        tvp->tv_usec) ;
	}
#endif /* CF_DEBUGS */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (nettime_udptryoner) */

#endif /* CF_USEUDP */


static uint64_t gettime_inet(cchar buf[])
{
	uint64_t	net = 0 ;
	uint64_t	netoff = 2208988800ULL ;
	uint64_t	rtime = 0 ;
	uchar		*ubuf = (uchar *) buf ;

	if (buf == NULL)
	    return 0 ;

	net = (net << 8) | ubuf[0] ;
	net = (net << 8) | ubuf[1] ;
	net = (net << 8) | ubuf[2] ;
	net = (net << 8) | ubuf[3] ;

/* can we extend "nettime" for one or more 136 year chunks? */

	while (net < NETTIMEROLL) net += 0x100000000ULL ;

	rtime = (net - netoff) ;

#if	CF_DEBUGS
	debugprintf("gettime_inet: net=%llu\n",net) ;
	debugprintf("gettime_inet: off=%llu\n",netoff) ;
	debugprintf("gettime_inet: rti=%llu\n",rtime) ;
#endif

	return rtime ;
}
/* end subroutine (gettime_inet) */


static uint64_t utime_timeval(struct timeval *tvp)
{
	uint64_t	r ;

#if	CF_DEBUGS
	debugprintf("utime_timeval: i-sec=%lu\n",tvp->tv_sec) ;
	debugprintf("utime_timeval: u-usec=%lu\n",tvp->tv_usec) ;
#endif

	r = 1000000 ;
	r *= tvp->tv_sec ;
	r += tvp->tv_usec ;

#if	CF_DEBUGS
	debugprintf("utime_timeval: r=%llu\n",r) ;
#endif

	return r ;
}
/* end subroutine (utime_timeval) */


static uint64_t utime_tcpcalc(uint64_t uti2,uint64_t uti1)
{
	uint64_t	utid ;
	uint64_t	r ;
	double		d ;

	utid = uti2 - uti1 ;
	d = utid ;
	d = ((d * 3) / 4) ;		/* formula for TCP */
	utid = d ;

	r = uti1 + utid ;
	return r ;
}
/* end subroutine (utime_tcpcalc) */


static uint64_t utime_udpcalc(uint64_t uti2,uint64_t uti1)
{
	uint64_t	utid ;
	uint64_t	r ;
	double		d ;

	utid = uti2 - uti1 ;
	d = utid ;
	d = (d / 2) ;		/* formula for UDP */
	utid = d ;

	r = uti1 + utid ;
	return r ;
}
/* end subroutine (utime_udpcalc) */


static int tv_loadusec(struct timeval *tvp,int64_t uti)
{

	if (tvp == NULL) return SR_FAULT ;

	tvp->tv_sec = (uti / 1000000) ;
	tvp->tv_usec = (uti % 1000000) ;
	return SR_OK ;
}
/* end subroutine (tv_loadusec) */


static int vechand_already(VECHAND *alp,void *aip)
{
	int		rs ;
	int		i ;
	void		*ep ;
	for (i = 0 ; (rs = vechand_get(alp,i,&ep)) >= 0 ; i += 1) {
	    if (ep != NULL) {
	        if (ep == aip) break ;
	    }
	} /* end for */
	return rs ;
}
/* end subroutine (vechand_already) */


static int connagain(int rs)
{
	int	i ;
	int	f = FALSE ;
	for (i = 0 ; connagains[i] != 0 ; i += 1) {
	    f = (rs == connagains[i]) ;
	    if (f) break ;
	}
	return f ;
}
/* end subroutine (connagain) */


static int isaddrsame(const void *addr1,const void *addr2)
{
	SOCKADDRESS	*sa1p = (SOCKADDRESS *) addr1 ;
	SOCKADDRESS	*sa2p = (SOCKADDRESS *) addr2 ;
	uint		af1, af2 ;
	uint		p1, p2 ;
	int		rs = SR_OK ;
	int		addrlen ;
	int		f = FALSE ;
	char		addrbuf1[ADDRBUFLEN + 1] ;
	char		addrbuf2[ADDRBUFLEN + 1] ;

	if (rs >= 0) {
	    rs = sockaddress_getaf(sa1p) ;
	    af1 = rs ;
	}
	if (rs >= 0) {
	    rs = sockaddress_getaf(sa2p) ;
	    af2 = rs ;
	}
	if (rs < 0)
	    goto ret0 ;

	addrlen = sockaddress_getaddrlen(sa1p) ;

#if	CF_DEBUGS
	debugprintf("isaddrsame: addrlen=%u\n",addrlen) ;
#endif

	f = (af1 == af2) ;

#if	CF_DEBUGS
	debugprintf("isaddrsame: f=%u af1=%u af2=%u\n",f,af1,af2) ;
#endif

	if (f) {
	    p1 = sockaddress_getport(sa1p) ;
	    p2 = sockaddress_getport(sa2p) ;
	    f = (p1 == p2) ;

#if	CF_DEBUGS
	    debugprintf("isaddrsame: f=%u p1=%u p2=%u\n",f,p1,p2) ;
#endif

	}

	if (f) {
	    sockaddress_getaddr(sa1p,addrbuf1,ADDRBUFLEN) ;
	    sockaddress_getaddr(sa2p,addrbuf2,ADDRBUFLEN) ;
	    f = (memcmp(addrbuf1,addrbuf2,addrlen) == 0) ;
#if	CF_DEBUGS
	    debugprintf("isaddrsame: MEM f=%u\n",f) ;
#endif
	}

ret0:

#if	CF_DEBUGS
	debugprintf("isaddrsame: ret f=%u\n",f) ;
#endif

	return f ;
}
/* end subroutine (isaddrsame) */


#if	CF_DEBUGS

static int mkprintaddr(printaddr,printaddrlen,ssap)
char		printaddr[] ;
int		printaddrlen ;
struct sockaddr	*ssap ;
{
	SOCKADDRESS	*sap = (SOCKADDRESS *) ssap ;
	SBUF		b ;
	uint		af ;
	uint		port ;
	uint		flow ;
	uint		v ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		n, i ;
	int		addrlen = 0 ;
	int		f_flow = FALSE ;
	char		addrbuf[ADDRBUFLEN + 1] ;


	if (rs >= 0) {
	    rs = sockaddress_getlen(sap) ;
	    addrlen = rs ;
	}

	if (rs >= 0) {
	    rs = sockaddress_getaf(sap) ;
	    af = rs ;
	}

	if (rs >= 0) {
	    rs = sockaddress_getport(sap) ;
	    port = rs ;
	}

	if (rs >= 0) {
	    rs1 = sockaddress_getflow(sap,&flow) ;
	    f_flow = (rs1 >= 0) ;
	}

	if (rs >= 0) {
	    rs = sockaddress_getaddr(sap,addrbuf,ADDRBUFLEN) ;
	}

	if (rs >= 0) {
	    if ((rs = sbuf_start(&b,printaddr,printaddrlen)) >= 0) {

	        sbuf_deci(&b,af) ;
	        sbuf_char(&b,'-') ;

	        sbuf_deci(&b,port) ;
	        sbuf_char(&b,'-') ;

	        if (f_flow) {
	            sbuf_hexui(&b,flow) ;
	            sbuf_char(&b,'-') ;
	        }

	        n = (af == AF_INET6) ? 16 : 4 ;
	        for (i = 0 ; i < n ; i += 1) {

	            if ((i > 0) && ((i & 1) == 0)) {
	                sbuf_char(&b,':') ;
		    }

		    {
			const int	hlen = 10 ;
		        char		hbuf[10+1] ;
	                v = (addrbuf[i] & 0xff) ;
	                cthexui(hbuf,hlen,v) ;
	                rs = sbuf_strw(&b,(hbuf + 6),2) ;
		    }

		    if (rs < 0) break ;
	        } /* end for */

	        rs1 = sbuf_finish(&b) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (sbuf) */
	} /* end if (ok) */

	return (rs >= 0) ? addrlen : rs ;
}
/* end subroutine (mkprintaddr) */

static int mkprintscope(printaddr,printaddrlen,ssap)
char		printaddr[] ;
int		printaddrlen ;
struct sockaddr	*ssap ;
{
	SOCKADDRESS	*sap ;
	SBUF		b ;
	uint		af ;
	uint		scope, extra ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		addrlen = 0 ;
	int		f_scope = FALSE ;

	printaddr[0] = '\0' ;
	sap = (SOCKADDRESS *) ssap ;

	if (rs >= 0) {
	    rs = sockaddress_getlen(sap) ;
	    addrlen = rs ;
	}

	if (rs >= 0) {
	    rs = sockaddress_getaf(sap) ;
	    af = rs ;
	}

	if (rs >= 0) {
	    rs1 = sockaddress_getscope(sap,&scope) ;
	    f_scope = (rs1 >= 0) ;
	}

	if ((rs >= 0) && f_scope) {
	    rs = sockaddress_getextra(sap,&extra) ;
	}

	if ((rs >= 0) && f_scope) {
	    if ((rs = sbuf_start(&b,printaddr,printaddrlen)) >= 0) {
	        sbuf_hexui(&b,scope) ;
	        sbuf_char(&b,'-') ;
	        sbuf_hexui(&b,extra) ;
	        rs1 = sbuf_finish(&b) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (sbuf) */
	} /* end if (ok) */

	return (rs >= 0) ? addrlen : rs ;
}
/* end subroutine (mkprintscope) */

#endif /* CF_DEBUGS */


