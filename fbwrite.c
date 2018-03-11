/* fbwrite */

/* write binary data to a STDIO stream */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-08-17, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a knock-off of the 'fwrite(3)' subroutine, only sensible!


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<string.h>
#include	<stdio.h>

#include	<vsystem.h>


/* local defines */


/* exported subroutines */


int fbwrite(FILE *fp,cchar *bbuf,int blen)
{
	int		rs = SR_OK ;

	if (fp == NULL) return SR_FAULT ;
	if (bbuf == NULL) return SR_FAULT ;

	if (blen < 0)
	    blen = strlen(bbuf) ;

	if (blen > 0) {
	    rs = fwrite(bbuf,1,blen,fp) ;
	    if ((rs == 0) && ferror(fp)) {
	       clearerr(fp) ;
	       rs = SR_IO ;
	    }
	}

	return rs ;
}
/* end subroutine (fbwrite) */


