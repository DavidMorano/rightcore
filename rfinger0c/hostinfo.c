/* hostinfo */

/* manipulate host entry structures */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_HOSTBYNAME	1		/* use 'gethostbyname(3nsl)' */
#define	CF_FASTADDR	1		/* use fast-addr */


/* revision history:

	= 2005-02-03, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2005 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object provides the functionality that was previosuly supplied by
        the subroutine 'gethostbyname(3snl)'. That subroutine only worked for
        INET4 class addresses. This object returns host-entry information for
        both INET4 and INET6 class addresses.

	Notes:

	Q. What is this crazy order of evalations being done?
	A. Is is supposed to try to reduce the amount of work needed to find any
	   single result for a method call.


*******************************************************************************/


#define	HOSTINFO_MASTER		0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<vecobj.h>
#include	<hostent.h>
#include	<localmisc.h>

#include	"hostinfo.h"

#if	CF_DEBUGS
#include	"inetaddr.h"
#endif


/* local defines */

#define	HOSTINFO_N	struct hostinfo_n
#define	HOSTINFO_A	struct hostinfo_a

#ifndef	INET4ADDRLEN
#define	INET4ADDRLEN	sizeof(in_addr_t)
#endif

#ifndef	INET6ADDRLEN
#define	INET6ADDRLEN	16
#endif

#ifndef	INETXADDRLEN
#define	INETXADDRLEN	MAX(INET6ADDRLEN,INET4ADDRLEN)
#endif

#ifndef	INET4_ADDRSTRLEN
#define	INET4_ADDRSTRLEN	16
#endif

#ifndef	INET6_ADDRSTRLEN
#define	INET6_ADDRSTRLEN	46	/* Solaris® says that this is 46! */
#endif

#ifndef	INETX_ADDRSTRLEN
#define	INETX_ADDRSTRLEN	MAX(INET4_ADDRSTRLEN,INET6_ADDRSTRLEN)
#endif

#ifndef	LOCALDOMAINNAME
#define	LOCALDOMAINNAME		"local"
#endif


/* external subroutines */

