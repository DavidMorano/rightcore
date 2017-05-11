/* dialticotsordnls */

/* dial out to a server listening on TI-COTS-ORD-NLS */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will dial out to the TICOTSORD transport which
	has the NLS listener on it.

	Synopsis:

	int dialticotsordnls(abuf,alen,svcbuf,to,opts)
	const char	abuf[] ;
	const char	svcbuf[] ;
	int		alen ;
	int		to ;
	int		opts ;

	Arguments:

	abuf		XTI address
	alen		address of XTI address
	svcbuf		service specification
	to		to ('>=0' mean use one, '-1' means don't)
	opts		any dial opts

	Returns:

	>=0		file descriptor
	<0		error in dialing


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sbuf.h>
#include	<char.h>
#include	<sigblock.h>
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
#define	NLSBUFLEN	(SVCLEN + 30)

#define	TBUFLEN		MAXNAMELEN

#define	HEXBUFLEN	((2 * MAXPATHLEN) + 2)


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	dialticotsord(const char *,int,int,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif /* CF_DEBUGS */

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */

#if	CF_DEBUGS
extern int	mkhexstr(char *,int,const void *,int) ;
#endif


/* local vaiables */


/* exported subroutines */


int dialticotsordnls(abuf,alen,svcbuf,to,opts)
const char	abuf[] ;
const char	svcbuf[] ;
int		alen ;
int		to ;
int		opts ;
{
	struct sigaction	sigs, oldsigs ;
	sigset_t	signalmask ;
	const int	nlslen = NLSBUFLEN ;
	int		rs = SR_OK ;
	int		svclen ;
	int		fd = -1 ;
	char		nlsbuf[NLSBUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("dialticotsordnls: ent to=%d\n",to) ;
#endif

	if ((abuf != NULL) && (abuf[0] == '\0'))
	    abuf = NULL ;

#if	CF_DEBUGS
	debugprintf("dialticotsordnls: alen=%d\n",alen) ;
#endif

#if	CF_DEBUGS
	{
	    int		hl ;
	    char	hbuf[HEXBUFLEN + 1] ;
	    if ((hl = mkhexstr(hbuf,HEXBUFLEN,abuf,alen)) >= 0) {
	        debugprintf("dialticotsordnls: XTI alen=%d abuf=%t\n",
	            alen,hbuf,hl) ;
	    }
	}
#endif /* CF_DEBUGS */

/* perform some cleanup on the service code specification */

	if (svcbuf == NULL)
	    return SR_INVAL ;

	while (CHAR_ISWHITE(*svcbuf)) svcbuf += 1 ;
	svclen = strlen(svcbuf) ;

	while (svclen && CHAR_ISWHITE(svcbuf[svclen - 1])) {
	    svclen -= 1 ;
	}

	if (svclen <= 0)
	    return SR_INVAL ;

#if	CF_DEBUGS
	debugprintf("dialticotsordnls: final svcbuf=%t\n",svcbuf,svclen) ;
#endif

	if (abuf == NULL) {
	    abuf = "local" ;
	    alen = strlen(abuf) ;
	} /* end if (default UNIX® address!) */

	if ((rs = mknlsreq(nlsbuf,nlslen,svcbuf,svclen)) >= 0) {
	    const int	blen = rs ;

#if	CF_DEBUGS
	    debugprintf("dialticotsordnls: nlsbuf len=%d\n",blen) ;
	    debugprintf("dialticotsordnls: nlsbuf >%t<\n",nlsbuf,(blen - 1)) ;
#endif

	    uc_sigsetempty(&signalmask) ;

	    memset(&sigs,0,sizeof(struct sigaction)) ;
	    sigs.sa_handler = SIG_IGN ;
	    sigs.sa_mask = signalmask ;
	    sigs.sa_flags = 0 ;
	    if ((rs = u_sigaction(SIGPIPE,&sigs,&oldsigs)) >= 0) {

#if	CF_DEBUGS
	        debugprintf("dialticotsordnls: about to 'dialticotsord'\n") ;
#endif

	        if ((rs = dialticotsord(abuf,alen,to,opts)) >= 0) {
	            fd = rs ;

#if	CF_DEBUGS
	            debugprintf("dialticotsordnls: dialticotsord() rs=%d\n",
			rs) ;
#endif

/* transmit the formatted service code and arguments */

#if	CF_DEBUGS
	            debugprintf("dialticotsordnls: writing service format\n") ;
#endif

	            rs = uc_writen(fd,nlsbuf,blen) ;

#if	CF_DEBUGS
	            debugprintf("dialticotsordnls: sent NLPS rs=%d\n",
	                rs) ;
#endif

	            if (rs >= 0) {
	                const int	tlen = TBUFLEN ;
	                char		tbuf[TBUFLEN+1] ;
	                rs = readnlsresp(fd,tbuf,tlen,to) ;
#if	CF_DEBUGS
	                debugprintf("dialticotsordnls: readnlsresp() rs=%d\n",
				rs) ;
	                debugprintf("dialticotsordnls: text=>%s<\n",tbuf) ;
#endif
	            } /* end if (reading response) */

#if	CF_DEBUGS
	            debugprintf("dialticotsordnls: readline-out rs=%d\n",rs) ;
#endif

	            if (rs < 0) u_close(fd) ;
	        } /* end if (opened) */

#if	CF_DEBUGS
	            debugprintf("dialticotsordnls: dial-out rs=%d\n",rs) ;
#endif

	        u_sigaction(SIGPIPE,&oldsigs,NULL) ;
	    } /* end if (sig-action) */

	} else
	    rs = SR_TOOBIG ;

#if	CF_DEBUGS
	debugprintf("dialticotsordnls: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (dialticotsordnls) */


