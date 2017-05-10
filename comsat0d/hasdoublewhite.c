/* hasdoublewhite */

/* determine if the given string has some (at least one) double-white-space */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-08-10 David A.D. Morano
	This was written from scratch.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine determines whether the given string has at least one
	double-white-space condition.

	This subroutine really (yes; alterior motive) checks a string to
	determine if white-space compaction is required for those situations
	where white-space compaction is wanted.  Besides detecting a
	double-white-space condition, extra conditions for returning a TRUE
	indication (thereby forcing white-space compaction) are:

	1. white-space at the beginning of the string
	2. white-space anywhere that is not a space (SP) character

	Synopsis:

	int hasdoublewhite(sp,sl)
	const char	sp[] ;
	int		sl ;

	Arguments:

	sp		string to check
	sl		length of given string

	Returns:

	0		No.  No, the given string does not have double-white.
	1		Yes.


	Extra-note: Note that non-breaking-white-space (NBSP) characters are
	*not* considered to be white-space!


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<estrings.h>
#include	<ascii.h>
#include	<char.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int hasdoublewhite(cchar *sp,int sl)
{
	int		i ;
	int		f_white ;
	int		f_prev = FALSE ;
	int		f = FALSE ;

	for (i = 0 ; sl && sp[i] ; i += 1) {
	    f_white = CHAR_ISWHITE(sp[i]) ;
	    if (f_white) {
		f = f_prev ;
	        f = f || (sp[i] != CH_SP) ;	/* to force compaction */
	        f = f || (i == 0) ; 		/* to force compaction */
	        if (f) break ;
	    } /* end if */
	    f_prev = f_white ;
	    sl -= 1 ;
	} /* end for */

	return f ;
}
/* end subroutine (hasdoublewhite) */


