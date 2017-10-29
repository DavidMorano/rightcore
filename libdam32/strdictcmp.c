/* strdictcmp */

/* string compare using dictionary order */


#define	CF_CHAR		1		/* use 'CHAR_XX(3dam)' */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is similar to the 'strcmp(3c)' subroutine except that
        with this subroutine all comparisions are done using "dictionary" order.
        Dictionary order only compares characters that are:

		letters
		digits
	
	Also, upper and lower case mostly ignored except that upper case still
	comes before lower case.

	Synopsis:

	int strdictcmp(s1,s2)
	const char	s1[], s2[] ;

	Arguments:

	s1	one string
	s2	second string

	Returns:

	>0	the first string is bigger than the second
	0	both strings are equal (as compared)
	<0	first string is less than the second


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<char.h>
#include	<localmisc.h>


/* external subroutines */

extern int	strnndictcmp(const char *,int,const char *,int) ;
extern int	dictdiff(int,int) ;
extern int	isdict(int) ;


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


int strdictcmp(cchar *s1,cchar *s2)
{
	int		rc ;

	rc = strnndictcmp(s1,-1,s2,-1) ;

	return rc ;
}
/* end subroutine (strdictcmp) */


