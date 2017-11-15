/* progpeername */

/* try to divine a peer-hostname from a socket address */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* switchable debug print-outs */


/* revision history:

	= 2008-06-23, David A­D­ Morano
	This is split off from the 'progwatch()' module.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine tries to divine a hostname associated with a socket
        address.

	progpeername(pip,cip,peername)
	PROGINFO	*pip ;
	CLIENTINFO	*cip ;
	char		peername[] ;

	Arguments:

	pip		program information pointer
	cip		client information pointer
	peername	caller-specified buffer to receive result

	Returns:

	- 		length of resulting hostname


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<sockaddress.h>
#include	<connection.h>
#include	<localmisc.h>

#include	"defs.h"
#include	"config.h"
#include	"clientinfo.h"


/* local defines */

#ifndef	LOCALHOST
#define	LOCALHOST	"localhost"
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#undef	HEXBUFLEN
#define	HEXBUFLEN	120


/* external subroutines */

extern int	snddd(char *,int,uint,uint) ;
extern int	snsdd(char *,int,const char *,uint) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	dupup(int,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	nlspeername(const char *,const char *,char *) ;
extern int	isascoket(int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	procsocket(PROGINFO *,CLIENTINFO *,char *) ;


/* local variables */


/* exported subroutines */


int progpeername(pip,cip,peername)
PROGINFO	*pip ;
CLIENTINFO	*cip ;
char		peername[] ;
{
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		rs1 = 0 ;
	int		f_success = FALSE ;
	const char	*cp ;

	if (cip == NULL) return SR_FAULT ;
	if (peername == NULL) return SR_FAULT ;

	peername[0] = '\0' ;

/* pipe? */

	if ((rs >= 0) && (peername[0] == '\0')) {
	    rs = u_fstat(cip->fd_input,&sb) ;
	    if ((rs >= 0) && S_ISFIFO(sb.st_mode)) {
	        cip->f_local = TRUE ;
	        rs = sncpy1(peername,MAXHOSTNAMELEN,LOCALHOST) ;
	    }
	}

/* use 'connection(3dam)' */

	if ((rs >= 0) && (peername[0] == '\0')) {
	    rs = procsocket(pip,cip,peername) ;
	    if ((rs >= 0) && (peername[0] == '/'))
		cip->f_local = TRUE ;
	}

/* use 'nlspeername(3dam)' */

	if ((rs >= 0) && (peername[0] == '\0')) {
	    if ((cp = getenv(VARNLSADDR)) != NULL) {
	        rs1 = nlspeername(cp,pip->domainname,peername) ;
		if (rs1 < 0) peername[0] = '\0' ;
	    }
	} /* end if */

/* done */

	if ((rs >= 0) && (peername[0] != '\0')) {
	    f_success = TRUE ;
	    rs = vecstr_store(&cip->stores,peername,-1,&cp) ;
	    if (rs >= 0) cip->peername = cp ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progserve/progpeername: ret rs=%d peername=%s\n",
	        rs,peername) ;
#endif

	return (rs >= 0) ? f_success : rs ;
}
/* end subroutines (progpeername) */


/* local subroutines */


/* use 'connection(3dam)' */
static int procsocket(pip,cip,peername)
PROGINFO	*pip ;
CLIENTINFO	*cip ;
char		peername[] ;
{
	CONNECTION	conn, *cnp = &conn ;
	int		rs ;
	int		rs1 = 0 ;
	int		f = FALSE ;

	peername[0] = '\0' ;
	if ((rs = connection_start(cnp,pip->domainname)) >= 0) {

	    if (cip->salen > 0) {
	        SOCKADDRESS	*sap = &cip->sa ;
	        const int	sal = cip->salen ;
	        rs1 = connection_peername(cnp,sap,sal,peername) ;
	    } else {
	        int		ifd = cip->fd_input ;
	        rs1 = connection_sockpeername(cnp,peername,ifd) ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("progserve/procsocket: rs1=%d\n",rs1) ;
#endif

	    rs1 = connection_finish(&conn) ;
	    if (rs >= 0) rs = rs1 ;
	    f = (peername[0] != '\0') ;
	} /* end if (connection) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) 
	    debugprintf("progserve/procsocket: rs=%d peername=%s\n",
		rs,peername) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (procsocket) */


