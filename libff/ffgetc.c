/* ffgetc */

/* read a coded line from the STDIO stream */


#define	CF_FGETS		0	/* faster or not ? */


/* revision history:

	= 1986-01-17, David A­D­ Morano

	This subroutine was originally written.


	= 2001-09-10, David A­D­ Morano

	I discovered that on the SGI Irix systems, 'getc()' does
	not seem to work properly so I hacked it out for that
	system.


*/

/* Copyright © 1986-2001 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This routine will only read at most 'len' number of bytes
	from the file.


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdio.h>
#include	<errno.h>

#include	<vsystem.h>

#include	"ffile.h"


/* local defines */

#define	TO_AGAIN	(5*60)

#if	defined(OSNAME_IRIX)		/* IRIX screws up! (what else is new) */
#define	mygetc(p)	fgetc(p)
#else
#define	mygetc(p)	getc(p)
#endif


/* exported subroutines */


int ffgetc(fp)
FFILE	*fp ;
{
	FILE	*sfp ;

	int	rs ;
	int	to = TO_AGAIN ;


	if (fp == NULL) return SR_FAULT ;

	sfp = fp->sfp ;

again:
	rs = mygetc(sfp) ;
	if (rs == EOF) {
		if (ferror(sfp)) {
		    rs = (- errno) ;
		} else if (feof(sfp)) {
		    rs = SR_EOF ;
		} else {
		    rs = SR_NOANODE ;
		}
	    clearerr(sfp) ;
	}

	if (rs < 0) {
	    switch (rs) {
	    case SR_INTR:
		goto again ;
	    case SR_AGAIN:
	        if (to-- > 0) {
		    sleep(1) ;
		    goto again ;
		}
		break ;
	    } /* end switch */
	} /* end if */

	return rs ;
}
/* end subroutine (ffgetc) */



