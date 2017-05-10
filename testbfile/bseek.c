/* bseek */

/* "Basic I/O" package (BFILE) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Seek in the file.

	Synopsis:

	int bseek(fp,wo,whence)
	bfile		*fp ;
	offset_t	wo ;
	int		whence ;

	Arguments:

	- fp		file pointer
	- wo		new offset relative to "whence"
	- whence
			SEEK_SET	0 = from beginning of file
			SEEK_CUR	1 = from current pointer of file
			SEEK_END	2 = from end of file

	Returns:

	>=0		OK
	<0		error


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

extern int	bfile_seeko(bfile *,offset_t,int,offset_t *) ;


/* external variables */


/* local structures */


/* local variables */


/* exported subroutines */


int bseek(fp,wo,w)
bfile		*fp ;
offset_t	wo ;
int		w ;
{
	int	rs = SR_OK ;
	if (fp == NULL) return SR_FAULT ;
	if (fp->magic != BFILE_MAGIC) return SR_NOTOPEN ;
	if (fp->f.notseek) return SR_NOTSEEK ;
	if (! fp->f.nullfile) {
	    rs = bfile_seeko(fp,wo,w,NULL) ;
	}
	return rs ;
}
/* end subroutine (bseek) */


int bseeko(fp,wo,w,rp)
bfile		*fp ;
offset_t	wo ;
int		w ;
offset_t	*rp ;
{
	int	rs = SR_OK ;
	if (fp == NULL) return SR_FAULT ;
	if (fp->magic != BFILE_MAGIC) return SR_NOTOPEN ;
	if (fp->f.notseek) return SR_NOTSEEK ;
	if (! fp->f.nullfile) {
	    rs = bfile_seeko(fp,wo,w,rp) ;
	}
	return rs ;
}
/* end subroutine (bseeko) */


