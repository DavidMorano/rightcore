/* uc_openproto */

/* interface component for UNIX® library-3c */
/* open a protocol */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_FINGERCLEAN	1		/* FINGER clean */
#define	CF_FINGERBACK	0		/* FINGER clean in background */
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
#include	<netdb.h>

#include	<vsystem.h>
#include	<ascii.h>
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

extern int	snwcpyclean(char *,int,int,cchar *,int) ;
extern int	snwcpy(char *,int,cchar *,int) ;
extern int	sncpy1w(char *,int,cchar *,int) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matcasestr(const char **,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	mkcleanline(char *,int,int) ;
extern int	opentmp(cchar *,int,mode_t) ;
extern int	openshmtmp(char *,int,mode_t) ;
extern int	hasalldig(const char *,int) ;
extern int	isprintlatin(int) ;

extern int	filebuf_writeblanks(FILEBUF *,int) ;
extern int	sbuf_addquoted(SBUF *,const char *,int) ;

extern int	getaf(cchar *) ;

#if	CF_TICOTSORD
extern int	dialticotsord(cchar *,int,int,int) ;
extern int	dialticotsordnls(cchar *,int,cchar *,int,int) ;
extern int	dialticotsordmux(cchar *,int,cchar *,cchar **,int,int) ;
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

static int	openproto_ussmux(SUBINFO *,const char *,const char *,int,int) ;
static int	openproto_finger(SUBINFO *,const char *,int,int) ;
static int	openproto_http(SUBINFO *,const char *,int,int) ;
static int	openproto_ticotsord(SUBINFO *,int,int,int,int) ;
static int	openproto_inet(SUBINFO *,int,cchar *,int,int) ;

static int	inetargs_start(INETARGS *,cchar *) ;
static int	inetargs_starter(INETARGS *,cchar *) ;
static int	inetargs_alloc(INETARGS *) ;
static int	inetargs_finish(INETARGS *) ;

#if	CF_TICOTSORD
static int	ticotsordargs_start(TICOTSORDARGS *,char *,int,cchar *,int) ;
static int	ticotsordargs_load(TICOTSORDARGS *,const char *,int) ;
static int	ticotsordargs_finish(TICOTSORDARGS *) ;
#endif /* CF_TICOTSORD */

static int	loadargs(vecstr *,const char *) ;
static int	sockshut(int,int) ;

static int	dialfinger(INETARGS *,const char *,int,int,int) ;

#if	CF_FINGERCLEAN
static int	fingerclean(int) ;
#if	CF_FINERBACK
static int	fingerworker(FINGERARGS *) ;
static int	fingerworker_loop(FINGERARGS *,FILEBUF *,FILEBUF *,
int,int,int) ;
static int	fingerworker_liner(FINGERARGS *,FILEBUF *,
int,int,int,cchar *,int) ;
#endif /* CF_FINERBACK */
#endif /* CF_FINGERCLEAN */

#if	CF_FINGERCLEAN
#if	CF_FINGERBACK
static int	getline(int,const char *,int) ;
static int	mkexpandtab(char *,int,int,const char *,int) ;
#endif /* CF_FINGERBACK */
static int	hasdirty(cchar *,int) ;
static int	hasmseol(cchar *,int) ;
static int	isdirty(int) ;
#endif /* CF_FINGERCLEAN */

static int	hasBadOflags(int) ;

#if	CF_DEBUGS
static int makeint(const void *) ;
#endif


/* local variables */

static cchar	*protonames[] = {
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
	protoname_overlast
} ;


/* exported subroutines */


int uc_openproto(cchar *fname,int of,int to,int opts)
{
	SUBINFO		si ;
	int		rs = SR_OK ;
	int		pni ;
	int		pnl, pal ;
	int		fd = -1 ;
	const char	*path ;
	const char	*pnp, *pap ;
	const char	*tp ;
	const char	*sp ;

#if	CF_DEBUGS
	debugprintf("uc_openproto: fname=%s\n",fname) ;
#endif

	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

	if (to < 0)
	    to = TO_DIAL ;

/* get the protocol name out of the filename */

	pnp = fname ;
	while (*pnp && (pnp[0] == '/')) {
	    pnp += 1 ;
	}

	if ((tp = strchr(pnp,'/')) != NULL) {

	    path = tp ;
	    sp = (tp + 1) ;

/* is there a protocol argument? */

	    pal = 0 ;
	    if ((pap = strnchr(pnp,(tp - pnp),':')) != NULL) {
	        pnl = (pap - pnp) ;
	        pap += 1 ;
	        pal = (tp - pap) ;
	    } else {
	        pnl = (tp - pnp) ;
	    }

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
	    debugprintf("uc_openproto: protocol=%t(%u)\n",pnp,pnl,pni) ;
#endif

	    switch (pni) {
	    case protoname_ticotsord:
	    case protoname_ticotsordnls:
	    case protoname_ticotsordmux:
	        rs = openproto_ticotsord(&si,pni,of,to,opts) ;
	        fd = rs ;
	        break ;
	    case protoname_tcp:
	    case protoname_tcpnls:
	    case protoname_tcpmux:
	    case protoname_udp:
	        if ((rs = openproto_inet(&si,pni,sp,to,opts)) >= 0) {
	            fd = rs ;
	            rs = sockshut(fd,of) ;
	        }
	        break ;
	    case protoname_usd:
	        if ((rs = dialusd(path,to,opts)) >= 0) {
	            fd = rs ;
	            rs = sockshut(fd,of) ;
	        }
	        break ;
	    case protoname_uss:
	        if ((rs = dialuss(path,to,opts)) >= 0) {
	            fd = rs ;
	            rs = sockshut(fd,of) ;
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
	                    fd = rs ;
	                    rs = sockshut(fd,of) ;
	                }
	            } else
	                rs = SR_DESTADDRREQ ;
	        } /* end block */
	        break ;
	    case protoname_finger:
	        rs = openproto_finger(&si,sp,of,to) ;
	        fd = rs ;
	        break ;
	    case protoname_http:
	        rs = openproto_http(&si,sp,of,to) ;
	        fd = rs ;
	        break ;
	    default:
	        rs = SR_PROTONOSUPPORT ;
	        break ;
	    } /* end switch (protocol name) */
	    if ((rs < 0) && (fd >= 0)) u_close(fd) ;

	} else {
	    rs = SR_INVALID ;
	}

