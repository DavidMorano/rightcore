/* strkeydictcmp */

/* string key comparison */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-09-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine makes a comparison of the key of a string that looks
	like a SHELL variable assignment.

	For example, consider the following compound string:

		A=the_dog_house

	The 'A' would be the key, and the part 'the_dog_house' is
	the value.

	Synopsis:

	int strkeydictcmp(s1,s2)
	const char	*s1, *s2 ;

	Arguments:

	s1		first string
	s2		second string

	Returns:

	>0		the second key is greater than the first
	0		the keys of the strings are equal
	<0		the first key is less than the second


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	dictdiff(int,int) ;
extern int	isdict(int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int strkeydictcmp(cchar *s1,cchar *s2)
{
	int		rc = 0 ;

	if ((s1 == NULL) && (s2 == NULL))
	    return 0 ;

	if (s1 == NULL)
	    return 1 ;

	if (s2 == NULL)
	    return -1 ;

	while (*s1 && *s2) {

	    if ((*s1 == '=') || (*s2 == '='))
	        break ;

	    if (! isdict(*s1)) {
	        s1 += 1 ;
	        continue ;
	    }

	    if (! isdict(*s2)) {
	        s2 += 1 ;
	        continue ;
	    }

	    rc = dictdiff(*s1,*s2) ;
	    if (rc != 0) break ;

	    s1 += 1 ;
	    s2 += 1 ;

	} /* end while */

	if (rc == *s2)
	    rc = 0 ;

	if (*s1 == '=')
	    rc = (*s2 == '\0') ? 0 : -1 ;

	if (*s2 == '=')
	    rc = (*s1 == '\0') ? 0 : 1 ;

	return rc ;
}
/* end subroutine (strkeydictcmp) */


