/* peerhostname */

/* get a peer host name if there is one */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0


/* revision history:

	= 1986-07-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will take an INET socket and a local domain name and
	find the hostname of the remote end of the socket.


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
#include	<hostent.h>
#include	<sockaddress.h>
#include	<inetaddr.h>
#include	<localmisc.h>


/* local defines */

#ifndef	INETXADDRLEN
#define	INETXADDRLEN	sizeof(struct in_addr)
#endif


/* external subroutines */

extern int	isindomain(cchar *,cchar *) ;
extern int	isNotPresent(int) ;

extern char	*strwcpy(char *,cchar *,int) ;


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int peerhostname(int s,cchar *domainname,char *peername)
{
	sockaddress	sa ;
	int		alen = sizeof(sockaddress) ;
	int		rs ;

	if (peername == NULL) return SR_FAULT ;

	peername[0] = '\0' ;
	if ((rs = u_getpeername(s,&sa,&alen)) >= 0) {
	    if ((rs = sockaddress_getaf(&sa)) >= 0) {
		const int	af = rs ;
		if (af == AF_INET) {
		    struct hostent	he ;
		    struct in_addr	naddr ;
		    const int		helen = getbufsize(getbufsize_he) ;
		    const int		ialen = INETXADDRLEN ;
		    char		*hebuf ;
	    	    sockaddress_getaddr(&sa,&naddr,ialen) ;
		    if ((rs = uc_malloc((helen+1),&hebuf)) >= 0) {
			cchar		*na = (cchar *) &naddr ;
			const int	nlen = MAXHOSTNAMELEN ;

/* lookup this IP (INET4) address */

	    	        rs = uc_gethostbyaddr(na,ialen,af,&he,hebuf,helen) ;
	    		if (rs >= 0) {
	        	    hostent_cur	hc ;
	        	    const char	*np ;

	        if (domainname != NULL) {
	            if ((rs = hostent_curbegin(&he,&hc)) >= 0) {
	                while ((rs = hostent_enumname(&he,&hc,&np)) >= 0) {
	                    if (isindomain(np,domainname)) break ;
	                } /* end while */
	                hostent_curend(&he,&hc) ;
		    } /* end if (hostent) */
	        } else
	            rs = SR_HOSTUNREACH ;

	        if (rs >= 0) {
		    cchar	*tp ;
	            strwcpy(peername,np,nlen) ;
	            if ((tp = strchr(peername,'.')) != NULL) {
			peername[tp-peername] = '\0' ;
		    }
	        } else {
	            hostent_getcanonical(&he,&np) ;
	            strwcpy(peername,np,nlen) ;
	        } /* end if */

	    } else if (isNotPresent(rs)) {
	        inetaddr	ia ;
	        if ((rs = inetaddr_start(&ia,&na)) >= 0) {
	            rs = inetaddr_getdotaddr(&ia,peername,nlen) ;
	            inetaddr_finish(&ia) ;
		} /* end if */
	    } /* end if */

			uc_free(hebuf) ;
		    } /* end if (m-a) */
		} /* end if (INET) */
	    } /* end if (sockaddress_getaf) */
	} /* end if (got an INET host entry) */

	return (peername[0] != '\0') ? strlen(peername) : SR_NOTFOUND ;
}
/* end subroutine (peerhostname) */


