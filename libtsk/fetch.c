/************************************************************************
									
*				fetch					
*									
*									
*	FUNCTIONAL DESCRIPTION:						
*	"fetch" gets the i-th word in a character string created	
*	by 'word' (ie, words separated by ';' or ':')			
*									
*	PARAMETERS:							
*	s	old character string					
*	i	number of word	 					
*	t	new character string					
*									
*	RETURNED VALUE:							
*	sr	pointer to retrieved word				
*									
*	SUBROUTINES CALLED:						
*									

************************************************************************/



#include <string.h>




char *fetch(s,i,t)
char *s,*t;
int i;
{
	char *sr;


	sr = t;		/* save pointer to start of new string */
	if( s == NULL || t == NULL ) return(NULL);

/* find right word */

	while (i > 0) {

	    if (*s == '\0') break;

	    if (ischar(*s++,";:")) i--;

	}

	while((*t = *s++) != '\0' && *t != '\n') {

	    if(ischar(*t,":;")) break;	/* stop at next word */

	    t++;
	}

	*t = '\0';	/* make sure null is at end */
	return(sr);
}


