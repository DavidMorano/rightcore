/* hostinfo */

/* manipulate host entry structures */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1994-02-03, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        These routines provide means to peruse the elements of a 'hostinfo'
        structure as returned by the system subroutines:

		gethostbyname
		gethostbyaddr

	and their kin.

        All INET addresses (when in any sort of binary form) are in network byte
        order! This is true of the above subroutines as well.

	Below is the structure that represents the object itself:


 * addrinfo introduced with IPv6 for Protocol-Independent Hostname
 * and Service Name Translation.

struct addrinfo {
	int ai_flags;		 	AI_PASSIVE, AI_CANONNAME 
	int ai_family;		 	PCF_xxx 
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


#define	HOSTINFO_MASTER		0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<localmisc.h>
#include	<hostinfo.h>

#if	CF_DEBUGS
#include	"inetaddr.h"
#endif


/* local defines */

#define	HOSTINFO_MAGIC		0x73625196

#define	TO_AGAIN		10


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */

#if	CF_DEBUGS
static int	makeint(void *) ;
#endif


/* local variables */








int hostinfo_init(op,pr,hostname,svc,hintp)
HOSTINFO	*op ;
const char	pr[] ;
const char	hostname[] ;
const char	svc[] ;
struct addrinfo	*hintp ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	memset(op,0,sizeof(HOSTINFO)) ;

#if	CF_DEBUGS
	debugprintf("hostinfo_init: hostname=%s\n",hostname) ;
	debugprintf("hostinfo_init: svc=%s\n",svc) ;
#endif

loop:
	rs = uc_getaddrinfo(hostname,svc,hintp,&op->aip) ;

	if (rs >= 0)
	    op->magic = HOSTINFO_MAGIC ;

#if	CF_DEBUGS
	debugprintf("hostinfo_init: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (hostinfo_init) */


int hostinfo_getoffical(op,rpp)
HOSTINFO	*op ;
char		**rpp ;
{
	struct addrinfo	*aip ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != HOSTINFO_MAGIC)
	    return SR_NOTOPEN ;

	if (rpp == NULL)
	    return SR_FAULT ;

	aip = op->aip ;
	*rpp = aip->ai_canonname ;

	return strlen(aip->ai_canonname) ;
}


int hostinfo_curbegin(op,curp)
HOSTINFO	*op ;
HOSTINFO_CURSOR	*curp ;
{


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != HOSTINFO_MAGIC)
	    return SR_NOTOPEN ;

	if (curp == NULL)
	    return SR_FAULT ;

	curp->aip = NULL ;
	return SR_OK ;
}
/* end subroutine (hostinfo_curbegin) */


int hostinfo_curend(op,curp)
HOSTINFO	*op ;
HOSTINFO_CURSOR	*curp ;
{


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != HOSTINFO_MAGIC)
	    return SR_NOTOPEN ;

	if (curp == NULL)
	    return SR_FAULT ;

	curp->aip = NULL ;
	return SR_OK ;
}
/* end subroutine (hostinfo_curend) */


#ifdef	COMMENT

