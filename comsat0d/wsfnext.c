/* wsfnext */

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

	int wsfnext(sp,sl,spp)
	const wchar_t	*sp ;
	int		sl ;
	const wchar_t	**spp ;

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
#include	<limits.h>
#include	<stddef.h>		/* for 'wchar_t' */
#include	<string.h>

#include	<char.h>
#include	<localmisc.h>


/* local defines */

#ifndef	WCHAR_ISWHITE
#define	WCHAR_ISWHITE(ch)	(((ch) <= UCHAR_MAX) && CHAR_ISWHITE(ch))
#endif


/* type definitions */


/* external subroutines */


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int wsfnext(const wchar_t *wsp,int wsl,const wchar_t **spp)
{
	int		ch ;

	while (wsl) {
	    ch = (int) *wsp ;
	    if (! WCHAR_ISWHITE(ch)) break ;
	    wsp += 1 ;
	    wsl -= 1 ;
	} /* end while */

	*spp = wsp ;

/* skip the non-white space */

	while (wsl && *wsp && (*wsp != '\n')) {
	    ch = (int) *wsp ;
	    if  (WCHAR_ISWHITE(ch)) break ;
	    wsp += 1 ;
	    wsl -= 1 ;
	} /* end while */

	return (wsp - (*spp)) ;
}
/* end subroutine (wsfnext) */


