/* getprogexec */

/* get the program execution name */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	This subroutine was originally written as a KSH built-in command.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	int getprogexec(ebuf,elen)
	char		ebuf[] ;
	int		elen ;

	where:

	ebuf		buffer to store result
	elen		length of supplied retult buffer


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<sncpy.h>
#include	<localmisc.h>


/* exported subroutines */


int getprogexec(char *ebuf,int elen)
{
	int		rs = SR_NOSYS ;
	const char	*en ;

	if (ebuf == NULL) return SR_FAULT ;

	ebuf[0] = '\0' ;
#if	defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)
	if ((en = getexecname()) != NULL) {
	    rs = sncpy1(ebuf,elen,en) ;
	}
#endif /* SOLARIS */

	return rs ;
}
/* end subroutine (getprogexec) */


