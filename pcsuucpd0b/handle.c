/* handle */

/* handle a connect request for a service */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable */
#define	CF_DEBUG	1		/* switchable */
#define	CF_SVCSHELLARG	0
#define	CF_PEERNAME	1		/* try to get peername */
#define	CF_WHOOPEN	0


/* revision history:

	= 1996-07-01, David A­D­ Morano

	This program was originally written.


	= 1998-07-01, David A­D­ Morano

	I added the ability to specify the "address_from"
	for the case when we add an envelope header to the message.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	This subrotuine processes a new connection that just came in.
	This connection may have been passed to us by our own daemon or
	it may have been passed to us by executing us with the connection
	on standard input.

	We:

	1) issue the pre-login text
	2) issue the login prompt
	3) read a login name and any arguments
	4) issue the password prompt
	5) read the password
	4) call the proper service handler subroutine


***************************************************************************/


#include	<envstandards.h>

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
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
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
#include	"builtin.h"
#include	"standing.h"
#include	"connection.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	PEERNAMELEN
#define	PEERNAMELEN	MAX(MAXHOSTNAMELEN,MAXPATHLEN)
#endif

#ifndef	INETXADDRLEN
#define	INETXADDRLEN	sizeof(struct in_addr)
#endif

#ifndef	SVCSPECLEN
#define	SVCSPECLEN	62
#endif

#ifndef	PASSWDLEN
#define	PASSWDLEN	32
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	O_FLAGS		(O_CREAT | O_RDWR | O_TRUNC)

#ifndef	TO_SVC
#define	TO_SVC		30
#endif


/* external subroutines */

extern int	field_svcargs(FIELD *,VECSTR *) ;
extern int	processargs(char *,VECSTR *) ;
extern int	quoteshellarg() ;
extern int	execute(struct proginfo *,int,char *,char *,
			VECSTR *,VECSTR *) ;
extern int	handle_srventry(struct proginfo *,
			struct clientinfo *,CONNECTION *,vecstr *,
			SRVTAB_ENT *) ;
extern int	inputlogin(struct proginfo *,
			struct clientinfo *,int,char *,int,char *,int) ;
extern int	passwdok(const char *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */


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

	FIELD	fsb ;

	vecstr	svcargs ;

	int	ifd = cip->fd_input, ofd = cip->fd_output ;
	int	rs, i, sl, len, si ;
	int	slen ;

	const char	*cp ;

	char	peername[MAXHOSTNAMELEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	char	svcspec[SVCSPECLEN + 1] ;
	char	netuser[LOGNAMELEN + 1] ;
	char	netpass[PASSWDLEN + 1] ;
	char	netident[MAXHOSTNAMELEN + 1] ;
	char	*service ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle: entered, ifd=%d ofd=%d\n",ifd,ofd) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    rs = srvtab_check(&pip->stab,pip->daytime,NULL) ;
	    debugprintf("handle: srvtab_check() rs=%d\n",rs) ;
	}
#endif /* CF_DEBUG */

#if	CF_DEBUG && CF_WHOOPEN
	if (pip->debuglevel > 1)
	    d_whoopen("handle open FDs") ;
#endif /* CF_DEBUG */

/* can we get the peername of the other end of this socket, if a socket? */

	connection_start(&conn,pip->domainname) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle: connection_peername()\n") ;
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

	} else
	    rs = connection_sockpeername(&conn,peername,ifd) ;

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


#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    rs = srvtab_check(&pip->stab,pip->daytime,NULL) ;
	    debugprintf("handle: srvtab_check() rs=%d\n",rs) ;
	}
#endif /* CF_DEBUG */

/* issue any pre-login text */

	cp = LOGIN_TEXT ;
	sl = strlen(cp) ;

	rs = uc_writen(ofd,cp,sl) ;

	if (rs < 0) {

	    logfile_printf(&pip->lh,"%s bad write rs=%d\n",
	        timestr_logz(pip->daytime,timebuf),rs) ;

	    goto badwrite ;
	}


	netuser[0] = '\0' ;
	netpass[0] = '\0' ;
	netident[0] = '\0' ;

