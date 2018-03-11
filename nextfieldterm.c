/* nextfieldterm */

/* get the next field in a white-space dilineated record */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine will extract the next white-space-separated field from
        the input buffer. If an NL is encountered, it is treated like a
        delimiter and all characters before it (including zero characters) are
        returned as the delimited field.

        The caller should test for zero characters being returned. This
        condition could indicate either that the line has no additional
        characters, or that a NL was encountered. If the caller determines that
        there are characters left in the line but that zero was returned, a
        check by the caller for a NL character is indicated. If the caller
        determines that a NL was present, the caller should arrange to "step
        over" the NL before making new calls in order to get the fields beyond
        the NL.

	Synopsis:

	int nextfieldterm(sp,sl,terms,spp)
	const char	*sp ;
	int		sl ;
	const char	*terms ;
	const char	**spp ;

	Arguments:

	sp		pointer to start of user supplied buffer
	sl		length of user supplied buffer
	terms		terminator characters (in a string)
	spp		pointer to pointer of the found field

	Returns:

	>0		length of found field
	==0		no field found or a NL character was encountered


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<char.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int nextfieldterm(cchar *sp,int sl,cchar *terms,cchar **spp)
{
	int		ch ;

	while (sl && CHAR_ISWHITE(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */

	*spp = sp ;

	while (sl && *sp) {
	    ch = MKCHAR(*sp) ;
	    if (ch == '\n') break ;
	    if (CHAR_ISWHITE(ch)) break ;
	    if (strchr(terms,ch) != NULL) break ;
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */

	return (sp - (*spp)) ;
}
/* end subroutine (nextfieldterm) */


