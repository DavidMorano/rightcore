/* sncpyfc */

/* copy a NUL-terminated string(s) to a fixed sized destination buffer */


#define	CF_CHAR		1		/* use 'char(3dam)' */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine constructs a single string from a single specificed
        string (folding the case in the process).


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>

#include	<vsystem.h>
#include	<char.h>


/* local defines */

#if	defined(CF_CHAR) && (CF_CHAR == 1)
#define	CF_CHAREXTERN	0
#define	tolc(c)		CHAR_TOLC(c)
#define	touc(c)		CHAR_TOUC(c)
#define	tofc(c)		CHAR_TOFC(c)
#else
#define	CF_CHAREXTERN	1
#endif


/* external subroutines */

#if	CF_CHAREXTERN
extern int	tolc(int) ;
extern int	touc(int) ;
extern int	tofc(int) ;
#endif


/* exported subroutines */


int sncpyfc(char *dbuf,int dlen,cchar *s1)
{
	int		i ;
	char		*dp = dbuf ;

	if (dlen < 0)
	    dlen = INT_MAX ;

	for (i = 0 ; s1[i] && (i < dlen) ; i += 1) {
	    *dp++ = tofc(s1[i]) ;
	}

	*dp = '\0' ;
	return (s1[i] == '\0') ? (dp - dbuf) : SR_OVERFLOW ;
}
/* end subroutine (sncpyfc) */


