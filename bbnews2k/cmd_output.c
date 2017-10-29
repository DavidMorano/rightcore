/* cmd_output */

/* output a message to a program! */


#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1994-02-01, David A­D­ Morano
        I wrote this from scratch when I took over the code. The previous code
        was a mess (still is in many places !).

	= 1998-02-01, David A­D­ Morano
        I just did a little bit of clean-up.

	= 2017-10-24, David A­D­ Morano
        Modernization: changed to using |uc_system(3uc)|.

*/

/* Copyright © 1998,2017 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>

#include	<vsystem.h>
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
	const int	cmdlen = CMDBUFLEN ;
	int		rs ;
	cchar		*fmt = "%s < %s" ;
	char		cmdbuf[CMDBUFLEN + 1] ;

	if ((rs = bufprintf(cmdbuf,cmdlen,fmt,cmd,afname)) >= 0) {
	    rs = uc_system(cmdbuf) ;
	}

	return rs ;
}
/* end subroutine (cmd_output) */


