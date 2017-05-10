/************************************************************************
*									*
*				convdt					*
*									*
*************************************************************************
*									*
*	FUNCTIONAL DESCRIPTION:						*
*	'Convdt' converts a character string of the form YYMMDD,	*
*	where YY = year, MM = month, and DD = day, to a Julian date	*
*									*
*	PARAMETERS:							*
*	s	old character string					*
*	i	Type of date:	 0 = 30JAN79 (default)			*
*				1 = 1/30/79				*
*				2 = 1-30-79				*
*				3 = January 30, 1979			*
*				4 = 30 january 1979			*
*	t	new character string 					*
*									*
*	RETURNED VALUE:							*
*	sr	pointer modified string					*
*									*
*	SUBROUTINES CALLED:						*
*									*
************************************************************************/



#include <string.h>

#include <stdio.h>



char *convdt(s,i,t)
int i;
char *s,*t;
{
	int mo,dy,yr;

	char month[10];
	char *sr;


	sr = t;			/* save pointer to new string */
	if( s == NULL || t == NULL ) return("");

	mo = dy = yr = 0;
	sscanf(s,"%2d%2d%2d",&yr,&mo,&dy);	/* get month, year, day */

/* **  check for good data ; if bad, return null string */

	*t = '\0';

	if(mo <= 0 || mo > 12
	    || yr <= 0
	    || dy <= 0 || dy > 31) {
	    return(sr);
	}

	if (i == 1) {	/* convert to '12/1/79' */

	    sprintf(t,"%d/%d/%d",mo,dy,yr);

	    return(sr);
	}

	if (i == 2) {	/* convert to '12-1-79' */

	    sprintf(t,"%d-%d-%d",mo,dy,yr);

	    return(sr);
	}

	if (i == 3) {	/* convert to 'December 12, 1979' */

	    get_month(mo,month);

	    sprintf(t,"%s %d, 19%d",month,dy,yr);

	    *t = toupper(*t);

	    return(sr);
	}

	if (i == 4) {	/* convert to '12 december 79' */

	    get_month(mo,month);

	    sprintf(t,"%d %s %d",dy,month,yr);

	    return(sr);
	}

/* convert to '01DEC79' (default) */

	get_month(mo,month);

	sprintf(t,"%2d%3.3s%2d",dy,month,yr);

	while (*t != '\0') {

	    if (*t == ' ') *t = '0';

	    if (islower(*t)) *t = toupper(*t);

	    t++;
	}
	return(sr);
}


