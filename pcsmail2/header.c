/* header */


#define	CF_DEBUG	0


/************************************************************************
 *									
 * The information contained herein is for the use of AT&T Information	
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	
 *									
 *	(c) 1984 AT&T Information Systems				
 *									
 * Authors of the contents of this file:				

	J.Mukerji						
	David A.D. Morano
	- rewritten 01/06/94


	The subroutine 'header_line' checks the first word in a line to
	see if the line is a header line.  If not, it returns (-1).  If
	it is, it returns the header type as its function value and
	puts a pointer to the string value of the header returned in
	the global string pointer 'hvalue' which is defined in
	'header.h'.


**************************************************************************/



#include	<sys/types.h>
#include	<string.h>
#include	<ctype.h>
#include	<stdio.h>

#include	<logfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"
#include	"header.h"



/* the super fast header scanner ! */

int header_line(line)
char	*line ;
{
	int	i ;

	char	*c, *d, **e ;


	i = 0 ;
	e = headers ;
	while (*e != NULL) {

	    c = line ;
	    if (LOWER(*c) == LOWER(**e)) {

/* try matching the rest of it	    */

	        d = *e + 1 ; 
		c += 1 ;
	        while (*d) {

	            if (LOWER(*d) != LOWER(*c)) goto nogood ;

		    d += 1 ; 
		    c += 1 ;
	        }

/* skip over any white space */

	        while (ISWHITE(*c)) c += 1 ;

	        hvalue = c ;

/* replace '\n' by '\0' */

	        while (*c) if (*c++ == '\n') {

	            c[-1] = '\0' ;
	            break ;
	        }

	        return i ;

nogood:
		;
	    }

	    e += 1 ;
	    i += 1 ;

	} /* end while */

	return -1 ;
}
/* end wubroutine (header_line) */


