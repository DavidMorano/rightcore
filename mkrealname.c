/* mkrealname */

/* make a realname */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.  

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine creates a realname from a GECOS-name or some other
	non-processed or non-canonical name.

	Synopsis:

	int mkrealname(rbuf,rlen,gnp,gnl)
	char		rbuf[] ;
	int		rlen ;
	const char	gnp[] ;
	int		gnl ;

	Arguments:

	rbuf		supplied buffer to receive result
	rlen		length of supplied buffer to receive result
	gnp		name to convert (usually a GECOS-name)
	gnl		length of name to convert

	Returns:

	>=0		length of result
	<0		error


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<realname.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int mkrealname(char *rbuf,int rlen,cchar *gnp,int gnl)
{
	REALNAME	rn ;
	int		rs ;
	int		rs1 ;
	int		len = 0 ;

	if (rbuf == NULL) return SR_FAULT ;
	if (gnp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("mkrealname: ent g=>%t<\n",gnp,gnl) ;
#endif

	if ((rs = realname_start(&rn,gnp,gnl)) >= 0) {
	    {
	        rs = realname_name(&rn,rbuf,rlen) ;
	        len = rs ;
	    }
	    rs1 = realname_finish(&rn) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (realname) */

#if	CF_DEBUGS
	debugprintf("mkrealname: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mkrealname) */


