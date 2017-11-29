/* cthexstr */

/* subroutine to convert a value (as a counted string) to a HEX string */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

	= 2017-08-15, David A­D­ Morano
	Rewrote to use the SBUF object.

*/

/* Copyright © 1998,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine converts a counted array of bytes to a hexadecimal
	string.

	Synopsis:

	int cthexstring(char *dbuf,int dlen,int f,cchar *vp,int vl)

	Arguments:

	dbuf		result buffer
	dlen		length of result buffer
	f		flag specifying type of output
	vp		pointer to source buffer (value)
	vl		length of source in bytes

	Returns:

	<0		error (overflow)
	>=0		length of result bytes

	Example-output:

	+ for (!f)
		48656C6C6F20776F726C64210A
 	+ for (f)
		48 65 6C 6C 6F 20 77 6F 72 6C 64 21 0A


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<strings.h>
#include	<sbuf.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int cthexstring(char *dbuf,int dlen,int f,cchar *sp,int sl)
{
	SBUF		b ;
	int		rs ;
	int		len = 0 ;

	if (sl < 0) sl = strlen(sp) ;

	if ((rs = sbuf_start(&b,dbuf,dlen)) >= 0) {
	    int		ch ;
	    int		v ;
	    int		i ;
	    const uchar	*vp = (const uchar *) sp ;
	    for (i = 0 ; (rs >= 0) && (i < sl) ; i += 1) {
	        ch = vp[i] ;
	        if (f && (i > 0)) {
		    sbuf_char(&b,' ') ;
	        }
		rs = sbuf_hexc(&b,ch) ;
	    } /* end for */
	    len = sbuf_finish(&b) ;
	    if (rs >= 0) rs = len ;
	} /* end if (sbuf) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (cthexstring) */


int cthexstr(char *dbuf,int dlen,cchar *sp,int sl)
{
	return cthexstring(dbuf,dlen,FALSE,sp,sl) ;
}
/* end subroutine (cthexstr) */


