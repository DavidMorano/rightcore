/************************************************************************
*									*
*				partial					*
*									*
*************************************************************************
*									*
*	FUNCTIONAL DESCRIPTION:						*
*	'Partial' performs a partial string comparison.			*
*									*
*	PARAMETERS:							*
*	s	test string						*
*	t	control string						*
*									*
*	RETURNED VALUE:							*
*	1	if each word in s is a partial match of a word in t	*
*	0	otherwise						*
*									*
*	SUBROUTINES CALLED:						*
*	ischar	character matching subroutine				*
*									*
************************************************************************/


#include <string.h>



int partial(s,t)
char *s,*t;
{
	char *st,*ss;


	if( s == NULL || t == NULL ) return(0);

	ss = s;	/* remember start of test string */
	st = t;	/* remember start of control string */

	while (*s != '\0' && *s != '\n') {	/* go through s */

	    if(ischar(*s,";:")) {	/* end of a word in s */

	        t = st;	/* go to start of t */
	        ss = ++s;	/* go to & remember next word in s */
	        continue;
	    }

	    if (*t == '\0' || *t == '\n') {

	        if(*s != '$') return(0);	/* end of t */

	        s++;
	        continue;
	    }

	    if (*t != *s &&
	        (*s != '$' || !ischar(*t,";:"))) {	/* mismatch found */

/* no more words ? */

	        while (!ischar(*t,";:"))
	            if (*t == '\0' || *t++ == '\n') return(0);	

	        t++;		/* have another word in t */
	        s = ss;	/* try word again */
	        continue;
	    }

/* **  so far so good */

	    s++;
	    t++;
	}

	return(1);		/* reaching here means success */
}


