/* uc_openproto */

/* interface component for UNIX® library-3c */
/* open a protocol */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_FINGERCLEAN	0		/* clean up FINGER output */
#define	CF_UNDERWORLD	0		/* print the "underworld" message */
#define	CF_EXPANDTAB	1		/* expand tabs (why do this?) */
#define	CF_TICOTSORD	0		/* compile in TICOTSORD */


/* revision history:

	= 1998-07-10, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	All protocol filenames have the format:

	/proto/<protoname>/<protoargs>

	where:

	<protoname>	protocol:
				ticotsord
				ticotsordnls
				ticotsordmux
				uss
				ussnls
				ussmux
				tcp
				tcpnls
				tcpmux
				udp
				usd
				finger
				http

	<protoargs>	arguments specific to a protocol

	- protocol		protocol-arguments

	ticotsord		/<addr>
	ticotsordnls[:<svc>]	/<addr>[/<svc>]
	ticotsordmux[:<svc>]	/<addr>[/<svc>]
	uss			<path>
	ussnls[:<svc>]		<path>
	ussmux[:<svc>]		<path>[­<arg(s)>]
	tcp			/<af>/<host>/<svc>
	tcpnls[:<port>]		/<af>/<host>[:<port>]/<svc>
	tcpmux[:<port>]		/<af>/<host>[:<port>]/<svc>­<arg(s)>
	udp[:<port>]		/<af>/<host>/<svc>
	usd			<path>
	finger[:<port>]		/<af>/<host>[:<port>]/<query>[+<ss>]­<arg(s)>
	http[:<port>]		/<af>/<host>/<path>

	where:

	<af>		address-family:
					unspec
					inet
					inet4
					inet6
	<host>		hostname of remote host to contact
	<svc>		service to contact
	<port>		INET-TCP or INET-UDP port
	<addr>		TLI address

	Examples:

	/proto/finger:5501/inet/rca/daytime
	/proto/tcpmux:5108/inet/rca/daytime
	/proto/tcp/inet/rca/daytime
	/proto/uss:daytime/tmp/local/tcpmuxd/srv
	/proto/http/inet/rca/robots.txt


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>
#include	<ctype.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<vecstr.h>
#include	<hostent.h>
#include	<sockaddress.h>
#include	<sbuf.h>
#include	<storeitem.h>
#include	<storebuf.h>
#include	<inetaddr.h>
#include	<filebuf.h>
#include	<linefold.h>
#include	<localmisc.h>

#include	"upt.h"


/* local defines */

#define	SUBINFO		struct subinfo

#define	INETARGS	struct inetargs

#define	TICOTSORDARGS	struct ticotsordargs

#define	FINGERARGS	struct fingerargs

#ifndef	AF_INET4
#define	AF_INET4	AT_INET
#endif

#define	BUFLEN		(3 * 1024)

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#ifndef	HEBUFLEN
#define	HEBUFLEN	(8 * 1024)
#endif

#ifndef	SVCLEN
#define	SVCLEN		MAXNAMELEN
#endif

#ifndef	REQBUFLEN
#define	REQBUFLEN	(4 * MAXPATHLEN)
#endif

#ifndef	POLLINTMULT
#define	POLLINTMULT	1000
#endif

#ifndef	PORTSPEC_FINGER
#define	PORTSPEC_FINGER	"finger"
#endif

#ifndef	CHAR_ISEND
#define	CHAR_ISEND(c)	(((c) == '\r') || ((c) == '\n'))
#endif

#undef	COLUMNS
#define	COLUMNS		80		/* display columns for FINGER */

#undef	COLBUFLEN
#define	COLBUFLEN	((2*COLUMNS) + 1) /* must be *twice* COLUMN length */

#undef	INDENT
#define	INDENT		2

#define	TO_DIAL		30
#define	TO_READ		(1*60)


/* external subroutines */

