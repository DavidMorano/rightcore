/* bputc */

/* "Basic I/O" package similiar to "stdio" */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_SAFE		1		/* safe mode */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Print out a single character (within the BFILE framework).


*******************************************************************************/

#define	BFILE_MASTER	0

#include	<envstandards.h>
#include	<sys/types.h>
#include	<vsystem.h>
#include	<localmisc.h>
#include	"bfile.h"


/* local defines */


/* external subroutines */


/* external variables */


/* externals within the library */

extern int	bfile_flush(bfile *) ;


/* exported subroutines */


int bputc(bfile *fp,int ch)
{
	int		rs ;
	int		wlen = 0 ;
	char		buf[2] ;

#if	CF_SAFE
	if (fp == NULL) return SR_FAULT ;
#endif /* CF_SAFE */

	buf[0] = ch ;
	if ((rs = bwrite(fp,buf,1)) > 0) {
	    wlen = rs ;
	    if ((ch == '\n') && (fp->bm == bfile_bmline)) {
	        rs = bfile_flush(fp) ;
	    }
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (bputc) */


