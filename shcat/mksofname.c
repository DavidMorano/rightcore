/* mksofname */

/* make a shared-object filename from components */


/* revision history:

	= 2001-12-03, David A­D­ Morano
	This code was born out of frustration with cleaning up bad legacy code
	(of which there is quite a bit -- like almost all of it).

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine constructs a filename from various specified components.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>

#include	<vsystem.h>
#include	<storebuf.h>


/* external subroutines */


/* external variables */


/* local strutures */


/* forward references */


/* local variables */


/* exported subroutines */


int mksofname(char *rbuf,cchar *dn,cchar *name,cchar *ext)
{
	const int	rlen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		i = 0 ;

	if (rs >= 0) {
	    rs = storebuf_strw(rbuf,rlen,i,dn,-1) ;
	    i += rs ;
	}

	if ((rs >= 0) && (i > 0) && (rbuf[i-1] != '/')) {
	    rs = storebuf_char(rbuf,rlen,i,'/') ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_strw(rbuf,rlen,i,name,-1) ;
	    i += rs ;
	}

	if (ext[0] != '\0') {

	    if ((rs >= 0) && (ext[0] != '.')) {
	        rs = storebuf_char(rbuf,rlen,i,'.') ;
	        i += rs ;
	    }

	    if (rs >= 0) {
	        rs = storebuf_strw(rbuf,rlen,i,ext,-1) ;
	        i += rs ;
	    }

	} /* end if (had extension) */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (mksofname) */


