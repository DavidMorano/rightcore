/************************************************************************
*									*
*				undate					*
*									*
*************************************************************************
*									*
*	FUNCTIONAL DESCRIPTION:						*
*	'Undate' converts a normal Julian date to a character string	*
*	composed of the digits YYMMDD, where YY = year, MM = month	*
*	and DD = day.	The old string may be in the form of:		*

		30JAN79
		1/30/79
		1-30-79
		January 30,1979
		30 january 1979						

*									*
*	PARAMETERS:							*
*	s	old character string					*
*	t	new character string (optional)				*
*									*
*	RETURNED VALUE:							*
*	sr	pointer to YYMMDD string 				*
*									*
*	SUBROUTINES CALLED:						*
*									*
************************************************************************/



#include <string.h>

#include <stdio.h>



char *undate(s,t)
char *s,*t;
{
	int mo,dy,yr;
	int number[4];	/* numbers in old string */
	int i,j;

	char alpha[10];	/* alpha chars in old string */
	char *sr;


#ifdef	COMMENT
     if ( t == NULL ) sr = "";

     else {*t = '\0'; sr = t;}
#endif

	if ( t == NULL ) sr = "";

	else sr = t;

/* ** find numbers and month string */

	i = j = 0;	/* no numbers or chars yet */
	number[0] = number[1] = number[2] = 0;
	while (*s != '\0') {

	    if (isdigit( *s ) && j < 4) {	/* found start of number */

	        while (*s == '0') s++;	/* skip leading zeroes */

	        sscanf(s,"%d",&number[j++]);	/* get number */

	        while (isdigit( *s )) s++;	/* go to end of digit */

	        continue;

	    } else if (isalpha( *s )) {		/* found a char */

	        alpha[i++] = *s++;
	        continue;
	    }
	    s++;
	}
	alpha[i] = '\0';

/* ** check date for right format */

	if ((alpha[0] == '\0' && j != 3)
	    || (alpha[0] != '\0' && j != 2)) {

	    return(sr);
	}

/*
     ** year is always last number
     */

	yr = number[j-1];
	if (yr >= 1900) yr -= 1900;

	if (yr < 1 || yr > 99) {	/* bad year */
	    return(sr);
	}

/*
     ** day is first or second number
     */

	if (alpha[0] == '\0') dy = number[1];

	else dy = number[0];

	if (dy < 1 || dy > 31) {	/* bad day */
	    return(sr);
	}

/*
     ** month is either a string or first number
     */

	if (alpha[0] == '\0') mo = number[0];

	else mo = num_month(alpha);

	if (mo < 1 || mo > 12) {	/* bad month */
	    return(sr);
	}

	if ( t == NULL ) return(sr);

	sprintf(t,"%2d%2d%2d",yr,mo,dy);

	while (*t != '\0') {

	    if (*t == ' ') *t = '0';

	    t++;
	}
	return(sr);
}


