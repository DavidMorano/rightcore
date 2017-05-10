/* bdup */

/* "Basic I/O" package */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0	/* compile-time debug print-outs */


/* revision history:

	= 1998-07-01, David A­D­ Morano

	This subroutine was originally written.


	= 1999-01-10, David A­D­ Morano

	I added the little extra code to allow for memory mapped I/O.
	It is all a waste because it is way slower than without it!
	This should teach me to leave old programs alone!!!


*/

/* Copyright © 1998,1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Duplicate a BFILE stream.


*******************************************************************************/


#define	BFILE_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/resource.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */

#define	BO_READ		1
#define	BO_WRITE	2
#define	BO_APPEND	4


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int bdup(fp,fnewp)
bfile		*fp ;
bfile		*fnewp ;
{
	int	rs = SR_OK ;

	char	*p ;


#if	CF_DEBUGS
	debugprintf("bdup: entered\n") ;
#endif

	if (fp == NULL)
	    return SR_FAULT ;

	if (fp->magic != BFILE_MAGIC)
	    return SR_NOTOPEN ;

	if (fnewp == NULL)
	    return SR_FAULT ;

	if (fp->f.nullfile) {
	    rs = SR_NOSYS ;
	    goto ret0 ;
	}

	memcpy(fnewp,fp,sizeof(bfile)) ;

	rs = bfile_flush(fp) ;
	if (rs < 0)
	    goto bad0 ;

	rs = u_dup(fp->fd) ;
	fnewp->fd = rs ;
	if (rs < 0)
	    goto bad0 ;

	if (fp->bsize > 0) {
	    rs = uc_malloc(fp->bsize,&p) ;
	    if (rs < 0) goto bad1 ;
	    fnewp->bdata = p ;
	    fnewp->bbp = p ;
	    fnewp->bp = p ;
	}

ret0:
	return rs ;

/* bad stuff */
bad1:
	u_close(fnewp->fd) ;
	fnewp->fd = -1 ;

bad0:
	fnewp->magic = 0 ;
	goto ret0 ;
}
/* end subroutine (bdup) */



