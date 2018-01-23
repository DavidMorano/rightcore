/* ns_getheaddr */

/* subroutine to get a single host entry by its address */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_LOG		1


/* revision history:

	= 1995-11-01, David A­D­ Morano
	This program was originally written.

	= 1998-11-22, David A­D­ Morano
        I did some clean-up.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is a platform independent subroutine to
	get an INET host entry by its INET address, 
	but does it dumbly on purpose.

	Synopsis:

	int ns_getheaddr(addr,hep,buf,buflen)
	char		addr[] ;
	struct hostent	*hep ;
	char		buf[] ;
	int		buflen ;

	Arguments:

	addr		address to lookup
	hep		pointer to 'hostent' structure
	buf		user supplied buffer to hold result
	buflen		length of user supplied buffer

	Returns:

	0		host was found OK
	-1		fault with call
	-2		host could not be found
	-3		request timed out (bad network someplace)


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<sys/utsname.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<pwd.h>
#include	<grp.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<bfile.h>

#if	CF_LOG
#include	<logfile.h>
#endif

#include	<localmisc.h>


/* local defines */

#ifndef	INET4DOTDECLEN
#define	INET4DOTDECLEN	16
#endif

#define	TIMEOUT		3

#ifndef	LOGIDLEN
#define	LOGIDLEN	80
#endif

#define	LOGFNAME	"/tmp/gethostbyaddr.log"
#define	SERIALFILE1	"/tmp/serial"
#define	SERIALFILE2	"/tmp/ns_gethe1.serial"
#define	DEFLOGSIZE	80000

#define	NSR_OK		0
#define	NSR_FAULT	-1
#define	NSR_NOTFOUND	-2
#define	NSR_TIMEDOUT	-3


/* external subroutines */


/* forward references */


/* external variables */

#ifndef	POSIX
extern int	h_errno ;
#endif


/* global variables */


/* local structures */


/* exported subroutines */


int ns_getheaddr(addr,hep,buf,buflen)
char		addr[] ;
struct hostent	*hep ;
char		buf[] ;
int		buflen ;
{
#if	CF_LOG
	logfile	lh ;
#endif

	struct hostent	*lp ;

#if	CF_LOG
	pid_t	pid ;
#endif

	int	rs ;
	int	i, serial ;

#ifdef	POSIX
	int	h_errno ;
#endif

#if	CF_LOG
	char	logid[LOGIDLEN + 1] ;
#endif


#if	CF_DEBUGS
	debugprintf("ns_gethe1: ent '%s'\n",
	    name) ;
#endif

/* do we want logging performed? */

#if	CF_LOG
	{
	    char	addrbuf[INET4DOTDECLEN+1] ;

	pid = getpid() ;

	if ((serial = ns_getserial(SERIALFILE1)) < 0)
		serial = ns_getserial(SERIALFILE2) ;

	bufprintf(logid,LOGIDLEN,"%d.%d",pid,serial) ;

	logfile_open(&lh,LOGFNAME,0,0666,logid) ;

	inet_ntoa_r(addr,addrbuf,INET4DOTDECLEN) ;

	logfile_printf(&lh,"addr=%s\n",addrbuf) ;

	} /* end block */
#endif /* CF_LOG */

	if (addr == NULL)
		return NSR_FAULT ;

/* do the real work */

	memset(hep,0,sizeof(struct hostent)) ;

#ifdef	POSIX

	h_errno = 0 ;
	for (i = 0 ; i < TIMEOUT ; i += 1) {

#if	CF_DEBUGS
	    debugprintf("ns_gethe1: top of loop\n") ;
#endif

	    lp = gethostbyaddr_r(addr,sizeof(long),AF_INET,
	        hep,buf,buflen,&h_errno) ;

#if	CF_DEBUGS
	    debugprint("ns_gethe1: bottom of loop\n") ;
#endif 

	    if ((lp != NULL) || (h_errno != TRY_AGAIN)) break ;

	    sleep(1) ;

	} /* end for */

#else /* ! defined(POSIX) */

	for (i = 0 ; i < TIMEOUT ; i += 1) {

#if	CF_DEBUGS
	    debugprint("ns_gethe1: not on POSIX\n") ;
#endif

#if	CF_DEBUGS
	    debugprint("ns_gethe1: about to call 'gethostbyaddr'\n") ;
#endif

	    lp = gethostbyaddr(addr) ;

#if	CF_DEBUGS
	    debugprint("ns_gethe1: returned from 'gethostbyaddr'\n") ;
#endif

	    if ((lp != NULL) || (h_errno != TRY_AGAIN)) break ;

#if	CF_DEBUGS
	    debugprint("ns_gethe1: maybe some sleep\n") ;
#endif

	    sleep(1) ;

	} /* end for */

#if	CF_DEBUGS
	debugprint("ns_gethe1: some memcpy\n") ;
#endif

	if (lp != NULL)
	    memcpy(hep,lp,sizeof(struct hostent)) ;

#endif /* POSIX or not */

	rs = NSR_OK ;
	if (i >= TIMEOUT)
		rs = NSR_TIMEDOUT ;

	else if (lp == NULL)
		rs = NSR_NOTFOUND ;

#if	CF_LOG
	if (rs == NSR_TIMEDOUT)
	    logfile_printf(&lh,"network timeout (rs=%d)\n",rs) ;

	else if (rs == NSR_NOTFOUND)
	    logfile_printf(&lh,"entry not found (rs=%d)\n",rs) ;

	else
	    logfile_printf(&lh,"result=%s (rs=%d)\n",
	        ((lp == NULL) ? STDNULLFNAME : lp->h_name),rs) ;
#endif /* CF_LOG */

#if	CF_DEBUGS
	debugprintf("ns_gethe1: return is %s\n",
	    (lp == NULL) ? "bad" : "good") ;
#endif

#if	CF_LOG
	logfile_checksize(&lh,DEFLOGSIZE) ;

	logfile_close(&lh) ;
#endif

	return rs ;
}
/* end subroutine (ns_getheaddr) */



