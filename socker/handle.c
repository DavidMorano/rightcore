/* handle */

/* handle a connect request for a service */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0
#define	CF_DEBUG	1
#define	CF_SVCSHELLARG	0		/* parse service as SHELL arg ? */
#define	CF_PEERNAME	1


/* revision history:

	= 96/07/01, David A­D­ Morano

	This program was originally written.


	= 98/07/01, David A­D­ Morano

	I added the ability to specify the "address_from"
	for the case when we add an envelope header to the message.


*/


/**************************************************************************

	This subrotuine processes a new connection that just came in.
	This connection may have been passed to us by our own daemon or
	it may have been passed to us by executing us with the
	connection on standard input.

	We :

	1) get the service name, which is terminated by a line feed
	2) search for the service if we have it
	3) acknowledge the client, positively or negatively
	4) call the proper service handler subroutine



***************************************************************************/


#include	<sys/types.h>
#include	<sys/stat.h>
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
#include	<bfile.h>
#include	<field.h>
#include	<vecstr.h>
#include	<hostent.h>
#include	<sockaddress.h>
#include	<inetaddr.h>

#include	"srvtab.h"

#include	"localmisc.h"
#include	"srventry.h"
#include	"builtin.h"
#include	"standing.h"
#include	"connection.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#define	O_FLAGS		(O_CREAT | O_RDWR | O_TRUNC)
#ifndef	TO_SVC
#define	TO_SVC		30
#endif
#define	INETXADDRLEN	sizeof(struct in_addr)

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	field_svcargs(FIELD *,VECSTR *) ;
extern int	nlspeername(const char *,const char *,char *) ;
extern int	quoteshellarg() ;
extern int	processargs(char *,VECSTR *) ;
extern int	execute(struct proginfo *,int,char *,char *,
			VECSTR *,VECSTR *) ;
extern int	badback(int,int,char *) ;

extern int	handle_srventry(struct proginfo *,
			struct clientinfo *,CONNECTION *,vecstr *,
			SRVTAB_ENT *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */


/* forward references */


/* local variables */






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

	int	ifd = cip->fd_input, ofd = cip->fd_output ;
	int	rs, len, si ;
	int	slen ;

	char	svcspec[BUFLEN + 1] ;
	char	timebuf[TIMEBUFLEN] ;
	char	peername[MAXHOSTNAMELEN + 1] ;
	char	*service ;
	char	*cp ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle: entered, ifd=%d ofd=%d\n",ifd,ofd) ;
#endif

/* can we get the peername of the other end of this socket, if a socket ? */

	connection_start(&conn,pip->domainname) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
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
	            rs = strwcpy(peername,"levo.levosim.org",-1) - peername ;

	        } else
	            rs = -1 ;

	    } /* end if (address family INET4) */

	    if (rs < 0)
	        rs = connection_peername(&conn,&cip->sa,cip->salen,peername) ;

	} else
	    rs = connection_sockpeername(&conn,peername,ifd) ;

	if ((rs < 0) && ((cp = getenv("NLSADDR")) != NULL)) {

	    rs = nlspeername(cp,pip->domainname,peername) ;

	} /* end if */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle: connection_peername rs=%d\n",rs) ;
#endif

	if (rs > 0)
	    logfile_printf(&pip->lh,"from host=%s\n",peername) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle: connection rs=%d peername=%s\n",
	        rs,conn.peername) ;
#endif


/* pop off the service name */

	len = uc_readlinetimed(ifd,svcspec,BUFLEN,TO_SVC) ;

	if (len <= 1) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("handle: no serivce (%d)\n",len) ;
#endif

	    logfile_printf(&pip->lh,"no service\n") ;

	    rs = SR_NOTFOUND ;
	    goto badnotfound1 ;
	}

	if (svcspec[len - 1] == '\n')
	    len -= 1 ;

	svcspec[len] = '\0' ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle: svcspec=%s\n",svcspec) ;
#endif

	fsb.lp = svcspec ;
	fsb.rlen = len ;

#if	CF_SVCSHELLARG
	slen = field_sharg(&fsb,NULL,srvargbuf,BUFLEN) ;

/* copy this service string into the 'svcspec' buffer for safe keeping */

	if (slen > 0)
	    strwcpy(svcspec,srvargbuf,slen) ;

	service = svcspec ;
	service[slen] = '\0' ;
#else /* CF_SVCSHELLARG */
	slen = field_get(&fsb,NULL) ;

	service = fsb.fp ;
	service[slen] = '\0' ;
#endif /* CF_SVCSHELLARG */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle: service=%s\n",service) ;
#endif

	fsb.lp[fsb.rlen] = '\0' ;

	logfile_printf(&pip->lh,"%s service=%s\n",
	    timestr_log(pip->daytime,timebuf),
	    service) ;

/* put the rest of the arguments into a string list */

	vecstr_start(&svcargs,6,0) ;

	field_svcargs(&fsb,&svcargs) ;

/* do we have this service in our server table ? */

	if (srvtab_match(&pip->stab,service,&sep) >= 0) {

	    rs = handle_srventry(pip,cip,&conn,
	        &svcargs,sep) ;

	} else if ((si = builtin_match(bip,service)) >= 0) {

	    char	srvname[MAXPATHLEN + 1] ;


		u_write(ofd,"+\r\n",3) ;

	    strwcpylc(srvname,service,MAXPATHLEN) ;

	    logfile_printf(&pip->lh,"server=%s\n",
	        srvname) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("handle: builtin_exec si=%d\n",si) ;
#endif

	    rs = builtin_execute(bip,ourp,cip,si,&conn,&svcargs) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("handle: builtin_exec rs=%d\n",rs) ;
#endif

	} else {

	    rs = SR_NOTFOUND ;
	    badback(ofd,TCPMUXD_CFNOSVC,NULL) ;

	    logfile_printf(&pip->lh,"service not found\n") ;

	}

	    vecstr_finish(&svcargs) ;

/* we are out of here */
done:
	u_close(ifd) ;

	u_close(ofd) ;

badnotfound1:
	connection_finish(&conn) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle: exiting rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (handle) */



