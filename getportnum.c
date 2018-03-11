/* getportnum */

/* get an INET port number */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-04-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine tries to retrieve a port number given:
		a. a protocol name
		b. a service name

	Synopsis:

	int getportnum(protoname,portspec)
	const char	protoname[] ;
	const char	portspec[] ;

	Arguments:

	protoname	protocol name
	portspec	port specification to lookup 

	Return:

	>=0		port-number
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	getserv_name(cchar *,cchar *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	hasalldig(const char *,int) ;
extern int	isdigitlatin(int) ;


/* external variables */


/* local typedefs */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int getportnum(cchar *pn,cchar *ps)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		pl ;
	int		port = -1 ;

	if (ps == NULL) return SR_FAULT ;

	if (ps[0] == '\0') return SR_INVALID ;

	pl = strlen(ps) ;

	if (hasalldig(ps,pl)) {
	    rs = cfdeci(ps,pl,&port) ;
	} /* end if */

	if ((rs >= 0) && (port < 0) && (pn != NULL)) {
	    if ((rs = getserv_name(pn,ps)) >= 0) {
	        port = rs ;
	    } /* end if (getserv_name) */
	} /* end if */

	if ((rs >= 0) && (port < 0)) {
	    rs = SR_NOTFOUND ;
	}

	return (rs >= 0) ? port : rs ;
}
/* end subroutine (getportnum) */


