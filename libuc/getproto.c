/* getproto */

/* get a protocol number given a protocol name */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We try to retrieve a protocol number given a protocol name.

	Synopsis:

	int getproto_name(pp,pl)
	cchar		*pp ;
	int		pl ;

	Arguments:

	pp		pointer to protocol name string
	pl		length of protocol name string

	Returns:

	>=0		protocol number
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<nulstr.h>
#include	<getbufsize.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int getproto_name(cchar *protop,int protol)
{
	NULSTR		n ;
	int		rs ;
	int		proto = 0 ;
	cchar		*pname ;
	if ((rs = nulstr_start(&n,protop,protol,&pname)) >= 0) {
	    struct protoent	pe ;
	    const int		pelen = getbufsize(getbufsize_pe) ;
	    char		*pebuf ;
	    if ((rs = uc_malloc((pelen+1),&pebuf)) >= 0) {
	        if ((rs = uc_getprotobyname(pname,&pe,pebuf,pelen)) >= 0) {
	            proto = pe.p_proto ;
	        }
	        uc_free(pebuf) ;
	    } /* end if (m-a) */
	    nulstr_finish(&n) ;
	} /* end if (nulstr) */
	return (rs >= 0) ? proto : rs ;
}
/* end subroutine (getproto_name) */


