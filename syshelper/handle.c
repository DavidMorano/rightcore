/* handle */

/* handle a connect request for a service */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0
#define	CF_DEBUG	1
#define	CF_SVCSHELLARG	0
#define	CF_PEERNAME	1


/* revision history:

	= 1996-07-01, David A­D­ Morano
	This program was originally written.

	= 1998-07-01, David A­D­ Morano
	I added the ability to specify the "address_from"
	for the case when we add an envelope header to the message.


*/

/* Copyright © 1996,1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subrotuine processes a new connection that just came in. This
        connection may have been passwed to us by our own daemon or it may have
        been passed to us by executing us with the connection on standard input.

	We:

	1. get the service name, which is terminated by a line feed
	2. search for the service if we have it
	3. acknowledge the client, positively or negatively
	4. call 'process' to finish the processing of this connection


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<time.h>
#include	<string.h>
#include	<pwd.h>
#include	<grp.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<vecstr.h>
#include	<sockaddress.h>
#include	<inetaddr.h>
#include	<srvtab.h>
#include	<localmisc.h>

#include	"srventry.h"
#include	"builtin.h"
#include	"standing.h"
#include	"connection.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	O_FLAGS		(O_CREAT | O_RDWR | O_TRUNC)
#ifndef	TO_SVC
#define	TO_SVC		30
#endif
#define	INETXADDRLEN	sizeof(struct in_addr)


/* external subroutines */

extern int	quoteshellarg() ;
extern int	field_svcargs(FIELD *,VECSTR *) ;
extern int	processargs(char *,VECSTR *) ;
extern int	execute(struct proginfo *,int,char *,char *,VECSTR *,VECSTR *) ;
extern int	badback(int,int,char *) ;

extern int	handle_srventry(struct proginfo *,struct serverinfo *,
			struct clientinfo *,CONNECTION *,ACCTAB *,
			FIELD *,SRVTAB_ENT *,int,int,varsub *,
			vecstr *) ;

extern char	*strwcpy(char *,char *,int) ;
extern char	*strwcpylc(char *,char *,int) ;
extern char	*timestr_log(), *timestr_edate(), *timestr_elapsed() ;


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int handle(gp,sip,bip,ourp,cip)
struct proginfo		*gp ;
struct serverinfo	*sip ;
BUILTIN			*bip ;
STANDING		*ourp ;
struct clientinfo	*cip ;
{
	time_t		daytime = time(NULL) ;

	varsub		*ssp = sip->ssp ;
	vecstr		*elp = sip->elp ;
	SRVTAB		*sfp = sip->sfp ;
	ACCTAB		*atp = sip->atp ;

	CONNECTION	conn ;

	SRVTAB_ENT	*sep ;

	FIELD	fsb ;

	int	ifd = FD_STDIN, ofd = FD_STDOUT ;
	int	i, len, rs, si ;
	int	slen ;

	char	srvspec[BUFLEN + 1] ;
	char	timebuf[TIMEBUFLEN] ;
	char	peername[MAXHOSTNAMELEN + 1] ;
	char	*service ;
	char	*cp ;


#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("handle: entered, ifd=%d ofd=%d\n",ifd,ofd) ;
#endif

#if	CF_DEBUG
	if (gp->debuglevel > 1)
		d_whoopen("handle open FDs") ;
#endif /* CF_DEBUG */

	connection_start(&conn,gp->domainname) ;

	logfile_printf(&gp->lh,"%s request pid=%d\n",
	    timestr_log(daytime,timebuf),gp->pid) ;


/* can we get the peername of the other end of this socket, if a socket ? */

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("handle: connection_peername()\n") ;
#endif

	if (cip->salen > 0) {
		int		af ;
		in_addr_t	addr1, addr2 ;

		rs = sockaddress_getaf(&cip->sa) ;
	        af = rs ;

		if ((rs >= 0) && (af == AF_INET)) {

			rs = sockaddress_getaddr(&cip->sa,
				(char *) &addr1,sizeof(in_addr_t)) ;

			addr2 = htonl(0x40c05865) ;

		if ((rs >= 0) && 
			(memcmp(&addr1,&addr2,sizeof(in_addr_t)) == 0)) {
				rs = strwcpy(peername,"levo.levosim.org",-1) -
					peername ;
			} else
				rs = -1 ;

		} /* end if (address family INET4) */

		if (rs < 0)
		rs = connection_peername(&conn,&cip->sa,cip->salen,peername) ;

	} else
		rs = connection_sockpeername(&conn,peername,ifd) ;

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("handle: connection_peername rs=%d\n",rs) ;
#endif

	if (rs > 0)
		logfile_printf(&gp->lh,"from host=%s\n",peername) ;

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("handle: connection rs=%d peername=%s\n",
		rs,conn.peername) ;
#endif


/* pop off the service name */

	if ((len = uc_readlinetimed(ifd,srvspec,BUFLEN,TO_SVC)) <= 1) {

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("handle: no serivce (%d)\n",len) ;
#endif

	    logfile_printf(&gp->lh,"no service\n") ;

	    rs = SR_NOTFOUND ;
	    goto badnotfound1 ;
	}

	if (srvspec[len - 1] == '\n')
	    len -= 1 ;

	srvspec[len] = '\0' ;

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("handle: srvspec=%s\n",srvspec) ;
#endif

	fsb.lp = srvspec ;
	fsb.rlen = len ;

#if	CF_SVCSHELLARG
	slen = field_sharg(&fsb,NULL,srvargbuf,BUFLEN) ;

/* copy this service string into the 'srvspec' buffer for safe keeping */

	if (slen > 0)
	    strwcpy(srvspec,srvargbuf,slen) ;

	service = srvspec ;
	service[slen] = '\0' ;
#else /* CF_SVCSHELLARG */
	slen = field_get(&fsb,NULL) ;

	service = fsb.fp ;
	service[slen] = '\0' ;
#endif /* CF_SVCSHELLARG */

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("handle: service=%s\n",service) ;
#endif

	fsb.lp[fsb.rlen] = '\0' ;

	logfile_printf(&gp->lh,"%s service=%s\n",
	    timestr_log(daytime,timebuf),
	    service) ;


/* do we have this service in our server table ? */

	if (srvtab_match(sfp,service,&sep) >= 0) {

	    rs = handle_srventry(gp,sip,cip,&conn,
			atp,&fsb,sep,ifd,ofd,ssp,elp) ;

	} else if ((si = builtin_match(bip,service)) >= 0) {

	    vecstr	svcargs ;

	    char	srvname[MAXPATHLEN + 1] ;


	    strwcpylc(srvname,service,MAXPATHLEN) ;

	    logfile_printf(&gp->lh,"server=%s\n",
	        srvname) ;

	    vecstr_start(&svcargs,6,0) ;

	    field_svcargs(&fsb,&svcargs) ;

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("handle: builtin_exec si=%d\n",si) ;
#endif

	    rs = builtin_execute(bip,ourp,cip,&conn,si,service,&svcargs) ;

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("handle: builtin_exec rs=%d\n",rs) ;
#endif

	    vecstr_finish(&svcargs) ;

	} else {

	    rs = SR_NOTFOUND ;
		badback(ofd,TCPMUXD_CFNOSVC,NULL) ;

	    logfile_printf(&gp->lh,"service not found\n") ;

	}


/* we are out of here */
done:
	u_close(ifd) ;

	u_close(ofd) ;

badnotfound1:
	connection_finish(&conn) ;

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("handle: exiting rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (handle) */


