/* newobjsub */

/* new-object-subroutine */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-12-01, David A­D­ Morano

	This module was originally written for hardware CAD support.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These routines are used when the caller wants to store a COPY
	of the passed string data into a vector.  These routines will
	copy and store the copied data in the list.  The advantage is
	that the caller does not have to keep the orginal data around
	in order for the list data to be accessed later.  String data
	(unlike "element" data) can not contain NULL characters/bytes.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


void *newobjsub(int n,int esize) {
	const int	size = (n*esize) ;
	void		*vp ;
	if (uc_malloc(size,&vp) < 0) vp = NULL ;
	return vp ;
}
/* end subroutine (newobjsub) */



