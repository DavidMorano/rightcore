/* getchostname */

/* subroutine to get a canonical hostname */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is used to get a canonical INET hostname for a supplied
        name. Note carefully that the returned hostname, if any, may NOT be a
        name that can be translated into a good INET address. In other words,
        this subroutine uses its own definition of a "canonical" name and that
        definition does NOT necessarily include the fact that the resulting name
        can be translated into a good INET address. If you want a name that is
        guaranteed to be translatable into a valid INET address, then you want
        to investigate the subroutine GETEHOSTNAME (Get Entry HostName).

        Having said that the resuling name is not guaranteed to be translatable,
        a good translation facility will generally figure out that the given
        name is something that can be translated given the existing host
        information.

	Synopsis:

	int getchostname(name,hostname)
	const char	name[] ;
	char		hostname[] ;

	Arguments:

	name		name to lookup
	hostname	if not NULL, a buffer to hold the resulting hostname

	Returns:

	>=0		<name> had a valid INET address
	<0		<name> did not have a valid address


*******************************************************************************/


#include	<envstandards.h>

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
#include	<localmisc.h>


/* local defines */

#ifndef	LOCALHOST
#define	LOCALHOST	"localhost"
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	getcname(const char *,char *) ;


/* forward references */


/* external variables */


/* local variables */


/* exported subroutines */


int getchostname(name,hostname)
const char	name[] ;
char		hostname[] ;
{


	return getcname(name,hostname) ;
}
/* end if (getchostname) */



