/* outbuf */

/* output buffer management (really?) */


#define	CF_DEBUGS	0		/* compile-time debug print-out */


/* revision history:

	= 1998-08-02, David A­D­ Morano
	This module was originally written to replace the old (yack) 'outbuf'
	mechanism.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This little object lets a caller start out thinking that his
	caller-supplied buffer will be returned by the '_get()' method.
	However, if the original call to the '_start()' method specified a NULL
	buffer, then one is allocated and returned.  In either case, a call to
	the '_finish()' method will deallocate any allocated buffer.


*******************************************************************************/


#define	OUTBUF_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"outbuf.h"


/* local defines */


/* external subroutines */


/* external variables */


/* exported subroutines */


int outbuf_start(OUTBUF *oop,char *obuf,int olen)
{

	if (oop == NULL) return SR_FAULT ;
	if (obuf == NULL) return SR_FAULT ;

	oop->f_alloc = FALSE ;
	oop->obuf = obuf ;
	oop->olen = (olen < 0) ? (MAXPATHLEN + 1) : olen ;
	return SR_OK ;
}
/* end subroutine (outbuf_start) */


int outbuf_finish(OUTBUF *oop)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (oop == NULL) return SR_FAULT ;

	if (oop->f_alloc && (oop->obuf != NULL)) {
	    rs1 = uc_free(oop->obuf) ;
	    if (rs >= 0) rs = rs1 ;
	    oop->obuf = NULL ;
	}

	oop->f_alloc = FALSE ;
	return rs ;
}
/* end subroutine (outbuf_finish) */


int outbuf_get(OUTBUF *oop,char **onpp)
{
	int		rs = SR_OK ;

	if (oop == NULL) return SR_FAULT ;
	if (onpp == NULL) return SR_FAULT ;

	if (oop->f_alloc) {
	    oop->obuf[0] = '\0' ;
	    *onpp = oop->obuf ;
	} else {
	    if (oop->obuf == NULL) {
		int	size = (oop->olen+1) ;
	        char	*p ;
	        if ((rs = uc_valloc(size,&p)) >= 0) {
	            oop->obuf = p ;
	            oop->f_alloc = TRUE ;
	            oop->obuf[0] = '\0' ;
	            *onpp = oop->obuf ;
	        }
	    } else {
	        oop->obuf[0] = '\0' ;
	        *onpp = oop->obuf ;
	        rs = SR_OK ;
	    } /* end if */
	} /* end if */

	return rs ;
}
/* end subroutine (outbuf_get) */