extern int	snsds(char *,int,cchar *,cchar *) ;
extern int	snwcpy(char *,int,cchar *,int) ;
extern int	sncpy1(char *,int,cchar *) ;
extern int	nleadstr(cchar *,cchar *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	getaflen(int) ;
extern int	inetpton(char *,int,int,cchar *,int) ;
extern int	inetntop(char *,int,int,const void *) ;
extern int	isinetaddr(cchar *) ;
extern int	isindomain(cchar *,cchar *) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	heaudit(HOSTENT *,const void *,int) ;
#endif

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;


/* external variables */


/* local structures */

struct hostinfo_n {
	cchar		*name ;
	int		namelen ;
	int		af ;
} ;

struct hostinfo_a {
	int		af ;
	int		addrlen ;
	uchar		addr[INETXADDRLEN + 1] ;
} ;

struct known {
	cchar		*name ;
	uint		a ;
} ;


/* forward references */

static int	hostinfo_argsbegin(HOSTINFO *,uint,const char *) ;
static int	hostinfo_argsend(HOSTINFO *) ;
static int	hostinfo_domain(HOSTINFO *) ;
static int	hostinfo_findcanonical(HOSTINFO *) ;
static int	hostinfo_getname(HOSTINFO *,int,const char *) ;
static int	hostinfo_getaddr(HOSTINFO *,int) ;
static int	hostinfo_loadaddrs(HOSTINFO *,int,HOSTENT *) ;
static int	hostinfo_loadnames(HOSTINFO *,int,HOSTENT *) ;
static int	hostinfo_addname(HOSTINFO *,const char *,int,int) ;
static int	hostinfo_finishnames(HOSTINFO *) ;
static int	hostinfo_addrbegin(HOSTINFO *,int) ;
static int	hostinfo_addrend(HOSTINFO *) ;
static int	hostinfo_loadknownaddr(HOSTINFO *,int,uint) ;

static int	getinet(HOSTINFO *,int) ;
static int	getinet_straight(HOSTINFO *,int) ;
static int	getinet_add(HOSTINFO *,int) ;
static int	getinet_rem(HOSTINFO *,int) ;
static int	getinet_remlocal(HOSTINFO *,int) ;
static int	getinet_known(HOSTINFO *,int) ;

static int	matknown(const char *,int) ;
static int	vcmpname(const void *,const void *) ;
static int	vcmpaddr(const void *,const void *) ;

#if	CF_DEBUGS
static int debugprintaliases(const char *,struct hostent *) ;
static int debugprintinetaddr(const char *,int,const void *) ;
#endif


/* local variables */

static int	(*getinets[])(HOSTINFO *,int) = {
	getinet_rem,
	getinet_remlocal,
	getinet_straight,
	getinet_add,
	getinet_known,
	NULL
} ;

static const struct known	knowns[] = {
	{ "localhost", 0x7F000001 },
	{ "anyhost",   0x00000000 },
	{ "allhost",   0xFFFFFFFF },
	{ "broadcast", 0xFFFFFFFF },
	{ "testhost",  0x7F0000FF },
	{ "local1",    0x7F000001 },
	{ NULL,        0x00000000 }
} ;


/* exported subroutines */


int hostinfo_start(HOSTINFO *op,int af,cchar *hostname)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (hostname == NULL) return SR_FAULT ;

	if (hostname[0] == '\0') return SR_INVALID ;
	if (af < 0) return SR_INVALID ;

	memset(op,0,sizeof(HOSTINFO)) ;

#if	CF_DEBUGS
	debugprintf("hostinfo_start: af=%d hostname=%s\n",af,hostname) ;
#endif

/* load caller arguments */

	if ((rs = hostinfo_argsbegin(op,af,hostname)) >= 0) {
	    const int	vo = VECOBJ_OCOMPACT ;
	    int		osize = sizeof(struct hostinfo_n) ;
	    if ((rs = vecobj_start(&op->names,osize,10,vo)) >= 0) {
	        osize = sizeof(struct hostinfo_a) ;
	        if ((rs = vecobj_start(&op->addrs,osize,10,vo)) >= 0) {
	            rs = 0 ;
	            if ((af == AF_UNSPEC) || (af == AF_INET4)) {
	                rs = getinet(op,AF_INET4) ;
	            }
	            if ((rs == 0) && ((af == AF_UNSPEC) || (af == AF_INET6))) {
	                rs = getinet(op,AF_INET6) ;
	            }
	            if (rs >= 0) {
	                op->magic = HOSTINFO_MAGIC ;
	            } else { /* error */
	                if (op->domainname != NULL) {
	                    uc_free(op->domainname) ;
	                    op->domainname = NULL ;
	                }
	                hostinfo_addrend(op) ;
	                vecobj_finish(&op->addrs) ;
	            }
	        } /* end if (vecobj-addrs) */
	        if (rs < 0) {
	            hostinfo_finishnames(op) ;
	            vecobj_finish(&op->names) ;
	        }
	    } /* end if (vecobj-names) */
	    if (rs < 0) {
	        hostinfo_argsend(op) ;
	    }
	} /* end if (hostinfo_argsbegin) */

#if	CF_DEBUGS
	debugprintf("hostinfo_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (hostinfo_start) */


int hostinfo_finish(HOSTINFO *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != HOSTINFO_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("hostinfo_finish: ent hostname=%s\n",
	    op->arg.hostname) ;
#endif

	rs1 = hostinfo_addrend(op) ;
	if (rs >= 0) rs = rs1 ;

	if (op->domainname != NULL) {
	    rs1 = uc_free(op->domainname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->domainname = NULL ;
	}

	rs1 = hostinfo_finishnames(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecobj_finish(&op->addrs) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecobj_finish(&op->names) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = hostinfo_argsend(op) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("hostinfo_finish: ret rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (hostinfo_finish) */


int hostinfo_getoffical(HOSTINFO *op,cchar **rpp)
{
	int		rs = SR_OK ;
	int		nlen = 0 ;

#if	CF_DEBUGS
	debugprintf("hostinfo_getofficial: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;

	if (op->magic != HOSTINFO_MAGIC) return SR_NOTOPEN ;

	if (op->chostname[0] == '\0') {
	    rs = hostinfo_findcanonical(op) ;
	}

	if ((rs >= 0) && (op->chostname[0] != '\0')) {
	    nlen = strlen(op->chostname) ;
	    if (rpp != NULL) *rpp = op->chostname ;
	} /* end if */

	if ((rs < 0) && (rpp != NULL)) {
	    *rpp = NULL ;
	}

	return (rs >= 0) ? nlen : rs ;
}
/* end subroutine (hostinfo_getofficial) */


int hostinfo_geteffective(HOSTINFO *op,cchar **rpp)
{
	int		rs = SR_NOTFOUND ;
	int		nlen = 0 ;

#if	CF_DEBUGS
	debugprintf("hostinfo_geteffective: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;

	if (op->magic != HOSTINFO_MAGIC) return SR_NOTOPEN ;

	if (op->ehostname[0] != '\0') {
	    rs = SR_OK ;
	    nlen = strlen(op->ehostname) ;
	    if (rpp != NULL)
	        *rpp = op->ehostname ;
	} /* end if */

	if ((rs < 0) && (rpp != NULL))
	    *rpp = NULL ;

	return (rs >= 0) ? nlen : rs ;
}
/* end subroutine (hostinfo_geteffective) */


int hostinfo_getcanonical(HOSTINFO *op,cchar **rpp)
{
	int		rs = SR_OK ;
	int		nlen = 0 ;

#if	CF_DEBUGS
	debugprintf("hostinfo_getcannonical: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;

	if (op->magic != HOSTINFO_MAGIC) return SR_NOTOPEN ;

	if (op->chostname[0] == '\0') {
	    rs = hostinfo_findcanonical(op) ;
	}

#if	CF_DEBUGS
	debugprintf("hostinfo_getcannonical: ernest\n") ;
	debugprintf("hostinfo_getcannonical: ch=%s\n",op->chostname) ;
#endif

	if (rs >= 0) {
	    if (op->chostname[0] != '\0') {
	        nlen = strlen(op->chostname) ;
	        if (rpp != NULL) *rpp = op->chostname ;
	    } else {
	        rs = SR_NOTFOUND ;
	    }
	} /* end if */

	if ((rs < 0) && (rpp != NULL)) {
	    *rpp = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("hostinfo_getcannonical: ret rs=%d nlen=%u\n",rs,nlen) ;
#endif

	return (rs >= 0) ? nlen : rs ;
}
/* end subroutine (hostinfo_getcanonical) */


int hostinfo_curbegin(HOSTINFO *op,HOSTINFO_CUR *curp)
{

#if	CF_DEBUGS
	debugprintf("hostinfo_curbegin: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != HOSTINFO_MAGIC) return SR_NOTOPEN ;

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (hostinfo_curbegin) */


int hostinfo_curend(HOSTINFO *op,HOSTINFO_CUR *curp)
{

#if	CF_DEBUGS
	debugprintf("hostinfo_curend: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != HOSTINFO_MAGIC) return SR_NOTOPEN ;

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (hostinfo_curend) */


/* enumerate the next hostname */
int hostinfo_enumname(HOSTINFO *op,HOSTINFO_CUR *curp,cchar **rpp)
{
	HOSTINFO_CUR	dcur ;
	int		rs = SR_OK ;
	int		nlen = 0 ;
	int		f_cur = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != HOSTINFO_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("hostinfo_enumname: ent\n") ;
#endif

	if (rpp != NULL)
	    *rpp = NULL ;

	if (curp == NULL) {
	    curp = &dcur ;
	    f_cur = TRUE ;
	    rs = hostinfo_curbegin(op,&dcur) ;
	} /* end if (user supplied no cursor) */

	if (rs >= 0) {
	    VECOBJ	*nlp = &op->names ;
	    HOSTINFO_N	*nep = NULL ;
	    const int	rsn = SR_NOTFOUND ;
	    int		ci = (curp->i >= 0) ? (curp->i + 1) : 0 ;
	    int		f_exit = FALSE ;

	    repeat {
	        int	i ;
#if	CF_DEBUGS
	        debugprintf("hostinfo_enumname: while-top rs=%d\n",rs) ;
#endif
	        for (i = ci ; (rs = vecobj_get(nlp,i,&nep)) >= 0 ; i += 1) {
	            if (nep != NULL) break ;
	        } /* end for */
#if	CF_DEBUGS
	        debugprintf("hostinfo_enumname: for-out rs=%d\n",rs) ;
#endif
	        ci = i ;
	        if (rs == rsn) {
	            rs = SR_OK ;
	            if ((! op->f.inet4) || (! op->f.inet6)) {
	                if ((rs == 0) && (! op->f.inet4)) {
	                    rs = getinet(op,AF_INET4) ;
	                }
	                if ((rs == 0) && (! op->f.inet6)) {
	                    rs = getinet(op,AF_INET6) ;
	                }
#if	CF_DEBUGS
	                debugprintf("hostinfo_enumname: getinet() rs=%d\n",rs) ;
#endif
	                f_exit = (rs == 0) ;
	            } else {
	                f_exit = TRUE ;
	            }
	        } else if (rs == 0) {
	            rs = 1 ;
	        } /* end if */
#if	CF_DEBUGS
	        debugprintf("hostinfo_enumname: while-bot rs=%d f_exit=%u\n",
	            rs,f_exit) ;
#endif
	    } until ((rs != 0) || f_exit) ;

#if	CF_DEBUGS
	    debugprintf("hostinfo_enumname: out rs=%d\n",rs) ;
#endif

	    if (rs > 0) { /* found */
	        curp->i = ci ;
	        if (nep != NULL) {
	            nlen = nep->namelen ;
	            if (rpp != NULL) *rpp = nep->name ;
	        }
	    } else if (rs == 0) {
	        rs = SR_NOTFOUND ;
	    } /* end if */

	    if (f_cur) {
	        rs = hostinfo_curend(op,&dcur) ;
	    }
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("hostinfo_enumname: ret rs=%d nlen=%u\n",rs,nlen) ;
#endif

	return (rs >= 0) ? nlen : rs ;
}
/* end subroutine (hostinfo_enumname) */


/* enumerate the next host address */
int hostinfo_enumaddr(HOSTINFO *op,HOSTINFO_CUR *curp,cuchar **rpp)
{
	HOSTINFO_CUR	dcur ;
	int		rs = SR_OK ;
	int		alen = 0 ;
	int		f_cur = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != HOSTINFO_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("hostinfo_enumaddr: ent\n") ;
#endif

	if (rpp != NULL)
	    *rpp = NULL ;

	if (curp == NULL) {
	    curp = &dcur ;
	    f_cur = TRUE ;
	    rs = hostinfo_curbegin(op,&dcur) ;
	} /* end if (user supplied no cursor) */

	if (rs >= 0) {
	    VECOBJ	*alp = &op->addrs ;
	    HOSTINFO_A	*aep = NULL ;
	    const int	rsn = SR_NOTFOUND ;
	    int		ci = (curp->i >= 0) ? (curp->i + 1) : 0 ;
	    int		f_exit = FALSE ;

	    repeat {
	        int		i ;
	        for (i = ci ; (rs = vecobj_get(alp,i,&aep)) >= 0 ; i += 1) {
	            if (aep != NULL) break ;
	        } /* end for */
	        ci = i ;
	        if (rs == rsn) {
	            rs = SR_OK ;
	            if ((op->f.inet4) || (! op->f.inet6)) {
	                if ((rs == 0) && (! op->f.inet4)) {
	                    rs = getinet(op,AF_INET4) ;
	                }
	                if ((rs == 0) && (! op->f.inet6)) {
	                    rs = getinet(op,AF_INET6) ;
	                }
	                f_exit = (rs == 0) ;
	            } else {
	                f_exit = TRUE ;
	            }
	        } else if (rs == 0) {
	            rs = 1 ;
	        } /* end if */
	    } until ((rs != 0) || f_exit) ;

	    if (rs > 0) { /* found */
	        curp->i = ci ;
	        if (aep != NULL) {
	            alen = aep->addrlen ;
	            if (rpp != NULL)
	                *rpp = aep->addr ;
	        }
	    } else if (rs == 0) {
	        rs = SR_NOTFOUND ;
	    } /* end if */

	    if (f_cur) {
	        rs = hostinfo_curend(op,&dcur) ;
	    }
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("hostinfo_enumaddr: ret rs=%d alen=%u\n",rs,alen) ;
#endif

	return (rs >= 0) ? alen : rs ;
}
/* end subroutine (hostinfo_enumaddr) */


/* private subroutines */


static int hostinfo_argsbegin(HOSTINFO *op,uint af,cchar *name)
{
	int		rs = SR_OK ;
	int		nl ;
	int		f = FALSE ;

	if (name == NULL) return SR_FAULT ;

	if (name[0] == '\0') return SR_INVALID ;

	op->arg.af = af ;
	op->arg.hostname = name ;

	nl = strlen(name) ;
	while ((nl > 0) && (name[nl - 1] == '.')) {
	    f = TRUE ;
	    nl -= 1 ;
	}

	if (f) {
	    const char	*np ;
	    if ((rs = uc_mallocstrw(name,nl,&np)) >= 0) {
	        op->arg.f_alloc = TRUE ;
	        op->arg.hostnamelen = nl ;
	        op->arg.hostname = np ;
	    }
	}

	return (rs >= 0) ? f : rs ;
}
/* end if (hostinfo_argsbegin) */


static int hostinfo_argsend(HOSTINFO *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->arg.f_alloc && (op->arg.hostname != NULL)) {
	    rs1 = uc_free(op->arg.hostname) ;
	    if (rs >= 0) rs = rs1 ;
	}

	op->arg.f_alloc = FALSE ;
	op->arg.hostname = NULL ;
	return rs ;
}
/* end if (hostinfo_argsend) */


static int hostinfo_domain(HOSTINFO *op)
{
	int		rs = SR_OK ;
	int		len = 0 ;
	char		domainname[MAXHOSTNAMELEN + 1] ;

#if	CF_DEBUGS
	debugprintf("hostinfo_domain: ent\n") ;
#endif

	if (op->domainname == NULL) {
	    if ((rs = getnodedomain(NULL,domainname)) >= 0) {
	        const char	*dp ;
	        len = strlen(domainname) ;
	        rs = uc_mallocstrw(domainname,len,&dp) ;
	        if (rs >= 0) op->domainname = dp ;
	    }
	} else {
	    len = strlen(op->domainname) ;
	}

#if	CF_DEBUGS
	debugprintf("hostinfo_domain: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (hostinfo_domain) */


static int hostinfo_findcanonical(HOSTINFO *op)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("hostinfo_findcannonical: ent\n") ;
#endif

	if (op->chostname[0] == '\0') {
	    VECOBJ	*nlp = &op->names ;
	    HOSTINFO_N	*nep ;
	    const int	hlen = MAXHOSTNAMELEN ;
	    const int	rsn = SR_NOTFOUND ;
	    int		si = 0 ;
	    int		f_continue = TRUE ;

#if	CF_DEBUGS
	        debugprintf("hostinfo_findcannonical: searching\n") ;
#endif

	    while ((rs >= 0) && f_continue) {
	        int	i ;
#if	CF_DEBUGS
	        debugprintf("hostinfo_findcannonical: while-top rs=%d\n",rs) ;
#endif
	        for (i = si ; (rs = vecobj_get(nlp,i,&nep)) >= 0 ; i += 1) {
	            if (nep != NULL) {
#if	CF_DEBUGS
	        	debugprintf("hostinfo_findcannonical: name=%s\n",
			nep->name) ;
#endif
	                if (strchr(nep->name,'.') != NULL) {
	                    rs = sncpy1(op->chostname,hlen,nep->name) ;
	                    break ;
	                }
	            }
	        } /* end for */
#if	CF_DEBUGS
	        debugprintf("hostinfo_findcannonical: for-out rs=%d\n",rs) ;
	        debugprintf("hostinfo_findcannonical: f_inet4=%u\n",
	            op->f.inet4) ;
	        debugprintf("hostinfo_findcannonical: f_inet6=%u\n",
	            op->f.inet6) ;
#endif
	        si = i ;
	        if (rs == rsn) {
	            rs = SR_OK ;
	            if ((! op->f.inet4) || (! op->f.inet6)) {
	                if ((rs == 0) && (! op->f.inet4)) {
	                    rs = getinet(op,AF_INET4) ;
	                }
	                if ((rs == 0) && (! op->f.inet6)) {
	                    rs = getinet(op,AF_INET6) ;
	                }
	                f_continue = (rs > 0) ;
	            } else {
	                f_continue = FALSE ;
	            }
	        } else if (rs == 0) {
	            f_continue = FALSE ;
	            rs = 1 ;
	        } else if (rs > 0) {
	            f_continue = FALSE ;
	        } /* end if */
#if	CF_DEBUGS
	        debugprintf("hostinfo_findcannonical: while-bot rs=%d\n",rs) ;
#endif
	    } /* end while */

#if	CF_DEBUGS
	    debugprintf("hostinfo_findcannonical: mid1 rs=%d\n",rs) ;
	    debugprintf("hostinfo_findcannonical: eh=%s\n",op->ehostname) ;
#endif

	    if ((rs == 0) && (matknown(op->ehostname,-1) >= 0)) {
#if	CF_DEBUGS
	    debugprintf("hostinfo_findcannonical: known rs=%d\n",rs) ;
#endif
	        rs = vecobj_count(&op->addrs) ;
	    }

	    if (rs > 0) { /* found */
	        rs = 0 ;
	        if (strchr(op->ehostname,'.') == NULL) {
	            if ((rs = hostinfo_domain(op)) >= 0) {
	                cchar	*eh = op->ehostname ;
	                cchar	*dn = op->domainname ;
	                rs = snsds(op->chostname,hlen,eh,dn) ;
	            }
	        }
	        if (rs == 0) {
	            rs = sncpy1(op->chostname,hlen,op->ehostname) ;
	        }
	    } /* end if (desperate) */

#if	CF_DEBUGS
	    debugprintf("hostinfo_findcannonical: mid2 rs=%d\n",rs) ;
#endif

	} else {
	    rs = strlen(op->chostname) ;
	} /* end if (needed) */

#if	CF_DEBUGS
	debugprintf("hostinfo_findcannonical: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (hostinfo_findcanonical) */


static int hostinfo_getname(HOSTINFO *op,int af,cchar *name)
{
	HOSTENT		he, *hep = NULL ;
	const int	helen = getbufsize(getbufsize_he) ;
	int		rs ;
	int		f_inet4 ;
	int		c = 0 ;
	char		*hebuf ;

#if	CF_DEBUGS
	debugprintf("hostinfo_getname: ent af=%u n=>%s<\n",af,name) ;
#endif

#if	CF_HOSTBYNAME
	f_inet4 = (af == AF_INET4) ;
#else
	f_inet4 = FALSE ;
#endif /* CF_HOSTBYNAME */

	if ((rs = uc_malloc((helen+1),&hebuf)) >= 0) {
	    int		flags ;
	    int		f_alloc = FALSE ;

	    if (f_inet4) {

	        f_alloc = FALSE ;
	        hep = &he ;
	        rs = uc_gethostbyname(name,hep,hebuf,helen) ;

#if	CF_DEBUGS
	        debugprintf("hostinfo_getname: uc_gethostbyname() rs=%d\n",rs) ;
	        if (rs >= 0)
	            debugprintaliases("hostinfo_getname",hep) ;
	        if (rs >= 0) {
	            rs = heaudit(hep,hebuf,helen) ;
	            debugprintf("hostinfo_getname: heaudit() rs=%d\n",rs) ;
	        }
#endif /* CF_DEBUGS */

	    } else {

	        f_alloc = TRUE ;
	        flags = 0 ;
	        rs = uc_getipnodebyname(name,af,flags,&hep) ;

	    } /* end if */

#if	CF_DEBUGS
	    debugprintf("hostinfo_getname: af=%u n=>%s< rs=%d\n",af,name,rs) ;
#endif

	    if (rs >= 0) {
	        int		nl ;
	        const char	*np ;

#if	CF_DEBUGS
	        debugprintaliases("hostinfo_getname2",hep) ;
#endif /* CF_DEBUGS */

	        if ((rs = hostinfo_loadnames(op,af,hep)) >= 0) {
	            c = rs ;
	            rs = hostinfo_loadaddrs(op,af,hep) ;
	        }

	        if ((rs >= 0) && (op->ehostname[0] == '\0')) {
	            if ((rs = hostent_getofficial(hep,&np)) >= 0) {
	                const int	hlen = MAXHOSTNAMELEN ;
	                nl = rs ;
	                rs = snwcpy(op->ehostname,hlen,np,nl) ;
	            }
	        }

	        if (f_alloc && (hep != NULL)) {
	            uc_freehostent(hep) ;
	        }
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    } /* end if (got host-entry) */

	    uc_free(hebuf) ;
	} /* end if (m-a) */

#if	CF_DEBUGS
	debugprintf("hostinfo_getname: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (hostinfo_getname) */


static int hostinfo_getaddr(HOSTINFO *op,int af)
{
	int		rs = SR_NOTFOUND ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("hostinfo_getaddr: af=%u \n",af) ;
#endif

	if (op->f.addr && (af == op->addr.af)) {
	    HOSTENT	he, *hep ;
	    const int	helen = getbufsize(getbufsize_he) ;
	    int		f_inet4 ;
	    char	*hebuf ;

#if	CF_HOSTBYNAME
	    f_inet4 = (af == AF_INET4) ;
#else
	    f_inet4 = FALSE ;
#endif /* CF_HOSTBYNAME */

	    if ((rs = uc_malloc((helen+1),&hebuf)) >= 0) {
	        int	flags ;
	        cchar	*name = op->arg.hostname ;

	        if (f_inet4) {
	            const int	alen = op->addr.addrlen ;
	            const int	type = op->addr.af ;
	            cchar	*abuf = op->addr.addr ;

	            hep = &he ;
	            rs = uc_gethostbyaddr(abuf,alen,type,hep,hebuf,helen) ;

	        } else {

	            flags = 0 ;
	            rs = uc_getipnodebyname(name,af,flags,&hep) ;

	        } /* end if */

#if	CF_DEBUGS
	        debugprintf("hostinfo_getaddr: af=%u n=%s rs=%d\n",af,name,rs) ;
#endif

	        if (rs >= 0) {
	            int		nl ;
	            const char	*np ;

	            if ((rs = hostinfo_loadnames(op,af,hep)) >= 0) {
	                c = rs ;
	                rs = hostinfo_loadaddrs(op,af,hep) ;
	            }

	            if ((rs >= 0) && (op->ehostname[0] == '\0')) {
	                if ((rs = hostent_getofficial(hep,&np)) >= 0) {
			    const int	hlen = MAXHOSTNAMELEN ;
	                    nl = rs ;
	                    rs = snwcpy(op->ehostname,hlen,np,nl) ;
	                }
	            }

	            if (! f_inet4) {
	                uc_freehostent(hep) ;
	            }
	        } /* end if (got host-entry) */

	        uc_free(hebuf) ;
	    } /* end if (m-a) */

	} /* end if (enabled) */

#if	CF_DEBUGS
	debugprintf("hostinfo_getaddr: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (hostinfo_getaddr) */


static int hostinfo_loadaddrs(HOSTINFO *op,int af,HOSTENT *hep)
{
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("hostinfo_loadaddrs: ent af=%d\n",af) ;
#endif

	if ((rs = hostent_getalen(hep)) >= 0) {
	    HOSTENT_CUR	hc ;
	    HOSTINFO_A	a ;
	    const int	nrs = SR_NOTFOUND ;
	    const int	alen = rs ;
	    const uchar	*ap ;

	    memset(&a,0,sizeof(HOSTINFO_A)) ;
	    a.af = af ;
	    a.addrlen = alen ;

	    if ((rs = hostent_curbegin(hep,&hc)) >= 0) {
	        vecobj	*alp = &op->addrs ;
	        while ((rs1 = hostent_enumaddr(hep,&hc,&ap)) >= 0) {

#if	CF_DEBUGS
	            {
	                int daf = (rs1 == 4) ? AF_INET4 : AF_INET6 ;
	                debugprintinetaddr("hostinfo_loadaddrs: ",daf,ap) ;
	            }
#endif

	            a.addrlen = rs1 ;
	            memcpy(&a.addr,ap,rs1) ;

	            if ((rs = vecobj_search(alp,&a,vcmpaddr,NULL)) == nrs) {
	                c += 1 ;
	                a.af = af ;
	                rs = vecobj_add(alp,&a) ;
	            } /* end if (entry not found) */

	            if (rs < 0) break ;
	        } /* end while */
	        hostent_curend(hep,&hc) ;
	    } /* end if (hostent) */

	} /* end if */

#if	CF_DEBUGS
	debugprintf("hostinfo_loadaddrs: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (hostinfo_loadaddrs) */


static int hostinfo_loadnames(HOSTINFO *op,int af,HOSTENT *hep)
{
	HOSTENT_CUR	hc ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		nl ;
	int		c = 0 ;
	const char	*np ;

/* get the "official" name */

#if	CF_DEBUGS
	debugprintaliases("hostinfo_loadnames1",hep) ;
#endif

	if (rs >= 0) {
	    if ((rs1 = hostent_getofficial(hep,&np)) >= 0) {
	        nl = rs1 ;
#if	CF_DEBUGS
	        debugprintf("hostinfo_loadnames: nl=%u official=%s\n",nl,np) ;
#endif
	        rs = hostinfo_addname(op,np,nl,af) ;
	        c += rs ;
	    } /* end if */
	} /* end if */

/* get the "canonical" name */

#if	CF_DEBUGS
	debugprintaliases("hostinfo_loadnames2",hep) ;
#endif /* CF_DEBUGS */

	if (rs >= 0) {
	    if ((rs1 = hostent_getcanonical(hep,&np)) >= 0) {
	        nl = rs1 ;
#if	CF_DEBUGS
	        debugprintf("hostinfo_loadnames: nl=%u canonical=%s\n",nl,np) ;
#endif
	        rs = hostinfo_addname(op,np,nl,af) ;
	        c += rs ;
	    } /* end if */
	} /* end if */

/* get all of the "alias" name(s) */

#if	CF_DEBUGS
	debugprintaliases("hostinfo_loadnames3",hep) ;
#endif /* CF_DEBUGS */

	if (rs >= 0) {
	    if ((hostent_curbegin(hep,&hc)) >= 0) {
	        while ((nl = hostent_enumname(hep,&hc,&np)) > 0) {
#if	CF_DEBUGS
	            debugprintf("hostinfo_loadnames: nl=%u other=%s\n",nl,np) ;
#endif
	            rs = hostinfo_addname(op,np,nl,af) ;
	            c += rs ;
	            if (rs < 0) break ;
	        } /* end while */
	        hostent_curend(hep,&hc) ;
	    } /* end if (hostent) */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintaliases("hostinfo_loadnames4",hep) ;
#endif /* CF_DEBUGS */

#if	CF_DEBUGS
	debugprintf("hostinfo_loadnames: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (hostinfo_loadnames) */


static int hostinfo_addname(HOSTINFO *op,cchar *np,int nl,int af)
{
	HOSTINFO_N	n ;
	const int	nrs = SR_NOTFOUND ;
	int		rs = SR_OK ;
	int		c = 0 ;

	if (np == NULL) return SR_FAULT ;

	if (nl != 0) {

	    memset(&n,0,sizeof(HOSTINFO_N)) ;
	    n.af = af ;
	    n.namelen = nl ;
	    n.name = np ;

	    if ((rs = vecobj_search(&op->names,&n,vcmpname,NULL)) == nrs) {
	        cchar	*cp ;
	        if ((rs = uc_mallocstrw(np,nl,&cp)) >= 0) {
	            n.name = cp ;
	            c += 1 ;
	            rs = vecobj_add(&op->names,&n) ;
	        } /* end if (allocated) */
	    } /* end if (entry not found) */

	} /* end if (non-zero) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (hostinfo_addname) */


static int hostinfo_finishnames(HOSTINFO *op)
{
	HOSTINFO_N	*nep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; vecobj_get(&op->names,i,&nep) >= 0 ; i += 1) {
	    if (nep != NULL) {
	        if (nep->name != NULL) {
	            rs1 = uc_free(nep->name) ;
	            if (rs >= 0) rs = rs1 ;
	            nep->name = NULL ;
	        }
	    }
	} /* end for */

	return rs ;
}
/* end subroutine (hostinfo_finishnames) */


static int hostinfo_addrbegin(HOSTINFO *op,int af)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("hostinfo_addrbegin: ent af=%u\n",af) ;
#endif

	if (! op->init.addr) {
	    const int	ilen = INETXADDRLEN ;
	    int		inetaddrlen ;
	    char	inetaddr[INETXADDRLEN + 1] ;

	    op->init.addr = TRUE ;
	    rs1 = inetpton(inetaddr,ilen,af,op->arg.hostname,-1) ;

	    if (rs1 >= 0) {
	        const void	*vp ;

	        inetaddrlen = getaflen(rs1) ;

	        if ((rs = uc_mallocbuf(inetaddr,inetaddrlen,&vp)) >= 0) {
	            op->addr.addr = vp ;
	            op->f.addr = TRUE ;
	            op->addr.af = rs1 ;
	            op->addr.addrlen = inetaddrlen ;
	        } else {
	            op->addr.addr = NULL ;
	        }

	    } /* end if (allocating space for ADDR) */

	} /* end if */

#if	CF_DEBUGS
	debugprintf("hostinfo_addrbegin: ret rs=%d addr=%u\n",rs,op->f.addr) ;
#endif

	return (rs >= 0) ? op->f.addr : rs ;
}
/* end subroutine (hostinfo_addrbegin) */


static int hostinfo_addrend(HOSTINFO *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->addr.addr != NULL) {
	    rs1 = uc_free(op->addr.addr) ;
	    if (rs >= 0) rs = rs1 ;
	    op->addr.addr = NULL ;
	}

	return rs ;
}
/* end subroutine (hostinfo_addrend) */


static int hostinfo_loadknownaddr(HOSTINFO *op,int af,uint ka)
{
	HOSTINFO_A	a ;
	uint		na = htonl(ka) ;
	const int	nrs = SR_NOTFOUND ;
	const int	addrlen = getaflen(af) ;
	int		rs ;
	int		c = 0 ;

	memset(&a,0,sizeof(HOSTINFO_A)) ;
	a.af = af ;
	a.addrlen = addrlen ;
	memcpy(&a.addr,&na,addrlen) ;

	if ((rs = vecobj_search(&op->addrs,&a,vcmpaddr,NULL)) == nrs) {
	    c += 1 ;
	    rs = vecobj_add(&op->addrs,&a) ;
	} /* end if (entry not found) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (hostinfo_loadknownaddr) */


static int getinet(HOSTINFO *op,int af)
{
	int		rs = SR_OK ;
	int		c = 0 ; /* count-of-names */

#if	CF_DEBUGS
	debugprintf("hostinfo/getinet: ent af=%u n=%s\n",af,op->arg.hostname) ;
	debugprintf("hostinfo/getinet: arg-af=%u\n",op->arg.af) ;
#endif

	if (af > 0) {
	    if ((op->arg.af == AF_UNSPEC) || (op->arg.af == af)) {
	        int	i ;

	        switch (af) {
	        case AF_INET4:
	            op->f.inet4 = TRUE ; /* this says that we have done this */
	            break ;
	        case AF_INET6:
	            op->f.inet6 = TRUE ; /* this says that we have done this */
	            break ;
	        } /* end switch */

#if	CF_DEBUGS
	        debugprintf("hostinfo/getinet: mid1 rs=%d\n",rs) ;
#endif

	        for (i = 0 ; getinets[i] != NULL ; i += 1) {
#if	CF_DEBUGS
	            debugprintf("hostinfo/getinet: i=%u\n",i) ;
#endif
	            rs = (*getinets[i])(op,af) ;
	            c = rs ;
#if	CF_DEBUGS
	            debugprintf("hostinfo/getinet: i=%u rs=%d\n",i,rs) ;
#endif
	            if (rs != 0) break ;
	        } /* end for */

	    } /* end if */
	} /* end if (have AF) */

#if	CF_DEBUGS
	debugprintf("hostinfo/getinet: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (getinet) */


static int getinet_straight(HOSTINFO *op,int af)
{
	int		rs = SR_OK ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("hostinfo/getinet_straight: ent af=%u h=>%s<\n",
	    af,op->arg.hostname) ;
#endif

	if (! op->init.addr) {
	    rs = hostinfo_addrbegin(op,af) ;
	}

#if	CF_DEBUGS
	debugprintf("hostinfo/getinet_straight: f_a=%u\n",
	    op->f.addr) ;
#endif

	if (rs >= 0) {
	    if (op->f.addr && (af == op->addr.af)) {
	        rs = hostinfo_getaddr(op,af) ;
	    } else {
	        rs = hostinfo_getname(op,af,op->arg.hostname) ;
	    }
	    if ((rs > 0) && (op->ehostname[0] == '\0')) {
	        const int	hlen = MAXHOSTNAMELEN ;
	        c = 1 ;
	        rs = sncpy1(op->ehostname,hlen,op->arg.hostname) ;
	    }
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("hostinfo/getinet_straight: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (getinet_straight) */


/* try adding our own domain on the end if it does not already have one */
static int getinet_add(HOSTINFO *op,int af)
{
	int		rs = SR_OK ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("hostinfo/getinet_add: ent af=%u h=>%s<\n",
	    af,op->arg.hostname) ;
#endif

	if (strchr(op->arg.hostname,'.') == NULL) { /* nodename-only */
	    const int	hlen = MAXHOSTNAMELEN ;
	    int		f_continue = FALSE ;
	    char	hbuf[MAXHOSTNAMELEN + 1] ;

#if	CF_FASTADDR
	    if ((rs = hostinfo_addrbegin(op,af)) >= 0) {
	        f_continue = (! op->f.addr) ;
	    }
#else
	    f_continue = (! isinetaddr(op->arg.hostname)) ;
#endif

	    if ((rs >= 0) && f_continue) {
	        if ((rs = hostinfo_domain(op)) >= 0) {
	            cchar	*hn = op->arg.hostname ;
	            cchar	*dn = op->domainname ;
	            if ((rs = snsds(hbuf,hlen,hn,dn)) >= 0) {
	                if ((rs = hostinfo_getname(op,af,hbuf)) > 0) {
	                    if (op->ehostname[0] == '\0') {
	                        c = 1 ;
	                        rs = sncpy1(op->ehostname,hlen,hbuf) ;
	                    }
	                }
	            }
	        }
	    } /* end if (continue) */

	} /* end if (possible address) */

#if	CF_DEBUGS
	debugprintf("hostinfo/getinet_add: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (getinet_add) */


/* try removing our own domain from the end if it is the same as we */
static int getinet_rem(HOSTINFO *op,int af)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	int		f_continue = FALSE ;

#if	CF_DEBUGS
	debugprintf("getinet_rem: ent af=%u\n",af) ;
#endif

#if	CF_FASTADDR
	if ((rs = hostinfo_addrbegin(op,af)) >= 0) {
	    f_continue = (! op->f.addr) ;
	}
#else
	f_continue = (! isinetaddr(op->arg.hostname)) ;
#endif

#if	CF_DEBUGS
	debugprintf("getinet_rem: rs=%d f_continue=%u\n",rs,f_continue) ;
#endif

	if ((rs >= 0) && f_continue) {
	    cchar	*tp ;
	    if ((tp = strchr(op->arg.hostname,'.')) != NULL) {
	        if ((rs = hostinfo_domain(op)) >= 0) {
	            if (isindomain(op->arg.hostname,op->domainname)) {
	                const int	hlen = MAXHOSTNAMELEN ;
	                int		cl = (tp - op->arg.hostname) ;
	                cchar		*cp = op->arg.hostname ;
	                char		hbuf[MAXHOSTNAMELEN + 1] ;

#if	CF_DEBUGS
	                debugprintf("getinet_rem: c=%t\n",cp,cl) ;
#endif

	                if ((rs = snwcpy(hbuf,hlen,cp,cl)) >= 0) {
#if	CF_DEBUGS
	                    debugprintf("getinet_rem: _getname() af=%u n=%s\n",
	                        af,hbuf) ;
#endif
	                    if ((rs = hostinfo_getname(op,af,hbuf)) > 0) {
	                        if (op->ehostname[0] == '\0') {
				    c = 1 ;
	                            rs = sncpy1(op->ehostname,hlen,hbuf) ;
	                        }
	                    }
	                }

	            } /* end if (the requested hostname is in our domain) */
	        } /* end if (hostinfo_domain) */
	    } /* end if (possible something) */
	} /* end if (continue) */

#if	CF_DEBUGS
	debugprintf("getinet_rem: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (getinet_rem) */


/* try removing a "LOCAL" domain from the end */
static int getinet_remlocal(HOSTINFO *op,int af)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	int		f_continue = FALSE ;

#if	CF_DEBUGS
	debugprintf("getinet_remlocal: ent af=%u\n",af) ;
#endif

#if	CF_FASTADDR
	if ((rs = hostinfo_addrbegin(op,af)) >= 0) {
	    f_continue = (! op->f.addr) ;
	}
#else
	f_continue = (! isinetaddr(op->arg.hostname)) ;
#endif

	if ((rs >= 0) && f_continue) {
	    cchar	*tp ;
	    if ((tp = strchr(op->arg.hostname,'.')) != NULL) {
	        if (isindomain(op->arg.hostname,LOCALDOMAINNAME)) {
	            const int	hlen = MAXHOSTNAMELEN ;
	            int		hl = (tp - op->arg.hostname) ;
	            char	hbuf[MAXHOSTNAMELEN + 1] ;
	            if ((rs = snwcpy(hbuf,hlen,op->arg.hostname,hl)) >= 0) {
	                if ((rs = hostinfo_getname(op,af,hbuf)) > 0) {
	                    if (op->ehostname[0] == '\0') {
	                        c = 1 ;
	                        rs = sncpy1(op->ehostname,hlen,hbuf) ;
	                    }
	                }
	            }
	        } /* end if (the requested hostname is in our domain) */
	    } /* end if */
	} /* end if (continue) */

#if	CF_DEBUGS
	debugprintf("getinet_remlocal: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (getinet_remlocal) */


static int getinet_known(HOSTINFO *op,int af)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	int		f_continue = FALSE ;

#if	CF_DEBUGS
	debugprintf("getinet_known: ent af=%u\n",af) ;
#endif

#if	CF_FASTADDR
	if ((rs = hostinfo_addrbegin(op,af)) >= 0) {
	    f_continue = (! op->f.addr) ;
	}
#else
	f_continue = (! isinetaddr(op->arg.hostname)) ;
#endif

	if ((rs >= 0) && f_continue) {
	    if ((af == AF_UNSPEC) || (af == AF_INET4)) {
	        if ((rs = hostinfo_domain(op)) >= 0) {
	            const int	hlen = MAXHOSTNAMELEN ;
	            int		nl ;
	            int		i = -1 ;
	            cchar	*local = LOCALDOMAINNAME ;
	            cchar	*tp ;
	            cchar	*np = op->arg.hostname ;
	            char	hbuf[MAXHOSTNAMELEN + 1] ;

	            nl = strlen(np) ;
	            while ((nl > 0) && (np[nl - 1] == '.')) {
	                nl -= 1 ;
	            }

	            if ((tp = strnchr(np,nl,'.')) != NULL) {
	                const int	cl = ((np + nl) - (tp + 1)) ;
	                cchar		*cp = (tp+1) ;

	                strwcpy(hbuf,np,MIN(nl,NODENAMELEN)) ;

	                if (isindomain(hbuf,op->domainname) ||
	                    (strncmp(local,cp,cl) == 0)) {

	                    nl = (tp - np) ;
	                    if ((i = matknown(np,nl)) >= 0) {
	                        if (op->ehostname[0] == '\0') {
				    c = 1 ;
	                            rs = snwcpy(op->ehostname,hlen,np,nl) ;
	                        }
	                        if ((rs >= 0) && (op->chostname[0] == '\0')) {
	                            rs = sncpy1(op->chostname,hlen,hbuf) ;
	                        }
	                    }

	                } /* end if (hit) */

	            } else {

	                if ((i = matknown(np,nl)) >= 0) {
	                    if (op->ehostname[0] == '\0') {
				c = 1 ;
	                        rs = sncpy1(op->ehostname,hlen,np) ;
	                    }
	                    if ((rs >= 0) && (op->chostname[0] == '\0')) {
	                        cchar	*dn = op->domainname ;
	                        rs = snsds(op->chostname,hlen,np,dn) ;
	                    }
	                }

	            } /* end if */

	            if ((rs >= 0) && (i >= 0)) {
	                rs = hostinfo_loadknownaddr(op,af,knowns[i].a) ;
	            } /* end if (loading known address) */

	        } /* end if (hostinfo_domain) */
	    } /* end if (address space ours) */
	} /* end if (continue) */

#if	CF_DEBUGS
	debugprintf("getinet_known: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (getinet_known) */


static int matknown(cchar *name,int nl)
{
	int		m, i ;

	if (nl < 0) nl = strlen(name) ;

	for (i = 0 ; knowns[i].name != NULL ; i += 1) {
	    m = nleadstr(knowns[i].name,name,nl) ;
	    if ((m > 0) && (knowns[i].name[m] == '\0') && (m == nl)) break ;
	} /* end for */

	return (knowns[i].name != NULL) ? i : -1 ;
}
/* end subroutine (matknown) */


int vcmpname(const void *e1pp,const void *e2pp)
{
	HOSTINFO_N	*ne1p ;
	HOSTINFO_N	*ne2p ;
	HOSTINFO_N	**nepp ;
	int		f = TRUE ;

	nepp = (HOSTINFO_N **) e1pp ;
	ne1p = *nepp ;
	nepp = (HOSTINFO_N **) e2pp ;
	ne2p = *nepp ;

	f = f && (ne1p->name[0] == ne2p->name[0]) ;
	f = f && (ne1p->namelen == ne2p->namelen) ;
	f = f && (strcmp(ne1p->name,ne2p->name) == 0) ;

	return (f) ? 0 : 1 ;
}
/* end subroutine (vcmpname) */


int vcmpaddr(const void *e1pp,const void *e2pp)
{
	HOSTINFO_A	*ae1p ;
	HOSTINFO_A	*ae2p ;
	HOSTINFO_A	**aepp ;
	int		af1, af2 ;
	int		f = TRUE ;

	aepp = (HOSTINFO_A **) e1pp ;
	ae1p = *aepp ;
	aepp = (HOSTINFO_A **) e2pp ;
	ae2p = *aepp ;

	f = f && (ae1p->addrlen == ae2p->addrlen) ;
	if (f) {
	    af1 = ae1p->af ;
	    af2 = ae2p->af ;
	    f = ((af1 > 0) && (af2 > 0) && (ae1p->af == ae2p->af)) ;
	}
	f = f && (memcmp(ae1p->addr,ae2p->addr,ae1p->addrlen) == 0) ;

	return (f) ? 0 : 1 ;
}
/* end subroutine (vcmpaddr) */


#if	CF_DEBUGS
static int debugprintaliases(cchar *s,struct hostent *hep)
{
	int		i = 0 ;
	debugprintf("%s: aliases>\n",s) ;
	if (hep->h_aliases != NULL) {
	    for (i = 0 ; hep->h_aliases[i] != NULL ; i += 1)
	        debugprintf("%s: alias[%u]=>%s<\n",
	            s,i,hep->h_aliases[i]) ;
	}
	return i ;
}
static int debugprintinetaddr(cchar *s,int af,const void *binaddr)
{
	const int	slen = INETX_ADDRSTRLEN ;
	int		rs1 ;
	char		sbuf[INETX_ADDRSTRLEN + 1] ;
	rs1 = inetntop(sbuf,slen,af,binaddr) ;
	if (rs1 < 0) strcpy(sbuf,"BAD") ;
	debugprintf("%s af=%d addr=%s\n",s,af,sbuf) ;
	return rs1 ;
}
#endif /* CF_DEBUGS */


