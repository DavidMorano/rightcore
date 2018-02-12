/* opendial */

/* open a channel using a dialer */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We brought several dialers together here to make it more convenient to
	allow for a range of dialers to be used for connecting to some
	service.

	Synopsis:

	int opendial(dialer,af,hostspec,portspec,svc,av,ev,to,opts)
	int		dialer ;
	int		af ;
	const char	*hostspec ;
	const char	*portspec ;
	const char	*svc;
	const char	*av[] ;
	const char	*ev[] ;
	int		to ;
	int		opts ;

	Arguments:

	dialer		dialer to use
	af		socket address-family
	hostspec	host (string)
	portspec	port (string)
	svc		service (string)
	av		argument string vector
	ev		argument string vector
	opts		options
	to		time-out


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<sigman.h>
#include	<localmisc.h>

#include	"opendial.h"


/* local defines */


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	isprintlatin(int) ;
extern int	getaf(const char *,int) ;
extern int	dialudp(cchar *,cchar *,int,int,int) ;
extern int	dialtcp(cchar *,cchar *,int,int,int) ;
extern int	dialtcpnls(cchar *,cchar *,int,cchar *,int,int) ;
extern int	dialtcpmux(cchar *,cchar *,int,cchar *,cchar **,int,int) ;
extern int	dialuss(const char *,int,int) ;
extern int	dialussnls(const char *,const char *,int,int) ;
extern int	dialussmux(const char *,const char *,const char **,int,int) ;
extern int	dialticotsord(const char *,int,int,int) ;
extern int	dialticotsordnls(const char *,int,const char *,int,int) ;
extern int	dialpass(const char *,int,int) ;
extern int	dialprog(const char *,int,const char **,const char **,int *) ;
extern int	dialfinger(cchar *,cchar *,int,cchar *,cchar **,int,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif /* CF_DEBUGS */

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strwcpyuc(char *,const char *,int) ;
extern char	*strcpylc(char *,const char *) ;
extern char	*strcpyuc(char *,const char *) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int opendial(dialer,af,hostspec,portspec,svc,av,ev,to,opts)
int		dialer ;
int		af ;
const char	*hostspec ;
const char	*portspec ;
const char	*svc ;
const char	*av[] ;
const char	*ev[] ;
int		opts ;
int		to ;
{
	int		rs = SR_OK ;
	int		oflags ;
	const char	*cp ;

	if (hostspec == NULL) return SR_FAULT ;

	if (dialer < 0) return SR_INVALID ;
	if (af < 0) return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("opendial: dialer=%d\n",dialer) ;
	debugprintf("opendial: hostspec=%s\n",hostspec) ;
	debugprintf("opendial: portspec=%s\n",portspec) ;
	debugprintf("opendial: svc=%s\n",svc) ;
#endif

	switch (dialer) {
	case opendialer_udp:
	    cp = portspec ;
	    if ((cp == NULL) || (cp[0] == '\0')) cp = svc ;
	    rs = dialudp(hostspec,cp,af,to,opts) ;
	    break ;
	case opendialer_tcp:
	    cp = portspec ;
	    if ((cp == NULL) || (cp[0] == '\0')) cp = svc ;
	    rs = dialtcp(hostspec,cp,af,to,opts) ;
	    break ;
	case opendialer_tcpmux:
	    rs = dialtcpmux(hostspec,portspec,af,svc,av,to,opts) ;
	    break ;
	case opendialer_tcpnls:
	    rs = dialtcpnls(hostspec,portspec,af,svc,to,opts) ;
	    break ;
	case opendialer_uss:
	    cp = portspec ;
	    if ((cp == NULL) || (cp[0] == '\0')) cp = svc ;
	    rs = dialuss(cp,to,opts) ;
	    break ;
	case opendialer_ussmux:
	    rs = dialussmux(portspec,svc,av,to,opts) ;
	    break ;
	case opendialer_ussnls:
	    rs = dialussnls(portspec,svc,to,opts) ;
	    break ;
	case opendialer_ticotsord:
	    cp = portspec ;
	    if ((cp == NULL) || (cp[0] == '\0')) cp = svc ;
	    rs = dialticotsord(cp,-1,to,opts) ;
	    break ;
	case opendialer_ticotsordnls:
	    rs = dialticotsordnls(portspec,-1,svc,to,opts) ;
	    break ;
	case opendialer_pass:
	    cp = portspec ;
	    if ((cp == NULL) || (cp[0] == '\0')) cp = svc ;
	    rs = dialpass(cp,to,opts) ;
	    break ;
	case opendialer_open:
	    cp = portspec ;
	    if ((cp == NULL) || (cp[0] == '\0')) cp = svc ;
	    oflags = O_RDWR ;
	    rs = uc_openenv(cp,oflags,0666,ev,to) ;
	    break ;
	case opendialer_prog:
	    cp = portspec ;
	    if ((cp == NULL) || (cp[0] == '\0')) cp = svc ;
	    oflags = O_RDWR ;
	    rs = dialprog(cp,oflags,av,ev,NULL) ;
	    break ;
	case opendialer_finger:
	    rs = dialfinger(hostspec,portspec,af,svc,av,to,opts) ;
	    break ;
	default:
	    rs = SR_INVALID ;
	    break ;
	} /* end switch */

#if	CF_DEBUGS
	debugprintf("opendial: ret rs=%d\n",rs) ;
#endif /* CF_DEBUGS */

	return rs ;
}
/* end subroutine (opendial) */


