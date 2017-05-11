/* uc_inetntop */

/* interface component for UNIX® library-3c */
/* convert an INET address to its "print"able form */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<string.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


int uc_inetntop(int af,const void *binaddr,char *rbuf,int rlen)
{
	int		rs ;
	const char	*rp ;

	if (binaddr == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	rp = inet_ntop(af,binaddr,rbuf,rlen) ;
	rs = (rp != NULL) ? strlen(rbuf) : (- errno) ;

	return rs ;
}
/* end subroutine (uc_inetntop) */


