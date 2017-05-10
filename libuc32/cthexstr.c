/* cthexstr */

/* subroutine to convert a value (as a counted string) to a HEX string */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine converts a counted array of bytes to a hexadecimal
	string.

	Synopsis:

	int cthexstr(char *dbuf,int dlen,cchar *vp,int vl)

	Arguments:

	dbuf		result buffer
	dlen		length of result buffer
	vp		pointer to source buffer (value)
	sl		length of source in bytes

	Returns:

	<0		error (overflow)
	>=0		length of result bytes


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<strings.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* local structures */


/* forward references */


/* local variables */

static const char	*digtab = "0123456789ABCDEF" ;


/* exported subroutines */


int cthexstr(char *dbuf,int dlen,cchar *sp,int sl)
{
	int		i ;
	int		ch ;
	int		j = 0 ;
	const uchar	*vp = (const uchar *) sp ;

	if (sl < 0) sl = strlen(sp) ;

	for (i = 0 ; (dlen >= 3) && (i < sl) ; i += 1) {
	    ch = vp[i] ;
	    if (i > 0) {
	        dbuf[j++] = ' ' ;
		dlen -= 1 ;
	    }
	    dbuf[j++] = digtab[(ch>>4)&15] ;
	    dbuf[j++] = digtab[(ch>>0)&15] ;
	    dlen -= 2 ;
	} /* end for */
	dbuf[j] = '\0' ;

	return j ;
}
/* end subroutine (cthexstr) */