/* enumerate the next hostname */
int hostinfo_enumname(op,cp,rpp)
HOSTINFO	*op ;
hostinfo_cur	*cp ;
char		**rpp ;
{
	int	nlen = -1 ;


	if (op == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("hostinfo_enumname: entered\n") ;
#endif

	if (cp == NULL) {

	    if (op->h_name != NULL) {

	        nlen = strlen(op->h_name) ;

	        if (rpp != NULL)
	            *rpp = op->h_name ;

	    }

	} else {

	    if (cp->i < 0) {

	        if (op->h_name != NULL) {

	            nlen = strlen(op->h_name) ;

	            if (rpp != NULL)
	                *rpp = op->h_name ;

	            cp->i = 0 ;
	        }

	    } else {

	        if ((op->h_aliases != NULL) &&
	            (op->h_aliases[cp->i] != NULL)) {

	            nlen = strlen(op->h_aliases[cp->i]) ;

#if	CF_DEBUGS
	            debugprintf("hostinfo_enumname: 
	                i=%d nlen=%d\n",cp->i,nlen) ;
	            debugprintf("hostinfo_enumname: 
	                name=%s\n",
	                op->h_aliases[cp->i]) ;
#endif

	            if (rpp != NULL)
	                *rpp = op->h_aliases[cp->i] ;

#if	CF_DEBUGS
	            debugprintf("hostinfo_enumname: 
	                more name=%s\n",*rpp) ;
#endif

	            cp->i += 1 ;
	        }

	    }
	}

	if ((nlen < 0) && (rpp != NULL))
	    *rpp = NULL ;

#if	CF_DEBUGS
	debugprintf("hostinfo_enumname: 
	    exiting i=%d nlen=%d\n",cp->i,nlen) ;
#endif

	return ((nlen < 0) ? SR_NOTFOUND : nlen) ;
}
/* end subroutine (hostinfo_enumname) */


/* enumerate the next host address */
int hostinfo_enumaddr(op,cp,rpp)
HOSTINFO	*op ;
HOSTINFO_CURSOR	*cp ;
unsigned char	**rpp ;
{
	int	rs = SR_NOTFOUND ;

#if	CF_DEBUGS
	inetaddr	ia ;

	char		iabuf[40] ;
#endif


	if (op == NULL)
	    return SR_FAULT ;

	if (cp == NULL) {

#if	CF_DEBUGS
	    debugprintf("hostinfo_enumaddr: 
	        no cursor\n") ;
#endif

	    if ((op->h_addr_list != NULL) && (op->h_addr_list[0] != NULL)) {

#if	CF_DEBUGS
	        debugprintf("hostinfo_enumaddr: 
	            addr=%08X\n",makeint(op->h_addr_list[0])) ;
#endif

	        rs = SR_OK ;
	        if (rpp != NULL)
	            *rpp = (uchar *) op->h_addr_list[0] ;

	    }

	} else {

#if	CF_DEBUGS
	    debugprintf("hostinfo_enumaddr: 
	        have cursor\n") ;
#endif

	    if (cp->i < 0) {

#if	CF_DEBUGS
	        debugprintf("hostinfo_enumaddr: 
	            starting\n") ;
#endif

	        if ((op->h_addr_list != NULL) && (op->h_addr_list[0])) {

#if	CF_DEBUGS
	            debugprintf("hostinfo_enumaddr: 
	                addr=%08X\n",makeint(op->h_addr_list[0])) ;
#endif

	            rs = SR_OK ;
	            cp->i = 0 ;
	            if (rpp != NULL)
	                *rpp = (uchar *) op->h_addr_list[0] ;

#if	CF_DEBUGS
	            if (rs >= 0) {
	                inetaddr_init(&ia,*rpp) ;

	                inetaddr_getdotaddr(&ia,iabuf,40) ;

	                debugprintf("hostinfo_enumaddr: 
	                    got addr=%s\n",
	                    (rs < 0) ? "*" : iabuf) ;

	            }
#endif /* CF_DEBUGS */

	        }

	    } else {

#if	CF_DEBUGS
	        debugprintf("hostinfo_enumaddr: 
	            continuing i=%d\n",cp->i) ;
#endif

	        if ((op->h_addr_list != NULL) &&
	            (op->h_addr_list[cp->i + 1] != NULL)) {

	            cp->i += 1 ;

#if	CF_DEBUGS
	            debugprintf("hostinfo_enumaddr: 
	                addr=%08X\n",
	                makeint(op->h_addr_list[cp->i])) ;
#endif

	            rs = SR_OK ;
	            if (rpp != NULL)
	                *rpp = (uchar *) op->h_addr_list[cp->i] ;

	        }

	    }
	}

#if	CF_DEBUGS
	if (rs >= 0) {
	    inetaddr_init(&ia,*rpp) ;
	    inetaddr_getdotaddr(&ia,iabuf,40) ;
	    debugprintf("hostinfo_enumaddr: 
	        got addr=%s\n",(rs < 0) ? "*" : iabuf) ;
	}
	inetaddr_free(&ia) ;
#endif /* CF_DEBUGS */

	if ((rs < 0) && (rpp != NULL))
	    *rpp = NULL ;

	return (rs >= 0) ? op->h_length : rs ;
}
/* end subroutine (hostinfo_enumaddr) */


int hostinfo_getcanonical(op,rpp)
HOSTINFO	*op ;
char		**rpp ;
{
	int	i ;


	if (op == NULL)
	    return SR_FAULT ;

	if (rpp == NULL)
	    return SR_FAULT ;

	if ((op->h_aliases == NULL) ||
	    (strchr(op->h_name,'.') != NULL)) {

	    *rpp = op->h_name ;
	    return strlen(op->h_name) ;
	}

	for (i = 0 ; op->h_aliases[i] != NULL ; i += 1) {

#if	CF_DEBUGS
	    debugprintf("getcanonical: 
	        alias%d a=\"%s\"\n",
	        i,op->h_aliases[i]) ;
#endif

	    if (strchr(op->h_aliases[i],'.') != NULL) {

	        *rpp = op->h_aliases[i] ;

#if	CF_DEBUGS
	        debugprintf("getcanonical: 
	            exiting w/ alias\n") ;
#endif

	        break ;

	    } /* end if */

	} /* end for */

	if (op->h_aliases[i] == NULL)
	    *rpp = op->h_name ;

	return strlen(*rpp) ;
}
/* end subroutine (hostinfo_getcanonical) */


/* get a fully qualified domain name */
int hostinfo_getfqdn(op,rpp)
HOSTINFO	*op ;
char		**rpp ;
{
	int	i ;
	int	nlen = -1 ;


	if (op == NULL)
	    return SR_FAULT ;


	if ((op->h_aliases == NULL) ||
	    (strchr(op->h_name,'.') != NULL)) {

	    if (rpp != NULL)
	        *rpp = op->h_name ;

	    return strlen(op->h_name) ;
	}

	for (i = 0 ; op->h_aliases[i] != NULL ; i += 1) {

#if	CF_DEBUGS
	    debugprintf("getcanonical: 
	        alias%d a=\"%s\"\n",
	        i,op->h_aliases[i]) ;
#endif

	    if (strchr(op->h_aliases[i],'.') != NULL) {

	        nlen = strlen(op->h_aliases[i]) ;

	        if (rpp != NULL)
	            *rpp = op->h_aliases[i] ;

#if	CF_DEBUGS
	        debugprintf("getcanonical: 
	            exiting w/ alias\n") ;
#endif

	        break ;

	    } /* end if */

	} /* end for */

	if ((nlen < 0) && (rpp != NULL))
	    *rpp = NULL ;

	return (nlen >= 0) ? nlen : SR_NOTFOUND ;
}
/* end subroutine (hostinfo_getfqdn) */

#endif /* COMMENT */


int hostinfo_enum(op,curp,rpp)
HOSTINFO	*op ;
HOSTINFO_CURSOR	*curp ;
struct addrinfo	**rpp ;
{
	struct addrinfo	*aip ;

	int	rs = SR_OK ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != HOSTINFO_MAGIC)
	    return SR_NOTOPEN ;

	if (curp == NULL)
	    return SR_FAULT ;

	if (rpp != NULL)
	    *rpp = NULL ;

#if	CF_DEBUGS
	debugprintf("hostinfo_enum: entered\n") ;
#endif

	aip = (curp->aip != NULL) ? curp->aip->ai_next : op->aip ;

	if (aip != NULL) {

	    if (rpp != NULL)
	        *rpp = aip ;

	    curp->aip = aip ;

	} else
	    rs = SR_NOTFOUND ;

#if	CF_DEBUGS
	debugprintf("hostinfo_enum: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (hostinfo_enum) */


int hostinfo_free(op)
HOSTINFO	*op ;
{


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != HOSTINFO_MAGIC)
	    return SR_NOTOPEN ;

	if (op->aip != NULL)
	    uc_freeaddrinfo(op->aip) ;

	op->magic = 0 ;
	return SR_OK ;
}
/* end subroutine (hostinfo_free) */


/* private subroutines */


#if	CF_DEBUGS

static int makeint(addr)
void	*addr ;
{
	int	hi = 0 ;

	uchar	*us = (uchar *) addr ;

	hi |= ((us[0] & 0xFF) << 24) ;
	hi |= ((us[1] & 0xFF) << 16) ;
	hi |= ((us[2] & 0xFF) << 8) ;
	hi |= (us[3] & 0xFF)  ;

	return hi ;
}
/* end subroutine (makeint) */

#endif /* CF_DEBUGS */


