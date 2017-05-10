/* snwcpylatin */

/* special (excellent) string-copy type of subroutine! */


/* revision history:

	= 1998-07-08, David A­D­ Morano

	This was written to clean up some problems with printing
	garbage charactes in a few place in some PCS utilities.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This is essentially the same as the 'snwcpy(3dam)' subroutine
	except that garbage characters are replaced with soft hyphens.
	
	Synopsis:

	int snwcpylatin(d,dlen,s,slen)
	char		*d ;
	int		dlen ;
	const char	s ;
	int		slen ;

	Arguments:

	d		destination string buffer
	dlen		destination string buffer length
	s		source string
	slen		source string length

	Returns:

	>=0		number of bytes in result
	<0		error


	Notes:

	This subroutine just calls either the 'sncpy1(3dam)' or the
	'strwcpy(3dam)' subroutine based on the arguments.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>

#include	"localmisc.h"



/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	isprintlatin(int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* exported subroutines */


int snwcpylatin(d,dlen,s,slen)
char		d[] ;
int		dlen ;
const char	s[] ;
int		slen ;
{
	uint	ch ;

	int	rs = SR_OK ;
	int	i ;
	int	dl ;

	char	*dp ;


	if ((d == NULL) || (s == NULL))
	    return SR_FAULT ;

	dp = d ;
	dl = dlen ;
	for (i = 0 ; slen && s[i] ; i += 1) {

	    if (dl == 0) {
	        rs = SR_OVERFLOW ;
	        break ;
	    }

	    ch = (s[i] & 0xff) ;
	    *dp++ = (isprintlatin(ch)) ? ch : ('-' + 128) ;
	    dl -= 1 ;

	    slen -= 1 ;

	} /* end for */

	*dp = '\0' ;
	return (rs >= 0) ? i : rs ;
}
/* end subroutine (snwcpylatin) */



