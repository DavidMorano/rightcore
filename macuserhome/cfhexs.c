/* cfhexs */

/* convert from a HEX string */


/* revision history:

	= 1998-10-01, David A­D­ Morano
	This subroutine was adapted from assembly.  The original assembly goes
	wa...ay back.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine converts a string of HEX digits into a character string.
	Every two hexadecimal digits are converted into one character.  Yes,
	this was originally in assembly language (and YES, was incredibly much
	faster).

	Synopsis:

	int cfhexs(cchar *sbuf,int slen,uchar *dbuf)

	Arguments:

	sbuf		address of string to be converted
	slen		len of source address to convert
	dbuf		address of buffer to store result

	Outputs:

	<0		error
	>=0		length of result


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* external subroutines */

extern int	sfshrink(cchar *,int,cchar **) ;
extern int	hexval(int) ;
extern int	ishexlatin(int) ;


/* forward references */


/* exported subroutines */


int cfhexs(cchar *sp,int sl,uchar *rp)
{
	int		rs = SR_OK ;
	int		cl ;
	cchar		*cp ;
	const uchar	*rbuf = (const uchar *) rp ;

	if ((cl = sfshrink(sp,sl,&cp)) > 0) {
	    int		ch0, ch1 ;
	    int		v ;
	    while ((rs >= 0) && (cl >= 2) && cp[0]) {
	        ch0 = MKCHAR(cp[0]) ;
	        ch1 = MKCHAR(cp[1]) ;
	        if (ishexlatin(ch0) && ishexlatin(ch1)) {
		    v = 0 ;
	            v |= (hexval(ch0)<<4) ;
	            v |= (hexval(ch1)<<0) ;
	            *rp++ = (char) v ;
	        } else {
		    rs = SR_INVALID ;
	        }
	        cp += 2 ;
	        cl -= 2 ;
	    } /* end while */
	    if ((rs >= 0) && (cl > 0)) {
		rs = SR_INVALID ;
	    }
	} else {
	    rs = SR_INVALID ;
	}

	return (rs >= 0) ? (rp-rbuf) : rs ;
}
/* end subroutine (cfhexs) */


