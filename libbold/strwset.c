/* strwset */

/* set a string to a number of characters */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Copy a specified character to a destination for the number of
	characters specified.  The resulting string is NUL-terminated.

	Synopsis:

	char *strwset(s1,ch,w)
	char		s1[] ;
	int		ch ;
	int		w ;

	Arguments:

	s1	string buffer that receives the copy
	ch	character to set in the destination string
	w	the maximum length to be copied

	Returns:

	-	the character pointer to the end of the destination


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


/* local defines */


/* exported subroutines */


char *strwset(char *dp,int ch,int w)
{
	while (w-- > 0) *dp++ = ch ;
	*dp = '\0' ;
	return dp ;
}
/* end subroutine (strwset) */


