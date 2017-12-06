/* dialtcpnls */

/* dial out to a machine server listening on TCPNLS */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-02-17, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will dial out to an INET host using the TCPMUX protocol
	and the optional TCP port and TCPMUX services that is specified.

	Synopsis:

	int dialtcpnls(hostname,portspec,af,svcbuf,to,options)
	const char	hostname[] ;
	const char	portspec[] ;
	int		af ;
	const char	svcbuf[] ;
	int		to, options ;

	Arguments:

	hostname	host to dial to
	portspec	port specification to use
	af		address family
	svcbuf		service specification
	to		to ('>0' means use one, '-1' means don't)
	options		any dial options

	Returns:

	>=0		file descriptor
	<0		error in dialing

	What is up with the 'to' argument?

	>0		use the to as it is

	==0             asynchronously span the connect and do not wait
			for it

	<0              use the system default to (xx minutes --
			whatever)


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<signal.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<sbuf.h>
#include	<char.h>
#include	<localmisc.h>

#include	"nlsdialassist.h"


/* local defines */

#define	SVC_TCPMUX	"tcpmux"
#define	SVC_LISTEN	"listen"
#define	SVC_TCPNLS	"tcpnls"

#ifndef	SVCLEN
#define	SVCLEN		MAXNAMELEN
#endif

#define	NLSBUFLEN	(SVCLEN + 30)
#define	RBUFLEN		MAXNAMELEN


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	dialtcp(const char *,const char *,int,int,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif /* CF_DEBUGS */

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int dialtcpnls(hostname,portspec,af,svcbuf,to,opts)
const char	hostname[] ;
const char	portspec[] ;
int		af ;
const char	svcbuf[] ;
int		to, opts ;
{
	struct sigaction	nsig, osig ;
	sigset_t	signalmask ;
	const int	nlslen = NLSBUFLEN ;
	int		rs ;
	int		svclen ;
	int		fd = -1 ;
	char		*nlsbuf ;

	if (hostname == NULL) return SR_FAULT ;
	if (svcbuf == NULL) return SR_FAULT ;

	if (hostname[0] == '\0') return SR_INVALID ;
	if (svcbuf[0] == '\0') return SR_INVALID ;

	if ((portspec == NULL) || (portspec[0] == '\0'))
	    portspec = NULL ;

#if	CF_DEBUGS
	debugprintf("dialtcpnls: hostname=%s\n",hostname) ;
	debugprintf("dialtcpnls: portspec=%s\n",portspec) ;
	debugprintf("dialtcpnls: svcbuf=%s\n",svcbuf) ;
	debugprintf("dialtcpnls: to=%d\n",to) ;
#endif

/* perform some cleanup on the service code specification */

	while (CHAR_ISWHITE(*svcbuf)) {
	    svcbuf += 1 ;
	}
	svclen = strlen(svcbuf) ;

	while (svclen && CHAR_ISWHITE(svcbuf[svclen - 1])) {
	    svclen -= 1 ;
	}

	if (svclen <= 0)
	    return SR_INVAL ;

#if	CF_DEBUGS
	debugprintf("dialtcpnls: final svcbuf=%t\n",svcbuf,svclen) ;
#endif

	if ((rs = uc_malloc((nlslen+1),&nlsbuf)) >= 0) {
	    if ((rs = mknlsreq(nlsbuf,nlslen,svcbuf,svclen)) >= 0) {
	        int	blen = rs ;

#if	CF_DEBUGS
	        debugprintf("dialtcpnls: nlsbuf len=%d\n",blen) ;
	        debugprintf("dialtcpnls: nlsbuf >%t<\n",nlsbuf,(blen - 1)) ;
#endif

	        uc_sigsetempty(&signalmask) ;

	        memset(&nsig,0,sizeof(struct sigaction)) ;
	        nsig.sa_handler = SIG_IGN ;
	        nsig.sa_mask = signalmask ;
	        nsig.sa_flags = 0 ;
	        if ((rs = u_sigaction(SIGPIPE,&nsig,&osig)) >= 0) {

#if	CF_DEBUGS
	            debugprintf("dialtcpnls: portspec=%s\n",
	                portspec) ;
#endif

	            if (portspec == NULL) {

#if	CF_DEBUGS
	                debugprintf("dialtcpnls: -listen- service\n") ;
#endif

	                portspec = SVC_LISTEN ;
	                rs = dialtcp(hostname,portspec,af,to,opts) ;
	                fd = rs ;

	                if ((rs < 0) && (rs != SR_CONNREFUSED)) {

#if	CF_DEBUGS
	                    debugprintf("dialtcpnls: dialing \n") ;
#endif

	                    portspec = SVC_TCPNLS ;
	                    rs = dialtcp(hostname,portspec,af,to,opts) ;
	                    fd = rs ;

#if	CF_DEBUGS
	                    debugprintf("dialtcpnls: dialtcp() rs=%d\n",rs) ;
#endif

	                } /* end if */

	            } else {
	                rs = dialtcp(hostname,portspec,af,to,opts) ;
	                fd = rs ;
	            }

#if	CF_DEBUGS
	            debugprintf("dialtcpnls: result rs=%d\n",rs) ;
#endif

	            if (rs >= 0) {

/* transmit the formatted service code and arguments */

#if	CF_DEBUGS
	                debugprintf("dialtcpnls: writing service l=%u\n",
	                    blen) ;
#endif

	                if ((rs = uc_writen(fd,nlsbuf,blen)) >= 0) {
	                    const int	rlen = RBUFLEN ;
	                    char	rbuf[RBUFLEN+1] = { 0 } ;
	                    rs = readnlsresp(fd,rbuf,rlen,to) ;
	                } /* end if (read response) */

	                if (rs < 0) u_close(fd) ;
	            } /* end if (opened) */

	            u_sigaction(SIGPIPE,&osig,NULL) ;
	        } /* end if (sigaction) */

	    } else
	        rs = SR_TOOBIG ;
	    uc_free(nlsbuf) ;
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("dialtcpnls: ret rs=%d s=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (dialtcpnls) */


