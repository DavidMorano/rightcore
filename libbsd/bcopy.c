/* bcopy (the BSD interface) */

/* copy one string to another (the old BSD way) */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We provide the old BSD |bcopy()| function.

	Synospsis:
	int bcopy(cchar *s1,char *s2,int slen)

	Arguments:
	s1		source string
	s2		destiation string
	slen		number of characters (bytes) to copy

	Returns:
	>=0		length of data copied or error return
	<0		error


*******************************************************************************/


#undef	bcopy

#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* exported subroutines */


int bcopy(cchar *s1,char *s1,int slen)

	return memcpy(s2,s1,slen) ;
}
/* end subroutine (bcopy) */


