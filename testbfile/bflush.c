/* bflush */

/* "Basic I/O" package */
/* last modifed %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* reivsion history:

	= 1998-07-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Flush interface for BFILE.

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


int bflushn(fp,n)
bfile	*fp ;
int	n ;
{
	int	rs = SR_OK ;


	if (fp == NULL) return SR_FAULT ;

	if (fp->magic != BFILE_MAGIC) return SR_NOTOPEN ;

	if (! fp->f.nullfile) {
	    if (fp->f.write && (fp->len > 0))
	        rs = bfile_flushn(fp,n) ;
	}

	return rs ;
}
/* end subroutine (bflushn) */


int bflush(fp)
bfile	*fp ;
{


	return bflushn(fp,-1) ;
}
/* end subroutine (bflush) */

	