#if	CF_DEBUGS
	debugprintf("uc_openproto: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (uc_openproto) */


/* local subroutines */


static int openproto_inet(SUBINFO *sip,int pni,cchar *ap,int to,int no)
{
	INETARGS	a ;
	int		rs ;
	int		rs1 ;
	int		fd = -1 ;

#if	CF_DEBUGS
	debugprintf("uc_openproto: INET protocol=%s(%u)\n",
	    protonames[pni],pni) ;
#endif

	if ((rs = inetargs_start(&a,ap)) >= 0) {
#if	CF_DEBUGS
	    debugprintf("uc_openproto: ­%d­ af=%t\n",
	        a.afl,a.afp,a.afl) ;
#endif
	    if ((rs = getaf(a.afp)) >= 0) {
	        const int	af = rs ;
	        cchar		*hp = a.hostp ;
	        cchar		*sp = a.svcp ;

#if	CF_DEBUGS
	        debugprintf("uc_openproto: af=%t\n",a.afp,a.afl) ;
	        debugprintf("uc_openproto: host=%t\n",a.hostp,a.hostl) ;
	        debugprintf("uc_openproto: port=%t\n",a.portp,a.portl) ;
	        debugprintf("uc_openproto: svc=%t\n",a.svcp,a.svcl) ;
#endif

	        switch (pni) {
	        case protoname_udp:
	            rs = dialudp(hp,sp,af,to,no) ;
	            break ;
	        case protoname_tcp:
	            rs = dialtcp(hp,sp,af,to,no) ;
	            break ;
	        case protoname_tcpmux:
	        case protoname_tcpnls:
	            {
	                cchar	*ps ;
	                char	pbuf[SVCLEN + 1] ;
	                ps = a.portp ;
	                if ((ps == NULL) && (sip->pap != NULL)) {
	                    ps = pbuf ;
	                    strdcpy1w(pbuf,SVCLEN,sip->pap,sip->pal) ;
	                }
	                switch (pni) {
	                case protoname_tcpmux:
	                    {
	                        const char	**av = NULL ;
	                        if (a.f_args) {
	                            rs = vecstr_getvec(&a.args,&av) ;
	                        }
	                        if (rs >= 0) {
	                            rs = dialtcpmux(hp,ps,af,sp,av,to,no) ;
	                        }
	                    }
	                    break ;
	                case protoname_tcpnls:
	                    rs = dialtcpnls(hp,ps,af,sp,to,no) ;
	                    break ;
	                } /* end switch */
	            } /* end block */
	            break ;
	        } /* end switch */
	        fd = rs ;

	    } /* end if (getaf) */
	    rs1 = inetargs_finish(&a) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (inetargs) */
	if ((rs < 0) && (fd >= 0)) u_close(fd) ;

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (openproto_inet) */


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
	        if ((rs = loadargs(&args,(tp+1))) >= 0) {
	            rs = vecstr_getvec(&args,&av) ;
	        }
	        if (rs < 0) {
	            f_args = FALSE ;
	            vecstr_finish(&args) ;
	        }
	    } /* end if (vecstr) */
	} /* end if (arguments) */

	if (rs >= 0) {
	    TICOTSORDARGS	targs, *tap = &targs ;
	    const int		alen = MAXPATHLEN ;
	    char		abuf[MAXPATHLEN + 1] ;
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
	            } /* end if (switch) */
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


static int openproto_ussmux(SUBINFO *sip,cchar *path,cchar *svc,int to,int no)
{
	int		rs = SR_OK ;
	int		f_args = FALSE ;
	const char	*tp ;
	const char	*pp = path ;
	char		pbuf[MAXPATHLEN + 1] ;

	if (sip == NULL) return SR_FAULT ;

	if ((tp = strchr(path,0xAD)) != NULL) {
	    pp = pbuf ;
	    rs = mkpath1w(pbuf,path,(tp - path)) ;
	    f_args = TRUE ;
	}

	if (rs >= 0) {
	    vecstr	args ;
	    cchar	**av = NULL ;
	    if (f_args) {
	        if ((rs = vecstr_start(&args,4,0)) >= 0) {
	            if ((rs = loadargs(&args,(tp+1))) >= 0) {
	                rs = vecstr_getvec(&args,&av) ;
	            }
	        }
	    }
	    if (rs >= 0) {
	        rs = dialussmux(pp,svc,av,to,no) ;
	    }
	    if (f_args) {
	        vecstr_finish(&args) ;
	    }
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (openproto_ussmux) */


static int openproto_finger(SUBINFO *sip,cchar *sp,int of,int to)
{
	INETARGS	a ;
	int		rs ;
	int		rs1 ;
	int		fd = -1 ;

	if (hasBadOflags(of)) return SR_ROFS ;

	if ((rs = inetargs_start(&a,sp)) >= 0) {
	    if ((rs = getaf(a.afp)) >= 0) {
	        const int	af = rs ;
	        cchar		*psp = NULL ;
	        char		pbuf[SVCLEN + 1] ;
	        psp = a.portp ;
	        if ((psp == NULL) || (psp[0] == '\0')) {
	            if (sip->pap != NULL) {
	                psp = pbuf ;
	                strdcpy1w(pbuf,SVCLEN,sip->pap,sip->pal) ;
	            }
	        } /* end if (port-spec) */
	        if (rs >= 0) {
	            const char	**av = NULL ;
	            if (a.f_args) {
	                rs = vecstr_getvec(&a.args,&av) ;
	            }
	            if (rs >= 0) {
	                rs = dialfinger(&a,psp,af,to,of) ;
	                fd = rs ;
	            }
	        } /* end if (ok) */
	    } /* end if (getaf) */
	    rs1 = inetargs_finish(&a) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (inetargs) */

#if	CF_FINGERCLEAN
	if (rs >= 0) {
	    if ((rs = fingerclean(fd)) >= 0) {
	        u_close(fd) ;
	        fd = rs ;
	    }
	}
#endif /* CF_FINGERCLEAN */

	if ((rs < 0) && (fd >= 0)) u_close(fd) ;

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (openproto_finger) */


static int openproto_http(SUBINFO *sip,cchar *sp,int of,int to)
{
	INETARGS	a ;
	int		rs ;
	int		rs1 ;
	int		fd = -1 ;

	if (sip == NULL) return SR_FAULT ;

	if (hasBadOflags(of)) return SR_ROFS ;

	if ((rs = inetargs_start(&a,sp)) >= 0) {
	    if ((rs = getaf(a.afp)) >= 0) {
	        const int	af = rs ;
	        cchar		**av = NULL ;
	        if (a.f_args) {
	            rs = vecstr_getvec(&a.args,&av) ;
	        }
	        if (rs >= 0) {
	            rs = dialhttp(a.hostp,a.portp,af,a.svcp,av,to,0) ;
	            fd = rs ;
	        }
	    } /* end if (getaf) */
	    rs1 = inetargs_finish(&a) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (inetargs) */
	if ((rs < 0) && (fd >= 0)) u_close(fd) ;

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (openproto_http) */


static int inetargs_start(INETARGS *iap,cchar *args)
{
	int		rs = SR_OK ;
	const char	*tp, *sp ;

#if	CF_DEBUGS
	debugprintf("uc_openproto/inetargs_start: args=%s\n",args) ;
#endif

	memset(iap,0,sizeof(INETARGS)) ;

	sp = args ;
	if ((tp = strchr(sp,'/')) != NULL) {
	    iap->afp = sp ;
	    iap->afl = (tp - sp) ;
	    if (iap->afl > 0) {
	        sp = (tp + 1) ;
	        if ((tp = strchr(sp,'/')) != NULL) {
	            iap->hostp = sp ;
	            iap->hostl = (tp - sp) ;
	            if (iap->hostl > 0) {
	                cchar	*pp ;
	                cchar	*hp = iap->hostp ;
	                cchar	hl = iap->hostl ;
	                iap->portp = NULL ;
	                iap->portl = 0 ;
	                sp = (tp + 1) ;
	                if ((pp = strnchr(hp,hl,':')) != NULL) {
	                    iap->portp = (pp + 1) ;
	                    iap->portl = (hp + hl) - (pp + 1) ;
	                    iap->hostl = (pp - hp) ;
	                }
	                rs = inetargs_starter(iap,sp) ;
	            } else
	                rs = SR_INVALID ;
	        } else
	            rs = SR_INVALID ;
	    } else
	        rs = SR_INVALID ;
	} else
	    rs = SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("uc_openproto/inetargs_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (inetargs_start) */


static int inetargs_starter(INETARGS *iap,cchar *sp)
{
	const int	vo = VECSTR_OCOMPACT ;
	int		rs ;
	if ((rs = vecstr_start(&iap->args,4,vo)) >= 0) {
	    cchar	*tp ;
	    iap->f_args = TRUE ;
#if	CF_DEBUGS
	    debugprintf("uc_openproto/inetargs_start: args\n") ;
#endif
/* service-name */
	    iap->svcp = sp ;
	    iap->svcl = -1 ;
#if	CF_DEBUGS
	    debugprintf("uc_openproto/inetargs_start: svc=%t\n",
	        iap->svcp,iap->svcl) ;
#endif

	    if ((tp = strchr(sp,0xAD)) != NULL) {
	        iap->svcl = (tp - sp) ;
	        rs = loadargs(&iap->args,(tp+1)) ;
#if	CF_DEBUGS
	        debugprintf("uc_openproto/inetargs_start: "
	            "loadargs() rs=%d\n",rs) ;
#endif
	    } /* end if (had dialer arguments) */
	    if (rs >= 0) {
	        if (iap->svcp[0] != '\0') {
	            rs = inetargs_alloc(iap) ;
	        } else {
	            rs = SR_INVALID ;
	        }
	    }
	    if (rs < 0) {
	        if (iap->f_args) {
	            iap->f_args = FALSE ;
	            vecstr_finish(&iap->args) ;
	        }
	    }
	} /* end if (vecstr_start) */
	return rs ;
}
/* end subroutine (inetargs_starter) */


static int inetargs_finish(INETARGS *iap)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (iap->a != NULL) {
	    rs1 = uc_libfree(iap->a) ;
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
	if (iap->protop != NULL) {
	    size += (strnlen(iap->protop,iap->protol) + 1) ;
	}
#endif /* COMMENT */
	if (iap->afp != NULL) {
	    size += (strnlen(iap->afp,iap->afl) + 1) ;
	}
	if (iap->hostp != NULL) {
	    size += (strnlen(iap->hostp,iap->hostl) + 1) ;
	}
	if (iap->portp != NULL) {
	    size += (strnlen(iap->portp,iap->portl) + 1) ;
	}
	if (iap->svcp != NULL) {
	    size += (strnlen(iap->svcp,iap->svcl) + 1) ;
	}
	if ((rs = uc_libmalloc(size,&bp)) >= 0) {
	    cchar	*cp ;
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


#if	CF_TICOTSORD

static int ticotsordargs_start(TICOTSORDARGS *tap,char *abuf,int alen,
cchar *pp,int pl)
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


static int ticotsordargs_finish(TICOTSORDARGS *tap)
{
	return storeitem_finish(&tap->ss) ;
}
/* end subroutine (ticotsordargs_finish) */


static int ticotsordargs_load(TICOTSORDARGS *tap,cchar *sp,int sl)
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

#endif /* CF_TICOTSORD */


static int dialfinger(INETARGS *iap,cchar *psp,int af,int to,int of)
{
	const int	reqlen = REQBUFLEN ;
	int		rs = SR_OK ;
	int		fd = -1 ;
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
#else
	        portspec = iap->portp ;
#endif /* COMMENT */
	    }
	}

	if (portspec == NULL) {
	    if (psp != NULL) {
	        portspec = psp ;
	    }
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
	            if (of & O_NOCTTY) {
	                sbuf_strw(&b," /W",3) ;
	            }
	            if (iap->f_args) {
	                vecstr	*alp = &iap->args ;
	                int		i ;
	                const char	*ap ;
	                for (i = 0 ; vecstr_get(alp,i,&ap) >= 0 ; i += 1) {
	                    if (ap != NULL) {
	                        rs = sbuf_char(&b,' ') ;
	                        if (rs >= 0) rs = sbuf_addquoted(&b,ap,-1) ;
	                    }
	                    if (rs < 0) break ;
	                } /* end for */
	            } /* end if (args) */
	            if (rs >= 0) {
	                sbuf_strw(&b,"\n\r",2) ;
	            }
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
	        if (rs >= 0) {
	            rs = u_shutdown(fd,SHUT_WR) ;
	        }
	        if (rs < 0)
	            u_close(fd) ;
	    } /* end if (dialtcp) */
	} /* end if (ok) */

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (dialfinger) */


#if	CF_FINGERCLEAN
#if	CF_FINGERBACK
static int fingerclean(int nfd)
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

static int fingerworker(FINGERARGS *fap)
{
	FILEBUF		out, *ofp = &out ;
	const int	nfd = fap->nfd ;
	const int	cfd = fap->cfd ;
	const int	cols = COLUMNS ;
	const int	ind = INDENT ;
	const int	to = TO_READ ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if ((rs = filebuf_start(ofp,cfd,0L,0,0)) >= 0) {
	    FILEBUF	fb ;
	    const int	fbo = FILEBUF_ONET ;
	    if ((rs = filebuf_start(&fb,nfd,0L,0,fbo)) >= 0) {
	        {
	            rs = fingerworker_loop(fap,ofp,&fb,cols,ind,to) ;
	            wlen = rs ;
	        }
	        rs1 = filebuf_finish(&fb) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (filebuf) */
	    rs1 = filebuf_finish(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (filebuf) */

	u_close(nfd) ;
	u_close(cfd) ;

	wlen &= INT_MAX ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (fingerworker) */


static int fingerworker_loop(FINGERARGS *fap,FILEBUF *ofp,FILEBUF *ifp,
int cols,int ind,int to)
{
	const int	llen = LINEBUFLEN ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	char		*lbuf ;
	if ((rs = uc_malloc((llen+1),&lbuf)) >= 0) {
	    int		clen = 0 ;
	    int		ll, sl ;
	    cchar	*lp, *sp ;
	    cchar	*cp ;

	    while ((rs = filebuf_readline(ifp,lbuf,llen,to)) > 0) {
	        int	len = rs ;
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
	                        if (sl > 0) {
	                            cp = strdcpy1w(colbuf,collen,sp,sl) ;
	                        }
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

	    rs1 = uc_free(lbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (m-a-f) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (fingerworker_loop) */


static int fingerworker_liner(fap,ofp,cols,ind,ln,sp,sl)
FINGERARGS	*fap ;
FILEBUF		*ofp ;
int		cols ;
int		ind ;
int		ln ;
const char	*sp ;
int		sl ;
{
	int		rs ;
	int		clen ;
	int		size ;
	int		gcols ;
	int		icols ;
	int		wlen = 0 ;
	char		*cbuf ;

	if (fap == NULL) return SR_FAULT ;

	if (cols <= 0) cols = 1 ;

	gcols = (ln == 0) ? cols : (cols-ind) ;
	icols = (ln == 0) ? 0 : ind ;

	clen = (2*cols) ;
	size = (clen+1) ;
	if ((rs = uc_libmalloc(size,&cbuf)) >= 0) {
	    int		i = 0 ;
	    int		ll, cl ;
	    int		ncols = gcols ;
	    int		nind = icols ;
	    const char	*lp, *cp ;
	    const char	*tp ;

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

	    uc_libfree(cbuf) ;
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("uc_openproto/fingerworker_liner: ret rs=%d wlen=%u\n",
	    rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (fingerworker_liner) */
#else /* CF_FINGERBACK */
static int fingerclean(const int fd)
{
	const mode_t	om = 0664 ;
	int		rs ;
	int		rs1 ;
	int		nfd = -1 ;
	if ((rs = openshmtmp(NULL,0,om)) >= 0) {
	    const int	fo = FILEBUF_ONET ;
	    FILEBUF	b ;
	    nfd = rs ;
	    if ((rs = filebuf_start(&b,fd,0L,0,fo)) >= 0) {
	        const int	to = (1*60) ;
	        const int	llen = LINEBUFLEN ;
	        const int	clen = LINEBUFLEN ;
	        int		size = 0 ;
	        char		*bp ;
	        char		*lbuf ;
	        char		*cbuf ;
	        size += (llen+1) ;
	        size += (clen+1) ;
	        if ((rs = uc_libmalloc(size,&bp)) >= 0) {
	            lbuf = bp ;
	            cbuf = (bp+(llen+1)) ;
	            while ((rs = filebuf_readline(&b,lbuf,llen,to)) > 0) {
	                int	len = rs ;
	                if (hasmseol(lbuf,len)) {
	                    len -= 1 ;
	                    lbuf[len-1] = CH_NL ;
	                }
	                if (hasdirty(lbuf,len)) {
	                    const int	cl = clen ;
	                    char	*cp = cbuf ;
	                    if ((rs = snwcpyclean(cp,cl,'¿',lbuf,len)) >= 0) {
	                        len = rs ;
	                        lbuf = cbuf ;
	                    }
	                }
	                if ((rs >= 0) && (len > 0)) {
	                    rs = u_write(nfd,lbuf,len) ;
	                }
	                if (rs < 0) break ;
	            } /* end while */
	            uc_libfree(bp) ;
	        } /* end if (m-a) */
	        rs1 = filebuf_finish(&b) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (filebuf) */
	    if (rs >= 0) rs = u_rewind(nfd) ;
	    if (rs < 0) u_close(nfd) ;
	} /* end if (tmp-file) */
	return (rs >= 0) ? nfd : rs ;
}
/* end subroutine (fingerclean) */
#endif /* CF_FINGERBACK */
#endif /* CF_FINGERCLEAN */


static int loadargs(vecstr *alp,cchar *sp)
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


static int sockshut(int fd,int of)
{
	const int	am = (of & O_ACCMODE) ;
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


#if	CF_FINGERCLEAN

#if	CF_FINGERBACK

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
	        } else {
	            rs = sbuf_char(&d,sp[i]) ;
	        }
	        ci += 1 ;
	    } /* end for */

	    len = sbuf_finish(&d) ;
	    if (rs >= 0) rs = len ;
	} /* end if (sbuf) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mkexpandtab) */

#endif /* CF_FINGERBACK */

static int hasmseol(cchar *lp,int ll)
{
	int	f = FALSE ;
	if (ll >= 2) {
	    f = ((lp[ll-2] == CH_CR) && (lp[ll-1] == CH_NL)) ;
	}
	return f ;
}
/* end subroutine (hasmseol) */

static int hasdirty(cchar *lp,int ll)
{
	int		ch ;
	int		f = FALSE ;
	int		i ;
	for (i = 0 ; (i < ll) && (lp[i] != '\0')  ; i += 1) {
	    ch = MKCHAR(lp[i]) ;
	    f = isdirty(ch) ;
	    if (f) break ;
	} /* end for */
	return f ;
}
/* end subroutine (hasdirty) */

static int isdirty(int ch)
{
	int	f = FALSE ;
	f = f || isprintlatin(ch) ;
	f = f || (ch == CH_BEL) ;
	f = f || (ch == CH_BS) ;
	f = f || (ch == CH_TAB) ;
	f = f || (ch == CH_NL) ;
	return (! f) ;
}
/* end subroutine (isdirty) */

#endif /* CF_FINGERCLEAN */


/* DEBUG subroutines */


#if	CF_DEBUGS
static int makeint(const void *addr)
{
	int	hi = 0 ;
	uchar	*us = (uchar *) addr ;
	hi |= (MKCHAR(us[0]) << 24) ;
	hi |= (MKCHAR(us[1]) << 16) ;
	hi |= (MKCHAR(us[2]) << 8) ;
	hi |= (MKCHAR(us[3]) << 0) ;
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


