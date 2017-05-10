/* hostaddr */

/* manipulate host entry structures */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2005-02-03, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2005 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object provides the capability to peruse the information returned
	by the subroutines:

		geteaddrinfo(3dam)

	which itself is an enhanved version of the system-provided:

		getaddrinfo(3socket)

	Additional information:

 * addrinfo introduced with IPv6 for Protocol-Independent Hostname
 * and Service Name Translation.

struct addrinfo {
	int ai_flags;		 	AI_PASSIVE, AI_CANONNAME 
	int ai_family;		 	PF_xxx 
	int ai_socktype;	 	SOCK_xxx 
	int ai_protocol;	 	0 or IPPROTO_xxx for IPv4 and IPv6 
	size_t ai_addrlen;		length of ai_addr 
	char *ai_canonname;	 	canonical name for hostname 
	struct sockaddr *ai_addr;	binary address 
	struct addrinfo *ai_next;	next structure in linked list 
};

 addrinfo flags 
#define	AI_PASSIVE	0x0008	 intended for bind() + listen() 
#define	AI_CANONNAME	0x0010	 return canonical version of host 
#define	AI_NUMERICHOST	0x0020	 use numeric node address string 

 getipnodebyname() flags 
#define	AI_V4MAPPED	0x0001	 IPv4 mapped addresses if no IPv6 
#define	AI_ALL		0x0002	 IPv6 and IPv4 mapped addresses 
#define	AI_ADDRCONFIG	0x0004	 AAAA or A records only if IPv6/IPv4 cnfg'd 
#define	AI_DEFAULT	(AI_V4MAPPED | AI_ADDRCONFIG)

 addrinfo errors 
#define	EAI_ADDRFAMILY	1	 address family not supported 
#define	EAI_AGAIN	2	 DNS temporary failure 
#define	EAI_BADFLAGS	3	 invalid ai_flags 
#define	EAI_FAIL	4	 DNS non-recoverable failure 
#define	EAI_FAMILY	5	 ai_family not supported 
#define	EAI_MEMORY	6	 memory allocation failure 
#define	EAI_NODATA	7	 no address 
#define	EAI_NONAME	8	 host/servname not known 
#define	EAI_SERVICE	9	 servname not supported for ai_socktype 
#define	EAI_SOCKTYPE	10	 ai_socktype not supported 
#define	EAI_SYSTEM	11	 system error in ERRNO

 getnameinfo max sizes as defined in spec 

#define	NI_MAXHOST	1025
#define	NI_MAXSERV	32

 getnameinfo flags 

#define	NI_NOFQDN	0x0001
#define	NI_NUMERICHOST	0x0002	 return numeric form of address 
#define	NI_NAMEREQD	0x0004	 request DNS name 
#define	NI_NUMERICSERV	0x0008
#define	NI_DGRAM	0x0010


*******************************************************************************/


#define	HOSTADDR_MASTER		0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"hostaddr.h"

#if	CF_DEBUGS
#include	"inetaddr.h"
#endif


/* local defines */

#define	TO_AGAIN	10

#define	ADDRINFO	struct addrinfo


/* external subroutines */

extern int	geteaddrinfo(const char *,const char *,
			struct addrinfo *,char *,struct addrinfo **) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif /* CF_DEBUGS */


/* external variables */


/* local structures */


/* forward references */

static int	hostaddr_resultbegin(HOSTADDR *) ;
static int	hostaddr_resultend(HOSTADDR *) ;

static int	vcmp(const void *,const void *) ;


/* local variables */


/* exported subroutines */


int hostaddr_start(HOSTADDR *op,cchar *hostname,cchar *svc,ADDRINFO *hintp)
{
	ADDRINFO	*aip ;
	int		rs ;
	char		ehostname[MAXHOSTNAMELEN + 1] ;

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(HOSTADDR)) ;

#if	CF_DEBUGS
	debugprintf("hostaddr_start: hostname=%s\n",hostname) ;
	debugprintf("hostaddr_start: svc=%s\n",svc) ;
	if (hintp != NULL) {
	    debugprintf("hostaddr_start: hint ai_protocol=%d\n",
		hintp->ai_protocol) ;
	    debugprintf("hostaddr_start: hint ai_family=%d\n",
		hintp->ai_family) ;
	}
#endif

	if ((rs = geteaddrinfo(hostname,svc,hintp,ehostname,&aip)) >= 0) {
	    const char	*cp ;
	    op->aip = aip ;
#if	CF_DEBUGS
	    debugprintf("hostaddr_start: ai_protocol=%d\n",
		aip->ai_protocol) ;
#endif
	    if ((rs = hostaddr_resultbegin(op)) >= 0) {
	        if ((rs = uc_mallocstrw(ehostname,-1,&cp)) >= 0) {
		    op->ehostname = cp ;
		    op->magic = HOSTADDR_MAGIC ;
	        }
		if (rs < 0)
		    hostaddr_resultend(op) ;
	    } /* end if (hostaddr_sort) */
	    if (rs < 0) {
	        uc_freeaddrinfo(op->aip) ;
		op->aip = NULL ;
	    }
	} /* end if (geteaddrinfo) */

