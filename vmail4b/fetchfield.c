/* fetchfield */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<char.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"mb.h"


/* external varaiables */

extern struct mailbox	mb ;


/* forward references */

int	hmatch() ;



/****************************************************************************

	get the value of the specified header within specified message

	return 0 if header field was found  (fvalue is the value)
 	return 1 if message number is too big or too small
	return 2 if that header is not found in the message


****************************************************************************/


int fetchfield(mn,f,fvalue,buflen)
int	mn ;
char	f[] ;
char	fvalue[] ;
int	buflen ;
{
	int	i, l, ml, flen = 0 ;

	char	field[LINEBUFLEN + 1], *fp = field ;


#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("fetchfield: trying %s\n",f) ;
#endif

	fvalue[0] = '\0' ;
	if ((mn < 0) || (mn >= mb.total)) 
		return 1 ;

	if (curr.fp == NULL)
		return SR_FAULT ;

/* assume getting from current mailbox which is already set up */

	fseek(curr.fp,messbeg[mn],0) ;

	while (ftell(curr.fp) < messend[mn]) {

	    l = freadline(curr.fp,field,LINEBUFLEN) ;

	    if (field[0] == '\n') 
		break ;

/* check to see if we got a truncated line */

	    if (field[l - 1] == '\n') 
		field[--l] = '\0' ;

	    else 
		field[l] = '\0' ;

	    if (! hmatch(f,field)) 
		continue ;

/* got a match, fast forward to colon and first non-white after that */

	    fp = field ;
	    ml = (l < 78) ? l : 78 ;
	    i = 0 ;
	    while ((i++ < ml) && (*fp != ':')) 
		fp += 1 ;

/* skip this header if there was NO colon character */

	    if (i >= ml) 
		continue ;

/* skip the colon character */

	    fp += 1 ;

/* skip over leading white space */

	    while (CHAR_ISWHITE(*fp)) 
		fp += 1 ;

	    l = strlen(fp) ;

	    ml = MIN(l,(buflen - flen)) ;

	    strncpy(fvalue,fp,ml) ;

	    flen += ml ;
	    fvalue[flen] = '\0' ;

/* OK, get more lines until a blank line or non-white 1st character */

	    while ((ftell(curr.fp) < messend[mn]) && (flen < buflen)) {

	        l = freadline(curr.fp,field,LINEBUFLEN) ;

	        if ((l < 1) || (! CHAR_ISWHITE(field[0]))) 
			break ;

	        if (field[l - 1] == '\n') field[l - 1] = '\0' ;

	        else field[l] = '\0' ;

	        fp = field ;
	        while (CHAR_ISWHITE(*fp)) 
			fp += 1 ;

	        if (*fp != '\0') {

	            fvalue[flen++] = ' ' ;

	            l = strlen(fp) ;

	            ml = MIN(l,(buflen - flen)) ;

	            strncpy(fvalue + flen,fp,ml) ;

	            flen += ml ;
	            fvalue[flen] = '\0' ;

	        }

	    } /* end while (inner) */

#ifdef	COMMENT
	    if (flen >= buflen) return BAD ;
#endif

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("fetchfield: returning OK\n") ;
#endif

	    return 0 ; /* found */

	} /* end outer while */

	return 2 ; /* not found */
}
/* end subroutine (fetchfield) */


/***************************************************************************

	Is the initial substring of 'field' the specified 'f' string?  
	Return 0 if there is no match, else we return the
	character position of the header value string.
	The match is case independent.

*/

int hmatch(f,field)
char	f[], field[] ;
{
	char	*fp = field, *hp = f ;


	while (*hp && (*hp != ':')) {

	    if (CHAR_TOLC(*fp) != CHAR_TOLC(*hp)) 
		return 0 ;

	    fp += 1 ;
	    hp += 1 ;
	}

	while (CHAR_ISWHITE(*fp)) 
		fp += 1 ;

	if (*fp != ':') 
		return 0 ;

	return (fp + 1 - field) ;
}
/* end subroutine (hmatch) */



