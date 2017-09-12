/* cup */

/* subroutine to convert a counted string to upper case */
/* last modified %G% version %I% */


#define	CF_CHAR		1		/* use 'char(3dam)' */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


/*
; string library subroutines

;	This file contains many of the string manipulation subroutines
;	used in other modules.
*/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<string.h>

#include	<char.h>
#include	<localmisc.h>


/* local defines */

#if	defined(CF_CHAR) && (CF_CHAR == 1)
#define	CF_CHAREXTERN	0
#define	tolc(c)		CHAR_TOLC(c)
#define	touc(c)		CHAR_TOUC(c)
#else
#define	CF_CHAREXTERN	1
#endif


/* external subroutines */

#if	CF_CHAREXTERN
extern int	tolc(int) ;
extern int	touc(int) ;
#endif


/* exported subroutines */


char *cup(len,src,dst)
int		len ;
const char	*src ;
char		*dst ;
{
	int		i ;
	int		ch ;

	for (i = 0 ; i < len ; i += 1) {
	    ch = (*src++ & 0xff) ;
	    *dst++ = touc(ch) ;
	}

	*dst = '\0' ;
	return dst ;
}
/* end subroutine (cup) */


