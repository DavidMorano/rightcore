/* getehostname (Get Entry Hostname) */

/* get a host name that has an INET address */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine takes the given name and finds a (possibly new) name
        which is guaranteed to be able to translate to an INET address. If no
        suitable substitute name is found that can be translated into an INET
        addres, a BAD return value is returned. If a suitable name can be found
        that can be translated into a valid INET address, then that name or
        another valid name is returned in the supplied buffer (if one is
        supplied) and an OK value is returned.

	Synopsis:

	int getehostname(name,hostname)
	char	name[] ;
	char	hostname[] ;

	Arguments:

	- name		given name to find a substitute for
	- hostname	a buffer to receive a substitute name (optional) ;
			this can be NULL and no substitute name is returned

	Retruns:

	OK		a valid substitute can be found
	BAD		no valid substitute can be found

	Important note:

        Although this routine may return an OK status to indicate that a valid
        substitute can be found, that does not mean that the original name can
        be translated into an INET address. An OK status only means that this
        subroutine can find a valid substitute. For other INET service
        subroutines which take a hostname type argument, you should always make
        sure that you pass it a name that CAN be translated into an INET
        address, if indeed it can have one as indicated by this subroutine. Got
        it? Good. If not, look at the code carefully and figure out what it is
        trying to test for and do!

        Oh, I almost forgot, a NULL specified name is recognized as the local
        host name.

        Another final note to know is that if a substitute host name can be
        found, and may indeed be returned, it does NOT mean that it is the
        canonical name of the given host. It does not even mean that it may be a
        real good name for the given host. It only means that the substitute
        name can be translated into an INET address which can represent the
        given host. Got that? Good.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/utsname.h>
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
#include	<localmisc.h>


/* local defines */

#define	HOSTBUFLEN	(4 * 1024)


/* external subroutines */

extern int	getourhe(const char *,char *,struct hostent *,char *,int) ;


/* forward references */


/* external variables */


/* local variables */


/* exported subroutines */


int getehostname(name,hostname)
const char	name[] ;
char		hostname[] ;
{
	struct hostent	he ;

	int	rs ;

	char	hostbuf[HOSTBUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("getehostname: ent name=%s\n",name) ;
#endif

	rs = getourhe(name,hostname,&he,hostbuf,HOSTBUFLEN) ;

#if	CF_DEBUGS
	debugprintf("getehostname: exiting hostname=%s\n",hostname) ;
#endif

	return rs ;
}
/* end subroutine (getehostname) */


