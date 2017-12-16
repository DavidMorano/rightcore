/* snses */

/* string formatting (String-Equal-String) */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine copies a:

	+ string
	+ another string

	into a target counted buffer, with an EQUAL character between
	the the source strings.

	Synopsis:

	int snses(dbuf,dlen,s1,s2)
	char		dbuf[] ;
	int		dlen ;
	const char	s1[], s2[] ;

	Arguments:

	dbuf		destination buffer
	dlen		destination buffer length
	s1		string 1
	s2		string 2

	Returns:

	>=0		length of created string
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<storebuf.h>
#include	<localmisc.h>


/* local defines */

#define	MIDDLECHAR	'='


/* external subroutines */

extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;


/* exported subroutines */


int snses(char *dbuf,int dlen,cchar *s1,cchar *s2)
{
	int		rs = SR_OK ;
	int		i = 0 ;

	if (rs >= 0) {
	    rs = storebuf_strw(dbuf,dlen,i,s1,-1) ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_char(dbuf,dlen,i,MIDDLECHAR) ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_strw(dbuf,dlen,i,s2,-1) ;
	    i += rs ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (snses) */


