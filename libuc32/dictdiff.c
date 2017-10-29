/* dictdiff */

/* compare two dictionary characters (like for a dictionary) */


#define	CF_CHAR		1		/* use 'CHAR_XX(3dam)' */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine essentially compares two dictionary characters.

	Synopsis:

	int dictdiff(ch1,ch2)
	int	ch1, ch2 ;

	Arguments:

	ch1	one character
	ch2	another character

	Returns:

	>0	the first dictionary character is bigger than the second
	0	both dictionary characters are equal
	<0	the first dictionary character is less than the second


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<char.h>
#include	<localmisc.h>


/* external subroutines */


/* external variables */

#if	CF_CHAR
#define	tolc(c)		CHAR_TOLC(c)
#define	touc(c)		CHAR_TOUC(c)
#else /* CF_CHAR */
extern int	tolc(int) ;
extern int	touc(int) ;
#endif /* CF_CHAR */


/* local structures */


/* forward references */


/* exported subroutines */


int dictdiff(int ch1,int ch2)
{
	int		rc ;

	ch1 &= 0xff ;
	ch2 &= 0xff ;
	rc = CHAR_TOFC(ch1) - CHAR_TOFC(ch2) ;
	if (rc == 0)
	    rc = (ch1 - ch2) ;

	return rc ;
}
/* end subroutine (dictdiff) */


