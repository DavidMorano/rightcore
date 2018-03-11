/* snwcpyhyphen */

/* similar to 'snwcpy(3dam)' w/ exceptions */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2001-03-14, David A­D­ Morano
        This subroutine was written as a hack to get a subroutine with the
        calling sematic of 'snwcpy(3dam)' but with the twist that any underscore
        characters found in the source string are converted into hyphen
        characters in the destination.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is similar to 'snwcpy(3dam)' except that any underscore
	characters found in the source string are substituted for hyphen
	characters in the target destination string.

	Why would anyone make something like this?

	We needed a hack for real-names that are grabbed from the GECOS field
	of the PASSWD database.  The names in GECOS cannot have hyphens in them
	when the AT&T GECOS convention is used (since it uses hyphens to
	delimit subfields itself.  Instead real-names that are put into AT&T
	conforming GECOS fields have underscrore characters where hyphens might
	otherwise be.  This subroutine undoes that character substitution that
	is already in the GECOS field of the PASSWD databases.

	See-also:

	snwcpy(3dam),
	snwcpylatin(3dam), 
	snwcpyopaque(3dam), 
	snwcpycompact(3dam), 
	snwcpyclean(3dam), 
	snwcpyhyphen(3dam),


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int snwcpyhyphen(char *dbuf,int dlen,cchar *sp,int sl)
{
	int		dl = 0 ;
	int		ch ;
	int		rs = SR_OK ;

	while (dlen-- && sl && *sp) {
	    ch = MKCHAR(*sp++) ;
	    if (ch == '_') ch = '-' ;
	    dbuf[dl++] = (char) ch ;
	    sl -= 1 ;
	} /* end while */
	if ((sl != 0) && (*sp != '\0')) rs = SR_OVERFLOW ;

	dbuf[dl] = '\0' ;
	return (rs >= 0) ? dl : rs ;
}
/* end subroutine (snwcpyhyphen) */


