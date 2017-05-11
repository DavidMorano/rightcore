/* substring */

/* routine to match a substring within a larger string */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	Description:

        This subroutine determines if the parameter string (argument 's2') is or
        is not in the buffer specified by the first two arguments. This
        subroutine either returns (-1) or it returns the character position in
        the buffer of where the string starts.

	Synopsis:

	int substring(s1,len,s2)
	const char	*s1, *s2 ;
	int		len ;

	Arguments:

	s1		string to be examined
	len		length of string to be examined
	s2		null terminated substring to search for

	Returns:

	<0		substring was not found in main string
	>=0		substring was found in the main string at index


******************************************************************************/ 


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<string.h>
#include	<localmisc.h>


/* external subroutines */

extern int	sisub(const char *,int,const char *) ;


/* exported subroutines */


int substring(cchar *sp,int sl,cchar *ss)
{
	return sisub(sp,sl,ss) ;
}
/* end subroutine (substring) */


