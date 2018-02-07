/* straltwchar */

/* counted-string copy while compacting white-space from the source */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-01-10, David A.D. Morano
	This was written from scratch.

	= 2017-12-23, David A.D. Morano
        I added some additional characters to the database. I also sped up the
        search for possible replacement characters by implementing a binary
        search.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Find a reasonable substitute string for a given wide-character.

	Synopsis:

	cchar *straltwchar(int wch)

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

static const struct special	specials[] = {
	{ 0x041C, "C" },
	{ 0x041D, "T" },
	{ 0x041E, "y" },
	{ 0x0430, "a" },
	{ 0x0433, "r" },
	{ 0x0435, "e" },
	{ 0x0437, "3" },
	{ 0x043C, "M" },
	{ 0x043D, "H" },
	{ 0x043E, "o" },
	{ 0x043F, "n" },
	{ 0x0440, "p" },
	{ 0x0441, "c" },
	{ 0x0443, "y" },
	{ 0x0445, "x" },
	{ 0x044A, "b" },
	{ 0x044C, "b" },
	{ 0x0455, "s" },
	{ 0x0456, "i" },
	{ 0x0456, "ï" },
	{ 0x0456, "j" },
	{ 0x0474, "V" },
	{ 0x0475, "v" },
	{ 0x047A, "Ø" },
	{ 0x047B, "ø" },
	{ 0x0480, "Ç" },
	{ 0x0481, "ç" },
	{ 0x0484, "^" },
	{ 0x0487, "~" },
	{ 0x0493, "f" },
	{ 0x049A, "K" },
	{ 0x049B, "k" },
	{ 0x049C, "K" },
	{ 0x049D, "k" },
	{ 0x049E, "K" },
	{ 0x049F, "k" },
	{ 0x04A0, "K" },
	{ 0x04A1, "k" },
	{ 0x04A2, "H" },
	{ 0x04A3, "h" },
	{ 0x04A4, "H" },
	{ 0x04A5, "h" },
	{ 0x04AA, "Ç" },
	{ 0x04AB, "ç" },
	{ 0x04AC, "T" },
	{ 0x04AD, "t" },
	{ 0x04AE, "Y" },
	{ 0x04AF, "y" },
	{ 0x04B0, "¥" },
	{ 0x04B1, "y" },
	{ 0x04B2, "X" },
	{ 0x04B3, "x" },
	{ 0x04BA, "h" },
	{ 0x04BB, "h" },
	{ 0x04C0, "|" },
	{ 0x04C1, "X" },
	{ 0x04C2, "x" },
	{ 0x04C7, "H" },
	{ 0x04C8, "h" },
	{ 0x04C9, "H" },
	{ 0x04CA, "h" },
	{ 0x04CB, "Y" },
	{ 0x04CC, "y" },
	{ 0x04CD, "M" },
	{ 0x04CE, "m" },
	{ 0x04CF, "l" },
	{ 0x04D0, "Ã" },
	{ 0x04D1, "ã" },
	{ 0x04D2, "Ä" },
	{ 0x04D3, "ä" },
	{ 0x04D4, "Æ" },
	{ 0x04D5, "æ" },
	{ 0x04D6, "Ê" },
	{ 0x04D7, "ê" },
	{ 0x04DC, "X" },
	{ 0x04DD, "x" },
	{ 0x04DE, "3" },
	{ 0x04DF, "3" },
	{ 0x04E0, "3" },
	{ 0x04E1, "3" },
	{ 0x04E2, "Ñ" },
	{ 0x04E3, "ñ" },
	{ 0x04E4, "Ñ" },
	{ 0x04E5, "ñ" },
	{ 0x04E6, "Ö" },
	{ 0x04E7, "ö" },
	{ 0x04EE, "Y" },
	{ 0x04EF, "y" },
	{ 0x04F0, "Y" },
	{ 0x04F1, "y" },
	{ 0x04F2, "Y" },
	{ 0x04F3, "y" },
	{ 0x04FA, "F" },
	{ 0x04FB, "f" },
	{ 0x04FC, "X" },
	{ 0x04FD, "x" },
	{ 0x04FE, "X" },
	{ 0x04FF, "x" },
	{ 0x0500, "d" },
	{ 0x0501, "d" },
	{ 0x050C, "G" },
	{ 0x050D, "g" },
	{ 0x050E, "V" },
	{ 0x050F, "v" },
	{ 0x0512, "N" },
	{ 0x0513, "n" },
	{ 0x0514, "X" },
	{ 0x0515, "x" },
	{ 0x0516, "P" },
	{ 0x0517, "p" },
	{ 0x0518, "Æ" },
	{ 0x0519, "æ" },
	{ 0x051A, "Q" },
	{ 0x051B, "q" },
	{ 0x051C, "W" },
	{ 0x051D, "w" },
	{ 0x051E, "K" },
	{ 0x051F, "k" },
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


cchar *straltwchar(uint sch)
{
	uint		ch ;
	int		front = 0 ;
	int		back = (nelem(specials)-1) ;
	int		i ;
	cchar		*ss = NULL ;
	i = (front + ((back-front)/2)) ;
	while (front < back) {
	    ch = specials[i].code ;
	    if (sch > ch) {
		front = (i+1) ;
	    } else if (sch < ch) {
		back = i ;
	    } else {
		break ;
	    }
	    i = (front + ((back-front)/2)) ;
	} /* end while */
	if (front < back) {
	   ss = specials[i].ss ;
	}
	return ss ;
}
/* end subroutine (straltwchar) */


