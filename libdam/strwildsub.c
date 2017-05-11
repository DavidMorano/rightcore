/* strwildsub */

/* find a substring in a given string w/ wild-card characters */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This was hacked together for some special reason.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is a special version of 'strstr(3c)' that is modified for use in
        old PCS programs. This should not have happened! Rather than making
        something with the same name as an existing standard thing, another name
        should have been picked (which has now been done). This subroutine looks
        like such crap because I did not originate it!

	This subroutine returns a boolean result (TRUE, FALSE).  If the second
	string argument is a substring of the first, we return TRUE, else
	FALSE.  The twist is that we will allow a '?' character to match any
	character.

	Synopsis:

	int strsub(str1,str2)		
	const char	*str1, *str2 ;		

	Arguments:

	str1		string to search through
	str2		string to search for in the first string above

	Returns:

	TRUE		if 'str2' was found to be a substring of 'str1'
	FALSE		'str2' was not a substring of 'str1'


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


/* exported subroutines */


int strwildsub(cchar *str1,cchar *str2)
{
	int		wild_cards = 0 ;
	int		s2 ;
	const char	*p1, *p2 ;
	const char	*last1 ;

	for (p1 = str1 ; *p1++ != '\0' ; ) ;

	p1 -= 1 ;				/* p1 at end of str1 */
	p2 = str2 ;
	do {

	    s2 = *(p2++) ;

/* set flag if wild cards in str2 */

	    if (s2 == '?') 
		wild_cards = 1 ;

	} while (s2 != '\0') ;

	p2 -= 1 ;				/* p2 at end of str2 */
	while (p2 > str2 && *p2 == '?') 
	    p2 -= 1 ;		/* trim trailing '?s' from str2 */

	last1 = p1-(p2-str2)+1 ;
	if (wild_cards) {

	    for (p1 = str1; p1 < last1; p1= ++str1) {

	        p2 = str2 ;
	        for (;;) {

	            if ((s2 = *p2++) == '\0') 
			return TRUE ;

	            if (s2 == *p1++ || s2 == '?' ) 
			continue ;

	            break ;
	        }

	    } /* end for */

	} else {

	    for (p1 = str1; p1 < last1; p1= ++str1) {

	        p2 = str2 ;
	        for (;;) {

	            if ((s2 = *p2++) == '\0') 
			return TRUE ;

	            if (s2 == *p1++) 
			continue ;

	            break ;
	        }

	    } /* end for */

	} /* end if */

	return FALSE ;
}
/* end subroutine (strwildsub) */


