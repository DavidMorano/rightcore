/* silbrace */

/* is the next character a left-brace? */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* run-time debug print-outs */


/* revision history:

	= 1992-03-01, David A­D­ Morano
	This subroutine was originally written.

	= 1998-09-01, David A­D­ Morano
	This subroutine was modified to process the way MMCITE does citation.
	It used to use the old GNU 'lookbib' program in addition to the (old)
	standard UNIX version.  But neither of these are used now.  Straight
	out citeation keywrd lookup is done directly in a BIB database (files
	of which are suffixed 'rbd').

*/

/* Copyright © 1992,1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine checks to see if the next non-white-space character is
	a left-brace.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<ascii.h>
#include	<char.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfsub(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int silbrace(const char *sp,int sl)
{
	int		si = 0 ;

	while (sl && CHAR_ISWHITE(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	    si += 1 ;
	}

	if ((sl == 0) || (sp[0] != CH_LBRACE))
	    si = -1 ;

	return si ;
}
/* end subroutine (silbrace) */


