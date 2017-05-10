/* breserve */

/* "Basic I/O" package */
/* last modifed %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* reivsion history:

	= 1998-07-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Some kind of reserve function.

*******************************************************************************/

#define	BFILE_MASTER	0

#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */


/* external subroutines */

extern int	bfile_flushn(bfile *,int) ;


/* external variables */


/* exported subroutines */


int breserve(fp,n)
bfile		*fp ;
int		n ;
{
	int	rs = SR_OK ;


	if (fp == NULL) return SR_FAULT ;

	if (fp->magic != BFILE_MAGIC) return SR_NOTOPEN ;

	if (n < 0) return SR_INVALID ;

	if (! fp->f.nullfile) {
	    if (fp->f.write) {
	        int	blenr = (fp->bdata + fp->bsize - fp->bp) ;
	        if (n > blenr) rs = bfile_flushn(fp,-1) ;
	    }
	}

	return rs ;
}
/* end subroutine (breserve) */



