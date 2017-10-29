/* uc_inetntop */

/* interface component for UNIX® library-3c */
/* convert an INET address to its "print"able form */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


int uc_inetntop(af,binaddr,sbuf,slen)
int		af ;
const void	*binaddr ;
char		sbuf[] ;
int		slen ;
{
	int	rs ;

	const char	*rp ;


	if ((sbuf == NULL) || (binaddr == NULL))
	    return SR_FAULT ;

	rp = inet_ntop(af,binaddr,sbuf,slen) ;
	rs = (rp != NULL) ? strlen(sbuf) : (- errno) ;

	return rs ;
}
/* end subroutine (uc_inetntop) */


