/* strnrpbrk */

/* find a character in a counted string */


/* revision history:

	= 1999-06-08, David A­D­ Morano
	This subroutine was originally written for some reason.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is similar to the standard 'strnpbrk(3c)' except that a
	pointer to the last character matching a character in the break-string
	is returned (instead of a pointer to the first character in the
	break-string).

	Synopsis:

	char *strnrpbrk(sp,sl,ss)
	const char	*sp ;
	int		sl ;
	const char	ss[] ;

	Arguments:

	sp		pointer to string to test
	sl		length of string to test
	ss		search-string

	Returns:

	address of found character or NULL if not found


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<localmisc.h>


/* local defines */


/* exported subroutines */


char *strnrpbrk(cchar *sp,int sl,cchar *ss)
{
	int		ch ;
	int		f = FALSE ;
	char		*rsp ;

	if (sl < 0)
	    sl = strlen(sp) ;

	rsp = (char *) (sp + sl) ;
	while (--rsp >= sp) {
	    ch = MKCHAR(*rsp) ;
	    f = (strchr(ss,ch) != NULL) ;
	    if (f) break ;
	} /* end while */

	return (f) ? rsp : NULL ;
}
/* end subroutine (strnrpbrk) */


