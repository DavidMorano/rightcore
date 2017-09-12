/* sicasechr */

/* subroutine to find the index of a character in a given string */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_CHAR		1		/* use 'char(3dam)' */


/* revision history:

	= 1998-03-23, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine searches for a character within a given string and
	returns the index to that character (if it is found).  It returns (-1)
	 if the character does not exist within the given string.

	Synopsis:

	int sicasechr(sp,sl,sch)
	const char	sp[] ;
	int		sl ;
	int		sch ;

	Arguments:

	sp	string to be examined
	sl	length of string of break character to break on
	sch	character to search for

	Returns:

	>=0	index of search character in the given string
	<0	the search character was not found


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<char.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

#if	CF_CHAR
#define		tolc(c)		CHAR_TOLC(c)
#define		touc(c)		CHAR_TOUC(c)
#else /* CF_CHAR */
extern int	tolc(int) ;
extern int	touc(int) ;
#endif /* CF_CHAR */

extern int	nleadcasestr(cchar *,cchar *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif /* CF_DEBUGS */


/* exported subroutines */


int sicasechr(const char *sp,int sl,int sch)
{
	int		i ;
	int		ch ;
	int		f = FALSE ;

	sch = tolc(sch) ;
	for (i = 0 ; sl-- && sp[i] ; i += 1) {
	    ch = tolc(sp[i]) ;
	    f = (ch == sch) ;
	    if (f) break ;
	} /* end for */

	return (f) ? i : -1 ;
}
/* end subroutine (sicasechr) */


