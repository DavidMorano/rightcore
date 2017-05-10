/************************************************************************
*									*
*				split					*
*									*
*************************************************************************
*									*
*	FUNCTIONAL DESCRIPTION:						*
*	'Split' breaks a character string into two parts along a word	*
*	boundary or after a backslash ( \ ).				*
*									*
*	ARGUMENTS:							*
*	s	old character string					*
*	i	length of split						*
*	t	new character string (optional)				*
*	tt	new character string (optional)				*
*									*
*	note: if no optional string is provided, the first		*
*		part of the split string is put back into 's'		*
*		and the overflow part is dropped.			*
*	      if one optional string is provided, the first		*
*		part of the split string is put back into 's'		*
*		and the overflow part is put in 't'.		*
*	      if both optional strings are provided, the first		*
*		part if the split string is put in 't' and		*
*		overflow is put into 'tt'.				*
*									*
*	RETURNED VALUE:							*
*									*
*	SUBROUTINES CALLED:						*
*									*
************************************************************************/



#include <string.h>



void split(s,i,t,tt)
int i;
char *s,*t,*tt;
{
	char *s0,*s1,*ss,*mark;


/* ** check number of arguments to determine where to put results */

	if( t == NULL ) s0 = ""; 

	else s0 = t;

	if( tt == NULL ) s1 = ""; 

	else s1 = tt;

	mark = ss = s0;		/* remember start of new string */
	rmhead(s,s0); /* if s is NULL rmhead will take care of that */

	while(*s0 != '\0') {

	    if(iswhite(*s0) || ischar(*s0,",-(")) mark = s0;

	    if(--i < 0 || *s0 == '\\') {

	        if(*s0 == '\\') {

	            mark = s0;
	            *s0 = ' ';

	        } else if (ischar(*mark,",-")) mark++;

	        if (mark == ss) {

	            sprintf(s1,"%s",s0);

	            *s0++ = '\0';
	            return;
	        }

	        rmhead(mark,s1);

	        *mark = '\0';
	        rmtail(ss);

	        return;
	    }
	    s0++;
	}
	*s1 = '\0';
}


