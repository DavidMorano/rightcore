/* listenspec */

/* hold (or manage) a "listen" specification */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_SAFE		0		/* extra safe? */
#define	CF_OPENPORT	1		/* use 'openport(3dam)' */
#define	CF_NONBLOCK	0		/* set non-blocking on sockets */


/* revision history:

	= 2001-03-23, David A­D­ Morano
	This object module was originally written.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object holds a "listen" specification.


*******************************************************************************/


#define	LISTENSPEC_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<stropts.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sockaddress.h>
#include	<vechand.h>
#include	<storebuf.h>
#include	<openport.h>
#include	<hostinfo.h>
#include	<localmisc.h>

#include	"listenspec.h"


/* local defines */

#define	LISTENSPEC_STARTLEN	50		/* starting listenspec length */
#define	LISTENSPEC_DEFPORT	"5108"

#ifndef	PROTONAME_TCP
#define	PROTONAME_TCP		"tcp"
#endif

#ifndef	PROTONAME_UDP
#define	PROTONAME_UDP		"udpp"
#endif

#ifndef	PROTONAME_SCTP
#define	PROTONAME_SCTP		"sctp"
#endif

#ifndef	ANYHOST
#define	ANYHOST			"anyhost"
#endif

#ifndef	INETXADDRLEN
#define	INETXADDRLEN		MAX(INET4ADDRLEN,INET6ADDRLEN)
#endif

#define	PRNAME			"LOCAL"

#define	TO_RECVFD		10 /* seconds */

#define	TCPADDR			struct tcpaddr

