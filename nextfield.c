/* nextfield */

/* get the next field in a white-space dilineated record */


#define	CF_DEBUGS	0		/* compile-time */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will extract the next white-space-separated field from
	the input buffer.  If an NL is encountered, it is treated like a
	delimiter and all characters before it (including zero characters) are
	returned as the delimited field.

	The caller should test for zero characters being returned.  This
	condition could indicate either that the line has no additional
	characters, or that a NL was encountered.  If the caller determines
	that there are characters left in the line but that zero was returned,
	a check by the caller for a NL character is indicated.  If the caller
	determines that a NL was present, the caller should arrange to "step
	over" the NL before making new calls in order to get the fields beyond
	the NL.

	Synopsis:

	int nextfield(sp,sl,spp)
	const char	*sp ;
	int		sl ;
	const char	**spp ;

	Arguments:

	sp		pointer to start of user supplied buffer
	sl		length of user supplied buffer
	spp		pointer to pointer of the found field

	Returns:

	>0		length of found field
	==0		no field found or a NL character was encountered


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<char.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int nextfield(cchar *sp,int sl,cchar **spp)
{

	while (sl && CHAR_ISWHITE(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */

	*spp = sp ;

/* skip the non-white space */

	while (sl && *sp && (*sp != '\n') && (! CHAR_ISWHITE(*sp))) {
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */

	return (sp - (*spp)) ;
}
/* end subroutine (nextfield) */


int sfnext(cchar *sp,int sl,cchar **spp)
{
	return nextfield(sp,sl,spp) ;
}
/* end subroutine (sfnext) */


