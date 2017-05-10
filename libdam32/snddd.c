/* snddd */

/* specialized string formatting (Decimal-Dot-Decimal) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine constructs a single string from two specificed integers
	and it also puts a period character between the string representations
	of the two integers (Decimal-Dot-Decimal).

	Synopsis:

	int snsds(dbuf,dlen,d1,d2)
	char		dbuf[] ;
	int		dlen ;
	uint		d1, d2 ;

	Arguments:

	dbuf		destination buffer
	dlen		destination buffer length
	d1		value 1
	d2		value 2

	Returns:

	>=0		length of created string
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vsystem.h>
#include	<storebuf.h>
#include	<localmisc.h>


/* local defines */

#define	MIDDLECHAR	'.'


/* external subroutines */


/* exported subroutines */


int snddd(char *dbuf,int dlen,uint d1,uint d2)
{
	int		rs = SR_OK ;
	int		i = 0 ;

	if (rs >= 0) {
	    rs = storebuf_decui(dbuf,dlen,i,d1) ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_char(dbuf,dlen,i,MIDDLECHAR) ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_decui(dbuf,dlen,i,d2) ;
	    i += rs ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (snddd) */