#define	ARGINFO			struct arginfo


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	cfnumi(const char *,int,int *) ;
extern int	cfnumui(const char *,int,uint *) ;
extern int	getaf(const char *,int) ;
extern int	listentcp(int,const char *,const char *,int) ;
extern int	listenuss(const char *,int,int) ;
extern int	listenpass(const char *,int,int) ;
extern int	acceptpass(int,struct strrecvfd *,int) ;
extern int	getportnum(const char *,const char *) ;
extern int	getprotofamily(int) ;
extern int	getnodedomain(char *,char *) ;
extern int	mkpr(char *,int,const char *,const char *) ;
extern int	getaddrlen(int) ;
extern int	isdigitlatin(int) ;
extern int	isFailConn(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */

struct arginfo {
	vechand		pargs ;
	int		slen ;
	int		argvalue ;
	int		mode ;
	int		f_adash ;
} ;

struct ss {
	const char	*sp ;
	int		sl ;
} ;

struct tcpaddr {
	struct ss	af, host, port ;
} ;


/* forward references */

int		listenspec_active(LISTENSPEC *,int,int) ;

static int	listenspec_parse(LISTENSPEC *,int,const char **) ;
static int	listenspec_tcpbegin(LISTENSPEC *,int,const char **) ;
static int	listenspec_tcpaddrload(LISTENSPEC *,TCPADDR *,int) ;
static int	listenspec_tcpend(LISTENSPEC *) ;
static int	listenspec_tcpsame(LISTENSPEC *,LISTENSPEC *) ;
static int	listenspec_tcpactive(LISTENSPEC *,int,int) ;
static int	listenspec_tcpaccept(LISTENSPEC *,void *,int *,int) ;
static int	listenspec_ussbegin(LISTENSPEC *,int,const char **) ;
static int	listenspec_ussend(LISTENSPEC *) ;
static int	listenspec_usssame(LISTENSPEC *,LISTENSPEC *) ;
static int	listenspec_ussactive(LISTENSPEC *,int,int) ;
static int	listenspec_ussaccept(LISTENSPEC *,void *,int *,int) ;
static int	listenspec_passbegin(LISTENSPEC *,int,const char **) ;
static int	listenspec_passend(LISTENSPEC *) ;
static int	listenspec_passsame(LISTENSPEC *,LISTENSPEC *) ;
static int	listenspec_passactive(LISTENSPEC *,int,int) ;
static int	listenspec_passaccept(LISTENSPEC *,void *,int *,int) ;

static int	listenspec_tcpinfo(LISTENSPEC *,LISTENSPEC_INFO *) ;
static int	listenspec_ussinfo(LISTENSPEC *,LISTENSPEC_INFO *) ;
static int	listenspec_passinfo(LISTENSPEC *,LISTENSPEC_INFO *) ;

static int	listenspec_procargs(LISTENSPEC *,ARGINFO *,int,cchar **) ;

#if	CF_OPENPORT
static int	listenspec_openport(LISTENSPEC *,int,cchar *,cchar *,int) ;
static int	listenspec_openporter(LISTENSPEC *,cchar *,int,
			cchar *,int,int) ;
static int	listenspec_openportaddr(LISTENSPEC *,cchar *,
			struct addrinfo *,char *,int,cchar *) ;
static int	listenspec_openportaddrone(LISTENSPEC *,char *,int,cchar *) ;
#endif /* CF_OPENPORT */

static int	listenspec_prlocal(LISTENSPEC *) ;

#ifdef	COMMENT
static int	tcp_nfield(const char *,int) ;
#endif

static int	tcpaddr_load(TCPADDR *,const char *,int) ;

static int	arginfo_start(ARGINFO *) ;
static int	arginfo_add(ARGINFO *,const char *) ;
static int	arginfo_get(ARGINFO *,int,const char **) ;
static int	arginfo_finish(ARGINFO *) ;


/* local variables */

static cchar	*ltypes[] = {
	"none",
	"tcp",
	"uss",
	"pass",
	NULL
} ;

enum ltypes {
	ltype_none,
	ltype_tcp,
	ltype_uss,
	ltype_pass,
	ltype_overlast
} ;

static cchar	*lopts[] = {
	"here",
	"reuse",
	"ra",
	NULL
} ;

enum lopts {
	lopt_here,
	lopt_reuse,
	lopt_ra,
	lopt_overlast
} ;


/* exported subroutines */


int listenspec_start(LISTENSPEC *op,int ac,cchar **av)
{
	int		rs ;
	int		n = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (av == NULL) return SR_FAULT ;

	if (ac <= 0) return SR_INVALID ;

#if	CF_DEBUGS
	{
	    int	i ;
	    for (i = 0 ; i < ac ; i += 1)
	        debugprintf("listenspec_start: av[%u]=>%s<\n",i,av[i]) ;
	}
#endif /* CF_DEBUGS */

	memset(op,0,sizeof(LISTENSPEC)) ;

	if ((rs = listenspec_parse(op,ac,av)) >= 0) {
	    n = rs ;
	    op->magic = LISTENSPEC_MAGIC ;
	}

#if	CF_DEBUGS
	debugprintf("listenspec_start: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (listenspec_start) */


int listenspec_finish(LISTENSPEC *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("listenspec_finish: ent\n") ;
#endif

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (op->magic != LISTENSPEC_MAGIC) return SR_NOTOPEN ;
#endif

	if (op->f.active) {
	    rs1 = listenspec_active(op,0,FALSE) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (op->info != NULL) {
	    rs1 = 0 ;
	    switch (op->ltype) {
	    case ltype_tcp:
	        rs1 = listenspec_tcpend(op) ;
	        break ;
	    case ltype_uss:
	        rs1 = listenspec_ussend(op) ;
	        break ;
	    case ltype_pass:
	        rs1 = listenspec_passend(op) ;
	        break ;
	    } /* end switch */
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(op->info) ;
	    if (rs >= 0) rs = rs1 ;
	    op->info = NULL ;
	} /* end if */

	if (op->prlocal != NULL) {
	    rs1 = uc_free(op->prlocal) ;
	    if (rs >= 0) rs = rs1 ;
	}

	op->ltype = 0 ;
	op->magic = 0 ;

#if	CF_DEBUGS
	debugprintf("listenspec_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (listenspec_finish) */


int listenspec_issame(LISTENSPEC *op,LISTENSPEC *otherp)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (op->magic != LISTENSPEC_MAGIC) return SR_NOTOPEN ;
#endif

	if (otherp == NULL) return SR_FAULT ;

	if (otherp->magic != LISTENSPEC_MAGIC) return SR_INVALID ;

	f = (op->ltype == otherp->ltype) ? 1 : 0 ;
	if (f) {
	    switch (op->ltype) {
	    case ltype_tcp:
	        rs = listenspec_tcpsame(op,otherp) ;
	        f = (rs > 0) ;
	        break ;
	    case ltype_uss:
	        rs = listenspec_usssame(op,otherp) ;
	        f = (rs > 0) ;
	        break ;
	    case ltype_pass:
	        rs = listenspec_passsame(op,otherp) ;
	        f = (rs > 0) ;
	        break ;
	    default:
	        rs = SR_DOM ;
	    } /* end switch */
	}

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (listenspec_issame) */


/* set the "active" status (either active or non-active) */
int listenspec_active(LISTENSPEC *op,int opts,int f)
{
	int		rs = SR_OK ;
	int		f_previous ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (op->magic != LISTENSPEC_MAGIC) return SR_NOTOPEN ;
#endif

#if	CF_DEBUGS
	debugprintf("listenspec_active: ent f=%u\n",f) ;
	debugprintf("listenspec_active: previous f_act=%u\n",op->f.active) ;
	debugprintf("listenspec_active: ltype=%d\n",op->ltype) ;
#endif

	f_previous = op->f.active ;
	switch (op->ltype) {
	case ltype_tcp:
	    rs = listenspec_tcpactive(op,opts,f) ;
	    break ;
	case ltype_uss:
	    rs = listenspec_ussactive(op,opts,f) ;
	    break ;
	case ltype_pass:
	    rs = listenspec_passactive(op,opts,f) ;
	    break ;
	default:
	    rs = SR_DOM ;
	    break ;
	} /* end switch */

/* if we just activated (rs > 0), then set Close-On-Exec */

#if	CF_DEBUGS
	debugprintf("listenspec_active: mid2 rs=%d fd=%d\n",rs,op->fd) ;
#endif

	if ((rs > 0) && (op->fd >= 0) && op->f.active) {
	    rs = uc_closeonexec(op->fd,TRUE) ;
#if	CF_NONBLOCK
	    if (rs >= 0)
	        rs = uc_nonblock(op->fd,TRUE) ;
#endif
	    if ((rs < 0) && f) {
	        listenspec_active(op,opts,FALSE) ;
	    }
	} /* end if (just activated) */

#if	CF_DEBUGS
	debugprintf("listenspec_active: ret rs=%d f_prev=%u fd=%d\n",
	    rs,f_previous,op->fd) ;
#endif

	return (rs >= 0) ? f_previous : rs ;
}
/* end subroutine (listenspec_active) */


int listenspec_isactive(LISTENSPEC *op)
{
	int		rs = SR_OK ;
	int		f ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (op->magic != LISTENSPEC_MAGIC) return SR_NOTOPEN ;
#endif

	f = op->f.active ;
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (listenspec_isactive) */


int listenspec_delset(LISTENSPEC *op,int f)
{
	int		rs = SR_OK ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (op->magic != LISTENSPEC_MAGIC) return SR_NOTOPEN ;
#endif

	op->f.delete = f ;
	return rs ;
}
/* end subroutine (listenspec_delset) */


int listenspec_delmarked(LISTENSPEC *op)
{
	int		rs = SR_OK ;
	int		f ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (op->magic != LISTENSPEC_MAGIC) return SR_NOTOPEN ;
#endif

	f = op->f.delete ;
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (listenspec_delmarked) */


int listenspec_getfd(LISTENSPEC *op)
{
	int		rs = SR_BADFD ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (op->magic != LISTENSPEC_MAGIC) return SR_NOTOPEN ;
#endif

	if (op->f.active) rs = op->fd ;
	return rs ;
}
/* end subroutine (listenspec_getfd) */


int listenspec_gettype(LISTENSPEC *op)
{

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (op->magic != LISTENSPEC_MAGIC) return SR_NOTOPEN ;
#endif

	return op->ltype ;
}
/* end subroutine (listenspec_gettype) */


int listenspec_accept(LISTENSPEC *op,void *fromp,int *fromlenp,int to)
{
	int		rs = SR_OK ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (op->magic != LISTENSPEC_MAGIC) return SR_NOTOPEN ;
#endif

	if (op->f.active) {
	    switch (op->ltype) {
	    case ltype_tcp:
	        rs = listenspec_tcpaccept(op,fromp,fromlenp,to) ;
	        break ;
	    case ltype_uss:
	        rs = listenspec_ussaccept(op,fromp,fromlenp,to) ;
	        break ;
	    case ltype_pass:
	        rs = listenspec_passaccept(op,fromp,fromlenp,to) ;
	        break ;
	    default:
	        rs = SR_DOM ;
	        break ;
	    } /* end switch */
	} else {
	    rs = SR_BADFD ;
	}

#if	CF_DEBUGS
	debugprintf("listenspec_accept: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (listenspec_accept) */


int listenspec_info(LISTENSPEC *op,LISTENSPEC_INFO *lip)
{
	int		rs = SR_OK ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (op->magic != LISTENSPEC_MAGIC) return SR_NOTOPEN ;
#endif

	if (lip == NULL) return SR_FAULT ;

	lip->type[0] = '\0' ;
	lip->addr[0] = '\0' ;

#if	CF_DEBUGS
	debugprintf("listenspec_info: type=%s (%u)\n",
	    ltypes[op->ltype],op->ltype) ;
#endif

	switch (op->ltype) {
	case ltype_tcp:
	    rs = listenspec_tcpinfo(op,lip) ;
	    break ;
	case ltype_uss:
	    rs = listenspec_ussinfo(op,lip) ;
	    break ;
	case ltype_pass:
	    rs = listenspec_passinfo(op,lip) ;
	    break ;
	default:
	    rs = SR_INVALID ;
	    break ;
	} /* end switch */

	if (rs >= 0) {
	    lip->state = 0 ;
	    if (op->f.active)
	        lip->state |= LISTENSPEC_MACTIVE ;
	    if (op->f.delete)
	        lip->state |= LISTENSPEC_MDELPEND ;
	    if (op->f.broken)
	        lip->state |= LISTENSPEC_MBROKEN ;
	    strwcpy(lip->type,ltypes[op->ltype],LISTENSPEC_TYPELEN) ;
	}

#if	CF_DEBUGS
	debugprintf("listenspec_info: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (listenspec_info) */


int listenspec_geterr(LISTENSPEC *op,int *rp)
{
#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (op->magic != LISTENSPEC_MAGIC) return SR_NOTOPEN ;
#endif
	if (rp != NULL) {
	    *rp = op->rs_error ;
	}
	return SR_OK ;
}
/* end subroutine (listenspec_geterr) */


int listenspec_clear(LISTENSPEC *op)
{
	int		rs = SR_OK ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (op->magic != LISTENSPEC_MAGIC) return SR_NOTOPEN ;
#endif

	op->f.broken = FALSE ;
	return rs ;
}
/* end subroutine (listenspec_clear) */


/* private subroutines */


static int listenspec_parse(LISTENSPEC *op,int ac,cchar **av)
{
	int		rs = SR_OK ;
	int		size = 0 ;
	int		ti ;
	int		n = 0 ;

#if	CF_DEBUGS
	debugprintf("listenspec_parse: type=>%s< \n",av[0]) ;
#endif

	ti = matstr(ltypes,av[0],-1) ;
	op->ltype = ti ;
	ac -= 1 ;

#if	CF_DEBUGS
	debugprintf("listenspec_parse: type=%s (%d)\n",av[0],ti) ;
#endif

/* allocate our information structure */

	switch (ti) {
	case ltype_tcp:
	    size = sizeof(LISTENSPEC_TCP) ;
	    break ;
	case ltype_uss:
	    size = sizeof(LISTENSPEC_USS) ;
	    break ;
	case ltype_pass:
	    size = sizeof(LISTENSPEC_PASS) ;
	    break ;
	} /* end switch */

	if (size > 0) {
	    void	*p ;
	    if ((rs = uc_malloc(size,&p)) >= 0) {
	        memset(p,0,size) ;
	        op->info = p ;
	        switch (ti) {
	        case ltype_tcp:
	            rs = listenspec_tcpbegin(op,ac,(av+1)) ;
	            n = rs ;
	            break ;
	        case ltype_uss:
	            rs = listenspec_ussbegin(op,ac,(av+1)) ;
	            n = rs ;
	            break ;
	        case ltype_pass:
	            rs = listenspec_passbegin(op,ac,(av+1)) ;
	            n = rs ;
	            break ;
	        default:
	            rs = SR_BUGCHECK ;
	            break ;
	        } /* end switch */
	        if (rs < 0) {
	            uc_free(op->info) ;
	            op->info = NULL ;
	        }
	    } /* end if (memory-allocation) */
	} else
	    rs = SR_PROTONOSUPPORT ;

#if	CF_DEBUGS
	debugprintf("listenspec_parse: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (listenspec_parse) */


static int listenspec_tcpbegin(LISTENSPEC *op,int ac,cchar **av)
{
	TCPADDR	a ;
	ARGINFO		ai, *aip = &ai ;
	int		rs ;
	int		pan = 0 ;
	int		al, cl ;
	int		size = 0 ;
	int		n = 0 ;
	const char	*ap ;
	const char	*cp ;

	tcpaddr_load(&a,NULL,0) ;

	if ((rs = arginfo_start(aip)) >= 0) {

	    if ((rs = listenspec_procargs(op,aip,ac,av)) >= 0) {

	        while ((al = arginfo_get(aip,pan,&ap)) >= 0) {

	            switch (pan) {
	            case 0:
	                if (al > 0) {
	                    size = al ;
	                    n = tcpaddr_load(&a,ap,al) ;
	                    if ((n > 0) && (a.port.sp == NULL))
	                        n = 0 ;
	                }
	                if (n == 0) {
	                    cp = LISTENSPEC_DEFPORT ;
	                    cl = strlen(cp) ;
	                    size = (cl + 1) ;
	                    a.port.sp = cp ;
	                    a.port.sl = cl ;
	                    n = 1 ;
	                }
	                break ;
	            } /* end switch */
	            pan += 1 ;

	        } /* end while (positional arguments) */

	        if (n > 0) {
	            rs = listenspec_tcpaddrload(op,&a,size) ;
#if	CF_DEBUGS
	            debugprintf("listenspec_tcpbegin: "
			"_tcpaddrload() rs=%d\n",rs) ;
#endif
	        }

	    } /* end if (procargs) */

	    arginfo_finish(aip) ;
	} /* end if (arginfo) */

#if	CF_DEBUGS
	debugprintf("listenspec_tcpbegin: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (listenspec_tcpbegin) */


static int listenspec_tcpaddrload(LISTENSPEC *op,TCPADDR *ap,int size)
{
	LISTENSPEC_TCP	*ip = (LISTENSPEC_TCP *) op->info ;
	int		rs ;
	char		*abuf = NULL ;
	char		*abp ;

	if ((rs = uc_malloc(size,&abuf)) >= 0) {

	    ip->a = abuf ;
	    abp = abuf ;
	    if (ap->af.sp != NULL) {
	        ip->af = abp ;
	        abp = strwcpy(abp,ap->af.sp,ap->af.sl) + 1 ;
	    }

	    if (ap->host.sp != NULL) {
	        ip->host = abp ;
	        abp = strwcpy(abp,ap->host.sp,ap->host.sl) + 1 ;
	    }

	    if (ap->port.sp != NULL) {
	        ip->port = abp ;
	        abp = strwcpy(abp,ap->port.sp,ap->port.sl) + 1 ;
	    }

	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (listenspec_tcpaddrload) */


static int listenspec_tcpend(LISTENSPEC *op)
{
	LISTENSPEC_TCP	*ip = (LISTENSPEC_TCP *) op->info ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (ip->a != NULL) {
	    rs1 = uc_free(ip->a) ;
	    ip->a = NULL ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (listenspec_tcpend) */


static int listenspec_tcpsame(LISTENSPEC *op,LISTENSPEC *otherp)
{
	LISTENSPEC_TCP	*ip = (LISTENSPEC_TCP *) op->info ;
	LISTENSPEC_TCP	*oip ;
	int		rs = SR_OK ;
	int		f = TRUE ;

	oip = (LISTENSPEC_TCP *) otherp->info ;
	f = f && LEQUIV((ip->af != NULL),(oip->af != NULL)) ;
	if (f && (ip->af != NULL))
	    f = (strcmp(ip->af,oip->af) == 0) ;

	f = f && LEQUIV((ip->host != NULL),(oip->host != NULL)) ;
	if (f && (ip->host != NULL))
	    f = (strcmp(ip->host,oip->host) == 0) ;

	f = f && LEQUIV((ip->port != NULL),(oip->port != NULL)) ;
	if (f && (ip->port != NULL))
	    f = (strcmp(ip->port,oip->port) == 0) ;

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (listenspec_tcpsame) */


/* f=1 make-active, f=0 make inactive */
static int listenspec_tcpactive(LISTENSPEC *op,int opts,int f)
{
	int		rs = SR_OK ;
	int		f_a = FALSE ;

#if	CF_DEBUGS
	debugprintf("listenspec_tcpactive: ent f=%u\n",f) ;
#endif

	if (f && (! op->f.active)) {
	    LISTENSPEC_TCP *ip = (LISTENSPEC_TCP *) op->info ;
	    int		af ;
	    cchar	*cp ;
	    cchar	*hostspec ;
	    cchar	*portspec ;

/* address family */

	    cp = ip->af ;
	    af = AF_INET4 ; /* default when none was specified */
	    if (cp != NULL) {
	        rs = getaf(cp,-1) ;
	        af = rs ;
	    }

/* hostname */

	    hostspec = ip->host ;
	    if (hostspec == NULL) hostspec = "" ;

/* port */

	    portspec = ip->port ;
	    if (portspec == NULL) portspec = LISTENSPEC_DEFPORT ;

/* activate */

#if	CF_DEBUGS
	    {
		int	f_ra = (opts & LISTENSPEC_MREUSE)?1:0 ;
	        debugprintf("listenspec_tcpactive: listen rs=%d ra=%u\n",
		    rs,f_ra) ;
	        debugprintf("listenspec_tcpactive: portspec=%s\n",
		    portspec) ;
	    }
#endif

	    if (rs >= 0) {
	        int	lopts = (opts & LISTENSPEC_MREUSE) ? 1 : 0 ;
	        rs = listentcp(af,hostspec,portspec,lopts) ;
	        op->fd = rs ;
#if	CF_DEBUGS
	        debugprintf("listenspec_tcpactive: listentcp() rs=%d\n",rs) ;
#endif
#if	CF_OPENPORT
	        if (rs == SR_ACCESS) {
	            rs = listenspec_openport(op,af,hostspec,portspec,opts) ;
	            op->fd = rs ;
#if	CF_DEBUGS
	            debugprintf("listenspec_tcpactive: _openport() rs=%d\n",
			rs) ;
#endif
	        }
#endif /* CF_OPENPORT */
	    } /* end if */

	    if (rs >= 0) {
	        op->f.active = TRUE ;
	        op->f.broken = FALSE ;
		f_a = TRUE ;
	    } else {
		op->rs_error = rs ;
	        op->f.broken = TRUE ;
	    }
	} else if ((! f) && op->f.active) {
	    if (op->fd >= 0) {
	        u_close(op->fd) ;
	        op->fd = -1 ;
	    }
	    op->f.active = FALSE ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("listenspec_tcpactive: ret rs=%d f_acive=%u\n",
		rs,op->f.active) ;
#endif

	return (rs >= 0) ? f_a : rs ;
}
/* end subroutine (listenspec_tcpactive) */


static int listenspec_tcpaccept(LISTENSPEC *op,void *fp,int *flp,int to)
{
	int		rs ;
	int		fd = -1 ;

	rs = uc_accepte(op->fd,fp,flp,to) ;
	fd = rs ;

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (listenspec_tcpaccept) */


static int listenspec_ussbegin(LISTENSPEC *op,int ac,cchar *av[])
{
	LISTENSPEC_USS	*ip = (LISTENSPEC_USS *) op->info ;
	ARGINFO		ai, *aip = &ai ;
	int		rs = SR_OK ;
	int		pan = 0 ;
	int		al ;
	int		n = 0 ;
	const char	*ap ;
	const char	*cp ;

/* initialize */

	ip->mode = -1 ;

	if ((rs = arginfo_start(aip)) >= 0) {

	    if ((rs = listenspec_procargs(op,aip,ac,av)) >= 0) {

	        if (aip->mode >= 0)
	            ip->mode = aip->mode ;

	        while ((al = arginfo_get(aip,pan,&ap)) >= 0) {

	            switch (pan) {
	            case 0:
	                if (al > 0) {
	                    n = 1 ;
	                    rs = uc_mallocstrw(ap,al,&cp) ;
	                    if (rs >= 0) ip->fname = cp ;
	                }
	                break ;
	            } /* end switch */
	            pan += 1 ;

	            if (rs < 0) break ;
	        } /* end while (positional arguments) */

/* default arguments */

#if	CF_DEBUGS
	        debugprintf("listenspec_ussbegin: arg mode=%06o\n",
	            (ip->mode & 0777)) ;
#endif

	        if (ip->mode < 0)
	            ip->mode = 0666 ;

	    } /* end if (procargs) */

	    arginfo_finish(aip) ;
	} /* end if (arginfo) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (listenspec_ussbegin) */


static int listenspec_ussend(LISTENSPEC *op)
{
	LISTENSPEC_USS	*ip = (LISTENSPEC_USS *) op->info ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (ip->fname != NULL) {
	    rs1 = uc_free(ip->fname) ;
	    ip->fname = NULL ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (listenspec_ussend) */


static int listenspec_usssame(LISTENSPEC *op,LISTENSPEC *otherp)
{
	LISTENSPEC_USS	*ip = (LISTENSPEC_USS *) op->info ;
	LISTENSPEC_USS	*oip ;
	int		rs = SR_OK ;
	int		f = TRUE ;

	oip = (LISTENSPEC_USS *) otherp->info ;
	f = f && LEQUIV((ip->fname != NULL),(oip->fname != NULL)) ;
	if (f && (ip->fname != NULL))
	    f = (strcmp(ip->fname,oip->fname) == 0) ;

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (listenspec_usssame) */


static int listenspec_ussactive(LISTENSPEC *op,int opts,int f)
{
	LISTENSPEC_USS	*ip = (LISTENSPEC_USS *) op->info ;
	int		rs = SR_OK ;
	int		f_a = FALSE ;

#if	CF_DEBUGS
	debugprintf("listenspec_ussactive: ent lo=%04x f=%u\n",opts,f) ;
#endif

	if (f && (! op->f.active)) {
#if	CF_DEBUGS
	    debugprintf("listenspec_ussactive: fname=%s\n",ip->fname) ;
#endif
	    if ((rs = listenuss(ip->fname,ip->mode,opts)) >= 0) {
	        op->fd = rs ;
	        op->f.active = TRUE ;
	        op->f.broken = FALSE ;
		f_a = TRUE ;
	    } else {
		op->rs_error = rs ;
	        op->f.broken = TRUE ;
	    }
	} else if ((! f) && op->f.active) {
	    if (op->fd >= 0) {
	        u_close(op->fd) ;
	        op->fd = -1 ;
	    }
	    op->f.active = FALSE ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("listenspec_ussactive: ret rs=%d f_a=%u\n",rs,f_a) ;
#endif

	return (rs >= 0) ? f_a : rs ;
}
/* end subroutine (listenspec_ussactive) */


static int listenspec_ussaccept(LISTENSPEC *op,void *fp,int *flp,int to)
{
	int		rs ;
	int		fd = -1 ;

	rs = uc_accepte(op->fd,fp,flp,to) ;
	fd = rs ;

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (listenspec_ussaccept) */


static int listenspec_passbegin(LISTENSPEC *op,int ac,cchar *av[])
{
	LISTENSPEC_PASS	*ip = (LISTENSPEC_PASS *) op->info ;
	ARGINFO		ai, *aip = &ai ;
	int		rs = SR_OK ;
	int		pan ;
	int		al ;
	int		n = 0 ;
	const char	*ap ;
	const char	*cp ;

/* initialize */

	ip->mode = -1 ;

	if ((rs = arginfo_start(aip)) >= 0) {

/* process */

	    if ((rs = listenspec_procargs(op,aip,ac,av)) >= 0) {

/* option arguments */

	        if (aip->mode >= 0)
	            ip->mode = aip->mode ;

/* positional arguments */

	        pan = 0 ;
	        while ((al = arginfo_get(aip,pan,&ap)) >= 0) {

	            switch (pan) {
	            case 0:
	                if (al > 0) {
	                    n = 1 ;
	                    rs = uc_mallocstrw(ap,al,&cp) ;
	                    if (rs >= 0) ip->fname = cp ;
	                }
	                break ;
	            } /* end switch */
	            pan += 1 ;

	            if (rs < 0) break ;
	        } /* end while (positional arguments) */

/* default arguments */

#if	CF_DEBUGS
	        debugprintf("listenspec_passbegin: arg mode=%06o\n",
	            (ip->mode & 0777)) ;
#endif

	        if (ip->mode < 0)
	            ip->mode = 0666 ;

	    } /* end if (procargs) */

	    arginfo_finish(aip) ;
	} /* end if (arginfo) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (listenspec_passbegin) */


static int listenspec_passend(LISTENSPEC *op)
{
	LISTENSPEC_PASS	*ip = (LISTENSPEC_PASS *) op->info ;
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("listenspec_passend: ent\n") ;
#endif

	if (ip->fname != NULL) {
	    rs1 = uc_free(ip->fname) ;
	    ip->fname = NULL ;
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUGS
	debugprintf("listenspec_passend: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (listenspec_passend) */


static int listenspec_passsame(LISTENSPEC *op,LISTENSPEC *otherp)
{
	LISTENSPEC_PASS	*ip = (LISTENSPEC_PASS *) op->info ;
	LISTENSPEC_PASS	*oip ;
	int		rs = SR_OK ;
	int		f = TRUE ;

	oip = (LISTENSPEC_PASS *) otherp->info ;
	f = f && LEQUIV((ip->fname != NULL),(oip->fname != NULL)) ;
	if (f && (ip->fname != NULL)) {
	    f = (strcmp(ip->fname,oip->fname) == 0) ;
	}

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (listenspec_passsame) */


static int listenspec_passactive(LISTENSPEC *op,int opts,int f)
{
	LISTENSPEC_PASS	*ip = (LISTENSPEC_PASS *) op->info ;
	int		rs = SR_OK ;
	int		f_a = FALSE ;

#if	CF_DEBUGS
	debugprintf("listenspec_passactive: ent fn=%s\n",ip->fname) ;
#endif

	if (f && (! op->f.active)) {
	    if ((rs = listenpass(ip->fname,ip->mode,opts)) >= 0) {
	        op->fd = rs ;
	        op->f.active = TRUE ;
	        op->f.broken = FALSE ;
		f_a = TRUE ;
#if	SYSHAS_STREAMS
	        u_ioctl(op->fd,I_SRDOPT,RMSGD) ;
#endif
	    } else {
		op->rs_error = rs ;
	        op->f.broken = TRUE ;
	    }
	} else if ((! f) && op->f.active) {
	    if (op->fd >= 0) {
	        u_close(op->fd) ;
	        op->fd = -1 ;
	    }
	    op->f.active = FALSE ;
	} /* end if */

	return (rs >= 0) ? f_a : rs ;
}
/* end subroutine (listenspec_passactive) */


static int listenspec_passaccept(LISTENSPEC *op,void *fp,int *flp,int to)
{
	int		rs ;
	int		fd = -1 ;

	if (fp != NULL) {
	    *flp = sizeof(long) ;
	    memset(fp,0,*flp) ;
	}

	rs = acceptpass(op->fd,NULL,to) ;
	fd = rs ;

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (listenspec_passaccept) */


static int listenspec_procargs(LISTENSPEC *op,ARGINFO *aip,int ac,cchar *av[])
{
	uint		uv ;

	int		rs = SR_OK ;
	int		rs1 ;
	int		fl ;
	int		v ;
	int		ai, ar ;
	int		kwi ;
	int		akl, avl ;
	int		pan = 0 ;
	int		f_optminus, f_optplus, f_optequal ;
	const char	*akp, *avp ;
	const char	*fp ;

	aip->mode = -1 ;

/* allocate buffer for further parsing */

	ar = ac ;
	for (ai = 0 ; (ai < ac) && (av[ai] != NULL) ; ai += 1) {
	    ar -= 1 ;

	    fp = av[ai] ;
	    fl = strlen(fp) ;

#if	CF_DEBUGS
	    debugprintf("listenspec_procargs: fl=%u fp=>%t<\n",
	        fl,fp,fl) ;
#endif

	    f_optminus = (fp[0] == '-') ;
	    f_optplus = (fp[0] == '+') ;
	    if ((fl > 1) && (f_optminus || f_optplus) && (! aip->f_adash)) {
		const int	ach = MKCHAR(fp[1]) ;

	        if (isdigitlatin(ach)) {

	            rs1 = cfdecti((fp + 1),(fl - 1),&v) ;
	            if (rs1 >= 0) aip->argvalue = v ;

	        } else if (ach == '-') {

	            aip->f_adash = TRUE ;
	            break ;

	        } else {

	            akp = (fp + 1) ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(akp,'=')) != NULL) {
	                f_optequal = TRUE ;
	                akl = avp - akp ;
	                avp += 1 ;
	                avl = akp + (fl - 1) - avp ;
	            } else {
	                avp = NULL ;
	                avl = 0 ;
	                akl = (fl - 1) ;
	            }

	            if ((kwi = matostr(lopts,2,akp,akl)) >= 0) {

	                switch (kwi) {
	                case lopt_here:
	                    break ;
	                case lopt_reuse:
	                case lopt_ra:
	                    op->f.reuse = TRUE ;
	                    break ;
	                } /* end switch */

	            } else {

	                while (akl--) {
	                    const int	kc = MKCHAR(*akp) ;

	                    switch (kc) {
	                    case 'm':
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs1 = cfnumui(avp,avl,&uv) ;
	                                if (rs1 >= 0)
	                                    aip->mode = (uv & 0777) ;
	                            }
	                        } else {
	                            if (ar > 0) {
	                                ar -= 1 ;
	                                fp = av[++ai] ;
	                                fl = strlen(fp) ;
	                                if (fl > 0) {
	                                    rs1 = cfnumui(fp,fl,&uv) ;
	                                    if (rs1 >= 0)
	                                        aip->mode = (uv & 0777) ;
	                                }
	                            }
	                        } /* end if */
	                        break ;
	                    } /* end switch */
	                    akp += 1 ;

			    if (rs < 0) break ;
	                } /* end while */

	            } /* end if */

	        } /* end if */

	    } else {

	        rs = arginfo_add(aip,fp) ;
	        pan += 1 ;

	    } /* end if */

	    if (rs < 0) break ;
	} /* end while */

	return rs ;
}
/* end subroutine (listenspec_procargs) */


static int listenspec_tcpinfo(LISTENSPEC *op,LISTENSPEC_INFO *lip)
{
	LISTENSPEC_TCP	*ip = (LISTENSPEC_TCP *) op->info ;
	int		rs = SR_OK ;
	int		sl = LISTENSPEC_ADDRLEN ;
	int		i = 0 ;
	char		*sp = lip->addr ;

#if	CF_DEBUGS
	debugprintf("listenspec_tcpinfo: af=%s\n",ip->af) ;
	debugprintf("listenspec_tcpinfo: host=%s\n",ip->host) ;
	debugprintf("listenspec_tcpinfo: port=%s\n",ip->port) ;
#endif

	if ((rs >= 0) && (ip->af != NULL)) {
	    rs = storebuf_strw(sp,sl,i,ip->af,-1) ;
	    i += rs ;
	    if (rs >= 0) {
	        rs = storebuf_char(sp,sl,i,':') ;
	        i += rs ;
	    }
	}

	if ((rs >= 0) && (ip->host != NULL)) {
	    rs = storebuf_strw(sp,sl,i,ip->host,-1) ;
	    i += rs ;
	    if (rs >= 0) {
	        rs = storebuf_char(sp,sl,i,':') ;
	        i += rs ;
	    }
	}

	if ((rs >= 0) && (ip->port != NULL)) {
	    rs = storebuf_strw(sp,sl,i,ip->port,-1) ;
	    i += rs ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (listenspec_tcpinfo) */


static int listenspec_ussinfo(LISTENSPEC *op,LISTENSPEC_INFO *lip)
{
	LISTENSPEC_USS	*ip = (LISTENSPEC_USS *) op->info ;
	int		rs = SR_OK ;
	int		sl = LISTENSPEC_ADDRLEN ;
	int		len = 0 ;
	char		*sp = lip->addr ;

	if (ip->fname == NULL) rs = SR_NOENT ;

	if (rs >= 0) {
	    len = strwcpy(sp,ip->fname,sl) - sp ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (listenspec_ussinfo) */


static int listenspec_passinfo(LISTENSPEC *op,LISTENSPEC_INFO *lip)
{
	LISTENSPEC_PASS	*ip = (LISTENSPEC_PASS *) op->info ;
	int		rs = SR_OK ;
	int		sl = LISTENSPEC_ADDRLEN ;
	int		len = 0 ;
	char		*sp = lip->addr ;

	if (ip->fname == NULL) rs = SR_NOENT ;

	if (rs >= 0) {
	    len = strwcpy(sp,ip->fname,sl) - sp ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (listenspec_passinfo) */


#if	CF_OPENPORT

/* note that the 'opts' argument is ignored; this always REUSEs the port */
static int listenspec_openport(op,af,hostspec,portspec,opts)
LISTENSPEC	*op ;
int		af ;
const char	hostspec[] ;
const char	portspec[] ;
int		opts ;
{
	int		rs ;
	int		fd = -1 ;
	const char	*protoname = PROTONAME_TCP ;
	const char	*hp = hostspec ;

#if	CF_DEBUGS
	debugprintf("listenspec_openport: af=%d\n",af) ;
	debugprintf("listenspec_openport: hostspec=%s\n",hostspec) ;
	debugprintf("listenspec_openport: portspec=%s\n",portspec) ;
#endif

	if (af < 0)
	    return SR_INVALID ;

	if (portspec[0] != '\0') {
	    if ((rs = getportnum(protoname,portspec)) >= 0) {
		const int	port = rs ;
		if ((rs = listenspec_prlocal(op)) >= 0) {
		     const char	*pr = op->prlocal ;
		     int	f = FALSE ;
		     f = f || (hp == NULL) ;
		     f = f || (hp[0] == '\0') ;
		     f = f || (strcmp(hp,"*") == 0) ;
		     if (f) hp = ANYHOST ;
		     rs = listenspec_openporter(op,pr,af,hp,port,opts) ;
		     fd = rs ;
		} /* end if (listenspec_prlocal) */
	    } /* end if (getportnum) */
	} else {
	    rs = SR_INVALID ;
	}

#if	CF_DEBUGS
	debugprintf("listenspec_openport: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (listenspec_openport) */


/* ARGSUSED */
static int listenspec_openporter(LISTENSPEC *op,cchar *pr,int af,cchar *hp,
		int port,int opts)
{
	struct addrinfo	ai ;
	int		rs ;
	int		fd = -1 ;
	char		addr[INETXADDRLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("listenspec_openporter: ent af=%u\n",af) ;
	debugprintf("listenspec_openporter: pr=%s\n",pr) ;
	debugprintf("listenspec_openporter: h=%s\n",hp) ;
#endif

	if ((rs = listenspec_openportaddr(op,pr,&ai,addr,af,hp)) >= 0) {
	    SOCKADDRESS	sa ;
	    const int	type = SOCK_STREAM ;
	    const int	proto = IPPROTO_TCP ;
	    const int	flow = 0 ;
	    int		alen = rs ;
	    int		pf = ai.ai_family ;

#if	CF_DEBUGS
	    {
		const char	*id = "listenspec_openporter: a¬" ;
	    debugprintf("listenspec_openporter: alen=%u\n",alen) ;
	    debugprinthexblock(id,80,addr,alen) ;
	    }
#endif

	    if ((rs = sockaddress_startaddr(&sa,af,addr,alen,port,flow)) >= 0) {

		if ((rs = openport(pf,type,proto,&sa)) >= 0) {
	            fd = rs ;

#if	CF_DEBUGS
	            debugprintf("listenspec_openporter: openport() rs=%d\n",
			rs) ;
#endif

	            rs = u_listen(fd,10) ;

		    if ((rs < 0) && (fd >= 0)) {
			u_close(fd) ;
			fd = -1 ;
		    }
	        } /* end if (openport) */

#if	CF_DEBUGS
	        debugprintf("listenspec_openporter: openport-ot rs=%d\n",
			rs) ;
#endif

	        sockaddress_finish(&sa) ;
	    } /* end if (sockaddress) */

#if	CF_DEBUGS
	    debugprintf("listenspec_openporter: sockaddress-out rs=%d\n",rs) ;
#endif

	} /* end if (open-port-addr) */

#if	CF_DEBUGS
	debugprintf("listenspec_openporter: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (listenspec_openporter) */


/* ARGSUSED */
static int listenspec_openportaddr(LISTENSPEC *op,cchar *pr,ADDRINFO *aip,
		char *addr,int af,cchar	*hn)
{
	int		rs = SR_OK ;
	int		raf = 0 ;
	int		addrlen = -1 ;

#if	CF_DEBUGS
	debugprintf("listenspec_openportaddr: ent af=%d\n",af) ;
	debugprintf("listenspec_openportaddr: kn=%s\n",hn) ;
#endif

	if (af < 0) return SR_INVALID ;

	addr[0] = '\0' ;
	memset(aip,0,sizeof(struct addrinfo)) ;

	if ((addrlen < 0) && ((af == AF_UNSPEC) || (af == AF_INET4))) {
	    raf = AF_INET4 ;
	    rs = listenspec_openportaddrone(op,addr,raf,hn) ;
	    addrlen = rs ;
	}

#if	CF_DEBUGS
	debugprintf("listenspec_openportaddr: mid1 rs=%d al=%d\n",
		rs,addrlen) ;
#endif

	if ((addrlen < 0) || (rs == SR_NOTFOUND)) {
	    if ((af == AF_UNSPEC) || (af == AF_INET6)) {
	        raf = AF_INET6 ;
	        rs = listenspec_openportaddrone(op,addr,raf,hn) ;
	        addrlen = rs ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("listenspec_openportaddr: mid2 rs=%d al=%d\n",
		rs,addrlen) ;
#endif

	if (rs >= 0) {
	    if (raf > 0) {
	        if ((rs = getprotofamily(raf)) >= 0) {
	            aip->ai_family = rs ;
	        }
	    } else {
		rs = SR_AFNOSUPPORT ;
	    }
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("listenspec_openportaddr: ret rs=%d al=%u\n",
		rs,addrlen) ;
#endif

	return (rs >= 0) ? addrlen : rs ;
}
/* end subroutine (listenspec_openportaddr) */


static int listenspec_openportaddrone(LISTENSPEC *op,char *addr,int af,
		cchar *hn)
{
	HOSTINFO	hi ;
	HOSTINFO_CUR	hicur ;
	int		rs ;
	int		rs1 ;
	int		addrlen ;

#if	CF_DEBUGS
	debugprintf("listenspec_openportaddrone: ent\n") ;
	debugprintf("listenspec_openportaddrone: hn=%s\n",hn) ;
#endif

	if (op == NULL) return SR_NOANODE ;

	if (af < 0) return SR_INVALID ;

	addr[0] = '\0' ;
	addrlen = getaddrlen(af) ;

	if ((rs = hostinfo_start(&hi,af,hn)) >= 0) {
	    if ((rs = hostinfo_curbegin(&hi,&hicur)) >= 0) {
	        const uchar	*ap ;

	        while ((rs = hostinfo_enumaddr(&hi,&hicur,&ap)) >= 0) {
	            if (rs == addrlen) {
	                memcpy(addr,ap,addrlen) ;
	                break ;
	            }
	        } /* end while */

	        rs1 = hostinfo_curend(&hi,&hicur) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (cursor) */
	    rs1 = hostinfo_finish(&hi) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (hostinfo) */

#if	CF_DEBUGS
	debugprintf("listenspec_openportaddrone: ret rs=%d al=%u\n",
		rs,addrlen) ;
#endif

	return (rs >= 0) ? addrlen : rs ;
}
/* end subroutine (listenspec_openportaddrone) */

#endif /* CF_OPENPORT */


static int listenspec_prlocal(LISTENSPEC *op)
{
	int		rs ;
	char		dn[MAXHOSTNAMELEN+1] ;

	if (op->prlocal == NULL) {
	    if ((rs = getnodedomain(NULL,dn)) >= 0) {
	        const int	plen = MAXPATHLEN ;
	        char		pbuf[MAXPATHLEN+1] ;
	        if ((rs = mkpr(pbuf,plen,PRNAME,dn)) >= 0) {
		    const char	*cp ;
		    if ((rs = uc_mallocstrw(pbuf,rs,&cp)) >= 0) {
		        op->prlocal = cp ;
			rs = (rs-1) ;
		    }
	        }
	    } /* end if (getrootdname) */
	} else {
	    rs = strlen(op->prlocal) ;
	}

	return rs ;
}
/* end subroutine (listenspec_prlocal) */


#ifdef	COMMENT
static int tcp_nfield(cchar *fp,int fl)
{
	int		n = 0 ;

	if (fl > 0) {
	    int		cl ;
	    const char	*cp ;
	    n += 1 ;
	    if ((cp = strnchr(fp,fl,':')) != NULL) {
	        n += 1 ;
	        cp += 1 ;
	        cl = ((fp + fl) - cp) ;
	        if (strnchr(cp,cl,':') != NULL)
	            n += 1 ;
	    } /* end if */
	} /* end if (non-zero) */

	return n ;
}
/* end subroutine (tcp_nfield) */
#endif /* COMMENT */


static int tcpaddr_load(TCPADDR *ap,cchar *sp,int sl)
{
	int		n = 0 ;

	if (sl < 0)
	    sl = strlen(sp) ;

	memset(ap,0,sizeof(TCPADDR)) ;

	if (sl > 0) {
	    int	cl ;
	    const char	*cp ;
	    const char	*tp ;
	    n += 1 ;
	    if ((tp = strnchr(sp,sl,':')) != NULL) {

	        n += 1 ;
	        ap->af.sp = sp ;
	        ap->af.sl = (tp - sp) ;
	        ap->host.sp = sp ;
	        ap->host.sl = (tp - sp) ;

	        cp = (tp + 1) ;
	        cl = ((sp + sl) - cp) ;
	        if ((tp = strnchr(cp,cl,':')) != NULL) {

	            n += 1 ;
	            ap->host.sp = cp ;
	            ap->host.sl = (tp - cp) ;
	            ap->port.sp = (tp + 1) ;
	            ap->port.sl = ((sp + sl) - (tp + 1)) ;

	        } else {

	            ap->af.sp = NULL ;
	            ap->af.sl = 0 ;
	            ap->port.sp = cp ;
	            ap->port.sl = cl ;

	        } /* end if */

	    } else {

	        ap->port.sp = sp ;
	        ap->port.sl = sl ;

	    } /* end if */
	} /* end if (non-zero) */

	return n ;
}
/* end subroutine (tcpaddr_load) */


static int arginfo_start(ARGINFO *aip)
{
	int		rs ;

	memset(aip,0,sizeof(ARGINFO)) ;
	aip->argvalue = -1 ;
	aip->slen = -1 ;
	aip->mode = -1 ;

	rs = vechand_start(&aip->pargs,0,0) ;

	return rs ;
}
/* end subroutine (arginfo_start) */


static int arginfo_add(ARGINFO *aip,cchar *sp)
{
	int		rs ;

	rs = vechand_add(&aip->pargs,sp) ;

	return rs ;
}
/* end subroutine (arginfo_add) */


static int arginfo_get(ARGINFO *aip,int i,cchar **rpp)
{
	int		rs ;
	if ((rs = vechand_get(&aip->pargs,i,rpp)) >= 0) {
	    if (*rpp != NULL) {
	        rs = strlen(*rpp) ;
	    }
	}
	return rs ;
}
/* end subroutine (arginfo_get) */


static int arginfo_finish(ARGINFO *aip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = vechand_finish(&aip->pargs) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (arginfo_finish) */


