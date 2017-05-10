/* btruncate */

/* "Basic I/O" package similiar to "stdio" */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We truncate a BFILE stream.


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


/* external variables */


/* externals within the library */

extern int	bfile_flush(bfile *) ;


/* exported subroutines */


int btruncate(bfile *fp,offset_t off)
{
	int		rs = SR_OK ;

	if (fp == NULL) return SR_FAULT ;

	if (fp->magic != BFILE_MAGIC) return SR_NOTOPEN ;

	if (! fp->f.nullfile) {
	    if ((rs = bfile_flush(fp)) >= 0) {
	        rs = uc_ftruncate(fp->fd,off) ;
	    }
	}

	return rs ;
}
/* end subroutine (btruncate) */


