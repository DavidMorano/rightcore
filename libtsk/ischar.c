/************************************************************************
*									*
*				ischar					*
*									*
*************************************************************************
*									*
*	FUNCTIONAL DESCRIPTION:						*
*	Check for character match.					*
*									*
*	PARAMETERS:							*
*	c	character						*
*	s	string							*
*									*
*	RETURNED VALUE:							*
*	1 	if 'c' is in 's'					*
*	0	otherwise						*
*									*
*	SUBROUTINES CALLED:						*
*									*
************************************************************************/


#define NULL (char *)0


int ischar(c,s)
char c,*s;
{

	if( s == NULL ) return(0);

	while(*s != '\0') if(c == *s++) return(1);

	return(0);
}


