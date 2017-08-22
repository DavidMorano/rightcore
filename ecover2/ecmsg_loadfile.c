/* ecmsg_loadfile */

/* load a message from a file */


/* revision history:

	= 1999-06-13, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This little dittie loads a message from a file.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"ecmsg.h"


/* local defines */


/* external subroutines */

extern int	isNotPresent(int) ;


/* exported subroutines */


int ecmsg_loadfile(ECMSG *op,cchar *fn)
{
	USTAT		sb ;
	int		rs ;
	int		mlen ;
	int		len = 0 ;
	char		*mbuf ;
	if (op == NULL) return SR_FAULT ;
	if (fn == NULL) return SR_FAULT ;
	if ((rs = uc_stat(fn,&sb)) >= 0) {
	    mlen = (int) (sb.st_size & INT_MAX) ;
	} else if (isNotPresent(rs)) {
	    mlen = LINEBUFLEN ;
	}
	if (mlen > ECMSG_MAXBUFLEN) mlen = ECMSG_MAXBUFLEN ;
	if ((rs = uc_malloc((mlen+1),&mbuf)) >= 0) {
	    if ((rs= uc_open(fn,O_RDONLY,0666)) >= 0) {
		const int	mfd = rs ;
		if ((rs = uc_readn(mfd,mbuf,mlen)) >= 0) {
	            rs = ecmsg_loadbuf(op,mbuf,rs) ;
		    len = rs ;
		}
		u_close(mfd) ;
	    } /* end if (uc_open) */
	    uc_free(mbuf) ;
	} /* end if (m-a-ffile) */
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (ecmsg_loadfile) */


