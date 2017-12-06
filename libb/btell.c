/* btell */

/* "Basic I/O" package (BIO) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We tell where we are (in a BFILE stream).


*******************************************************************************/

#define	BFILE_MASTER	0

#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */


/* external subroutines */

extern int	bfile_flush(bfile *) ;


/* external variables */


/* exported subroutines */


int btell(bfile *fp,offset_t *rp)
{
	offset_t	telloff = 0 ;
	int		rs = SR_OK ;

	if (fp == NULL) return SR_FAULT ;

	if (fp->magic != BFILE_MAGIC) return SR_NOTOPEN ;

	if (! fp->f.nullfile) {
	    telloff = fp->offset ;
	    if (! fp->f.notseek) rs = SR_NOTSEEK ;
	}

	if (rp != NULL) *rp = telloff ;
	rs = (telloff & UINT_MAX) ;
	return rs ;
}
/* end subroutine (btell) */


