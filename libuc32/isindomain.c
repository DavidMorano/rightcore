/* isindomain */

/* subroutine to determine if the name is in the given domain */
/* last modified %G% version %I% */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine determines if a given hostname is in the specified
	domain or not.

	Synopsis:

	int isindomain(nodename,domainname)
	const char	nodename[] ;
	const char	domainname[] ;

	Arguments:

	nodename	if not NULL, a buffer to hold the resulting hostname
	domainname	if not NULL, the domain name to check against

	Returns:

	TRUE		it is an INET address
	FALSE		it is not


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<stdlib.h>
#include	<strings.h>		/* for |strcasecmp(3c)| */
#include	<netdb.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* exported subroutines */


int isindomain(const char *name,const char *domainname)
{
	char		*tp ;

	if ((tp = strchr(name,'.')) == NULL)
	    return TRUE ;

	if (tp[1] == '\0')
	    return FALSE ;

	return (strcasecmp((tp + 1),domainname) == 0) ;
}
/* end subroutine (isindomain) */


