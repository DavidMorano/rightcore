/* strnchr */

/* find a character in a counted string */


#define	CF_STRCHR	1		/* use 'strchr(3c)' */


/* revision history:

	= 1999-06-08, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Yes, this is quite the same as 'strchr(3c)' except that a length of the
	test string (the first argument) can be given.

	Synopsis:

	char *strnchr(sp,sl,ch)
	const char	sp[] ;
	int		sl ;
	int		ch ;

	Arguments:

	sp		string to search through
	sl		maximum number of character to search
	ch		the character to search for

	Returns:

	NULL		did not find the character
	!= NULL		address of found character in searched string


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<localmisc.h>


/* exported subroutines */


char *strnchr(cchar *sp,int sl,int sch)
{
	register int	f = FALSE ;
	register cchar	*lsp ;
	char		*rsp ;

	sch &= 0xff ;
	if (sl < 0) {

#if	CF_STRCHR
	    rsp = strchr(sp,sch) ;
#else
	    while (*sp) {
	        f = ((*sp & 0xff) == sch) ;
		if (f) break ;
	        sp += 1 ;
	    } /* end while */
	    rsp = (f) ? ((char *) sp) : NULL ;
#endif /* CF_STRCHR */

	} else {

	    lsp = (sp + sl) ;
	    while ((sp < lsp) && *sp) {
	        f = ((*sp & 0xff) == sch) ;
		if (f) break ;
	        sp += 1 ;
	    } /* end while */
	    rsp = (f) ? ((char *) sp) : NULL ;

	} /* end if */

	return rsp ;
}
/* end subroutine (strnchr) */


