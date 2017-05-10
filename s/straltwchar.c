/* straltwchar */

/* counted-string copy while compacting white-space from the source */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-01-10 David A.D. Morano
	This was written from scratch.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Find a reasonable substitute string for a given wide-character.

	Synopsis:

	cchar *straltwchar(int wch) ;

	Arguments:

	wch		wide-character to look-up

	Returns:

	<0		error
	>=0		resulting string length


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>
#include	<stddef.h>		/* presumably for 'wchar_t' type */

#include	<vsystem.h>
#include	<ascii.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif


/* external variables */


/* local structures */

struct special {
	uint		code ;
	char		*ss ;
} ;


/* forward references */


/* local variables */

struct special	specials[] = {
	{ 0x2010, "­" },
	{ 0x2011, "­" },
	{ 0x2012, "-" },
	{ 0x2013, "-" },
	{ 0x2014, "--" },
	{ 0x2015, "--" },
	{ 0x2016, "||" },
	{ 0x2017, "=" },
	{ 0x2018, "\047" }, /* single-quote */
	{ 0x2019, "\047" }, /* single-quote */
	{ 0x201A, "," },
	{ 0x201C, "\042" }, /* double-quote */
	{ 0x201D, "\042" }, /* double-quote */
	{ 0x201E, ",," },
	{ 0x201F, "\042" }, /* double-quote */
	{ 0x2020, "÷" },
	{ 0x2021, "±" },
	{ 0x2022, "·" },
	{ 0x2023, "->" },
	{ 0x2024, "." },
	{ 0x2027, "­" },
	{ 0x2032, "\047" }, /* single-quote */
	{ 0x2033, "\042" }, /* double-quote */
	{ 0x2035, "`" }, /* back-quote */
	{ 0x2038, "^" },
	{ 0x2039, "<" },
	{ 0x203A, ">" },
	{ 0x203B, "*" },
	{ 0x203C, "!!" },
	{ 0x203D, "?" },
	{ 0x203E, "¯" },
	{ 0x2043, "-" },
	{ 0x2044, "/" },
	{ 0x2045, "{" },
	{ 0x2046, "}" },
	{ 0x2047, "??" },
	{ 0x2048, "?!" },
	{ 0x2049, "!?" },
	{ 0x204A, "¬" },
	{ 0x204B, "¶" },
	{ 0x204E, "*" },
	{ 0x2053, "~" },
	{ 0x2055, "*" },
	{ 0x2059, "×" },
	{ 0x205A, ":" },
	{ 0x2070, "°" },
	{ 0x2071, "¡" },
	{ 0x2080, "°" },
	{ 0x2081, "¹" },
	{ 0x2082, "²" },
	{ 0x2083, "³" },
	{ 0x208B, "_" },
	{ 0x2095, "?" },
	{ 0x2096, "?" },
	{ 0x2097, "?" },
	{ 0x2098, "?" },
	{ 0x2099, "?" },
	{ 0x209A, "?" },
	{ 0x209B, "?" },
	{ 0x209C, "?" },
	{ 0x209D, "?" },
	{ 0x209E, "?" },
	{ 0x209F, "?" },
	{ 0x20AC, "¤" }, /* this should be the Euro-currency synbol */
	{ 0x20B5, "¢" },
	{ 0x20D2, "|" },
	{ 0, 0 }
} ;


/* exported subroutines */


cchar *straltwchar(uint wch)
{
	int		i ;
	int		f = FALSE ;
	cchar		*ss = NULL ;
	for (i = 0 ; specials[i].code > 0 ; i += 1) {
	    f = (specials[i].code == wch) ;
	    if (f) break ;
	} /* end for */
	if (f) {
	   ss = specials[i].ss ;
	}
	return ss ;
}
/* end subroutine (straltwchar) */


