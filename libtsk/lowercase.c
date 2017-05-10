/************************************************************************
*									*
*				lowercase				*
*									*
*************************************************************************
*									*
*	FUNCTIONAL DESCRIPTION:						*
*	"Lowercase" converts a character string to lower case		*
*									*
*	PARAMETERS:							*
*	s	old character string					*
*	t	new character string (optional)				*
*									*
*	RETURNED VALUE:							*
*	sr	pointer to modified character string			*
*									*
*	SUBROUTINES CALLED:						*
*									*
************************************************************************/



#include <stdio.h>



char *lowercase(s,t)
char *s,*t;
{
	char *sr;


	sr = t;		/* save pointer of new string */

	if( s == NULL || t == NULL ) return("");

	while(*s != '\0') {

	    if (isupper(*s)) *t = tolower(*s);

	    else *t = *s;

	    s++;
	    t++;
	}
	*t = '\0';
	return(sr);
}


