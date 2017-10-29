/* strnrchr */

/* find a character in a counted string */


#define	CF_STRRCHR	1		/* use 'strrchr(3c)' */


/* revision history:

	= 1999-06-08, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Yes, this is quite the same as 'strrchr(3c)' except that a length of
	the test string (the first argument) can be given.

	Synopsis:

	char *strnrchr(sp,sl,ch)
	const char	*sp ;
	int		sl ;
	int		ch ;

	Arguments:

	sp		pointer to string
	sl		length of string
	ch		character to search for

	Returns:

	address of found character or NULL if not found


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<localmisc.h>


/* local defines */


/* exported subroutines */


char *strnrchr(cchar *sp,int sl,int sch)
{
	char		*rsp = NULL ;

	sch &= 0xff ;
	if (sl < 0) {
#if	CF_STRRCHR
	    rsp = strrchr(sp,sch) ;
#else
	    sl = strlen(sp) ;
#endif /* CF_STRRCHR */
	}

	if (sl >= 0) {
	    int		ch ;
	    int		f = FALSE ;
	    cchar	*csp = (sp+sl) ;
	    while (--csp >= sp) {
	        ch = MKCHAR(*csp) ;
	        f = (ch == sch) ;
	        if (f) break ;
	    } /* end while */
	    rsp = (f) ? ((char *) csp) : NULL ;
	} /* end if */

	return rsp ;
}
/* end subroutine (strnrchr) */


