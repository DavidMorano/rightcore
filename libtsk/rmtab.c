/************************************************************************
*									*
*				rmtab					*
*									*
*************************************************************************
*									*
*	FUNCTIONAL DESCRIPTION:						*
*	'Rmtab' replaces TABs in a character string with the		*
*	correct number of spaces.					*
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



#include <string.h>

#include <stdio.h>



char *rmtab(s,t)
char s[],t[];
{
	int i;

	char *sr,*ss;
	char tt[256];


	if ( s == NULL ) return("");

	if ( t == NULL ) {

	    fprintf(stderr, "null pointer passed to rmhead\n");

	    exit();
	}     

/* ** find where to put string */

	sr = t;
	ss = tt;

	i = 0;
	while(*s != '\0') {

	    if(*s == '	') {
	        *ss++ = ' ';

	        while(++i%8 != 0) *ss++ = ' ';

	    } else {

	        *ss++ = *s;
	        i++;
	    }
	    s++;
	}
	*ss = '\0';

	sprintf(sr,"%s",tt);

	return(sr);
}


