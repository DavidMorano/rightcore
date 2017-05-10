/* snfsflags */

/* make string version of the file-system flags */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Ths subroutine is similar to 'sncpy1(3dam)' but it takes a counted
	string for the source rather than only a NUL-terminated string.

	Synopsis:

	int snfsflags(dbuf,dlen,flags)
	char		*dbuf ;
	int		dlen ;
	ulong		flags ;

	Arguments:

	dbuf		destination string buffer
	dlen		destination string buffer length
	flags		FS-flags

	Returns:

	>=0		number of bytes in result
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/statvfs.h>
#include	<string.h>

#include	<vsystem.h>
#include	<storebuf.h>
#include	<localmisc.h>

#include	"snflags.h"


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */

struct flagstrs {
	int		f ;
	const char	*s ;
} ;


/* foward references */


/* local variables */

static const struct flagstrs	fs_vstat[] = {
	{ ST_RDONLY, "RDONLY" },
	{ ST_NOSUID, "NOSUID" },
	{ ST_NOTRUNC, "NOTRUNC" },
	{ 0, NULL }
} ;


/* exported subroutines */


int snfsflags(char *dbuf,int dlen,ulong flags)
{
	SNFLAGS		ss ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (dbuf == NULL) return SR_FAULT ;

	if ((rs = snflags_start(&ss,dbuf,dlen)) >= 0) {
	    int	i ;
	    for (i = 0 ; (rs >= 0) && fs_vstat[i].f ; i += 1) {
	        if (flags & fs_vstat[i].f) {
	            rs = snflags_addstr(&ss,fs_vstat[i].s) ;
		}
	    } /* end for */
	    rs1 = snflags_finish(&ss) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (snflags) */

	return rs ;
}
/* end subroutine (snfsflags) */


