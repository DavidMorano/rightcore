/* pathadd */

/* add a component to an existing path */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2002-07-13, David A­D­ Morano
	This was originally written.

*/

/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine adds a new component to an existing file-path.

	Synopsis:

	int pathadd(char *pbuf,int plen,const char *add)

	Arguments:

	pbuf		existing path
	plen		length of existing path
	add		new componment to add

	Returns:

	>=0		new length of new path
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vsystem.h>
#include	<storebuf.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int pathaddw(char *pbuf,int pl,const char *np,int nl)
{
	const int	plen = MAXPATHLEN ;
	int		rs = SR_OK ;

	if ((rs >= 0) && (pl > 0) && (pbuf[pl-1] != '/')) {
	    rs = storebuf_char(pbuf,plen,pl,'/') ;
	    pl += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_strw(pbuf,plen,pl,np,nl) ;
	    pl += rs ;
	}

	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (pathaddw) */


int pathadd(char *pbuf,int pl,const char *np)
{
	return pathaddw(pbuf,pl,np,-1) ;
}
/* end subroutine (pathadd) */


