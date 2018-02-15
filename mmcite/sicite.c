/* sicite */

/* string-index to a citation escape */
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

	This subroutine finds the index up to a citation escape.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

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


int sicite(cchar *sp,int sl,cchar *ep,int el)
{
	int		cl = sl ;
	int		si = -1 ;
	const char	*tp ;
	const char	*cp = sp ;

	while ((tp = strnchr(cp,cl,'\\')) != NULL) {

	    if (strncmp((tp + 1),ep,el) == 0) {
	        si = (tp - sp) ;
	        break ;
	    }

	    cl -= ((tp + 1) - cp) ;
	    cp = (tp + 1) ;

	} /* end while */

	return si ;
}
/* end subroutine (sicite) */


