/* handle */

/* handle a connect request for a service */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0
#define	CF_DEBUG	1
#define	CF_SRVSHELLARG	0


/* revision history:

	= 1998, David A.D. Morano
	I added the ability to specify the "address_from"
	for the case when we add an envelope header to the message.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subrotuine processes a new connection that just came in.
	This connection may have been passwed to us by our own daemon
	or it may have been passed to us by executing us with the connection
	on standard input.

	We:

	1. get the service name, which is terminated by a line feed
	2. search for the service if we have it
	3. acknowledge the client, positively or negatively
	4. call 'process' to finish the processing of this connection


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/utsname.h>
#include	<sys/param.h>
#include	<fcntl.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<time.h>
#include	<string.h>
#include	<pwd.h>
#include	<grp.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<vecstr.h>
#include	<srvtab.h>
#include	<localmisc.h>

#include	"srvpe.h"
#include	"srventry.h"
#include	"builtin.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	O_FLAGS		(O_CREAT | O_RDWR | O_TRUNC)
#ifndef	SRVTO
#define	SRVTO		30
#endif


/* external subroutines */

extern int	quoteshellarg() ;
extern int	field_srvargs(FIELD **,VECSTR *) ;
extern int	processargs(char *,VECSTR *) ;
extern int	execute(struct global *,int,char *,char *,VECSTR *,VECSTR *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*timestr_log(), *timestr_edate(), *timestr_elapsed() ;


/* external variables */


/* forward references */


/* local data */


/* exported subroutines */


int handle(gp,pbp,ifd,ofd,elp,sfp,bip)
struct global	*gp ;
SRVPE		*pbp ;
int		ifd, ofd ;
VECSTR		*elp ;
SRVTAB		*sfp ;
BUILTIN		*bip ;
{
	SRVTAB_ENT	*sep ;

	VECSTR		srvargs ;

	FIELD		fsb ;

	time_t		daytime ;

	int	rs = SR_OK ;
	int	len, si ;
	int	slen ;

	char	srvspec[BUFLEN + 1] ;
	char	timebuf[TIMEBUFLEN] ;
	char	peername[MAXHOSTNAMELEN + 1] ;
	char	*service ;
	char	*cp ;


#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("handle: entered, s=%d\n",ifd) ;
#endif

	vecstr_start(&srvargs,6,0) ;

	daytime = time(NULL) ;

	logfile_printf(&gp->lh,"%s request pid=%d\n",
	    timestr_log(daytime,timebuf),gp->pid) ;

/* can we get the peername of the other end of this socket, if a socket ? */

#ifdef	COMMENT
	if (isasocket(s)) {

	    char	inethost[MAXHOSTNAMELEN + 1] ;


	    rs = u_getpeername(s,....) ;


	    logfile_printf(&gp->lh,"connection=%s\n", inethost) ;

	} /* end if (internet connection) */
#endif /* COMMENT */


/* pop off the service name */

	if ((len = uc_readlinetimed(ifd,srvspec,BUFLEN,SRVTO)) <= 1) {

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

#if	CF_SRVSHELLARG
	slen = fieldsharg(&fsb,NULL,srvargbuf,BUFLEN) ;

/* copy this service string into the 'srvspec' buffer for safe keeping */

	if (slen > 0)
	    strwcpy(srvspec,srvargbuf,slen) ;

	service = srvspec ;
	service[slen] = '\0' ;
#else /* CF_SRVSHELLARG */
	slen = field(&fsb,NULL) ;

	service = fsb.fp ;
	service[slen] = '\0' ;
#endif /* CF_SRVSHELLARG */

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("handle: service=%s\n",service) ;
#endif

	fsb.lp[fsb.rlen] = '\0' ;

	logfile_printf(&gp->lh,"service=%s\n",service) ;


/* do we have this service in our server table ? */

	if (srvtab_match(sfp,service,&sep) >= 0) {
		rs = handle_srventry(gp,&fsb,&srvargs,pbp,sep,ifd,ofd,elp) ;

	} else if ((si = builtin_match(bip,service)) >= 0) {
	    char	srvname[MAXPATHLEN + 1] ;

	    strwcpylc(srvname,service,MAXPATHLEN) ;

	    field_srvargs(&fsb,&srvargs) ;

	    u_time(&daytime) ;

	    logfile_printf(&gp->lh,"%s starting server=%s\n",
	        timestr_log(daytime,timebuf),srvname) ;

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("handle: builtin_exec si=%d\n",si) ;
#endif

	    rs = builtin_execute(bip,si,ifd,ofd,peername,service,&srvargs) ;

	} else {

	rs = SR_NOTFOUND ;
	cp = "- service not found\n" ;
	uc_writen(ofd,cp,strlen(cp)) ;

	}


/* we are out of here */
done:
	u_close(ifd) ;

	u_close(ofd) ;

badnotfound1:
	vecstr_finish(&srvargs) ;

	return rs ;
}
/* end subroutine (handle) */


