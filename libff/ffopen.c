/* ffopen */

/* open a STDIO stream */


/* revision history:

	= 1986-01-17, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1986 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This is a knock-off of the 'fopen(3c)' subroutine, only sensible!

	Synopsis:

	int ffopen(FFILE *,fname,modestr)
	FFILE		*fp ;
	const char	fname[] ;
	const char	modestr[] ;

	Arguments:

	fp		pointer to FFILE object
	fname		filename
	modestr		open mode string

	Returns:

	<0		error
	>=0		OK


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdio.h>
#include	<errno.h>

#include	<vsystem.h>

#include	"ffile.h"


/* local defines */


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;


/* local variables */

const char	*ffile_fnames[] = {
	"*STDIN*",
	"*STDOUT*",
	"*STDERR*",
	"*STDNULL*",
	NULL
} ;


/* exported subroutines */


int ffopen(fp,fname,modestr)
FFILE		*fp ;
const char	fname[] ;
const char	modestr[] ;
{
	FILE	*sfp ;

	int	rs = SR_OK ;
	int	fi ;


	if (fp == NULL) return SR_FAULT ;
	if ((fname == NULL) || (modestr == NULL)) return SR_FAULT ;

	if ((fi = matstr(ffile_fnames,fname,-1)) >= 0) {
	    switch (fi) {
	    case 0:
		fname = "/dev/stdin" ;
		break ;
	    case 1:
		fname = "/dev/stdout" ;
		break ;
	    case 2:
		fname = "/dev/stderr" ;
		break ;
	    case 3:
	    default:
		fname = "/dev/null" ;
		break ;
	    } /* end switch */
	} /* end if */

	sfp = fopen(fname,modestr) ;
	if (sfp == NULL) rs = (- errno) ;

	fp->sfp = (rs >= 0) ? sfp : NULL ;

	return rs ;
}
/* end subroutine (ffopen) */



