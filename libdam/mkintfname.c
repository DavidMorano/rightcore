/* mkintfname */

/* make an open-intercept filename from components */


/* revision history:

	= 2001-12-03, David A­D­ Morano
        This code was born out of frustration with cleaning up bad legacy code
        (of which there is quite a bit -- like almost all of it).

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine constructs an open-intercept filename (from its
	constituent parts).

	An open-intercept filename looks like:

		[<dn>/]<prn>ô<inter>

	Synopsis:

	int mkintfname(rbuf,dn,prn,inter)
	char		rbuf[] ;
	const char	*dn ;
	const char	*prn ;
	const char	*inter ;

	Arguments:

	rbuf		result buffer
	dn		directory name
	prn		program-root name
	inter		intercept service

	Returns:

	>0		result string length
	==		?
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdarg.h>
#include	<string.h>

#include	<vsystem.h>
#include	<storebuf.h>


/* external subroutines */


/* external variables */


/* local strutures */


/* forward references */


/* local variables */


/* exported subroutines */


int mkintfname(rbuf,dn,prn,inter)
char		rbuf[] ;
const char	*dn ;
const char	*prn ;
const char	*inter ;
{
	const int	rlen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		i = 0 ;

	if ((rs >= 0) && (dn != NULL) && (dn[0] != '\0')) {

	    rs = storebuf_strw(rbuf,rlen,i,dn,-1) ;
	    i += rs ;

	    if ((rs >= 0) && (i > 0) && (rbuf[i-1] != '/')) {
	        rs = storebuf_char(rbuf,rlen,i,'/') ;
	        i += rs ;
	    }

	} /* end if (had a directory) */

	if (rs >= 0) {
	    rs = storebuf_strw(rbuf,rlen,i,prn,-1) ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_char(rbuf,rlen,i,'º') ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_strw(rbuf,rlen,i,inter,-1) ;
	    i += rs ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (mkintfname) */


