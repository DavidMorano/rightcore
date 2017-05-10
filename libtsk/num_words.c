/************************************************************************
*									*
*				num_words				*
*									*
*************************************************************************
*									*
*	FUNCTIONAL DESCRIPTION:						*
*	'Num_words' counts the number of words in a string.		*
*									*
*	ARGUMENTS:							*
*	s	character string					*
*									*
*	RETURNED VALUE:							*
*	i	number of words						*
*									*
*	SUBROUTINES CALLED:						*
*	ischar	test for character					*
*									*
************************************************************************/



#include <string.h>



int num_words(s)
char *s;
{
	int i;


	if( s == NULL || *s == '\0') return(0);

	i = 1;
	while(*s != '\0') {

	    if(ischar(*s++,":;")) i++;

	}
	return(i);
}

