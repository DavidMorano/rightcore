/* strrpbrk */

/* find a character in a counted string */


/* revision history:

	= 1999-06-08, David A­D­ Morano
	This subroutine was originally written for some reason.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is, of course, like the standard 'strpbrk(3c)' except
	that length of the string to be tested can be given.  If a test string
	length of <0 is given, then this subroutine acts just like
	'strpbrk(3c)'.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<localmisc.h>


/* local defines */


/* exported subroutines */


char *strrpbrk(cchar *s,cchar *ss)
{
	int		ch ;
	int		n = strlen(s) ;
	int		f = FALSE ;
	char		*rsp ;

	rsp = (char *) (s + n) ;
	while (--rsp >= s) {
	    ch = (*rsp & 0xff) ;
	    f = (strchr(ss,ch) != NULL) ;
	    if (f) break ;
	} /* end while */

	return (f) ? rsp : NULL ;
}
/* end subroutine (strrpbrk) */