extern int	snwcpy(char *,int,cchar *,int) ;
extern int	sncpy1w(char *,int,cchar *,int) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matcasestr(const char **,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	mkcleanline(char *,int,int) ;
extern int	hasalldig(const char *,int) ;
extern int	filebuf_writeblanks(FILEBUF *,int) ;
extern int	sbuf_addquoted(SBUF *,const char *,int) ;

extern int	getproto_name(cchar *,int) ;
extern int	getserv_name(cchar *,cchar *) ;
extern int	getsocktype(int) ;

#if	CF_TICOTSORD
extern int	dialticotsord(const char *,int,int,int) ;
extern int	dialticotsordnls(const char *,int,const char *,int,int) ;
extern int	dialticotsordmux(const char *,int,const char *,const char **,
			int,int) ;
#endif /* CF_TICOTSORD */
extern int	dialuss(const char *,int,int) ;
extern int	dialussnls(const char *,const char *,int,int) ;
extern int	dialussmux(const char *,const char *,const char **,int,int) ;
extern int	dialtcp(const char *,const char *,int,int,int) ;
extern int	dialtcpnls(const char *,const char *,int,const char *,int,int) ;
extern int	dialtcpmux(cchar *,cchar *,int,cchar *,cchar **,int,int) ;
extern int	dialudp(const char *,const char *,int,int,int) ;
extern int	dialusd(const char *,int,int) ;
extern int	dialhttp(cchar *,cchar *,int,cchar *,cchar **,int,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strdcpy1(char *,int,const char *) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;
extern char	*strdcpy2w(char *,int,const char *,const char *,int) ;


/* local structures */

struct subinfo {
	const char	*path ;		/* caller supplied (path after proto) */
	const char	*pnp ;		/* caller supplied (protocol-name) */
	const char	*pap ;		/* caller supplied (protocol-args) */
	int		pnl ;
	int		pal ;
} ;

struct inetargs {
	const char	*protop ;	/* only specially used */
	const char	*afp ;
	const char	*hostp ;
	const char	*portp ;
	const char	*svcp ;
	const char	*a ;		/* memory allocation */
	vecstr		args ;
	int		protol ;
	int		afl ;
	int		hostl ;
	int		portl ;
	int		svcl ;
	int		f_args ;
} ;

struct ticotsordargs {
	STOREITEM	ss ;
	const char	*addr ;
	const char	*svc ;
	int		c ;
} ;

struct socktype {
	int		proto ;
	int		type ;
} ;

struct spacename {
	const char	*name ;
	int		af ;
} ;

struct fingerargs {
	int		nfd ;
	int		cfd ;
} ;


/* forward references */

static int	inetargs_start(INETARGS *,cchar *) ;
static int	inetargs_alloc(INETARGS *) ;
static int	inetargs_finish(INETARGS *) ;

static int	ticotsordargs_start(TICOTSORDARGS *,char *,int,cchar *,int) ;
static int	ticotsordargs_load(TICOTSORDARGS *,const char *,int) ;
static int	ticotsordargs_finish(TICOTSORDARGS *) ;

static int	openproto_ussmux(SUBINFO *,const char *,const char *,int,int) ;
static int	openproto_finger(SUBINFO *,const char *,int,int) ;
static int	openproto_http(SUBINFO *,const char *,int,int) ;
static int	openproto_ticotsord(SUBINFO *,int,int,int,int) ;

static int	inetargs(INETARGS *,const char *) ;
static int	dialinet(int,INETARGS *,int) ;

static int	loadargs(vecstr *,const char *) ;
static int	aflookup(const struct spacename *,const char *) ;
static int	sockshut(int,int) ;

static int	dialfinger(INETARGS *,const char *,int,cchar **,int,int) ;

#ifdef	CF_FINGERCLEAN
static int	fingerclean(int) ;
static int	fingerworker(FINGERARGS *) ;
static int	fingerworker_liner(FINGERARGS *,FILEBUF *,
			int,int,int,const char *,int) ;
#endif /* CF_FINGERCLEAN */

static int	getline(int,const char *,int) ;
static int	mkexpandtab(char *,int,int,const char *,int) ;

static int	hasBadOflags(int) ;

#if	CF_DEBUGS
static int makeint(const void *) ;
#endif


/* local variables */

static const char *protonames[] = {
	"ticotsord",
	"ticotsordnls",
	"ticotsordmux",
	"uss",
	"ussnls",
	"ussmux",
	"tcp",
	"tcpnls",
	"tcpmux",
	"usd",
	"udp",
	"finger",
	"http",
	"inet",
	"inet4",
	"inet6",
	NULL
} ;

enum protonames {
	protoname_ticotsord,
	protoname_ticotsordnls,
	protoname_ticotsordmux,
	protoname_uss,
	protoname_ussnls,
	protoname_ussmux,
	protoname_tcp,
	protoname_tcpnls,
	protoname_tcpmux,
	protoname_usd,
	protoname_udp,
	protoname_finger,
	protoname_http,
	protoname_inet,
	protoname_inet4,
	protoname_inet6,
	protoname_overlast
} ;

static const struct spacename	afspaces[] = {
	{ "unix", AF_UNIX },
	{ "inet", AF_INET },
	{ "inet4", AF_INET4 },
	{ "inet6", AF_INET6 },
	{ "unspec", AF_UNSPEC },
	{ NULL, 0 }
} ;

/* these protocol numbers are registered with ICANN (IANA) */
static const struct socktype	socktypes[] = {
	{ IPPROTO_TCP, SOCK_STREAM },
	{ IPPROTO_UDP, SOCK_DGRAM },
	{ IPPROTO_ICMP, SOCK_DGRAM },
	{ IPPROTO_EGP, SOCK_DGRAM },
	{ IPPROTO_GGP, SOCK_DGRAM },
	{ 0, 0 }
} ;


/* exported subroutines */


int uc_openproto(cchar *fname,int oflags,int to,int opts)
{
	SUBINFO		si ;
	int		rs = SR_OK ;
	int		pni ;
	int		pnl, pal ;
	const char	*path ;
	const char	*pnp, *pap ;
	const char	*tp ;
	const char	*sp ;
	const char	*portspec = NULL ;

#if	CF_DEBUGS
	debugprintf("uc_openproto: fname=%s\n",fname) ;
#endif

	if (fname == NULL)
	    return SR_FAULT ;

	if ((fname[0] == '\0') || (fname[1] == '\0'))
	    return SR_INVALID ;

	if (to < 0)
	    to = TO_DIAL ;

/* get the protocol name out of the filename */

	pnp = fname ;
	while (*pnp && (pnp[0] == '/'))
	    pnp += 1 ;

	if ((tp = strchr(pnp,'/')) == NULL) {
	    rs = SR_INVALID ;
	    goto ret0 ;
	}

	path = tp ;
	sp = (tp + 1) ;

/* is there a protocol argument? */

	pal = 0 ;
	if ((pap = strnchr(pnp,(tp - pnp),':')) != NULL) {
	    pnl = (pap - pnp) ;
	    pap += 1 ;
	    pal = (tp - pap) ;
	} else
	    pnl = (tp - pnp) ;

#if	CF_DEBUGS
	debugprintf("uc_openproto: path=%s\n",path) ;
	debugprintf("uc_openproto: protocol=%t\n",pnp,pnl) ;
	if (pal >= 0)
	    debugprintf("uc_openproto: protocol arg=%t\n",pap,pal) ;
#endif

	memset(&si,0,sizeof(SUBINFO)) ;
	si.path = path ;
	si.pnp = pnp ;
	si.pnl = pnl ;
	si.pap = pap ;
	si.pal = pal ;

/* lookup the protocol name */

	pni = matcasestr(protonames,pnp,pnl) ;

#if	CF_DEBUGS
	debugprintf("uc_openproto: protocol=%t(%u)\n",
	    pnp,pnl,pni) ;
#endif

	if (pni < 0) {

#if	CF_DEBUGS
	    debugprintf("uc_openproto: invalid protocol\n") ;
#endif

	    rs = SR_PROTONOSUPPORT ;
	    goto ret0 ;
	}

	switch (pni) {

	case protoname_ticotsord:
	case protoname_ticotsordnls:
	case protoname_ticotsordmux:
	    rs = openproto_ticotsord(&si,pni,oflags,to,opts) ;
	    break ;

	case protoname_tcp:
	case protoname_tcpnls:
	case protoname_tcpmux:
	case protoname_udp:
	    {
	        INETARGS	a ;

#if	CF_DEBUGS
	        debugprintf("uc_openproto: INET protocol=%s(%u)\n",
	            protonames[pni],pni) ;
#endif

	        if ((rs = inetargs_start(&a,sp)) >= 0) {
	            int		af ;
	            char	portbuf[SVCLEN + 1] ;

/* convert the address space name into a number */

	            rs = aflookup(afspaces,a.afp) ;
	            af = rs ;

#if	CF_DEBUGS
	            debugprintf("uc_openproto: "
	                "af=%u host=%s port=%s svc=%s\n",
	                af,a.hostp,a.portp,a.svcp) ;
#endif

/* perform any special pre-handling */

	            if (rs >= 0) {
	                switch (pni) {

	                case protoname_tcpnls:
	                case protoname_tcpmux:
	                    portspec = a.portp ;
	                    if ((portspec == NULL) && (pap != NULL)) {
	                        portspec = portbuf ;
	                        strdcpy1w(portbuf,SVCLEN,pap,pal) ;
#if	CF_DEBUGS
	                        debugprintf("uc_openproto: portspec=%s\n",
	                            portspec) ;
#endif
	                    }
	                    break ;

	                } /* end switch */
	            } /* end if */

/* continue with the dialing process */

#if	CF_DEBUGS
	            debugprintf("uc_openproto: dial() i=%d to=%d\n",
	                pni,to) ;
#endif

	            if (rs >= 0) {
	                const char	*hp = a.hostp ;
	                const char	*pp = portspec ;
	                const char	*sp = a.svcp ;

	                switch (pni) {

	                case protoname_udp:
	                    rs = dialudp(hp,sp,af,to,opts) ;
	                    break ;

	                case protoname_tcp:
	                    rs = dialtcp(hp,sp,af,to,opts) ;
#if	CF_DEBUGS
	                    debugprintf("uc_openproto: dialtcp() rs=%d\n",rs) ;
#endif
	                    break ;

	                case protoname_tcpmux:
	                    {
	                        const char	**av = NULL ;
	                        if (a.f_args) {
	                            rs = vecstr_getvec(&a.args,&av) ;
	                        }
	                        if (rs >= 0)
	                            rs = dialtcpmux(hp,pp,af,sp,av,to,opts) ;
	                    }
	                    break ;

	                case protoname_tcpnls:
	                    rs = dialtcpnls(hp,pp,af,sp,to,opts) ;
	                    break ;

	                } /* end switch */

	            } /* end if */

	            if (rs >= 0)
	                sockshut(rs,oflags) ;

	            inetargs_finish(&a) ;
	        } /* end if (inetargs) */

	    } /* end block */
	    break ;

	case protoname_usd:
	    if ((rs = dialusd(path,to,opts)) >= 0) {
	        sockshut(rs,oflags) ;
	    }
	    break ;

	case protoname_uss:
	    if ((rs = dialuss(path,to,opts)) >= 0) {
	        sockshut(rs,oflags) ;
	    }
	    break ;

	case protoname_ussnls:
	case protoname_ussmux:
	    {
	        char	svcspec[SVCLEN + 1] ;
	        if (pap != NULL) {
	            strdcpy1w(svcspec,SVCLEN,pap,pal) ;
#if	CF_DEBUGS
	            debugprintf("uc_openproto: USS path=%s\n",path) ;
	            debugprintf("uc_openproto: USS svc=%s\n",svcspec) ;
#endif
	            if (pni == protoname_ussmux) {
	                rs = openproto_ussmux(&si,path,svcspec,to,opts) ;
	            } else {
	                rs = dialussnls(path,svcspec,to,opts) ;
	            }
	    	    if (rs >= 0) {
			sockshut(rs,oflags) ;
	    	    }
	        } else
	            rs = SR_DESTADDRREQ ;
	    } /* end block */
	    break ;

	case protoname_finger:
	    rs = openproto_finger(&si,sp,oflags,to) ;
	    break ;

	case protoname_http:
	    rs = openproto_http(&si,sp,oflags,to) ;
	    break ;

	case protoname_inet:
	case protoname_inet4:
	case protoname_inet6:
	    {
	        INETARGS	a ;
	        if ((rs = inetargs(&a,sp)) >= 0) {
	            int		pf  ;
	            pf = (pni == protoname_inet6) ? PF_INET6 : PF_INET ;
	            rs = dialinet(pf,&a,to) ;
	        } /* end if */
	        if (rs >= 0) {
	            sockshut(rs,oflags) ;
	        }
	    } /* end block */
	    break ;

	default:
	    rs = SR_PROTONOSUPPORT ;
	    break ;

	} /* end switch (protocol name) */

ret0:

#if	CF_DEBUGS
	debugprintf("uc_openproto: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uc_openproto) */


/* local subroutines */


static int inetargs_start(INETARGS *iap,cchar *args)
{
	int		rs = SR_OK ;
	int		vopts ;
	const char	*tp, *sp ;

#if	CF_DEBUGS
	debugprintf("uc_openproto/inetargs_start: args=%s\n",args) ;
#endif

	memset(iap,0,sizeof(INETARGS)) ;

	sp = args ;
	if ((tp = strchr(sp,'/')) == NULL) {
	    rs = SR_INVALID ;
	    goto ret0 ;
	}

	iap->afp = sp ;
	iap->afl = (tp - sp) ;

	if (iap->afl == 0) {
	    rs = SR_INVALID ;
	    goto ret0 ;
	}

	sp = (tp + 1) ;
	if ((tp = strchr(sp,'/')) == NULL) {
	    rs = SR_INVALID ;
	    goto ret0 ;
	}

/* host-name */

	iap->hostp = sp ;
	iap->hostl = (tp - sp) ;

	if (iap->hostl == 0) {
	    rs = SR_INVALID ;
	    goto ret0 ;
	}

	sp = (tp + 1) ;

/* is there a port in this host name */

	{
	    const char	*pp ;

	    iap->portp = NULL ;
	    iap->portl = 0 ;
	    if ((pp = strnchr(iap->hostp,iap->hostl,':')) != NULL) {

	        iap->portp = (pp + 1) ;
	        iap->portl = (iap->hostp + iap->hostl) - (pp + 1) ;
	        iap->hostl = (pp - iap->hostp) ;

	    }

	} /* end block */

	vopts = VECSTR_OCOMPACT ;
	rs = vecstr_start(&iap->args,4,vopts) ;
	if (rs < 0)
	    goto bad0 ;

	iap->f_args = TRUE ;

/* service-name */

	iap->svcp = sp ;
	iap->svcl = -1 ;
	if ((tp = strchr(sp,0xAD)) != NULL) {

	    iap->svcl = (tp - sp) ;
	    rs = loadargs(&iap->args,(tp+1)) ;

	} /* end if (had dialer arguments) */

	if (rs < 0)
	    goto bad1 ;

	if (iap->svcl == 0) {
	    rs = SR_INVALID ;
	    goto bad1 ;
	}

	rs = inetargs_alloc(iap) ;
	if (rs < 0)
	    goto bad1 ;

ret0:

#if	CF_DEBUGS
	debugprintf("uc_openproto/inetargs_start: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff */
bad1:
	if (iap->f_args) {
	    iap->f_args = FALSE ;
	    vecstr_finish(&iap->args) ;
	}

bad0:
	goto ret0 ;
}
/* end subroutine (inetargs_start) */


static int inetargs_finish(INETARGS *iap)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (iap->a != NULL) {
	    rs1 = uc_free(iap->a) ;
	    iap->a = NULL ;
	    if (rs < 0) rs = rs1 ;
	}

	if (iap->f_args) {
	    iap->f_args = FALSE ;
	    rs1 = vecstr_finish(&iap->args) ;
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUGS
	debugprintf("uc_openproto/inetargs_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (inetargs_finish) */


static int inetargs_alloc(INETARGS *iap)
{
	int		rs = SR_OK ;
	int		size = 0 ;
	char		*bp ;

#ifdef	COMMENT
	if (iap->protop != NULL)
	    size += (strnlen(iap->protop,iap->protol) + 1) ;
#endif /* COMMENT */
	if (iap->afp != NULL)
	    size += (strnlen(iap->afp,iap->afl) + 1) ;
	if (iap->hostp != NULL)
	    size += (strnlen(iap->hostp,iap->hostl) + 1) ;
	if (iap->portp != NULL)
	    size += (strnlen(iap->portp,iap->portl) + 1) ;
	if (iap->svcp != NULL)
	    size += (strnlen(iap->svcp,iap->svcl) + 1) ;

	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    const char	*cp ;
	    iap->a = bp ;

#ifdef	COMMENT
	    if (iap->protop != NULL) {
	        cp = bp ;
	        bp = (strwcpy(bp,iap->protop,iap->protol) + 1) ;
	        iap->protop = cp ;
	    }
#endif /* COMMENT */
	    if (iap->afp != NULL) {
	        cp = bp ;
	        bp = (strwcpy(bp,iap->afp,iap->afl) + 1) ;
	        iap->afp = cp ;
	    }
	    if (iap->hostp != NULL) {
	        cp = bp ;
	        bp = (strwcpy(bp,iap->hostp,iap->hostl) + 1) ;
	        iap->hostp = cp ;
	    }
	    if (iap->portp != NULL) {
	        cp = bp ;
	        bp = (strwcpy(bp,iap->portp,iap->portl) + 1) ;
	        iap->portp = cp ;
	    }
	    if (iap->svcp != NULL) {
	        cp = bp ;
	        bp = (strwcpy(bp,iap->svcp,iap->svcl) + 1) ;
	        iap->svcp = cp ;
	    }

	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (inetargs_alloc) */


static int inetargs(INETARGS *iap,cchar *args)
{
	cchar		*sp, *cp ;

#if	CF_DEBUGS
	debugprintf("uc_open/inetargs: args=%s\n",args) ;
#endif

	sp = args ;
	if ((cp = strchr(sp,'/')) == NULL)
	    return SR_INVALID ;

	iap->protop = sp ;
	iap->protol = (cp - sp) ;

	if (iap->protol == 0)
	    return SR_INVALID ;

	sp = cp + 1 ;
	if ((cp = strchr(sp,'/')) == NULL)
	    return SR_INVALID ;

	iap->hostp = sp ;
	iap->hostl = (cp - sp) ;

	if (iap->hostl == 0)
	    return SR_INVALID ;

	sp = cp + 1 ;

/* trailing slash characters are now optional */

	iap->svcp = sp ;
	if ((cp = strchr(sp,'/')) != NULL) {
	    iap->svcl = (cp - sp) ;
	} else
	    iap->svcl = strlen(sp) ;

	if (iap->svcl == 0)
	    return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("uc_open/inetargs: ret OK\n") ;
#endif

	return SR_OK ;
}
/* end subroutine (inetargs) */


static int dialinet(pf,iap,to)
int		pf ;
INETARGS	*iap ;
int		to ;
{
	struct sockaddr	*sap ;
	HOSTENT		he, *hep ;
	SOCKADDRESS	server ;
	const in_addr_t	noaddr = (const in_addr_t) (-1) ;
	in_addr_t	addr ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		type ;
	int		af, salen ;
	int		s = -1 ;
	int		proto = -1 ;
	int		port = -1 ;
	int		f_address ;
	char		hostname[MAXHOSTNAMELEN + 1] ;
	char		svc[SVCLEN+1] ;
	char		hebuf[HEBUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("uc_open/dialinet: pf=%d host=%t svc=%t to=%d\n",
	    pf,
	    iap->hostp,iap->hostl,
	    iap->svcp,iap->svcl,
	    to) ;
#endif

/* get the protocol number */

	if ((rs >= 0) && isdigit(iap->protop[0])) {
	    rs1 = cfdeci(iap->protop,iap->protol,&proto) ;
	    if (rs1 < 0)
	        proto = -1 ;
	}

	if ((rs >= 0) && (proto < 0)) {
	    rs = getproto_name(iap->protop,iap->protol) ;
	    proto = rs ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("uc_open/dialinet: protospec=%t rs=%d proto=%d\n",
	    iap->protop,iap->protol,rs,proto) ;
#endif

	if ((rs >= 0) && (proto < 0))
	    rs = SR_NOPROTOOPT ;

	if (rs < 0)
	    goto ret0 ;

/* get the socket type that we need based on the protocol */

	rs = getsocktype(proto) ;
	type = rs ;
	if (rs < 0) goto ret0 ;

#if	CF_DEBUGS
	debugprintf("uc_open/dialinet: proto=%d type=%d\n",proto,type) ;
#endif

/* get the service */

#if	CF_DEBUGS
	debugprintf("uc_open/dialinet: svcspec=%t\n",iap->svcp,iap->svcl) ;
#endif

	if ((rs >= 0) && isdigit(iap->svcp[0])) {
	    rs1 = cfdeci(iap->svcp,iap->svcl,&port) ;
	    if (rs1 < 0) port = -1 ;
	}

#if	CF_DEBUGS
	debugprintf("uc_open/dialinet: 1 rs=%d port=%d\n",rs,port) ;
#endif

	if ((rs >= 0) && (port < 0)) {
	    const int	plen = 100 ;
	    char	pbuf[00+1] ;
	    strdcpy1w(pbuf,plen,iap->protop,iap->protol) ;
	    strdcpy1w(svc,SVCLEN,iap->svcp,iap->svcl) ;
	    rs = getserv_name(pbuf,svc) ;
#if	CF_DEBUGS
	debugprintf("uc_open/dialinet: getserv_name() rs=%d\n",rs) ;
#endif
	    port = rs ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("uc_open/dialinet: svcspec=%t rs=%d port=%d\n",
	    iap->svcp,iap->svcl,rs,port) ;
#endif

	if ((rs >= 0) && (port < 0))
	    rs = SR_NOPROTOOPT ; /* this really should indicate a bad port */

	if (rs < 0)
	    goto ret0 ;

/* get hostname */

	strwcpy(hostname,iap->hostp,MIN(MAXHOSTNAMELEN,iap->hostl)) ;

	f_address = TRUE ;
	af = AF_INET ;
	addr = inet_addr(hostname) ;
	if ((rs >= 0) && (addr == noaddr)) {

#if	CF_DEBUGS
	    debugprintf("uc_openproto/dialinet: trying 'gethe'\n",hostname) ;
#endif

	    f_address = FALSE ;
	    hep = &he ;
	    rs = uc_gethostbyname(hostname,hep,hebuf,HEBUFLEN) ;

	    if (rs == SR_NOTFOUND) {
	        rs = SR_HOSTUNREACH ;
	        goto badret ;
	    }

	    if ((rs >= 0) && (hep->h_addrtype != AF_INET)) {
	        rs = SR_HOSTUNREACH ;
	        goto badret ;
	    }

#ifdef	OPTIONAL
	    memcpy(&addr,hep->h_addr_list[0],sizeof(in_addr_t)) ;
#endif

	} /* end if (straight address or translation) */

	if (rs >= 0) {
	    rs = sockaddress_start(&server,af,&addr,port,0) ;
	    salen = rs ;
	}

	if (rs >= 0) {

#if	CF_DEBUGS
	    debugprintf("uc_openproto/diainet: server address retrieved\n") ;
#endif

/* create the socket that we want */

	    rs = u_socket(pf,type,proto) ;
	    s = rs ;
	    if (rs < 0) goto ret0 ;

/* try to connect to the remote machine */

#if	CF_DEBUGS
	    debugprintf("uc_openproto/dialinet: trying makeconn \n") ;
#endif

	    if (! f_address) {
	        hostent_cur	hc ;
	        const uchar	*straddr ;

	        if ((rs = hostent_curbegin(&he,&hc)) >= 0) {

	            while (hostent_enumaddr(&he,&hc,&straddr) >= 0) {

#if	CF_DEBUGS
	                {
	                    inetaddr	ia ;
	                    char		abuf[100] ;
	                    inetaddr_start(&ia,straddr) ;
	                    inetaddr_getdotaddr(&ia,abuf,100) ;
	                    debugprintf("uc_openproto/dialinet: "
	                        "addr=%s (\\x%08x)\n",
	                        abuf,makeint(straddr)) ;
	                    inetaddr_finish(&ia) ;
	                }
#endif /* CF_DEBUGS */

	                sockaddress_putaddr(&server,straddr) ;

	                sap = (struct sockaddr *) &server ;
	                rs = uc_connecte(s,sap,salen,to) ;
	                if (rs >= 0)
	                    break ;

	            } /* end while (looping through available addresses) */

	            hostent_curend(&he,&hc) ;
	        } /* end if (hostent-cursor) */

	    } else {
	        sap = (struct sockaddr *) &server ;
	        rs = uc_connecte(s,sap,salen,to) ;
	    }

	    sockaddress_finish(&server) ;
	} /* end if (sockaddress) */

	if ((rs < 0) && (s >= 0))
	    u_close(s) ;

ret0:
badret:

#if	CF_DEBUGS
	debugprintf("uc_openproto/dialinet: ret rs=%d fd=%d\n",rs,s) ;
#endif

	return (rs >= 0) ? s : rs ;
}
/* end subroutine (dialinet) */


static int ticotsordargs_start(tap,abuf,alen,pp,pl)
TICOTSORDARGS	*tap ;
char		*abuf ;
int		alen ;
const char	*pp ;
int		pl ;
{
	int		rs ;
	memset(tap,0,sizeof(TICOTSORDARGS)) ;
	if (pl < 0) pl = strlen(pp) ;
	if ((rs = storeitem_start(&tap->ss,abuf,alen)) >= 0) {
	    const char	*tp ;
	    while (pl && (pp[0] == '/')) {
	        pp += 1 ;
	        pl -= 1 ;
	    }
	    while ((tp = strnchr(pp,pl,'/')) != NULL) {
	        rs = ticotsordargs_load(tap,pp,(tp-pp)) ;
	        pl -= ((tp+1)-pp) ;
	        pp = (tp+1) ;
	        if (rs < 0) break ;
	    } /* end while */
	    if ((rs >= 0) && pl)
	        rs = ticotsordargs_load(tap,pp,pl) ;
	    if (rs >= 0)
	        rs = storeitem_getlen(&tap->ss) ;
	} /* end if (storeitem) */
	return rs ;
}
/* end subroutine (ticotsordargs_start) */


static int ticotsordargs_finish(tap)
TICOTSORDARGS	*tap ;
{
	return storeitem_finish(&tap->ss) ;
}
/* end subroutine (ticotsordargs_finish) */


static int ticotsordargs_load(tap,sp,sl)
TICOTSORDARGS	*tap ;
const char	*sp ;
int		sl ;
{
	int		rs = SR_OK ;
	if (sl > 0) {
	    switch (tap->c++) {
	    case 0:
	        rs = storeitem_strw(&tap->ss,sp,sl,&tap->addr) ;
	        break ;
	    case 1:
	        rs = storeitem_strw(&tap->ss,sp,sl,&tap->svc) ;
	        break ;
	    } /* end switch */
	} /* end if (non-zero-length string) */
	return rs ;
}
/* end subroutine (ticotsordargs_load) */


#if	CF_TICOTSORD
static int openproto_ticotsord(sip,pni,of,to,opts)
SUBINFO		*sip ;
int		pni ;
int		of ;
int		to ;
int		opts ;
{
	vecstr		args ;
	int		rs = SR_OK ;
	int		pl = -1 ;
	int		f_args = FALSE ;
	const char	*tp ;
	const char	*pp = sip->path ;
	const char	**av = NULL ;

	if ((tp = strchr(sip->path,0xAD)) != NULL) {
	    f_args = TRUE ;
	    pp = sip->path ;
	    pl = (tp-sip->path) ;
	    if ((rs = vecstr_start(&args,4,0)) >= 0) {
	        if ((rs = loadargs(&args,(tp+1))) >= 0)
	            rs = vecstr_getvec(&args,&av) ;
	        if (rs < 0) {
	            f_args = FALSE ;
	            vecstr_finish(&args) ;
	        }
	    } /* end if (vecstr) */
	} /* end if (arguments) */

	if (rs >= 0) {
	    TICOTSORDARGS	targs, *tap = &targs ;
	    const int	alen = MAXPATHLEN ;
	    char	abuf[MAXPATHLEN + 1] ;
	    if ((rs = ticotsordargs_start(&targs,abuf,alen,pp,pl)) >= 0) {
	        const char	*svc = tap->svc ;
	        const int	ai = rs ;
	        if (tap->svc == NULL) {
	            const char	*pap = sip->pap ;
	            int		pal = sip->pal ;
	            svc = (abuf+ai) ;
#ifdef	COMMENT
	            rs = storebuf_strw(abuf,alen,ai,pap,pal) ;
#else
	            rs = snwcpy((abuf+ai),(alen-ai),pap,pal) ;
#endif /* COMMENT */
	        }
#if	CF_DEBUGS
	        debugprintf("openproto_ticotsord: addr=>%s<\n",tap->addr) ;
	        debugprintf("openproto_ticotsord: svc=>%s<\n",svc) ;
	        debugprintf("openproto_ticotsord: pni=%u\n",pni) ;
#endif
	        if (rs >= 0) {
	            switch (pni) {
	            case protoname_ticotsord:
	                rs = dialticotsord(tap->addr,-1,to,opts) ;
	                break ;
	            case protoname_ticotsordnls:
	                rs = dialticotsordnls(tap->addr,-1,svc,to,opts) ;
	                break ;
	            case protoname_ticotsordmux:
	                rs = dialticotsordmux(tap->addr,-1,svc,av,to,opts) ;
	                break ;
	            } /* end if (switch */
	        } /* end if */
	        ticotsordargs_finish(&targs) ;
	    } /* end if (ticotsordargs) */
	} /* end if */

	if (f_args)
	    vecstr_finish(&args) ;

#if	CF_DEBUGS
	debugprintf("openproto_ticotsord: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (openproto_ticotsord) */
#else /* CF_TICOTSORD */
/* ARGSUSED */
static int openproto_ticotsord(sip,pni,of,to,opts)
SUBINFO		*sip ;
int		pni ;
int		of ;
int		to ;
int		opts ;
{
	return SR_NOSYS ;
}
#endif /* CF_TICOTSORD */


static int openproto_ussmux(sip,path,svcspec,to,opts)
SUBINFO		*sip ;
const char	path[] ;
const char	svcspec[] ;
int		to ;
int		opts ;
{
	vecstr		args ;
	int		rs = SR_OK ;
	int		f_args = FALSE ;
	const char	*tp ;
	const char	*pp = path ;
	const char	**av = NULL ;
	char		pathbuf[MAXPATHLEN + 1] ;

	if ((tp = strchr(path,0xAD)) != NULL) {
	    pp = pathbuf ;
	    rs = mkpath1w(pathbuf,path,(tp - path)) ;
	    f_args = TRUE ;
	}

	if (rs < 0)
	    goto ret0 ;

	if (f_args)
	    rs = vecstr_start(&args,4,0) ;

	if (rs < 0)
	    goto ret0 ;

	if (f_args) {

	    if ((rs = loadargs(&args,(tp+1))) >= 0)
	        rs = vecstr_getvec(&args,&av) ;

	} /* end if */

	if (rs >= 0) {
	    rs = dialussmux(pp,svcspec,av,to,opts) ;

#if	CF_DEBUGS
	    debugprintf("uc_openproto/openproto_ussmux: dialussmux() rs=%d\n",
	        rs) ;
#endif

	}

	if (f_args)
	    vecstr_finish(&args) ;

ret0:
	return rs ;
}
/* end subroutine (openproto_ussmux) */


static int openproto_finger(sip,sp,oflags,to)
SUBINFO		*sip ;
const char	sp[] ;
int		oflags ;
int		to ;
{
	INETARGS	a ;
	int		rs ;
	int		fd = -1 ;

	if (hasBadOflags(oflags)) return SR_ROFS ;

	if ((rs = inetargs_start(&a,sp)) >= 0) {
	    int		af ;
	    const char	*psp = NULL ;
	    char	portbuf[SVCLEN + 1] ;

/* convert the address space name into a number */

	    rs = aflookup(afspaces,a.afp) ;
	    af = rs ;

#if	CF_DEBUGS
	    debugprintf("uc_openproto: af=%u host=%s port=%s svc=%s\n",
	        af,a.hostp,a.portp,a.svcp) ;
#endif

	    if (rs >= 0) {
	        psp = a.portp ;
	        if ((psp == NULL) || (psp[0] == '\0')) {
	            if (sip->pap != NULL) {
	                psp = portbuf ;
	                strdcpy1w(portbuf,SVCLEN,sip->pap,sip->pal) ;
	            }
	        }
	    }

	    if (rs >= 0) {
	        const char	**av = NULL ;
	        if (a.f_args) {
	            rs = vecstr_getvec(&a.args,&av) ;
	        }
	        if (rs >= 0) {
	            rs = dialfinger(&a,psp,af,av,to,oflags) ;
	            fd = rs ;
	        }
	    }

	    inetargs_finish(&a) ;
	} /* end if (inetargs) */

#if	CF_FINGERCLEAN
	if (rs >= 0) {
	    int nfd ;
	    rs = fingerclean(fd) ;
	    nfd = rs ;
	    if (rs < 0) {
	        u_close(fd) ;
	        fd = -1 ;
	    } else
	        fd = nfd ;
	}
#endif /* CF_FINGERCLEAN */

	if ((rs < 0) && (fd >= 0)) u_close(fd) ;

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (openproto_finger) */


static int openproto_http(sip,sp,oflags,to)
SUBINFO		*sip ;
const char	sp[] ;
int		oflags ;
int		to ;
{
	INETARGS	a ;
	int		rs ;

	if (hasBadOflags(oflags)) return SR_ROFS ;

	if ((rs = inetargs_start(&a,sp)) >= 0) {
	    int	af ;

/* convert the address space name into a number */

	    rs = aflookup(afspaces,a.afp) ;
	    af = rs ;

#if	CF_DEBUGS
	    debugprintf("uc_openproto: af=%u host=%s port=%s svc=%s\n",
	        af,a.hostp,a.portp,a.svcp) ;
#endif

	    if (rs >= 0) {
	        const char	**av = NULL ;
	        if (a.f_args) {
	            rs = vecstr_getvec(&a.args,&av) ;
	        }
	        if (rs >= 0)
	            rs = dialhttp(a.hostp,a.portp,af,a.svcp,av,to,0) ;
	    }

	    inetargs_finish(&a) ;
	} /* end if (inetargs) */

	return rs ;
}
/* end subroutine (openproto_http) */


static int dialfinger(iap,psp,af,av,to,oflags)
INETARGS	*iap ;
const char	*psp ;
const char	**av ;
int		af ;
int		to ;
int		oflags ;
{
	const int	reqlen = REQBUFLEN ;
	int		rs = SR_OK ;
	int		fd = 0 ;
	const char	*portspec = NULL ;
	char		reqbuf[REQBUFLEN + 1], *rp = reqbuf ;

#if	CF_DEBUGS
	debugprintf("uc_openproto/dialfinger: ps=%s port=%t\n",
	    psp,iap->portp,iap->portl) ;
#endif

	if (portspec == NULL) {
	    if ((iap->portp != NULL) && (iap->portp[0] != '\0')) {
#ifdef	COMMENT
	        portspec = argbuf ;
	        rs = snwcpy(argbuf,arglen,iap->portp,iap->portl) ;
	        if (rs < 0) goto ret0 ;
#else
	        portspec = iap->portp ;
#endif /* COMMENT */
	    }
	}

	if (portspec == NULL) {
	    if (psp != NULL)
	        portspec = psp ;
	}

	if (portspec == NULL) {
	    portspec = PORTSPEC_FINGER ;
	}

	if (rs >= 0) {
	    if ((rs = dialtcp(iap->hostp,portspec,af,to,0)) >= 0) {
	        SBUF	b ;
	        int	rl ;
	        fd = rs ;
	        if ((rs = sbuf_start(&b,reqbuf,reqlen)) >= 0) {
	            sbuf_strw(&b,iap->svcp,iap->svcl) ;
	            if (oflags & O_NOCTTY) {
	                sbuf_strw(&b," /W",3) ;
	            }
	            if (iap->f_args) {
	                vecstr	*alp = &iap->args ;
	                int		i ;
	                const char	*ap ;
	                for (i = 0 ; vecstr_get(alp,i,&ap) >= 0 ; i += 1) {
	                    if (ap == NULL) continue ;
	                    rs = sbuf_char(&b,' ') ;
	                    if (rs >= 0) rs = sbuf_addquoted(&b,ap,-1) ;
	                    if (rs < 0) break ;
	                } /* end for */
	            }
	            if (rs >= 0)
	                sbuf_strw(&b,"\n\r",2) ;
	            rl = sbuf_finish(&b) ;
	            if (rs >= 0) rs = rl ;
	        } /* end if (sbuf) */
	        if (rs >= 0) {
#if	CF_DEBUGS
	            debugprintf("uc_openproto/dialfinger: q=>%t<\n",
	                rp,strlinelen(rp,rl,40)) ;
#endif
	            rs = uc_writen(fd,rp,rl) ;
	        }
	        if (rs >= 0)
	            rs = u_shutdown(fd,SHUT_WR) ;
	        if (rs < 0)
	            u_close(fd) ;
	    } /* end if (dialtcp) */
	} /* end if (ok) */

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (dialfinger) */


#if	CF_FINGERCLEAN

static int fingerclean(nfd)
int	nfd ;
{
	int		rs = SR_OK ;
	int		pfds[2] ;
	int		fd = -1 ;

	if ((rs = u_pipe(pfds)) >= 0) {
	    FINGERARGS	fa ;
	    PTA		ta ;
	    pthread_t	tid ;
	    fd = pfds[0] ; /* return read end */

	    memset(&fa,0,sizeof(FINGERARGS)) ;
	    fa.nfd = nfd ;
	    fa.cfd = pfds[1] ; /* pass down write end */

	    if ((rs = pta_create(&ta)) >= 0) {
	        int	v = PTHREAD_CREATE_DETACHED ;
	        if ((rs = pta_setdetachstate(&ta,v)) >= 0) {
	            int (*worker)(void *) = (int (*)(void *)) fingerworker ;
	            rs = uptcreate(&tid,&ta,worker,&fa) ;
	        }
	        pta_destroy(&ta) ;
	    } /* end if (pthread-attribute) */

	    if (rs < 0) {
	        int	i ;
	        for (i = 0 ; i < 2 ; i += 1) u_close(pfds[i]) ;
	    }
	} /* end if (pipe) */

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (fingerclean) */

static int fingerworker(fap)
FINGERARGS	*fap ;
{
	FILEBUF		fb ;
	FILEBUF		out, *ofp = &out ;
	const int	to = TO_READ ;
	const int	llen = LINEBUFLEN ;
	const int	cols = COLUMNS ;
	const int	ind = INDENT ;
	int		rs = SR_OK ;
	int		nfd = fap->nfd ;
	int		cfd = fap->cfd ;
	int		opts ;
	int		len ; /* length read from network */
	int		clen ; /* cleaned up length */
	int		ll, sl ;
	int		wlen = 0 ;
	const char	*lp ;
	const char	*sp ;
	char		lbuf[LINEBUFLEN + 1] ;
#ifdef	COMMENT
	char		colbuf[COLBUFLEN + 1] ;
#endif

	rs = filebuf_start(ofp,cfd,0L,0,0) ; /* write */
	if (rs < 0)
	    goto ret1 ;

	opts = FILEBUF_ONET ;
	rs = filebuf_start(&fb,nfd,0L,0,opts) ; /* read - network */
	if (rs < 0)
	    goto ret2 ;

#if	CF_UNDERWORLD
	{
	    const char *s = "hello from the underworld!\n" ;
	    rs = filebuf_print(ofp,s,-1) ;
	}
#endif /* CF_UNDERWORLD */

	if (rs >= 0) {
	    while ((rs = filebuf_readline(&fb,lbuf,llen,to)) > 0) {
	        len = rs ;
#if	CF_DEBUGS
	        {
	            debugprintf("uc_openproto: "
	                "filebuf_readline() len=%u eof=%u\n",
	                len,(lbuf[len-1]=='\n')) ;
	            debugprintf("uc_openproto: line=>%t<\n",
	                lbuf,strlinelen(lbuf,len,40)) ;
#ifdef	COMMENT
	            {
	                int	i ;
	                for (i = (len-1) ; i >= 0 ; i -= 1)
	                    debugprintf("uc_openproto: len=%u line=>%t<\n",
	                        i,lbuf,strlinelen(lbuf,i,40)) ;
	            }
#endif /* COMMENT */
	        }
#endif /* CF_DEBUGS */
	        clen = mkcleanline(lbuf,len,0) ;
#if	CF_DEBUGS
	        debugprintf("uc_openproto: clen=%u cline=>%t<\n",
	            clen,lbuf,strlinelen(lbuf,clen,40)) ;
#endif
	        if (clen > 0) {
	            LINEFOLD	lf ;
	            lp = lbuf ;
	            ll = clen ;
	            if ((rs = linefold_start(&lf,cols,ind,lp,ll)) >= 0) {
	                int	i = 0 ;
	                while ((sl = linefold_get(&lf,i,&sp)) >= 0) {
#if	CF_DEBUGS
	                    debugprintf("uc_openproto: get=%u line=>%t<\n",
	                        i,sp,strlinelen(sp,sl,40)) ;
#endif
#ifdef	COMMENT
	                    if ((sl == 0) || (sp[sl-1] != '\n')) {
	                        char *cp = colbuf ;
	                        if (sl > 0)
	                            cp = strdcpy1w(colbuf,collen,sp,sl) ;
	                        *cp++ = '\n' ;
	                        *cp = '\0' ;
	                        sp = colbuf ;
	                        sl = (cp-colbuf) ;
	                    }
#endif /* COMMENT */
	                    rs = fingerworker_liner(fap,ofp,cols,ind,i,sp,sl) ;
	                    wlen += rs ;
#if	CF_DEBUGS
	                    debugprintf("uc_openproto: "
	                        "fingerworlder_line() rs=%d\n",rs) ;
#endif
	                    i += 1 ;
	                    if (rs < 0) break ;
	                } /* end while */
	                linefold_finish(&lf) ;
	            } /* end if */
#if	CF_DEBUGS
	            debugprintf("uc_openproto: if-end rs=%d\n",rs) ;
#endif
	        } else {
	            rs = filebuf_print(ofp,lbuf,0) ;
	            wlen += rs ;
	        } /* end if (clen) */

	        if (rs < 0) break ;
	    } /* end while (reading lines) */
	} /* end if (ok) */
	*/

	    filebuf_finish(&fb) ;

ret2:
	filebuf_finish(&out) ;

ret1:
	u_close(nfd) ;
	u_close(cfd) ;

ret0:
	wlen &= INT_MAX ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (fingerworker) */

static int fingerworker_liner(fap,ofp,cols,ind,ln,sp,sl)
FINGERARGS	*fap ;
FILEBUF		*ofp ;
int		cols ;
int		ind ;
int		ln ;
const char	*sp ;
int		sl ;
{
	int		rs = SR_OK ;
	int		clen ;
	int		size ;
	int		gcols ;
	int		ncols ;
	int		icols ;
	int		nind ;
	int		ll, cl ;
	int		wlen = 0 ;
	const char	*tp ;
	const char	*lp, *cp ;
	char		*cbuf ;

	if (fap == NULL) return SR_FAULT ;

	if (cols <= 0) cols = 1 ;

	gcols = (ln == 0) ? cols : (cols-ind) ;
	icols = (ln == 0) ? 0 : ind ;

	clen = (2*cols) ;
	size = (clen+1) ;
	if ((rs = uc_malloc(size,&cbuf)) >= 0) {
	    int	i = 0 ;

	    ncols = gcols ;
	    nind = icols ;
	    while ((ll = getline(ncols,sp,sl)) >= 0) {
	        if ((ll == 0) && (i > 0)) break ;
	        lp = sp ;

#if	CF_DEBUGS
	        debugprintf("uc_openproto/fingerworker_liner: ol=>%t<\n",
	            lp,strlinelen(lp,ll,30)) ;
#endif

	        cp = lp ;
	        cl = ll ;
	        while (cl && CHAR_ISEND(cp[sl-1])) cl -= 1 ;

#if	CF_EXPANDTAB
	        if ((tp = strnchr(lp,ll,'\t')) != NULL) {
	            char	*bp = strwcpy(cbuf,lp,(tp-lp)) ;
	            int	bl ;
	            bl = (bp-cbuf) ;
	            rs = mkexpandtab(bp,(clen-bl),bl,tp,((lp+ll)-tp)) ;
	            cl = (rs + bl) ;
	            cp = cbuf ;
	        }
#endif /* CF_EXPANDTAB */

#if	CF_DEBUGS
	        debugprintf("uc_openproto/fingerworker_liner: el=>%t<\n",
	            cp,strlinelen(cp,cl,40)) ;
	        debugprintf("uc_openproto/fingerworker_liner: nind=%d\n",
	            nind) ;
#endif

	        if ((rs >= 0) && (nind > 0)) {
	            rs = filebuf_writeblanks(ofp,nind) ;
	            wlen += rs ;
	        }

	        if (rs >= 0) {
	            rs = filebuf_print(ofp,cp,cl) ;
	            wlen += rs ;
	        }

	        sl -= ((lp + ll) - sp) ;
	        sp = (lp + ll) ;

	        i += 1 ;
	        nind = (icols+ind) ;
	        ncols = (gcols-ind) ;
	        if (rs < 0) break ;
	    } /* end while */

	    uc_free(cbuf) ;
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("uc_openproto/fingerworker_liner: ret rs=%d wlen=%u\n",
	    rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (fingerworker_liner) */

#endif /* CF_FINGERCLEAN */


static int loadargs(alp,sp)
vecstr		*alp ;
const char	*sp ;
{
	int		rs = SR_OK ;
	int		c = 0 ;
	const char	*tp ;

	while ((tp = strchr(sp,0xAD)) != NULL) {
#if	CF_DEBUGS
	    debugprintf("uc_openproto/loadargs: a%u=>%t<\n",c,sp,(tp-sp)) ;
#endif
	    c += 1 ;
	    rs = vecstr_add(alp,sp,(tp - sp)) ;
	    sp = (tp+1) ;
	    if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && (sp[0] != '\0')) {
#if	CF_DEBUGS
	    debugprintf("uc_openproto/loadargs: a%u=>%t<\n",c,sp,-1) ;
#endif
	    c += 1 ;
	    rs = vecstr_add(alp,sp,-1) ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadargs) */


static int aflookup(afspaces,name)
const struct spacename	afspaces[] ;
const char		name[] ;
{
	int		rs = SR_OK ;
	int		j ;
	int		af = 0 ;

	for (j = 0 ; afspaces[j].name != NULL ; j += 1) {
	    if (strcmp(name,afspaces[j].name) == 0) break ;
	} /* end for */

	af = afspaces[j].af ;
	if (afspaces[j].name == NULL) {
	    rs = SR_AFNOSUPPORT ;
	    if (hasalldig(name,-1)) {
	        rs = cfdeci(name,-1,&af) ;
	        if (rs == SR_INVALID) rs = SR_AFNOSUPPORT ;
	    }
	}

	return (rs >= 0) ? af : rs ;
}
/* end subroutine (aflookup) */


static int sockshut(fd,oflags)
int		fd ;
int		oflags ;
{
	const int	am = (oflags & O_ACCMODE) ;
	int		rs = SR_OK ;

	switch (am) {
	case O_WRONLY:
	    rs = u_shutdown(fd,SHUT_RD) ;
	    break ;
	case O_RDONLY:
	    rs = u_shutdown(fd,SHUT_WR) ;
	    break ;
	} /* end switch */

	return rs ;
}
/* end subroutine (sockshut) */


static int getline(int linelen,cchar *sp,int sl)
{
	int		len = 0 ;

	if (sl > 0) {
	    const char	*tp ;
	    len = MIN(sl,linelen) ;
	    if ((tp = strnpbrk(sp,len,"\r\n")) != NULL) {
	        len = ((tp + 1) - sp) ;
	    }
	    if ((len > 0) && (len < sl) && CHAR_ISEND(sp[len])) {
	        len += 1 ;
	        if ((len < sl) && (sp[len-1] == '\r') && (sp[len] == '\n')) {
	            len += 1 ;
	        }
	    }
	} /* end if (non-zero) */

	return len ;
}
/* end subroutine (getline) */


static int mkexpandtab(char *dp,int dl,int ci,cchar *sp,int sl)
{
	SBUF		d ;
	int		rs ;
	int		len = 0 ;

	if ((rs = sbuf_start(&d,dp,dl)) >= 0) {
	    int		n, i ;

	    for (i = 0 ; (rs >= 0) && (i < sl) && *sp ; i += 1) {
	        if (sp[i] == '\t') {
	            n = (8 - (ci % 8)) ;
	            rs = sbuf_blanks(&d,n) ;
	            ci += (n-1) ;
	        } else
	            rs = sbuf_char(&d,sp[i]) ;
	        ci += 1 ;
	    } /* end for */

	    len = sbuf_finish(&d) ;
	    if (rs >= 0) rs = len ;
	} /* end if (sbuf) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mkexpandtab) */


/* DEBUG subroutines */


#if	CF_DEBUGS
static int makeint(const void *addr)
{
	int	hi = 0 ;
	uchar	*us = (uchar *) addr ;
	hi |= ((us[0] & 0xFF) << 24) ;
	hi |= ((us[1] & 0xFF) << 16) ;
	hi |= ((us[2] & 0xFF) << 8) ;
	hi |= ((us[3] & 0xFF) << 0) ;
	return hi ;
}
#endif /* CF_DEBUGS */


static int hasBadOflags(int of)
{
	int	f = FALSE ;
	f = f || (of & O_WRONLY) ;
	f = f || (of & O_RDWR) ;
	f = f || (of & O_CREAT) ;
	f = f || (of & O_TRUNC) ;
	return f ;
}
/* end subroutine (hasBadOflags) */


