/* handle */

/* handle a connect request for a service */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time print-outs */
#define	CF_DEBUG	0		/* run-time print-outs */


/* revision history:

	= 1996-07-01, David A­D­ Morano

	This program was originally written.


	= 1998-07-01, David A­D­ Morano

	I added the ability to specify the "address_from"
	for the case when we add an envelope header to the message.


	= 1998-02-23, David A­D­ Morano

	This was grabbed from something similar previous and
	modified for use by the Finger Server.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	This subrotuine processes a new connection that just came in.
	This connection may have been passed to us by our own daemon or
	it may have been passed to us by executing us with the
	connection on standard input.

	We:

	1) read the finer query
	2) check if it is in our service table
	3) check if it is a local username
	4) check if there is a "default" service table entry


***************************************************************************/


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
#include	<vecstr.h>
#include	<hostent.h>
#include	<sockaddress.h>
#include	<inetaddr.h>
#include	<pwfile.h>
#include	<srvtab.h>
#include	<localmisc.h>

#include	"srventry.h"
#include	"connection.h"
#include	"config.h"
#include	"defs.h"
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
#define	SVCBUFLEN	MAXNAMELEN
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	O_FLAGS		(O_CREAT | O_RDWR | O_TRUNC)

#ifndef	TO_SVC
#define	TO_SVC		30
#endif


/* external subroutines */

extern int	sisub(const char *,int,const char *) ;
extern int	field_svcargs(FIELD *,VECSTR *) ;
extern int	nlspeername(const char *,const char *,char *) ;
extern int	isNotPresent(int) ;

extern int	processargs(char *,VECSTR *) ;
extern int	login_match(struct proginfo *,struct passwd *,
			char *,int,char *) ;
extern int	handle_srventry(struct proginfo *,struct clientinfo *,
			CONNECTION *,vecstr *, SRVTAB_ENT *,int,int) ;
