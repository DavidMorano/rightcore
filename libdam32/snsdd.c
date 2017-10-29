/* snsdd */

/* specialized string formatting (Decimal-Dot-Decimal) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine constructs a single string from:

	+ a string
 	+ a fixed decimal point (a period character)
	+ a decimal number

	Synopsis:

	int snsdd(dbuf,dlen,s1,d1)
	char		dbuf[] ;
	int		dlen ;
	const char	s1[] ;
	uint		d1 ;

	Arguments:

	dbuf		destination buffer
	dlen 		destination buffer length
	s1		source string
	d1 		digit

	Returns:

	<0		error
	>=0		length of created string in destination


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


int snsdd(char *dbuf,int dlen,cchar *s1,uint d1)
{
	int		rs = SR_OK ;
	int		i = 0 ;

	if (rs >= 0) {
	    rs = storebuf_strw(dbuf,dlen,i,s1,-1) ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_char(dbuf,dlen,i,MIDDLECHAR) ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_decui(dbuf,dlen,i,d1) ;
	    i += rs ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (snsdd) */


