/************************************************************************
*									*
*				rmtail					*
*									*
*************************************************************************
*									*
*	FUNCTIONAL DESCRIPTION:						*
*	'Rmtail' removes newlines and white space at the end of		*
*	a character string.						*
*									*
*	PARAMETERS:							*
*	s	old character string					*
*	t	new character string (optional)				*
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



char *rmtail(s)
char *s;
{
	char *sr;


	if ( s == NULL ) return("");

	sr = s;		/* save pointer to new string */

	s += strlen(s);

	while (s >= sr && iswhite( *s )) *s-- = '\0'; /* remove white */

	return(sr);
}


