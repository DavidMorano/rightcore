/* snsd */

/* string-decimal (String-Decimal) */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine copies a:

	+ string
	+ an integer value (coded into a decimal string)

	into a counted result buffer.

	Synopsis:

	int snsd(rbuf,rlen,s1,v)
	char		rbuf[] ;
	int		rlen ;
	const char	s1[] ;
	uint		n ;

	Arguments:

	rbuf		result buffer
	rlen		size of supplied result buffer
	s1		string
	v		number

	Returns:

	>=0		resulting length
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<storebuf.h>
#include	<localmisc.h>


/* external subroutines */


/* exported subroutines */


int snsd(char *dbuf,int dlen,cchar *s1,uint v)
{
	int		rs = SR_OK ;
	int		i = 0 ;

	if (rs >= 0) {
	    rs = storebuf_strw(dbuf,dlen,i,s1,-1) ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_decui(dbuf,dlen,i,v) ;
	    i += rs ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (snsd) */


