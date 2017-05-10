/************************************************************************
*									*
*				rmcntrl					*
*									*
*************************************************************************
*									*
*	FUNCTIONAL DESCRIPTION:						*
*	'rmcntrl' replaces control characters (value < 040)		*
*	with the sequence '^*' where * is an printable character.	*
*									*
*	PARAMETERS:							*
*	s	old character string					*
*	t	new character string 					*
*									*
*	RETURNED VALUE:							*
*	t	pointer to modified string				*
*									*
*	SUBROUTINES CALLED:						*
*									*
************************************************************************/



#include <string.h>

#include <stdio.h>



char *rmcntrl(s,t)
char *s,*t;
{
	int i;

	char tt[256];


	if( s == NULL ) return("");

	if( t == NULL ) {

	    fprintf(stderr, "null pointer passed to rmcntrl\n");

	    fflsuh(stdout) ;

	    fflsuh(stderr) ;

	    exit();
	}
	i = 0;
	while((tt[i] = *s++) != '\0') {

	    if(tt[i] < 040) {

	        tt[i+1] = tt[i] + 0100;
	        tt[i++] = '^';
	    }
	    i++;
	}
	strcpy(t,tt);

	return(t);
}


