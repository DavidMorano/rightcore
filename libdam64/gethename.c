/* gethename */

/* subroutine to get a single host entry by name (raw) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_LOG		0


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is a platform independent subroutine to get an INET host
        address entry, but does it dumbly on purpose.

	Synopsis:

	int gethename(name,hep,buf,buflen)
	const char	name[] ;
	struct hostent	*hep ;
	char		buf[] ;
	int		buflen ;

	Arguments:

	- name		name to lookup
	- hep		pointer to 'hostent' structure
	- buf		user supplied buffer to hold result
	- buflen	length of user supplied buffer

	Returns:

	-2		request timed out (bad network someplace)
	-1		host could not be found
	0		host was found OK


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
#include	<time.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<ugetpid.h>
#include	<bfile.h>
#include	<localmisc.h>

#if	CF_LOG
#include	<logfile.h>
#endif


/* local defines */

#define	TIMEOUT		3

#ifndef	LOGIDLEN
#define	LOGIDLEN	80
#endif

#define	LOGFNAME	"/tmp/gethostbyname.log"
#define	SERIALFILE1	"/tmp/serial"
#define	SERIALFILE2	"/tmp/gethename.serial"
#define	DEFLOGSIZE	80000


/* external subroutines */

extern int	snddd(char *,int,uint,uint) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int gethename(name,hep,buf,buflen)
const char	name[] ;
struct hostent	*hep ;
char		buf[] ;
int		buflen ;
{

#if	CF_LOG
	logfile	lh ;
#endif

#if	CF_LOG
	pid_t	pid ;
#endif

#if	CF_LOG
	int	f_log = FALSE ;
#endif

	int	rs ;

#if	CF_LOG
	char	logid[LOGIDLEN + 1] ;
#endif


#if	CF_DEBUGS
	debugprintf("gethename: name=%s\n", name) ;
#endif

	if (name == NULL)
	    return SR_FAULT ;

	if ((hep == NULL) || (buf == NULL))
	    return SR_FAULT ;

/* do we want logging performed? */

#if	CF_LOG
	if (u_access(LOGFNAME,W_OK) >= 0) {
	    pid = ugetpid() ;
	    int		serial = -1 ;
	    char	*cp ;

	    if (serial < 0) {
	        cp = SERIALFILE1 ;
	        serial = getserial(cp) ;
	    }
	    if (serial < 0) {
	        cp = SERIALFILE2 ;
	        serial = getserial(cp) ;
	    }

	    if (serial == 0)
	        u_chmod(cp,0666) ;

	    if (serial < 0)
	        serial = 0 ;

	    snddd(logid,LOGIDLEN, pid,serial) ;

	    rs = logfile_open(&lh,LOGFNAME,0,0666,logid) ;
	    f_log = (rs >= 0) ;

	    if (f_log)
	        logfile_printf(&lh,
			"name=%s\n",(name == NULL) ? STDNULLFNAME : name) ;

	} /* end if */
#endif /* CF_LOG */

/* do the real work */

	rs = uc_gethostbyname(name,hep,buf,buflen) ;

#if	CF_LOG
	if (f_log) {

	    if (rs == SR_TIMEDOUT) {
	        logfile_printf(&lh,"network timeout (rs=%d)\n",rs) ;

	    } else if (rs == SR_NOTFOUND) {
	        logfile_printf(&lh,"entry not found (rs=%d)\n",rs) ;

	    } else
	        logfile_printf(&lh,"result=%s (rs=%d)\n",
	            ((rs >= 0) ? hep->h_name : STDNULLFNAME),rs) ;

	}
#endif /* CF_LOG */

ret1:

#if	CF_DEBUGS
	debugprintf("gethename: return is %s (%d)\n",
	    ((rs >= 0) ? "good" : "bad"),rs) ;
#endif

#if	CF_LOG
	if (f_log) {
	    logfile_checksize(&lh,DEFLOGSIZE) ;
	    logfile_close(&lh) ;
	}
#endif /* CF_LOG */

ret0:

#if	CF_DEBUGS
	debugprintf("gethename: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (gethename) */


/* ALTERNATE API */


int gethe1(name,hep,buf,buflen)
char		name[] ;
struct hostent	*hep ;
char		buf[] ;
int		buflen ;
{


	return gethename(name,hep,buf,buflen) ;
}
/* end subroutine (gethe1) */


