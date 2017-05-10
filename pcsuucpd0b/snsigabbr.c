/* snsigabbr */

/* make the string repreentation of a signal number */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We create the string repreentation of a signal number.

	Synopsis:

	int snsigabbr(dbuf,dlen,n)
	char		*dbuf ;
	int		dlen ;
	int		n ;
	
	Arguments:

	dbuf		destination string buffer
	dlen		destination string buffer length
	n		signal number

	Returns:

	>=0		number of bytes in result
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	ctdeci(char *,int,int) ;
extern int	ctdecpui(char *,int,int,uint) ;
extern int	ctdecui(char *,int,uint) ;

extern cchar	*strsigabbr(int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int snsigabbr(char *dbuf,int dlen,uint n)
{
	int		rs ;
	const char	*s ;

	if (dbuf == NULL)
	    return SR_FAULT ;

	if ((s = strsigabbr(n)) != NULL) {
	    rs = sncpy1(dbuf,dlen,s) ;
	} else {
	    rs = ctdecui(dbuf,dlen,n) ;
	}

	return rs ;
}
/* end subroutine (snsigabbr) */


