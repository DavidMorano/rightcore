/* quoteshellarg */

/* subroutine to quote arguments for safe passage through a SHELL */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine examines a string supplied by the caller and produces
	an output string with any necessary quotes applied such that
	interpretation by a POSIX-conforming SHELL will result in the original
	string.

	Synopsis:

	int quoteshellarg(arg,arglen,buf,buflen,nvpp)
	const char	arg[] ;
	char		buf[] ;
	int		arglen, buflen ;
	char		**nvpp ;

	Arguments:

	arg		shell argument to be quoted
	arglen		length of shell argument to be quoted
	buf		buffer to place result in
	buflen		length of user supplied buffer for result
	nvpp		pointer to user pointer to receive the user supplied
			buffer address if the operation is successful
			(where did this come from?)

	Returns:

	>=0		length of used destination buffer from conversion
	<0		destination buffer was not big enough or other problem


**************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* external subroutines */

extern int	mkquoted(char *,int,const char *,int) ;
extern int	fieldterms(char *,int,const char *) ;


/* forward references */


/* local variables */


/* exported subroutines */


int quoteshellarg(arg,arglen,buf,buflen,nvpp)
const char	arg[] ;
char		buf[] ;
int		arglen, buflen ;
const char	**nvpp ;
{
	int		rs ;

	rs = mkquoted(buf,buflen,arg,arglen) ;

	if (nvpp != NULL) {
	    *nvpp = (rs >= 0) ? buf : NULL ;
	}

#if	CF_DEBUGS
	debugprintf("quoteshellarg: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (quoteshellarg) */