/* get the login information */

	rs = inputlogin(pip,cip,TO_SVC,
	    svcspec,SVCSPECLEN,netpass,PASSWORDLEN) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle: inputlogin() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto badlogin1 ;

#if	CF_DEBUG
	if (pip->debuglevel > 1) {
	    debugprintf("handle: svcspec=%s\n",svcspec) ;
	    debugprintf("handle: passwd=%s\n",netpass) ;
	}
#endif

	fsb.lp = svcspec ;
	fsb.rlen = strlen(svcspec) ;

#if	CF_SVCSHELLARG
	slen = field_sharg(&fsb,NULL,svcargbuf,BUFLEN) ;

/* copy this service string into the 'netuser' buffer for safe keeping */

	if (slen > 0)
	    strwcpy(netuser,svcargbuf,MIN(slen,LOGNAMELEN)) ;

#else /* CF_SVCSHELLARG */
	slen = field_get(&fsb,NULL) ;

	strwcpy(netuser,fsb.fp,MIN(slen,LOGNAMELEN)) ;

#endif /* CF_SVCSHELLARG */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle: netuser=%s\n",netuser) ;
#endif

	fsb.lp[fsb.rlen] = '\0' ;

	logfile_printf(&pip->lh,"%s login=%s\n",
	    timestr_logz(pip->daytime,timebuf),
	    netuser) ;


#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    rs = srvtab_check(&pip->stab,pip->daytime,NULL) ;
	    debugprintf("handle: srvtab_check() rs=%d\n",rs) ;
	}
#endif /* CF_DEBUG */

/* OK, lookup the login and passwd and see if they are legit */

	{
	    PWFILE_CUR	cur ;

	    PWENTRY		pe ;

	    char	pwentrybuf[PWENTRY_RECLEN + 1] ;


	    pwfile_curbegin(&pip->passwd,&cur) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5)) {
	        rs = srvtab_check(&pip->stab,pip->daytime,NULL) ;
	        debugprintf("handle: before loop srvtab_check() rs=%d\n",rs) ;
	    }
#endif /* CF_DEBUG */

	    rs = SR_ACCESS ;
	    while (rs < 0) {

	        rs = pwfile_fetchuser(&pip->passwd,netuser,&cur,
	            &pe,pwentrybuf,PWENTRY_RECLEN) ;

	        if (rs < 0)
	            break ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1) {
	            debugprintf("handle: pwentry->name=%s\n",pe.username) ;
	            debugprintf("handle: pwentry->password=%s\n",pe.password) ;
	            debugprintf("handle: supplied password=%s\n",netpass) ;
	        }
#endif

#if	CF_DEBUG
	        if (DEBUGLEVEL(5)) {
	            rs = srvtab_check(&pip->stab,pip->daytime,NULL) ;
	            debugprintf("handle: before passwdok() rs=%d\n",rs) ;
	        }
#endif /* CF_DEBUG */

	        rs = SR_ACCESS ;
	        if (passwdok(netpass,pe.password))
	            rs = SR_OK ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("handle: password lookup rs=%d\n",rs) ;
#endif

	    } /* end while (looping through matching usernames) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(5)) {
	        rs = srvtab_check(&pip->stab,pip->daytime,NULL) ;
	        debugprintf("handle: after loop srvtab_check() rs=%d\n",rs) ;
	    }
#endif /* CF_DEBUG */

	    pwfile_curend(&pip->passwd,&cur) ;
	} /* end block (checking password) */

	if (rs < 0) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("handle: lookup login=%s rs=%d\n",
	            netuser,rs) ;
#endif

	    goto badlogin2 ;
	}


#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    rs = srvtab_check(&pip->stab,pip->daytime,NULL) ;
	    debugprintf("handle: srvtab_check() rs=%d\n",rs) ;
	}
