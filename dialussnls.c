/* dialussnls */

/* dial out to a server listening on UNIX®NLS */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is the NLS version of the TCP dialer.


*******************************************************************************/

#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<buffer.h>
#include	<char.h>
#include	<localmisc.h>

#include	"nlsdialassist.h"


/* local defines */

#define	SRV_TCPMUX	"tcpmux"
#define	SRV_LISTEN	"listen"
#define	SRV_TCPNLS	"tcpnls"

#ifndef	SVCLEN
#define	SVCLEN		MAXNAMELEN
#endif

#define	NLSBUFLEN	(SVCLEN + 30)
#define	RBUFLEN		MAXNAMELEN


/* external subroutines */

extern int	dialuss(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int dialussnls(cchar *portspec,cchar *svcbuf,int to,int aopts)
{
	const int	nlslen = NLSBUFLEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		svclen ;
	int		fd = -1 ;
	char		*nlsbuf ;

#if	CF_DEBUGS
	debugprintf("dialussnls: ent to=%d\n",to) ;
#endif

	if ((portspec == NULL) || (portspec[0] == '\0'))
	    portspec = NULL ;

#if	CF_DEBUGS
	debugprintf("dialussnls: portspec=%s\n",portspec) ;
#endif

/* perform some cleanup on the service code specification */

	if (svcbuf == NULL)
	    return SR_INVAL ;

	while (CHAR_ISWHITE(*svcbuf))
	    svcbuf += 1 ;

	svclen = strlen(svcbuf) ;

	while (svclen && CHAR_ISWHITE(svcbuf[svclen - 1]))
	    svclen -= 1 ;

	if (svclen <= 0)
	    return SR_INVAL ;

#if	CF_DEBUGS
	debugprintf("dialussnls: final svcbuf=%t\n",svcbuf,svclen) ;
#endif

/* format the service string to be transmitted */

	if ((rs = uc_malloc((nlslen+1),&nlsbuf)) >= 0) {
	    if ((rs = mknlsreq(nlsbuf,nlslen,svcbuf,svclen)) >= 0) {
	        struct sigaction	osig, nsig ;
	        sigset_t		signalmask ;
	        int			blen = rs ;

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

	            if (portspec == NULL)
	                portspec = "/tmp/unix" ;

#if	CF_DEBUGS
	            debugprintf("dialussnls: dialing the -listen- service\n") ;
#endif

	            if ((rs = dialuss(portspec,to,aopts)) >= 0) {
	                fd = rs ;

	                if ((rs = uc_writen(fd,nlsbuf,blen)) >= 0) {
	                    const int	rlen = RBUFLEN ;
	                    char	rbuf[RBUFLEN+1] = { 0 } ;
	                    rs = readnlsresp(fd,rbuf,rlen,to) ;
	                } /* end if (read response) */

	                if (rs < 0) u_close(fd) ;
	            } /* end if (opened) */

	            u_sigaction(SIGPIPE,&osig,NULL) ;
	        } /* end if (sigaction) */

	    } else {
	        rs = SR_TOOBIG ;
	    }
	    rs1 = uc_free(nlsbuf) ;
	    if (rs >= 0) rs = rs1 ;
	    if ((rs < 0) && (fd >= 0)) u_close(fd) ;
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("dialussnls: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (dialussnls) */


