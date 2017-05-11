/* sfkey */

/* get the key part of a compound string */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-09-10, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will extract the key part of a compound string.

	For example, consider the following compound string:

		A=the_dog_house

	The 'A' would be the key, and the part 'the_dog_house' is the value.

	Important: If there is no key (there is no '=' character), then we
	return an error!

	Synopsis:

	int sfkey(s,slen,rpp)
	const char	s[] ;
	int		slen ;
	char		**rpp ;

	Arguments:

	s		pointer to string to test
	slen		length of string to test
	rpp		pointer to result pointer to store found key

	Returns:

	>=0		length of found key string
	<0		no key was found


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<char.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int sfkey(cchar *sp,int sl,cchar **rpp)
{
	const char	*tp ;
	int		kl = -1 ;

	if (sp == NULL)
	    return BAD ;

	if (sl < 0)
	    sl = strlen(sp) ;

	tp = strnchr(sp,sl,'=') ;
	if (tp != NULL) {

	    kl = (tp - sp) ;
	    while ((kl > 0) && CHAR_ISWHITE(sp[kl - 1]))
	        kl -= 1 ;

	} /* end if */

	if (rpp != NULL) {
	    *rpp = (kl >= 0) ? sp : NULL ;
	}

	return kl ;
}
/* end subroutine (sfkey) */


