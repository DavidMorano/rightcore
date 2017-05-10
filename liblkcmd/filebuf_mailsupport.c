/* filebuf_mailsupport */

/* mail supprt from the FILEBUF object */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Write a specified number of blanks to a FILEBUF object.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"filebuf.h"


/* local defines */


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int filebuf_hdrkey(FILEBUF *fbp,cchar *kn)
{
	int		rs ;
	int		wlen = 0 ;

	if (fbp == NULL) return SR_FAULT ;
	if (kn == NULL) return SR_FAULT ;

	if (kn[0] == '\0') return SR_INVALID ;

	if ((rs = filebuf_write(fbp,kn,-1)) >= 0) {
	    wlen += rs ;
	    rs = filebuf_write(fbp,": ",2) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (filebuf_hdrkey) */


int filebuf_printcont(FILEBUF *fbp,int leader,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (fbp == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (sl < 0) sl = strlen(sp) ;

	if (sl > 0) {
	    char	buf[2] ;

	    if ((rs >= 0) && (leader > 0)) {
	        buf[0] = leader ;
	        buf[1] = '\0' ;
	        rs = filebuf_write(fbp,buf,1) ;
	        wlen += rs ;
	    }

	    if (rs >= 0) {
	        rs = filebuf_write(fbp,sp,sl) ;
	        wlen += rs ;
	    }

	    if (rs >= 0) {
	        buf[0] = '\n' ;
	        buf[1] = '\0' ;
	        rs = filebuf_write(fbp,buf,1) ;
	        wlen += rs ;
	    }

	} /* end if (non-empty value) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (filebuf_printcont) */


