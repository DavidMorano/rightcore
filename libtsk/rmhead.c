/************************************************************************
*									*
*				rmhead					*
*									*
*************************************************************************
*									*
*	FUNCTIONAL DESCRIPTION:						*
*	'rmhead' removes newlines and white space from the 		*
*	beginning of a character string.				*
*									*
*	PARAMETERS:							*
*	s	old character string					*
*	t	new character string 					*
*									*
*	RETURNED VALUE:							*
*	sr	pointer to modified string				*
*									*
*	SUBROUTINES CALLED:						*
*	iswhite		check for white space				*
*									*
************************************************************************/


#include <string.h>

#include <stdio.h>


char *rmhead(s,t)
char *s,*t;
{
	char *sr;


	if ( s == NULL ) return("");

	if( t == NULL ) {

	    fprintf(stderr, "-rmhead: null pointer passed to rmhead\n");

	    fflush(stdout) ;

	    fflush(stderr) ;

	    exit();

	}
	sr = t;		/* save pointer to new string */

	while(*s != '\0' && iswhite(*s)) s++;  /* go to first nonwhite char */

	while((*t++ = *s++) != '\0') ;	   /* copy to  new string */

	return(sr);
}


