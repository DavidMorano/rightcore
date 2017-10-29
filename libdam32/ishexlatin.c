/* ishexlatin */

/* is the specified character a digit? */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_LOOKUP	0		/* try look-up table */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is sort of like 'isxdigit(3c)' but allows for ISO
	Latin-1 characters also.

	Synopsis:

	int ishexlatin(int ch)

	Arguments:

	ch		character to test (can be an 8-bit character)

	Returns:

	TRUE		yes
	FALSE		no


	= Compile-option:
	CF_LOOKUP	set if you want to use a look-up table algorithm

        As we suspected, the version of this subroutine *WITHOUT* the look-up
        table appears to be faster! Do not turn ON "LOOKUP". Normally, we would
        think that look-up tables beat everything. But obviously that is not
        always the case. You can mull over why it might be the case that the
        non-look-up table algorithm wins out, but taking a little bit of a
        closer look at that algorithm (and comparing with the look-up table
        version -- closely) might reveal the answer!


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<ascii.h>
#include	<baops.h>
#include	<localmisc.h>


/* external subroutines */


/* forward references */


/* local variables */

#if	CF_LOOKUP
static const uchar	hexchars[] = {
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xFF, 0x03,
	0x7E, 0x00, 0x00, 0x00,
	0x7E, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;
#endif /* CF_LOOKUP */


/* exported subroutines */


#if	CF_LOOKUP
int ishexlatin(int ch)
{
	int		f = FALSE ;
	if ((ch >= 0) && (ch < 256)) {
	    f = BATST(hexchars,ch) ;
	}
	return f ;
}
/* end subroutine (ishexlatin) */
#else /* CF_LOOKUP */
int ishexlatin(int ch)
{
	int		f = FALSE ;
	f = f || ((ch >= '0') && (ch <= '9')) ;
	f = f || ((ch >= 'a') && (ch <= 'f')) ;
	f = f || ((ch >= 'A') && (ch <= 'F')) ;
	return f ;
}
/* end subroutine (ishexlatin) */
#endif /* CF_LOOKUP */


