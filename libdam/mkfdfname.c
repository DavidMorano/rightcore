/* mkfdfname */

/* make a FD file-name (for BFILE and others) */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine creates a string that represents a file that is actually
        already open on a given file-descriptor (FD).

	The filename looks like:

		*<fd>

	where:

	fd		is the decimal representation of the file-descriptor

	Synopsis:

	int mkfdfname(dbuf,fd)
	char		dbuf[] ;
	int		fd ;

	Arguments:

	dbuf		destination buffer
	int		fd ;

	Returns:

	>=0		length of created string
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	ctdeci(char *,int,int) ;
extern int	ctdecui(char *,int,uint) ;


/* exported subroutines */


int mkfdfname(char *dbuf,int fd)
{
	const int	dlen = BFILE_FDNAMELEN ;
	int		rs ;
	int		i = 0 ;

	if (dbuf == NULL) return SR_FAULT ;

	if (fd < 0) return SR_INVALID ;

	dbuf[i++] = BFILE_FDCH ;

	rs = ctdeci((dbuf+i),(dlen-i),fd) ;
	i += rs ;

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (mkfdfname) */


