/* vecstr_adds */

/* add white-space separated substrings */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-09-10, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine adds white-space separated substrings of a given string
        to the object.

	Synopsis:

	int vecstr_adds(op,sp,sl)
	vecstr		*op ;
	const char	sp[] ;
	int		sl ;

	Arguments:

	op		pointer to VECSTR object
	sp		string to parse for substrings
	sl		length of string to parse for substrings

	Returns:

	>=0		number of elements loaded
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<vecstr.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	nextfield(const char *,int,const char **) ;


/* external variables */


/* forward references */


/* local structures */


/* local variables */


/* exported subroutines */


int vecstr_adds(vecstr *op,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		wl ;
	int		c = 0 ;
	const char	*wp ;

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (sl < 0)
	    sl = strlen(sp) ;

	while ((sl > 0) && ((wl = nextfield(sp,sl,&wp)) > 0)) {

	    c += 1 ;
	    rs = vecstr_add(op,wp,wl) ;

	    sl -= ((wp + wl) - sp) ;
	    sp = (wp + wl) ;

	    if (rs < 0) break ;
	} /* end while */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecstr_adds) */


