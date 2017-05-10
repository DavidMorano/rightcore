/* process */

/* process a name */


#define	CF_DEBUG	0		/* compile-time debugging */


/* revision history:

	= 1996-03-01, David A­D­ Morano

	The subroutine was adapted from others programs that did similar
	types of functions.


*/

/* Copyright © 1996 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This module does the work of decoding the BASE64 input.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<base64.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */


/* external variables */


/* global variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int decode(pip,name,ofd)
struct proginfo	*pip ;
char		name[] ;
int		ofd ;
{
	int	rs ;
	int	len, ifd ;
	int	cl ;

	char	*buf, *bp ;
	char	*cp ;


	if ((name == NULL) || (name[0] == '\0'))
	    return SR_INVALID ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("process: entered name=\"%s\"\n",name) ;
#endif

	rs = u_open(name,O_RDONLY,0666) ;
	ifd = rs ;
	if (rs < 0) goto badopen ;

	rs = uc_valloc(BUFLEN,&buf) ;
	if (rs < 0) goto badalloc ;

/* loop */

	while ((len = u_read(ifd,buf,BUFLEN)) > 0) {




	} /* end while */

	rs = len ;

badalloc:
	u_close(ifd) ;

badopen:
	return rs ;
}
/* end subroutine (process) */