extern int	handle_login(struct proginfo *,struct clientinfo *,
			CONNECTION *,vecstr *, int, struct passwd *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int handle(pip,bip,ourp,cip)
struct proginfo		*pip ;
BUILTIN			*bip ;
STANDING		*ourp ;
struct clientinfo	*cip ;
{
	CONNECTION	conn ;
	SRVTAB_ENT	*sep ;
	FIELD		fsb ;
	vecstr		svcargs ;
	int	rs = SR_OK ;
	int	ifd = cip->fd_input, ofd = cip->fd_output ;
	int	len, si ;
	int	svclen ;
	int	f_socket = FALSE ;

	cchar		*tp, *cp ;
	char	peername[PEERNAMELEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	char	svcspec[BUFLEN + 1] ;
	char	svcbuf[SVCBUFLEN + 1] ;
	char	buf[SVCBUFLEN + 1] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("handle: entered, ifd=%d ofd=%d\n",ifd,ofd) ;
#endif

/* can we get the peername of the other end of this socket, if a socket? */

	connection_start(&conn,pip->domainname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("handle: connection salen=%d\n",cip->salen) ;
#endif

	peername[0] = '\0' ;
	if (cip->salen > 0) {
	    int		af ;
	    in_addr_t	addr1, addr2 ;

	    rs = sockaddress_getaf(&cip->sa) ;
	    af = rs ;

	    if ((rs >= 0) && (af == AF_INET)) {

	        rs = sockaddress_getaddr(&cip->sa,
	            (char *) &addr1,sizeof(in_addr_t)) ;

	        addr2 = htonl(SPECIALHOSTADDR) ;

	        if ((rs >= 0) && 
	            (memcmp(&addr1,&addr2,sizeof(in_addr_t)) == 0)) {
	            rs = strwcpy(peername,SPECIALHOSTNAME,-1) - peername ;

	        } else
	            rs = -1 ;

	    } /* end if (address family INET4) */

	    if (rs < 0)
	        rs = connection_peername(&conn,&cip->sa,cip->salen,peername) ;

	} else {
	    rs = connection_sockpeername(&conn,peername,ifd) ;
	}

	if ((rs < 0) && ((cp = getenv("NLSADDR")) != NULL)) {

	    rs = nlspeername(cp,pip->domainname,peername) ;

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("handle: connection rs=%d peername=%s\n",
	        rs,conn.peername) ;
#endif

	if (rs > 0)
	    logfile_printf(&pip->lh,"from host=%s\n",peername) ;

	f_socket = isasocket(ifd) ;

/* pop off the service name */

	rs = uc_readlinetimed(ifd,svcspec,BUFLEN,TO_SVC) ;
	len = rs ;
	if (rs <= 1) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	        debugprintf("handle: no serivce (%d)\n",rs) ;
#endif

	    rs = SR_NOTFOUND ;
	    if (pip->f.log)
	        logfile_printf(&pip->lh,"no service\n") ;

	    goto ret3 ;
	}

	if (svcspec[len - 1] == '\n')
	    len -= 1 ;

	svcspec[len] = '\0' ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("handle: svcspec=%s\n",svcspec) ;
#endif

/* we have our input data => make sure we don't get any more! :-) */

	if (f_socket)
	    u_shutdown(ifd,SHUT_RD) ;

/* OK, parse the stuff to get 1) service 2) the '/W' thing */

	cip->f_long = FALSE ;
	rs = vecstr_start(&svcargs,6,0) ;
	if (rs < 0)
		goto ret4 ;

	if ((rs = field_start(&fsb,svcspec,len)) >= 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("handle: extracting\n") ;
#endif

	    field_get(&fsb,NULL) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("handle: first field=%t\n",fsb.fp,fsb.flen) ;
#endif

	    if ((fsb.flen == 2) && 
		(strncasecmp(fsb.fp,"/w",2) == 0)) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("handle: leading hack\n") ;
#endif

	        cip->f_long = TRUE ;
		field_get(&fsb,NULL) ;

		svclen = MIN(SVCBUFLEN,fsb.flen) ;
		strwcpy(svcbuf,fsb.fp,svclen) ;

	    } else {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("handle: regular \n") ;
#endif

		svclen = MIN(SVCBUFLEN,fsb.flen) ;
		strwcpy(svcbuf,fsb.fp,svclen) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("handle: stored=%t\n",svcbuf,svclen) ;
#endif

		if ((si = sisub(svcbuf,svclen,"/w")) > 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("handle: extra switch si=%u\n",si) ;
#endif

	            cip->f_long = TRUE ;
		    svclen = si ;
		    svcbuf[si] = '\0' ;

		}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("handle: regular svc=%t\n",svcbuf,svclen) ;
#endif

		field_sharg(&fsb,NULL,buf,SVCBUFLEN) ;

	        if ((fsb.flen == 2) && 
			(strncasecmp(fsb.fp,"/w",2) == 0)) {

	            cip->f_long = TRUE ;

	        } else
			vecstr_add(&svcargs,fsb.fp,fsb.flen) ;

	    }

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("handle: how we doing rs=%d\n",rs) ;
#endif

/* put the rest of the arguments into a string list */

	    if (rs >= 0)
	        rs = field_svcargs(&fsb,&svcargs) ;

	    field_finish(&fsb) ;
	} /* end if (field extraction) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("handle: svc field extraction rs=%d\n",rs) ;
#endif

	if (rs < 0)
		goto ret4 ;

/* do we have a subservice? */

	cip->service = svcbuf ;
	cip->subservice = svcbuf + svclen ;
	if ((tp = strchr(svcbuf,'+')) != NULL) {

	    svclen = (tp - svcbuf) ;
	    svcbuf[svclen] = '\0' ;
	    cip->subservice = (tp + 1) ;

	} /* end if (subservice) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("handle: service=%s\n",cip->service) ;
	    debugprintf("handle: subservice=%s\n",cip->subservice) ;
	}
#endif

	logfile_printf(&pip->lh,"%s service=%s\n",
	    timestr_logz(pip->daytime,timebuf),
	    cip->service) ;

/* do we have this service in our server table? */

	{
	    PASSWDENT	pw ;
	    const int	pwlen = getbufsize(getbufsize_pw) ;
	    char	*pwbuf ;
	    if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
		cchar	*svc = cip->serivice ;
	        if (srvtab_match(&pip->stab,svc,&sep) >= 0) {

	    if (pip->f.log)
	        logfile_flush(&pip->lh) ;

	    rs = handle_srventry(pip,cip,&conn,
	        &svcargs,sep,ifd,ofd) ;

		} else if (login_match(pip,&pw,pwbuf,pwlen,svc) >= 0) {

	    if (pip->f.log)
	        logfile_flush(&pip->lh) ;

	    rs = handle_login(pip,cip,&conn,
	        &svcargs,ofd,&pw) ;

		} else if ((si = builtin_match(bip,svc)) >= 0) {
	    	    char	srvname[MAXPATHLEN + 1] ;

	    	strwcpylc(srvname,svc,MAXPATHLEN) ;

	    logfile_printf(&pip->lh,"server=%s\n",
	        srvname) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("handle: builtin_exec si=%d\n",si) ;
#endif

	    if (pip->f.log)
	        logfile_flush(&pip->lh) ;

	    rs = builtin_execute(bip,ourp,cip,si,&conn,&svcargs) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("handle: builtin_exec rs=%d\n",rs) ;
#endif

	} else {
	    rs = SR_NOTFOUND ;
	    if (pip->f.log)
	    logfile_printf(&pip->lh,"service not found\n") ;
	}
		uc_free(pwbuf) ;
	    } /* end if (memory-allocation) */
	} /* end block */

	vecstr_finish(&svcargs) ;

done:
ret4:
	if (rs < 0) {
	    if (pip->f.log) {
	    switch (rs) {
	    case SR_NOTFOUND:
	        cp = "service not found" ;
	        break ;
	    case SR_BADSLT:
	        cp = "service not configured" ;
	        break ;
	    case SR_UNATCH:
	        cp = "server not attached to login" ;
	        break ;
	    case SR_NOEXEC:
	        cp = "could not execute server" ;
	        break ;
	    case SR_ACCESS:
	        cp = "access denied to service" ;
	        break ;
	    default:
		cp = "other" ;
		break ;
	    } /* end switch */
		logfile_printf(&pip->lh,"%s (%d)",cp,rs) ;
	    } /* end if (log error) */

/* send back to client */

	    cp = "no user\n" ;
	    rs = uc_writen(ofd,cp,strlen(cp)) ;

	    if ((rs < 0) && pip->f.log)
		logfile_printf(&pip->lh,"final write (%d)",rs) ;

	} else if (pip->f.log)
		logfile_printf(&pip->lh,"sentback=%u",rs) ;

/* we are out of here */
ret3:
	u_close(ifd) ;

	u_close(ofd) ;

ret2:
	connection_finish(&conn) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("handle: ret rs=%d\n",rs) ;
#endif

ret1:
	if (pip->f.log)
	    logfile_flush(&pip->lh) ;

ret0:
	return rs ;
}
/* end subroutine (handle) */


