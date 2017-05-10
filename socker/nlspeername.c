/* nlspeername */

/* handle a connect request for a service */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subrotuine tries to find the peername for the given NLS (TLI)
	address.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<bfile.h>
#include	<field.h>
#include	<hostent.h>
#include	<sockaddress.h>
#include	<inetaddr.h>
#include	<localmisc.h>


/* local defines */

#ifndef	INETXADDRLEN
#define	INETXADDRLEN	sizeof(struct in_addr)
#endif

#ifndef	PEERNAMELEN
#define	PEERNAMELEN	MAX(MAXHOSTNAMELEN,MAXPATHLEN)
#endif


/* external subroutines */

extern int	cfhexui(const char *,int,uint *) ;
extern int	cfhexi(const char *,int,int *) ;
extern int	cfhexs(const char *,int,char *) ;
extern int	getheaddr(const void *,struct hostent *,char *,int) ;
extern int	isindomain(cchar *,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* forward references */

static int	nlspeername_unix(char *,cchar *,cchar *,int) ;
static int	nlspeername_inet4(char *,cchar *,cchar *,int) ;


/* local variables */


/* exported subroutines */


int nlspeername(cchar *addr,cchar *dn,char *pn)
{
	int		rs = SR_OK ;
	int		al ;
	int		family ;
	int		len = 0 ;

	if (addr == NULL) return SR_FAULT ;
	if (pn == NULL) return SR_FAULT ;

	pn[0] = '\0' ;
	al = strlen(addr) ;

	if (al >= 16) {
	    if ((rs = cfhexi(addr,4,&family)) >= 0) {
	        if (family == AF_UNIX) {
	            rs = nlspeername_unix(pn,dn,addr,al) ;
	        } else if (family == AF_INET4) {
	            rs = nlspeername_inet4(pn,dn,addr,al) ;
	        } /* end if (recognized the address family) */
	    } /* end if (cfhexi) */
	} else
	    rs = SR_INVALID ;

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (nlspeername) */


/* local subroutines */


/* ARGSUSED */
static int nlspeername_unix(char *pn,cchar *dn,cchar *addr,int al)
{
	const int	nlen = MAXHOSTNAMELEN ;
	int		rs = SR_OK ;
	int		len = 0 ;
	al -= 4 ;
	if ((al / 2) <= nlen) {
	    rs = cfhexs((addr + 4),al,pn) ;
	    len = rs ;
	} else {
	    rs = SR_TOOBIG ;
	}
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (nlspeername_unix) */


static int nlspeername_inet4(char *pn,cchar *dn,cchar *ap,int al)
{
	uint		inetaddr ;
	int		rs ;
	int		len = 0 ;
	if ((rs = cfhexui(ap+8,8,&inetaddr)) >= 0) {
	    HOSTENT	he ;
	    HOSTENT_CUR	hc ;
	    const int	helen = getbufsize(getbufsize_he) ;
	    const int	family = rs ;
	    char	*hebuf ;
	    if ((rs = uc_malloc((helen+1),&hebuf)) >= 0) {
	        if ((rs = getheaddr(&inetaddr,&he,hebuf,helen)) >= 0) {
	            if ((rs = hostent_getaf(&he)) >= 0) {
	                const int	af = rs ;
	                if (af == family) {
	                    const int	nlen = MAXHOSTNAMELEN ;
	                    cchar	*cp ;

	                    if ((rs >= 0) && (dn != NULL)) {
	                        if ((rs = hostent_curbegin(&he,&hc)) >= 0) {
	                            cchar	*np ;
	                            while (hostent_enumname(&he,&hc,&np) >= 0) {
	                                if (isindomain(np,dn)) {
	                                    cp = strwcpy(pn,he.h_name,nlen) ;
	                                    len = (cp - pn) ;
	                                    break ;
	                                }
	                            } /* end while */
	                            hostent_curend(&he,&hc) ;
	                        } /* end if */
	                    } /* end if (have a domain name) */

	                    if (rs >= 0) {
	                        if ((pn[0] == '\0') && (he.h_name != NULL)) {
	                            cp = strwcpy(pn,he.h_name,nlen) ;
	                            len = (cp - pn) ;
	                        }
	                    }

	                } else
	                    rs = SR_NOTFOUND ;
	            } /* end if (gethostent_af) */
	        } /* end if (getheaddr) */
	        uc_free(hebuf) ;
	    } /* end if (m-a) */
	} /* end if (cfhexui) */
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (nlspeername_inet4) */


