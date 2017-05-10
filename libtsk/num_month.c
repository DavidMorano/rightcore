/************************************************************************
*									*
*				num_month				*
*									*
*************************************************************************
*									*
*	FUNCTIONAL DESCRIPTION:						*
*	'Num_month' returns the number of a month (eg, jan = 1).	*
*									*
*	PARAMETERS:							*
*	month	name of month						*
*									*
*	RETURNED VALUE:							*
*	n	number of month (January = 1)				*
*									*
*	SUBROUTINES CALLED:						*
*									*
************************************************************************/



#include <stdio.h>

#include <ctype.h>



int num_month(monthin)
char *monthin;
{
	int i;

	char m[256];
	char *month;


	if ( monthin == NULL ) return(0);

/* convert month to lowercase */

	i = 0;
	while( (m[i] = *monthin++) != '\0') {

	    if (isupper(m[i])) m[i] = tolower( m[i] );

	    i++;
	}

	if (strncmp(m,"january",i) == 0) return(1);

	if (strncmp(m,"february",i) == 0) return(2);

	if (strncmp(m,"march",i) == 0) return(3);

	if (strncmp(m,"april",i) == 0) return(4);

	if (strncmp(m,"may",i) == 0) return(5);

	if (strncmp(m,"june",i) == 0) return(6);

	if (strncmp(m,"july",i) == 0) return(7);

	if (strncmp(m,"august",i) == 0) return(8);

	if (strncmp(m,"september",i) == 0) return(9);

	if (strncmp(m,"october",i) == 0) return(10);

	if (strncmp(m,"november",i) == 0) return(11);

	if (strncmp(m,"december",i) == 0) return(12);

	return 0 ;
}


