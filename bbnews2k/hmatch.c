/* hmatch */

/* header key match */


/* revision history:

	= 1995-04-01, David A­D­ Morano
	This is part of our cleanup-compatibility effort.

	= 1998-11-22, David A­D­ Morano
        I did some clean-up.

*/

/* Copyright © 1995,1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Is the initial substring of 'buf' the specified 'header' string? Return
        0 if there is no match, else we return the character position of the
        header value string. The match is case independent.

	Arguments:

	- header	header string that we are looking for
	- value		string value of result if matched correctly

	Returns:

	>0		index to first non-white-space value character
	0		no header match


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<char.h>
#include	<localmisc.h>


/* exported subroutines */


int hmatch(cchar *header,cchar *buf)
{
	const char	*hp = header ;
	const char	*bp = buf ;
	int		rc = 0 ;
	int		f ;

	while (*hp && (*hp != ':')) {
	    f = (CHAR_TOLC(*bp) != CHAR_TOLC(*hp)) ;
	    if (f) break ;
	    bp += 1 ;
	    hp += 1 ;
	} /* end while */

	if (! f) {
	    while (CHAR_ISWHITE(*bp)) bp += 1 ;
	    if (*bp++ == ':')  {
	        while (CHAR_ISWHITE(*bp)) bp += 1 ;
	        rc = (bp - buf) ;
	    }
	}

	return rc ;
}
/* end subroutine (hmatch) */


