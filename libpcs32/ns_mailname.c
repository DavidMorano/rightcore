/* ns_mailname */

/* fix a name to make it like a mail name */


/* revision history:

	= 1994-01-12, David A­D­ Morano
	This code was originally written.

*/

/* Copyright © 1994 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************
*									
*	FUNCTIONAL DESCRIPTION:						

        This routine takes an input name string (which is usually a GECOS name
        from the password file but does not have to be) and cleans it up into
        what is thought to be a reasonably respectable name suitable for a mail
        address.

        Note carefully, the resulting name is by no means guaranteed to be a
        valid mail address name. This routine only makes it LOOK like it may be
        a mail address name!

	Arguments:

	gn		input name string (usually a GECOS name)
	rn		buffer to hold result name
	len		length of buffer to receive result

*	RETURNED VALUE:							

		OK	- success (found a valid GECOS name)
		BAD	- failed (could not find a valid GECOS name)
*									
*	SUBROUTINES CALLED:						

*		Only system routines are called.
*									
*	GLOBAL VARIABLES USED:						

*		None!!  AS IT SHOULD BE!! (comment added by David A.D. Morano)
*									
*									
*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<char.h>
#include	<ascii.h>
#include	<localmisc.h>


/* exported subroutines */


int ns_mailname(gn,rn,len)
const char	gn[] ;
char		rn[] ;
int		len ;
{
	int	l, ol ;
	int	f_sep ;

	const char	*cp2, *cp3 ;

	char	*cp ;


	rn[0] = '\0' ;
	if ((gn == NULL) || (gn[0] == '\0')) return BAD ;

	len -= 1 ;

/* strip off AT&T GECOS information separator garbage if present */

	if (((cp = strchr(gn,'-')) != NULL) &&
	    ((cp2 = strchr(cp,CH_LPAREN)) != NULL)) {

	    cp += 1 ;
	    cp3 = cp ;
	    l = (cp2 - cp) ;

	} else {

	    cp3 = gn ;
	    l = strlen(gn) ;

	}

/* delete trailing trash characters */

	while ((l > 0) && (CHAR_ISWHITE(cp3[l - 1]) || (cp3[l - 1] == '.')))
	    l -= 1 ;

	cp3[l] = '\0' ;

/* compact if necessary */

	cp = rn ;
	cp[0] = '\0' ;
	ol = 0 ;

/* skip over leading trash */

	while ((l > 0) && (CHAR_ISWHITE(*cp3) || (*cp3 == '.'))) {

	    cp3 += 1 ;
	    l -= 1 ;
	}

	while ((l > 0) && (ol < len)) {

	    if (strpbrk(cp3,". \t") != NULL) {

/* put in the first initial of the name */

	        *cp++ = CHAR_TOLC(*cp3) ;
	        cp3 += 1 ;
	        l -= 1 ;
	        ol += 1 ;

/* skip over intermediate characters until a separator character */

	        while ((l > 0) && (! CHAR_ISWHITE(*cp3)) && (*cp3 != '.')) {

	            cp3 += 1 ;
	            l -= 1 ;
	        }

/* skip over any separators */

		f_sep = FALSE ;
	        while ((l > 0) && (CHAR_ISWHITE(*cp3) || (*cp3 == '.'))) {

		    f_sep = TRUE ;
	            cp3 += 1 ;
	            l -= 1 ;
	        }

/* add a period character in place of the above separator */

	        if (f_sep && (ol < len)) {

	            *cp++ = '.' ;
	            ol += 1 ;

	        }

	    } else {

/* just put the rest of the characters in the output buffer */

	        while ((l > 0) && (ol < len)) {

	            *cp++ = CHAR_TOLC(*cp3) ;

	            cp3 += 1 ;
	            l -= 1 ;
	            ol += 1 ;
	        }

	    } /* end if */

	} /* end while */

	*cp++ = '\0' ;
	return OK ;
}
/* end subroutine (ns_mailname) */


