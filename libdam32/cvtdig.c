/* cvtdig */

/* convert digits into a string given a value and base */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine converts a value (and a base) into its string
        representation. It only works for bases that are a power-of-two, and
        also only for bases up to sixteen (16).

	Synopsis:

	int cvtdig(char *buf,ULONG val,int n,int b)

	Arguments:

	buf		result buffer
	val		value to convert
	n		number of bytes in given value to convert, generally:
				1, 2, 4, or 8
	b		the base to use, generally:
				2=binary
				8=octal
				16=hexadecimal

	Returns:

	-		length of result characters


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* local structures */


/* forward references */

static int	ffbs(int) ;


/* local variables */

static cchar	*digtab = "0123456789ABCDEF" ;


/* exported subroutines */


int cvtdig(char *rbuf,int rlen,ulonglong val,int n,int b)
{
	const int	nshift = ffbs(b) ;
	int		rs = SR_OK ;
	int		ndig ;
	ndig = ((n*8)+nshift-1)/nshift ;
	if (ndig <= rlen) {
	    const int	mask = (b-1) ;
	    int		i ;
	    for (i = (ndig - 1) ; i >= 0 ; i -= 1) {
	        rbuf[i] = digtab[val & mask] ;
	        val >>= nshift ;
	    } /* end for */
	    rbuf[ndig] = '\0' ;
	} else {
	    rs = SR_OVERFLOW ;
	}
	return (rs >= 0) ? ndig : rs ;
}
/* end subroutine (cvtdig) */


/* local subroutines */


/* Find-First-Bit-Set (in an integer) */
static int ffbs(int v)
{
	const int	n = (sizeof(int)*8) ;
	int		i ;
	for (i = 0 ; i < n ; i += 1) {
	    if (v&1) break ;
	    v >>= 1 ;
	}
	return i ;
}
/* end subroutine (ffbs) */


