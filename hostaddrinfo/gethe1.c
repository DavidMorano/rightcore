/* gethename */

/* subroutine to get a single host entry by name (raw) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


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
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>


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







int gethename(name,hep,buf,buflen)
const char	name[] ;
struct hostent	*hep ;
char		buf[] ;
int		buflen ;
{
	int		rs ;
	int		i, serial ;

#if	CF_DEBUGS
	debugprintf("gethename: ent name=%s\n", name) ;
#endif

	if (name == NULL) return SR_FAULT ;
	if ((hep == NULL) || (buf == NULL)) return SR_FAULT ;

/* do the real work */

	rs = uc_gethostbyname(name,hep,buf,buflen) ;

#if	CF_DEBUGS
	debugprintf("gethename: return is %s (%d)\n",
	    ((rs >= 0) ? "good" : "bad"),rs) ;
#endif

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


