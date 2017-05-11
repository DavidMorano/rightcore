/* strtoken */

/* break a string out into tokens */


/* revision history:

	= 1998-08-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This routine returns a pointer within the given string to the start of
	the next tokenized substring.  This routine is similar to the standard
	UNIX 'strtok' routine except that this one takes additional arguments
	so that the subroutine can be reentrant.

	Synopsis:

	char *strtoken(s,ts,lp)
	char		*s ;
	const char	*ts ;
	int		*lp ;

	Arguments:

	s	input string to break into tokens (same as UNIX version
		except that this can never be NULL)
	ts	string containing token characters (same as UNIX version)
	lp	pointer to an integer for the subroutine to maintain state

	Returns:

	-	pointer to the token string just tokenized!


*******************************************************************************/


/* local defines */

#ifndef	NULL
#define	NULL		0
#endif


/* exported subroutines */


char *strtoken(s,ts,lp)
char		*s ;
const char	*ts ;
int		*lp ;
{
	int		si = *lp ;
	const char	*tp = ts ;

	if (s == NULL) return NULL ;

	if (s[*lp] == '\0') return NULL ;

	while (s[*lp]) {

	    tp = ts ;
	    while (*tp) {
		if (s[*lp] == *tp++) {

	            s[*lp] = '\0' ;
	            (*lp) += 1 ;
	            goto done ;

	        } /* end if */
	    } /* end while */

	    (*lp) += 1 ;

	} /* end while */

done:
	return (s + si) ;
}
/* end subroutine (strtoken) */