#endif /* CF_DEBUG */

/* store some client information */

	cip->peername = peername ;
	cip->netuser = netuser ;
	cip->netpass = netpass ;
	cip->netident = NULL ;


/* get a service associated with this network user */

	cip->service = netuser ;
	service = netuser ;

/* put the rest of the arguments into a string list */

	vecstr_start(&svcargs,6,0) ;

	field_svcargs(&fsb,&svcargs) ;

/* do we have this service in our server table? */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle: searching for service=%s\n",service) ;
#endif

	if (srvtab_match(&pip->stab,service,&sep) >= 0) {

	    if (pip->f.log)
	        logfile_flush(&pip->lh) ;

	    rs = handle_srventry(pip,cip,&conn,
	        &svcargs,sep) ;

	} else if ((si = builtin_match(bip,service)) >= 0) {

	    char	srvname[MAXPATHLEN + 1] ;


	    strwcpylc(srvname,service,MAXPATHLEN) ;

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
	    logfile_printf(&pip->lh,"service not found\n") ;

	}


	if (rs < 0) {

	    int	cl ;

	    char	*cp2 = NULL ;


	    switch (rs) {

	    case SR_NOTFOUND:
	        cp = "login failed\n" ;
	        cp2 = "service not found" ;
	        break ;

	    case SR_BADSLT:
	        cp = "service not configured\n" ;
	        break ;

	    case SR_UNATCH:
	        cp = "server not attached to login\n" ;
	        break ;

	    case SR_NOEXEC:
	        cp = "could not execute server\n" ;
	        break ;

	    case SR_ACCESS:
	        cp = "access denied to service\n" ;
	        break ;

	    } /* end switch */

	    cl = strlen(cp) ;

	    uc_writen(ofd,cp,cl) ;

	    if (cp2 == NULL) {

	        cp2 = cp ;
	        if (cp2[cl - 1] == '\n')
	            cl -= 1 ;

	    }

	    logfile_printf(&pip->lh,"%t",cp2,cl) ;

	} /* end if (couldn't execute the server) */

	vecstr_finish(&svcargs) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle: arrived at done rs=%d\n",rs) ;
#endif


/* we are out of here */
done:
	u_close(ifd) ;

	u_close(ofd) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle: in done rs=%d\n",rs) ;
#endif

retnotfound1:
	connection_finish(&conn) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle: ret rs=%d\n",rs) ;
#endif

	if (pip->f.log)
	    logfile_flush(&pip->lh) ;

	return rs ;

/* bad things come down here */
badwrite:
	goto done ;

badlogin1:
	cp = "incomplete login\n" ;
	uc_writen(ofd,cp,strlen(cp)) ;

	pip->daytime = time(NULL) ;

	logfile_printf(&pip->lh,"%s incomplete login >%c%c< (%d)\n",
	    timestr_logz(pip->daytime,timebuf),
	    ((svcspec[0]) ? 'l' : ' '),
	    ((netpass[0]) ? 'p' : ' '),
	    rs) ;

	if (svcspec[0] != '\0') {

	    sl = MIN(SVCSPECLEN,48) ;
	    for (i = 0 ; (svcspec[i] != '\0') && (i < sl) ; i += 1) {

	        if (isprint(svcspec[i]))
	            peername[i] = svcspec[i] ;

	    }

	    peername[i] = '\0' ;
	    logfile_printf(&pip->lh,"login information=>%s<\n",peername) ;

	} /* end if */

	goto done ;

badlogin2:
	cp = "login failed\n" ;
	uc_writen(ofd,cp,strlen(cp)) ;

	pip->daytime = time(NULL) ;

	cp = (rs == SR_ACCESS) ? "incorrect password" : "no login" ;
	logfile_printf("%s login=%s -> %s\n",
	    timestr_logz(pip->daytime,timebuf),
	    service,cp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("handle: bad login\n") ;
#endif

	goto done ;
}
/* end subroutine (handle) */



