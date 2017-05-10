/************************************************************************
*									
*				word					
*									
*									
*	FUNCTIONAL DESCRIPTION:						
*	This subroutine scans a character string for words and		
*	creates a new string containing each word			
*	separated by a single ';'.  A 'word' is defined as		
*       a string of characters separated from other words by		
*	white space (blanks, tabs, control characters, etc) or		
*	a special chacacters: ;  :  -  (  )   or / .			
*	Backslash (\) can be used to escape a single special		
*	character or quotes ( ' or " ) to escape a string.		
*									
*	PARAMETERS:							
*	s	old character string					
*	t	new character string (optional)				
*									
*	RETURNED VALUE:							
*	n	number of words						
*									
*	SUBROUTINES CALLED:						
*	iswhite		check if character is white space		
*	ischar		check if character is in string			
*									

************************************************************************/



#include <stdio.h>

#include <string.h>



int word(s,t)
char	*s,*t;
{
	int n;	/* number of words */
	int esc;	/* =1, if escape special characters */
	int i;	/* set to 1 when first word is encountered */

	char *ss;	/* pointer to replacement string */


	if ( s == NULL ) return(0);

	if( t == NULL ) { 
	    fprintf(stderr, "null pointer passed to word\n");
	    exit();
	}    
	ss = t;		/* save pointer to new string */


	i = n = 0;		/* no words yet */
	esc = 0;		/* no escape yet */

	while(1) {			/* look at whole string */

/*
       ** check for escape sequence
       */

	    if(*s == '\\') {
	        if(*(++s) != '\0') esc += 2;	/* character escape */
	        else s--;
	    }

	    else if(*s == '"' || *s == '\'') {  /* quoted word */
	        if(esc) {	/* end of quote */
	            esc = 0;
	            s++;
	            continue;
	        }
	        else if(*(++s) != '\0') {	/* start of string */
	            esc = 1;
	            i = 1;
	            continue;
	        }
	        else s--;
	    }

/*
       ** check for normal character
       */

	    if(*s != '\0' && (esc ||
	        (!iswhite(*s) && !ischar(*s,":;,/-()") ))) {  /* good char */

	        *ss++ = *s++;	/* save character	*/
	        i = 1;
	        if(esc > 1) esc -= 2;	/* turn off character escape */

	        continue;		/* go on to next character */
	    }

/*
       **  white space detected
       **  replace all adjacent white space with ';'
       **  except if at beginning or end of line
       */

/* get all white */

	    while(*s != '\0' && (iswhite(*s) || ischar(*s,":;,-()/")) ) s++;  

	    if(i == 0 && *s != '\0') continue;  /* skip leading white */

	    if(i == 1) n++;		/* found a word */

	    if(*s == '\0') {		/* end of line */

	        *ss = '\0';
	        return(n);	/* return number of words */

	    } else *ss++ = ';';		/* in the middle */

	}
}


