/* sfdequote */

/* find the dequoted sub-string of the given string */


#define	CF_DEBUGS	0		/* switchable debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	The subroutine was written from scratch.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine strips leading and trailing double quote characters from
        the given string. It also removes leading and trailing white space (as
        defined by 'sfshrink()').

	Synopsis:

	int sfdequote(sp,sl,rpp)
	const char	sp[] ;
	int		sl ;
	const char	**rpp ;

	Arguments:

	sp		string to strip
	sl		length of string to strip
	rpp		pointer to pointer to resulting stripped string

	Returns:

	-		length of resulting stripped string


*******************************************************************************/


#include	<envstandards.h>	/* must be before others */
#include	<sys/types.h>
#include	<string.h>
#include	<char.h>
#include	<ascii.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sfshrink(const char *,int,const char **) ;


/* external variables */


/* forward references */

static int	isours(int) ;


/* local variables */


/* exported subroutines */


int sfdequote(cchar *sp,int sl,cchar **rpp)
{
	if (sl < 0) sl = strlen(sp) ;
	while (sl && isours(sp[0])) {
	    sp += 1 ;
	    sl -= 1 ;
	}
	if (sp[0] == '\0') sl = 0 ;
	if (rpp != NULL) *rpp = sp ;
	while (sl && isours(sp[sl-1])) {
	    sl -= 1 ;
	}
	return sl ;
}
/* end subroutine (sfdequote) */


/* local subroutines */


static int isours(int ch) /* integer promotion of 8-bit chars does not matter */
{
	return (CHAR_ISWHITE(ch) || (ch == CH_QUOTE)) ;
}
/* end subroutine (isours) */


