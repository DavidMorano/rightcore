/* getnames */


#define	CF_DEBUG	0


/************************************************************************
 *									
 * The information contained herein is for the use of AT&T Information	
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	
 *									
 *	(c) 1984 AT&T Information Systems				
 *									
 * Authors of the contents of this file:				
 *									
 *		J.Mukerji						
		David A.D. Morano
 *									

*									
*	FUNCTIONAL DESCRIPTION:						
*	'Getnames' converts a string into an array of names.		
*									
*	PARAMETERS:							
*	recipient	string of recipients				
*									
*	RETURNED VALUE:							
*	number of recipients						
*									
*	SUBROUTINES CALLED:						
*	char *parsename(string)						
*									
*	DESCRIPTION:							
*	getnames is given a string of names separated by commas.
*	It breaks that string into individual names and passes each	
*	of them to parsename. 

	Parsename deals with the internal 	
*	structure of the name. It eventually puts the name together in	
* 	a canonical form and returns it as the function value.
*									
*	NAME STRUCTURE:							
*									
*	<recipient-list> ::= <recipient>|<recipient>','<recipient-list>	
*	<recipient> ::= <name>|<name>' ''<'<address>'>'|<address>	
*	<name> ::= <string of characters not conataining ','>		
*	<address> ::= <string of characters not containing ','>		
*									

************************************************************************/



#include	<sys/types.h>
#include	<stdio.h>
#include	<string.h>

#include	<baops.h>
#include	<logfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"




#define	HFLEN	62		/* header maximum line length for folding */



#define openstring(ptr, str)	ptr = str	/* char *ptr, *str */

#define addstring(ptr,strn)	for( ; *ptr++ = *strn++; ); ptr--

#define closestring(ptr)	*ptr = '\0'



/* external variables */

extern struct global		g ;


/* forward references */

char	*parsename() ;



int getnames(recipient)
char	*recipient ;
{
	int	slen, l, i ;

	char *c, *d, *e ;
	char s[2*BUFSIZE] ;
	char *separator ;
	char	*cp ;


#if	CF_DEBUG
	if (g.debuglevel > 0)
	logfile_printf(&g.eh,"getnames: called w/ \"%s\"\n",recipient) ;
#endif

	if (recipient == NULL || *recipient == '\0') /* PAS-JM 2/12/85 */
	    return 0 ;

#if	CF_DEBUG
	if (g.debuglevel > 0)
	logfile_printf(&g.eh,"getnames: going the long way\n") ;
#endif

	i = 0 ;
	slen = 0 ;

#ifdef	COMMENT
	if (strchr(recipient, '<') != NULL) separator = "," ;

	else separator = ", " ;
#else
	separator = "," ;
#endif

	openstring( c, s ) ;

	d = parsename(strtok(recipient, separator)) ;

/* PAS-JM 2/12/85 */

	if (d != NULL) {

		l = strlen(d) ;

		if ((slen + l) > HFLEN) {

			slen = 0 ;
			cp = "\n\t" ;
			addstring(c,cp) ;
		}

		slen += l ;
		addstring( c, d ) ;

	}

	i = 1 ;
	d = strtok( 0, separator ) ;

	while (d != NULL) {

	    e = ", " ;
	    addstring( c, e ) ;

	    d = parsename(d) ;

		l = strlen(d) ;

		if ((slen + l) > HFLEN) {

			slen = 0 ;
			cp = "\n\t" ;
			addstring(c,cp) ;
		}

		slen += l ;
	    addstring( c, d ) ;

	    i++ ;
	    d = strtok( 0, separator ) ;

	}

	closestring(c) ;

	if (strlen(s) > ((unsigned) BUFSIZE)) {

	    printf("%s: too many recipients, list truncated\n",
		g.progname) ;

	    s[BUFSIZE-1] = '\0' ;
	    *strrchr(s, ',') = '\0' ;
	}

	strncpy(recipient,s,(BUFSIZE - 1)) ;

#if	CF_DEBUG
	if (g.debuglevel > 0)
	logfile_printf(&g.eh,"getnames: returning w/ %d \"%s\"\n",i,recipient) ;
#endif

	return i ;
}
/* end subroutine (getnames) */


/************************************************************************

  The subroutine 'parsename' does the following :

  - strip any blank prefix from the string				
  - strip any blank postfix from the string
  - copy the address into global string 'curaddr'
  - copy the name into global variable 'selectname'
  - return the stripped string						


************************************************************************/

char *parsename(recipient)
char	*recipient ;
{
	int	l ;

	char *name, *addr, *temp ;


	name = recipient ;
	*selectname = *curaddr = '\0' ;

/* strip leading white space */

	while ((name != NULL) && *name && 
		(ISWHITE(*name) || (*name == '\n')))
	    name += 1 ;

/* strip trailing white space */

	l = strlen(name) ;

	while ((l > 0) && 
		(ISWHITE(name[l - 1]) || (name[l - 1] == '\n')))
		l -= 1 ;

	name[l] = '\0' ;

/* continue */

	if ((addr = strchr(name, '<')) != NULL) {

	    temp = addr ;
	    strncpy(curaddr, ++addr, SYSLEN) ;

	    if ((addr = strchr( curaddr, '>' )) == NULL)
	        *curaddr = '\0' ;

	    else
	        *addr = '\0' ;

	    addr = temp ;
	    addr-- ;
	    while( *addr-- == ' ' ) ;

	    addr = addr + 2 ;
	    *addr = '\0' ;
	    strncpy(selectname,name,LENNAME) ;

	    *addr = ' ' ;
	    *temp = '<' ;

	} else strncpy(selectname,name,LENNAME) ;

	return name ;
}
/* end subroutine (parsename) */



