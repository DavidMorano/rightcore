/* snsds */

/* string formatting (String-Dot-String) */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine copies a string and a second another string into a
	target counted buffer, with an DOT character between the the source
	strings.

	Synopsis:

	int snsds(dbuf,dlen,s1,s2)
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


	Implementation (and usage) note:

	Is this really (really?) faster than using 'sncpy3(3dam)'?

		sncpy3(dbug,dlen,s1,".",s2) ;

	Really?


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<storebuf.h>
#include	<localmisc.h>


/* local defines */

#define	MIDDLECHAR	'.'


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;


/* exported subroutines */


int snsds(char *dbuf,int dlen,cchar *s1,cchar *s2)
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
/* end subroutine (snsds) */


