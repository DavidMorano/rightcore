/* proghandle */

/* handle a connect request for a service */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time print-outs */
#define	CF_DEBUG	0		/* run-time print-outs */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subrotuine processes a new connection that just came in. This
        connection may have been passed to us by our own daemon or it may have
        been passed to us by executing us with the connection on standard input.

	We:

	1) read the finer query
	2) check if it is in our service table
	3) check if it is a local username
	4) check if there is a "default" service table entry

	Note: Note (carefully) that zero-length service strings
	*are* acceptable.  They cause a default service to be
	invoked.


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
#include	<netdb.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<vecstr.h>
#include	<hostent.h>
#include	<sockaddress.h>
#include	<inetaddr.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"clientinfo.h"
#include	"builtin.h"
#include	"standing.h"


/* local defines */

#ifndef	PEERNAMELEN
#define	PEERNAMELEN	MAX(MAXHOSTNAMELEN,MAXPATHLEN)
#endif

#ifndef	INETXADDRLEN
#define	INETXADDRLEN	sizeof(struct in_addr)
#endif

#ifndef	PASSWDLEN
#define	PASSWDLEN	32
#endif

#ifndef	SVCBUFLEN
#define	SVCBUFLEN	(2*LINEBUFLEN)
#endif

#ifndef	SVCSPECLEN
#define	SVCSPECLEN	MAXNAMELEN
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	O_FLAGS		(O_CREAT | O_RDWR | O_TRUNC)

#ifndef	TO_SVC
#define	TO_SVC		30
#endif

#define	ISEOL(c)	(((c) == '\n') || ((c) == '\r'))


/* external subroutines */

extern int	sisub(const char *,int,const char *) ;
extern int	field_svcargs(FIELD *,VECSTR *) ;
extern int	isasocket(int) ;

extern int	progserve(PROGINFO *,STANDING *,BUILTIN *,
			CLIENTINFO *,vecstr *,cchar *,cchar **) ;

extern int	proglog_printf(PROGINFO *,cchar *,...) ;
extern int	proglog_flush(PROGINFO *) ;

#if	CF_DEBUG || CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* forward references */

static int	procsvcspec(PROGINFO *,CLIENTINFO *,
			char *,int,vecstr *,cchar *,int) ;


/* local variables */

/* service argument terminators */
static const uchar	sterms[] = {
	0x00, 0x1A, 0x00, 0x00,
	0x01, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int proghandle(pip,sop,bop,cip)
PROGINFO	*pip ;
STANDING	*sop ;
BUILTIN		*bop ;
CLIENTINFO	*cip ;
{
	vecstr		sargs ;
	const int	svclen = SVCBUFLEN ;
	const int	to = TO_READSVC ;
	int		rs ;
	int		ifd = cip->fd_input ;
	int		ofd = cip->fd_output ;
	int		len ;
	int		opts ;
	const int	f_socket = isasocket(ifd) ;
	char		svcspec[SVCSPECLEN + 1] ;
	char		svcbuf[SVCBUFLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("proghandle: ent f_sock=%u\n",f_socket) ;
#endif

	if (ofd < 0) return SR_INVALID ;

	if ((rs = uc_readlinetimed(ifd,svcbuf,svclen,to)) >= 0) {
	    len = rs ;

	    if ((len > 0) && ISEOL(svcbuf[len - 1])) len -= 1 ;
	    svcbuf[len] = '\0' ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("proghandle: svcbuf=>%t<\n",svcbuf,len) ;
#endif

/* we have our input data => make sure we don't get any more! :-) */

#if	defined(P_FINGERS)
	    if (f_socket)
	        u_shutdown(ifd,SHUT_RD) ;
#endif

/* OK, parse the stuff to get 1) service 2) the '/W' thing */

	    cip->f_long = FALSE ;
	    opts = (VECSTR_OCOMPACT) ;
	    if ((rs = vecstr_start(&sargs,6,opts)) >= 0) {

	        rs = procsvcspec(pip,cip,svcspec,SVCSPECLEN,&sargs,
	            svcbuf,len) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            int 	i ;
	            cchar	*cp ;
	            debugprintf("proghandle: svcspec=>%s<\n",svcspec) ;
	            for (i = 0 ; vecstr_get(&sargs,i,&cp) >= 0 ; i += 1) {
	                if (cp == NULL) continue ;
	                debugprintf("proghandle: svcarg%u=>%s<\n",i,cp) ;
	            }
	        }
#endif /* CF_DEBUG */

	        if (rs >= 0) {
	            cchar	**sav ;
	            if ((rs = vecstr_getvec(&sargs,&sav)) >= 0) {
	                rs = progserve(pip,sop,bop,cip,NULL,svcspec,sav) ;
	            }
	        } /* end if (ok) */

	        vecstr_finish(&sargs) ;
	    } /* end if (sargs) */

	    if (pip->open.logprog)
	        proglog_flush(pip) ;

	} /* end if (uc_readlinetimed) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("proghandle: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (proghandle) */


/* local subroutines */


static int procsvcspec(pip,cip,snbuf,snlen,sap,svcbuf,svclen)
PROGINFO	*pip ;
CLIENTINFO	*cip ;
char		snbuf[] ;
int		snlen ;
vecstr		*sap ;
const char	svcbuf[] ;
int		svclen ;
{
	FIELD		fsb ;
	int		rs ;
	int		len = 0 ;

	if (pip == NULL) return SR_FAULT ;

	snbuf[0] = '\0' ;
	if ((rs = field_start(&fsb,svcbuf,svclen)) >= 0) {
	    int		fl ;
	    cchar	*fp ;
	    char	fbuf[SVCSPECLEN + 1] ;
	    if ((fl = field_get(&fsb,sterms,&fp)) > 0) {
	        int	si ;
	        cchar	*ols = "/w" ;

	        if ((fl == 2) && (strncasecmp(fp,ols,2) == 0)) {

	            cip->f_long = TRUE ;
	            if ((fl = field_get(&fsb,sterms,&fp)) > 0) {
	                len = strdcpy1w(snbuf,snlen,fp,fl) - snbuf ;
	            }

	        } else {

	            len = strdcpy1w(snbuf,snlen,fp,fl) - snbuf ;

	            if ((si = sisub(snbuf,len,ols)) > 0) {
	                cip->f_long = TRUE ;
	                len = si ;
	                snbuf[len] = '\0' ;
	            }

	            fp = fbuf ;
	            fl = field_sharg(&fsb,sterms,fbuf,SVCSPECLEN) ;

	            if ((fl == 2) && (strncasecmp(fp,ols,2) == 0)) {
	                cip->f_long = TRUE ;
	            } else if (fl > 0) {
	                rs = vecstr_add(sap,fp,fl) ;
	            }

	        } /* end if */

/* put the rest of the arguments into a string list */

	        if ((rs >= 0) && (len > 0)) {
	            rs = field_svcargs(&fsb,sap) ;
	        }

	    } /* end if (field_get) */
	    field_finish(&fsb) ;
	} /* end if (field) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (procsvcspec) */


