/* mkmaildirtest */

/* make a test file-directory name for testing a mail directory */


/* revision history:

	= 2001-12-03, David A­D­ Morano
	This code was born out of frustration with cleaning up bad legacy code
	(of which there is quite a bit -- like almost all of it).

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine constructs a filename suitable for testing whether a
	mail-directory is accessible or not.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdarg.h>
#include	<string.h>

#include	<vsystem.h>
#include	<storebuf.h>


/* local defines */

#define	MAILDIR_SAVED	":saved"


/* external subroutines */


/* external variables */


/* local strutures */


/* forward references */


/* local variables */


/* exported subroutines */


int mkmaildirtest(char *rbuf,cchar *dp,int dl)
{
	const int	rlen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		i = 0 ;
	cchar		*saved = MAILDIR_SAVED ;

	if (rs >= 0) {
	    rs = storebuf_strw(rbuf,rlen,i,dp,dl) ;
	    i += rs ;
	}

	if ((rs >= 0) && (i > 0) && (rbuf[i-1] != '/')) {
	    rs = storebuf_char(rbuf,rlen,i,'/') ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_strw(rbuf,rlen,i,saved,-1) ;
	    i += rs ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (mkmaildirtest) */


