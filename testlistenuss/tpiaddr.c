/* tpiaddr */

/* manipulate socket addresses */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1994-01-13, David A­D­ Morano
	This object module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#define	TPIADDR_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<netinet/in.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"tpiaddr.h"


/* local object defines */

#ifndef	AF_INET6
#define	AF_INET6	26
#endif

#ifndef	INETXADDRLEN
#define	INETXADDRLEN	sizeof(struct in_addr)
#endif

#ifndef	INET6ADDRLEN
#define	INET6ADDRLEN	16
#endif


/* external subroutines */

extern char	*strwcpy(char *,char *,int) ;


/* local structures */


/* forward references */

static int	tpiaddr_initaddr() ;


/* local data */

static const char	hextable[] = {
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
} ;


/* exported subroutines */


int tpiaddr_init(sap,type,addr,port,flow)
TPIADDR	*sap ;
int		type, port ;
void		*addr ;
uint		flow ;
{
	int	rs ;


	rs = tpiaddr_initaddr(sap,type,addr,-1,port,flow) ;

	return rs ;
}
/* end subroutine (tpiaddr_init) */


int tpiaddr_initaddr(sap,type,addr,alen,port,flow)
TPIADDR	*sap ;
int		type, port ;
void		*addr ;
int		alen ;
uint		flow ;
{
	struct sockaddr		*dsap ;

	struct sockaddr_in	*inet_sap ;

	struct tpiaddr_inet6	*inet6_sap ;

	char	*cp ;


	if (sap == NULL)
	    return SR_FAULT ;

	switch (type) {

	case AF_UNIX:
	    sap->a_unspec.sa_family = (ushort) htons(type) ;

	    dsap = (struct sockaddr *) sap ;
	    cp = strwcpy(dsap->sa_data,addr,MAXPATHLEN) ;

	    (void) memset(cp,0,(MAXPATHLEN - (cp - dsap->sa_data))) ;

	    break ;

	case AF_INET:
	    inet_sap = (struct sockaddr_in *) sap ;
	    (void) memset(inet_sap,0,sizeof(struct sockaddr_in)) ;

	    inet_sap->sin_family = (short) htons(type) ;

	    inet_sap->sin_port = (ushort) htons(port) ;

	    (void) memcpy(&inet_sap->sin_addr,addr,sizeof(struct in_addr)) ;

	    break ;

	case AF_INET6:
	    inet6_sap = (struct tpiaddr_inet6 *) sap ;
	    (void) memset(inet_sap,0,sizeof(struct tpiaddr_inet6)) ;

	    inet6_sap->af = (ushort) htons(type) ;

	    inet6_sap->port = (ushort) htons(port) ;

	    inet6_sap->flow = (uint) htonl(flow) ;

	    (void) memcpy(inet6_sap->aa,addr,16) ;

	    break ;

	default:
	    sap->a_unspec.sa_family = 0 ;
	    return SR_INVALID ;

	} /* end switch */

	return SR_OK ;
}
/* end subroutine (tpiaddr_initaddr) */


/* get the address family type for this tpiaddr */
int tpiaddr_getaf(sap,rp)
TPIADDR	*sap ;
int		*rp ;
{
	int	type ;


	if (sap == NULL)
	    return SR_FAULT ;

	type = ntohs(sap->a_unspec.sa_family) ;

	if (rp != NULL)
	    *rp = type ;

	return type ;
}
/* end subroutine (tpiaddr_getaf) */


/* get the port for this tpiaddr (based on address family type) */
int tpiaddr_getport(sap,rp)
TPIADDR	*sap ;
int		*rp ;
{
	struct sockaddr_in	*inet_sap ;

	struct tpiaddr_inet6	*inet6_sap ;

	int		type, port ;


	if (sap == NULL)
	    return SR_FAULT ;

	type = ntohs(sap->a_unspec.sa_family) ;

	switch (type) {

	case AF_INET:
	    inet_sap = (struct sockaddr_in *) sap ;
	    port = ntohs(inet_sap->sin_port) ;
	    break ;

	case AF_INET6:
	    inet6_sap = (struct tpiaddr_inet6 *) sap ;
	    port = ntohs(inet6_sap->port) ;
	    break ;

	default:
	    port = SR_INVALID ;

	} /* end switch */

	if (rp != NULL)
	    *rp = port ;

	return port ;
}
/* end subroutine (tpiaddr_getport) */


/* get the address for this tpiaddr (based on address type) */
int tpiaddr_getaddr(sap,abuf,abuflen)
TPIADDR	*sap ;
char		abuf[] ;
int		abuflen ;
{
	struct sockaddr_un	*unix_sap ;

	struct sockaddr_in	*inet_sap ;

	struct tpiaddr_inet6	*inet6_sap ;

	int		rs, type ;


	if (sap == NULL)
	    return SR_FAULT ;

	if (abuf == NULL)
	    return SR_FAULT ;

	type = ntohs(sap->a_unspec.sa_family) ;

	switch (type) {

	case AF_UNIX:
	    unix_sap = (struct sockaddr_un *) sap ;
	    if (abuflen >= 0) {
	        strwcpy(abuf,unix_sap->sun_path,MIN(abuflen,MAXPATHLEN)) ;
	    } else
	        strcpy(abuf,unix_sap->sun_path) ;

	    break ;

	case AF_INET:
	    inet_sap = (struct sockaddr_in *) sap ;
	    if (abuflen >= INETXADDRLEN) {

	        rs = SR_OK ;
	        (void) memcpy(abuf,&inet_sap->sin_addr,INETXADDRLEN) ;

	    } else
	        rs = SR_TOOBIG ;

	    break ;

	case AF_INET6:
	    inet6_sap = (struct tpiaddr_inet6 *) sap ;
	    if (abuflen >= 16) {

	        rs = SR_OK ;
	        (void) memcpy(abuf,&inet6_sap->aa,16) ;

	    } else
	        rs = SR_TOOBIG ;

	    break ;

	default:
	    return SR_INVALID ;

	} /* end switch */

	return rs ;
}
/* end subroutine (tpiaddr_getaddr) */