#if	CF_DEBUGS
	debugprintf("hostaddr_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (hostaddr_start) */


int hostaddr_finish(HOSTADDR *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != HOSTADDR_MAGIC) return SR_NOTOPEN ;

	if (op->ehostname != NULL) {
	    rs1 = uc_free(op->ehostname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->ehostname = NULL ;
	}

	rs1 = hostaddr_resultend(op) ;
	if (rs >= 0) rs = rs1 ;

	if (op->aip != NULL) {
	    rs1 = uc_freeaddrinfo(op->aip) ;
	    if (rs >= 0) rs = rs1 ;
	    op->aip = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("hostaddr_finish: ret rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (hostaddr_finish) */


int hostaddr_getcanonical(HOSTADDR *op,cchar **rpp)
{
	ADDRINFO	*aip ;

	if (op == NULL) return SR_FAULT ;
	if (rpp == NULL) return SR_FAULT ;

	if (op->magic != HOSTADDR_MAGIC) return SR_NOTOPEN ;

	aip = op->aip ;
	*rpp = aip->ai_canonname ;

	return strlen(aip->ai_canonname) ;
}
/* end subroutine (hostaddr_cannonical) */


int hostaddr_curbegin(HOSTADDR *op,HOSTADDR_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != HOSTADDR_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("hostaddr_curbegin: ent\n") ;
#endif

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (hostaddr_curbegin) */


int hostaddr_curend(HOSTADDR *op,HOSTADDR_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != HOSTADDR_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("hostaddr_curend: ent\n") ;
#endif

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (hostaddr_curend) */


int hostaddr_enum(HOSTADDR *op,HOSTADDR_CUR *curp,ADDRINFO **rpp)
{
	int		rs = SR_OK ;
	int		i ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != HOSTADDR_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("hostaddr_enum: ent\n") ;
#endif

	i = (curp->i >= 0) ? (curp->i+1) : 0 ;
	if (i < op->n) {
	    if (rpp != NULL) *rpp = op->resarr[i] ;
	    curp->i = i ;
	} else {
	    if (rpp != NULL) *rpp = NULL ;
	    rs = SR_NOTFOUND ;
	}

#if	CF_DEBUGS
	debugprintf("hostaddr_enum: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (hostaddr_enum) */


/* private subroutines */


static int hostaddr_resultbegin(HOSTADDR *op)
{
	ADDRINFO	*aip = op->aip, **resarr ;
	const int	esize = sizeof(ADDRINFO) ;
	int		rs ;
	int		n = 0 ;
	int		size ;
	n += 1 ;
	while (aip->ai_next != NULL) {
	    aip = aip->ai_next ;
	    n += 1 ;
	} /* end while */
	size = ((n+1) * esize) ;
	if ((rs = uc_malloc(size,&resarr)) >= 0) {
	    int		i = 0 ;
	    op->resarr = resarr ;
	    op->n = n ;
	    aip = op->aip ;
	    resarr[i++] = aip ;
	    while (aip->ai_next != NULL) {
		aip = aip->ai_next ;
		resarr[i++] = aip ;
	    } /* end while */
	    resarr[i] = NULL ;
	    if (n > 1) {
	        qsort(resarr,n,sizeof(void *),vcmp) ;
	    }
	} /* end if (m-a) */
	return (rs >= 0) ? n : rs ;
}
/* end subroutine (hostaddr_resultbegin) */


static int hostaddr_resultend(HOSTADDR *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (op->resarr != NULL) {
	    rs1 = uc_free(op->resarr) ;
	    if (rs >= 0) rs = rs1 ;
	    op->resarr = NULL ;
	    op->n = 0 ;
	}
	return rs ;
}
/* end subroutine (hostaddr_resultend) */


static int vcmp(const void *v1p,const void *v2p)
{
	ADDRINFO	**a1pp = (ADDRINFO **) v1p ;
	ADDRINFO	**a2pp = (ADDRINFO **) v2p ;
	int		rc = 0 ;
	if ((*a1pp != NULL) || (*a2pp != NULL)) {
	    if (*a1pp != NULL) {
	        if (*a2pp != NULL) {
		    ADDRINFO	*a1p = (ADDRINFO *) *a1pp ;
		    ADDRINFO	*a2p = (ADDRINFO *) *a2pp ;
	            rc = (a1p->ai_family - a2p->ai_family) ;
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;
}
/* end subroutine (vcmp) */


