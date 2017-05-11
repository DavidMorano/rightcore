/* snopenflags */

/* make string version of the open-call flags */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Ths subroutine creates in the result string a symbolic representation of
        open-file flags.

	Synopsis:

	int snopenflags(dbuf,dlen,flags)
	char		*dbuf ;
	int		dlen ;
	int		flags ;

	Arguments:

	dbuf		destination string buffer
	dlen		destination string buffer length
	flags		open-flags

	Returns:

	>=0		number of bytes in result
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<fcntl.h>
#include	<string.h>

#include	<vsystem.h>
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

static const struct flagstrs	fs_open[] = {
	{ O_APPEND, "APPEND" },
	{ O_CREAT, "CREAT" },
	{ O_EXCL, "EXCL" },
	{ O_TRUNC, "TRUNC" },
	{ O_NOCTTY, "NOCTTY" },
	{ O_SYNC, "SYNC" },
	{ O_DSYNC, "DSYNC" },
	{ O_RSYNC, "RSYNC" },
	{ O_NDELAY, "NDELAY" },
	{ O_NONBLOCK, "NONBLOCK" },
	{ O_LARGEFILE, "LARGE" },
#ifdef	O_PRIV
	{ O_PRIV, "PRIV" },
#endif
#ifdef	O_SETSID
	{ O_SETSID, "SETSID" },
#endif
#ifdef	O_CLOEXEC
	{ O_CLOEXEC, "CLOEXEC" },
#endif
#ifdef	O_NETWORK
	{ O_NETWORK, "NETWORK" },
#endif
	{ 0, NULL }
} ;


/* exported subroutines */


int snopenflags(char *dbuf,int dlen,int flags)
{
	SNFLAGS		ss ;
	int		rs ;
	int		rs1 ;

	if (dbuf == NULL) return SR_FAULT ;

	if ((rs = snflags_start(&ss,dbuf,dlen)) >= 0) {
	    const int	am = (flags & O_ACCMODE) ;
	    int		i ;
	    const char	*ms = NULL ;
	    switch (am) {
	    case O_RDONLY:
		ms = "RDONLY" ;
		break ;
	    case O_WRONLY:
		ms = "WRONLY" ;
		break ;
	    case O_RDWR:
		ms = "RDWR" ;
		break ;
	    default:
		ms = "ACCINV" ; /* access invalid */
		break ;
	    } /* end switch */
	    if (ms != NULL) {
		rs = snflags_addstr(&ss,ms) ;
	    }
	    for (i = 0 ; (rs >= 0) && fs_open[i].f ; i += 1) {
	        if (flags & fs_open[i].f) {
	            rs = snflags_addstr(&ss,fs_open[i].s) ;
		}
	    } /* end for */
	    rs1 = snflags_finish(&ss) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (snflags) */

	return rs ;
}
/* end subroutine (snopenflags) */


