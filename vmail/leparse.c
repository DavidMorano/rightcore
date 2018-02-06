/* leparse */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<signal.h>
#include	<string.h>
#include	<time.h>

#include	<bfile.h>

#include	"config.h"
#include	"defs.h"


/*******************************************************************************

 parses the specified logical expression, placing the tokens into
  global variables for use by "search" as follows:
  "isop" denotes whether a token is an operator or a HEADER:value.
  Each token is identified by a number in "etoken" which gives
  the operator number (as referenced into "operator") or 
  the  number (as referenced into "").
  Headers have an associated value which is pointed to by "hvalpt"
  (the actual string space is in "hvalues").
  "numletok" is the number of tokens (max bound of these arrays).

  Returns 0 if successful, 1 if error.


*******************************************************************************/


/* exported subroutines */


int leparse(exp)
char	exp[] ;
{
	int k ;

	char *valpt, *ep, headstr[50] ;


	valpt = hvalues ;	/* start of next value in string space */
	numletok = 0 ;
	ep = exp ;		/* current parsing point */

	while (*ep != '\0') {

/* found operator */

	    if ((k = findoperator(*ep))  >  -1) {

	        isop [numletok]  =  1 ;
	        etoken [numletok]  =  k ;
	        numletok++ ;
	        ep++ ;
	    }

	    if (findoperator(*ep) > -1)
	        continue; 	/* two operators in row, eg &( */
	    if (*ep == '\0')
	        break ;		/* end with operator, eg ) */

/*  build */
	    k=0 ;
	    while (*ep != ':') {

	        if ((findoperator(*ep) > -1) || (*ep == '\0'))
	            return 1 ;

	        headstr[k++] =  *ep++ ;
	    }
	    headstr[k++] = ':' ;
	    headstr[k] = '\0' ;

	    isop [numletok]  =  0 ;
	    etoken [numletok]  =  find (headstr) ;

	    if (etoken[numletok] == -1) {

	        return 1 ;
	    }
	    ep++ ;

	    if ((findoperator(*ep) > -1) || (*ep == '\0')) {

	        return 1 ;
	    }

/* value build */
	    hvalpt [numletok]  =  valpt ;
	    while ((findoperator(*ep) == -1) && (*ep != '\0'))
	        *valpt++ = *ep++ ;

	    *valpt++ = '\0' ;	/* complete & pt to next */
	    numletok++ ;
	}
	return(0) ;
}
/* end subroutine (leparse) */


/* finds operator number of character.   returns -1 if not an operator */

int findoperator(ch)
int	ch ;
{
	int	i ;


	for (i = 0 ; i < (int) strlen(operator) ; i += 1) {

	    if (ch == operator[i])
	        return i ;
	}

	return -1 ;
}
/* end subroutine */


/* finds  number of string.  returns -1 if not a .
  match allows case folding (eg  from: matches FROM:).
  match done on initial substring of str (so "FROM:schatz" matches "FROM:").
*/

int find(str)
char	str[] ;
{
	int k ;

	char headonly[LINEBUFLEN] ;


/* extract leading substring */

	for (k=0;  str[k] != ':';  k++) {

	    if (str[k] == '\0')
		return(-1);      /* no ':' */

	    headonly[k] = str[k] ;

	} /* end for */

	headonly[k] = ':' ;
	headonly[++k] = '\0' ;
	k=0 ;

/* find  */

	while ((int) strlen(header[k]) > 0) {	

	    if (strcasecmp(header[k],headonly))
	        return k ;

	    k += 1 ;
	}

	return -1 ;
}
/* end subroutine (find) */



