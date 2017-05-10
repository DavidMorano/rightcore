/************************************************************************
*									
*				match					
*									
*									
*	FUNCTIONAL DESCRIPTION:						
*	'Match' compares two strings for an exact match.		
*									
*	PARAMETERS:							
*	s	first character string					
*	t	second character string					
*									
*	RETURNED VALUE:							
*									
*	SUBROUTINES CALLED:						
*									
*									
************************************************************************/



#include <string.h>




int match(s,t)
char *s,*t;
{


	if (s == NULL || t == NULL) {

	    if (s == NULL && t == NULL ) return(1);

	    else return(0);
	}

	while(1) {

	    if (*s != *t++) return(0);	/* no match */

	    if (*s++ == '\0') return(1);	/* match found */

	}
}


