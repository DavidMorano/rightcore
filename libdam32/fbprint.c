/* fbprint */

/* print some (binary) data out to a STDIO stream */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-08-17, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This is a knock-off of the 'fwrite(3)' subroutine, only
	sensible!


******************************************************************************/


#include	<envstandards.h>

#include	<stdio.h>

#include	<vsystem.h>


/* local defines */


/* exported subroutines */


int fbprint(fp,lbuf,llen)
FILE		*fp ;
const char	lbuf[] ;
int		llen ;
{
	int	rs = SR_OK ;
	int	wlen = 0 ;


	if (fp == NULL)
	    return SR_FAULT ;

	if (lbuf == NULL)
	    return SR_FAULT ;

	if (llen < 0) llen = strlen(lbuf) ;

	if (llen > 0) {
	    rs = fbwrite(fp,lbuf,llen) ;
	    wlen += rs ;
	}

	if ((rs >= 0) && ((llen == 0) || (lbuf[llen-1] != '\n'))) {
	    rs = fbwrite(fp,"\n",1) ;
	    wlen += rs ;
	}

	return rs ;
}
/* end subroutine (fbprint) */



