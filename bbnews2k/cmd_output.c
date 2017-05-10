/* cmd_output */

/* output a message to a program! */


#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1994-02-01, David A­D­ Morano

	I wrote this from scratch when I took over the code.  The
	previous code was a mess (still is in many places !).


*/

/* Copyright © 1994,1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<errno.h>

#include	<bfile.h>
#include	<localmisc.h>

#include	"artlist.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	CMDBUFLEN
#define	CMDBUFLEN	(2 * MAXPATHLEN)
#endif


/* exported subroutines */


int cmd_output(pip,ap,ngdir,afname, cmd)
struct proginfo	*pip ;
ARTLIST_ENT	*ap ;
const char	ngdir[] ;
const char	afname[] ;
const char	cmd[] ;
{
	int	rs ;

	char	cmdbuf[CMDBUFLEN + 1] ;


	rs = bufprintf(cmdbuf,CMDBUFLEN,"%s < %s",cmd,afname) ;

	if (rs >= 0) {
	    rs = system(cmdbuf) ;
	    if (rs == -1) rs = (- errno) ;
	}

	return rs ;
}
/* end subroutine (cmd_output) */