/* get the entire socket address (the TLI address) in HEXADECIMAL */
int tpiaddr_gethex(sap,buf,buflen)
TPIADDR	*sap ;
char		buf[] ;
int		buflen ;
{
	int	i, j ;
	int	v ;
	int	type, salen ;


	if (sap == NULL)
	    return SR_FAULT ;

	if (buf == NULL)
	    return SR_FAULT ;

	type = ntohs(sap->a_unspec.sa_family) ;

	switch (type) {

	case AF_UNIX:
	    {
	        char	*ap = (char *) sap ;


	        salen = MIN((strlen(ap + 2) + 2),sizeof(TPIADDR)) ;

	    }

	    break ;

	case AF_INET:
	    salen = sizeof(struct sockaddr_in) ;
	    break ;

	case AF_INET6:
	    salen = sizeof(struct tpiaddr_inet6) ;
	    break ;

	default:
	    return SR_NOTFOUND ;

	} /* end switch */

	if ((buflen >= 0) && (buflen < ((salen * 2) + 1)))
	    return SR_TOOBIG ;

#if	CF_DEBUGS
	debugprintf("tpiaddr_gethex: salen=%d\n",salen) ;
#endif

	j = 0 ;
	for (i = 0 ; i < salen ; i += 1) {

	    v = ((int) sap->str[i]) & 0xFF ;
	    buf[j++] = hextable[(v >> 4) & 15] ;
	    buf[j++] = hextable[v & 15] ;

	} /* end for */

	buf[j++] = '\0' ;
	return j ;
}
/* end subroutine (tpiaddr_gethex) */


/* get the entire socket address length (like for a TPI address) */
int tpiaddr_getlen(sap)
TPIADDR	*sap ;
{
	int	type, salen ;


	if (sap == NULL)
	    return SR_FAULT ;

	type = ntohs(sap->a_unspec.sa_family) ;

	switch (type) {

	case AF_UNIX:
	    {
	        char	*ap = (char *) sap ;


	        salen = MIN((strlen(ap + 2) + 2),sizeof(TPIADDR)) ;

	    }

	    break ;

	case AF_INET:
	    salen = sizeof(struct sockaddr_in) ;
	    break ;

	case AF_INET6:
	    salen = sizeof(struct tpiaddr_inet6) ;
	    break ;

	default:
	    return SR_NOTFOUND ;

	} /* end switch */

	return salen ;
}
/* end subroutine (tpiaddr_getlen) */


int tpiaddr_free(sap)
TPIADDR	*sap ;
{


	if (sap == NULL)
	    return SR_FAULT ;

	sap->a_unspec.sa_family = 0 ;
	return SR_OK ;
}
/* end subroutine (tpiaddr_free) */


/* put a transport-specific-part address in the current socket address object */
int tpiaddr_putaddr(sap,buf)
TPIADDR	*sap ;
char		buf[] ;
{
	struct sockaddr		*dsap ;

	struct sockaddr_in	*inet_sap ;

	struct tpiaddr_inet6	*inet6_sap ;

	int	i, j = 0 ;
	int	v ;
	int	type, salen ;


	if (sap == NULL)
	    return SR_FAULT ;

	if (buf == NULL)
	    return SR_FAULT ;

	type = ntohs(sap->a_unspec.sa_family) ;

	switch (type) {

	case AF_UNIX:
	    dsap = (struct sockaddr *) sap ;
	    strcpy(dsap->sa_data,buf) ;

	    break ;

	case AF_INET:
	    inet_sap = (struct sockaddr_in *) sap ;
	    (void) memcpy(&inet_sap->sin_addr,buf,sizeof(struct in_addr)) ;

	    break ;

	case AF_INET6:
	    inet6_sap = (struct tpiaddr_inet6 *) sap ;
	    (void) memcpy(&inet6_sap->aa,buf,16) ;

	    break ;

	default:
	    return SR_NOTFOUND ;

	} /* end switch */

	return SR_OK ;
}
/* end subroutine (tpiaddr_putaddr) */



/* CONSTRUCTORS & DESTRUCTORS */



TPIADDR obj_tpiaddr(type,addr,port,flow)
int		type, port ;
void		*addr ;
int		flow ;
{
	tpiaddr	temp ;


	(void) tpiaddr_init(&temp,type,addr,port,flow) ;

	return temp ;
}


TPIADDR *new_tpiaddr(type,addr,port,flow)
int		type, port ;
void		*addr ;
int		flow ;
{
	tpiaddr	*sop = NULL ;


	if (uc_malloc(sizeof(tpiaddr),((void *) &sop)) >= 0)
	    (void) tpiaddr_init(sop,type,addr,port,flow) ;

	return sop ;
}


void free_tpiaddr(sop)
TPIADDR	*sop ;
{


	(void) tpiaddr_free(sop) ;

	free(sop) ;

}



