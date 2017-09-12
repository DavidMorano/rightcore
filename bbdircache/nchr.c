/* nchr */

/* count number of characters in a string */


#define	CF_STRNCHR	0		/* use |strnchr(3dam)| */


/* revision history:

	= 1998-10-10, David A­D­ Morano
        This subroutine was originally written but modeled from assembly
        language.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine counts the occurences of a character in a string.

	Synopsis:

	int nchr(sp,sl,sch)
	const char	*sp ;
	int		sl ;
	int		sch ;

	Arguments:

	sp	string to test
	sl	length of string to test
	sch	character to search for

	Returns:

	-	number of occurences of the character in the string


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<strings.h>

#include	<char.h>
#include	<localmisc.h>


/* external subroutines */

extern char	*strnchr(const char *,int,int) ;


/* exported subroutines */


#if	CF_STRNCHR
int nchr(cchar *sp,int sl,int sch)
{
	int		n = 0 ;
	cchar		*tp ;

	if (sl < 0) sl = strlen(sp) ;

	while ((tp = strnchr(sp,sl,sch)) != NULL) {
	    n += 1 ;
	    sl -= ((tp+1)-sp) ;
	    sp = (tp+1) ;
	} /* end while */

	return n ;
}
/* end subroutine (nchr) */
#else /* CF_STRNCHR */
int nchr(cchar *sp,int sl,int sch)
{
	int		ch ;
	int		n = 0 ;

	while (sl-- && sp[0]) {
	   ch = MKCHAR(sp[0]) ;
	   if (ch == sch) n += 1 ;
	   sp += 1 ;
	}

	return n ;
}
/* end subroutine (nchr) */
#endif /* CF_STRNCHR */


