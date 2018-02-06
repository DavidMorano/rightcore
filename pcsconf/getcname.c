/* getcname */

/* subroutine to get a canonical hostname */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This program was originally written.

	= 2017-05-06, David A.D. Morano
	This rewrote this from scratch to use the HOSTINFO object.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is used to get a canonical INET hostname for a supplied
        name. Note carefully that the returned hostname, if any, may NOT be a
        name that can be traslated into a good INET address. In other words,
        this subroutine defines its own definition of a "canonical" name and
        that definition does NOT necessarily include the fact that the resulting
        name can be translated into a good INET address. If you want a name that
        is guaranteed to be translatable into a valid INET address, then you
        want to investigate the subroutine GETEHOSTNAME (Get Entry HostName).

	Synopsis:

	int getcname(name,hostname)
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
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<hostinfo.h>
#include	<localmisc.h>
#include	<getbufsize.h>


/* local defines */

#ifndef	LOCALHOST
#define	LOCALHOST	"localhost"
#endif

#ifndef	INETADDRBAD
#define	INETADDRBAD	((unsigned int) (~ 0))
#endif


/* external subroutines */

extern int	sncpy1(char *,int,cchar *) ;
extern int	snwcpy(char *,int,cchar *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	mkhexstr(char *,int,const void *,int) ;
#endif /* CF_DEBUGS */

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local typedefs */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int getcname(cchar *name,char *hbuf)
{
	HOSTINFO	hi ;
	const int	af = AF_UNSPEC ;
	int		rs ;
	int		rs1 ;
	int		len = 0 ;
	if ((rs = hostinfo_start(&hi,af,name)) >= 0) {
	    cchar	*cnp ;
	    if ((rs = hostinfo_getcanonical(&hi,&cnp)) >= 0) {
		const int	hlen = MAXHOSTNAMELEN ;
		rs = sncpy1(hbuf,hlen,cnp) ;
		len = rs ;
	    }
	    rs1 = hostinfo_finish(&hi) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (hostinfo) */
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (getcname) */


