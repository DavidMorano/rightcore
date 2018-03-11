/* babiesfu */

/* header management for BABIES shared-memory segment */


#define	CF_DEBUGS 	0		/* run-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine reads and writes the BABIES shared-memory segment
	header.

	Synopsis:

	int babiesfu(ep,f,hbuf,hlen)
	BABIESFU	*ep ;
	int		f ;
	char		hbuf[] ;
	int		hlen ;

	Arguments:

	- ep		object pointer
	- f		read=1, write=0
	- hbuf		buffer containing object
	- hlen		length of buffer

	Returns:

	>=0		OK
	<0		error code


*******************************************************************************/


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<endian.h>
#include	<localmisc.h>

#include	"babiesfu.h"


/* local defines */


/* external subroutines */

extern int	mkmagic(char *,int,cchar *) ;
extern int	isValidMagic(cchar *,int,cchar *) ;

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int babiesfu(BABIESFU *ep,int f,char *hbuf,int hlen)
{
	uint		*header ;
	const int	headsize = babiesfuh_overlast * sizeof(uint) ;
	const int	magicsize = BABIESFU_MAGICSIZE ;
	int		rs = SR_OK ;
	int		bl = hlen ;
	const char	*magicstr = BABIESFU_MAGICSTR ;
	char		*bp = hbuf ;

	if (ep == NULL) return SR_FAULT ;
	if (hbuf == NULL) return SR_FAULT ;

	if (f) { /* read */
	    if ((bl > magicsize) && isValidMagic(bp,magicsize,magicstr)) {
	        bp += magicsize ;
	        bl -= magicsize ;

/* read out the VETU information */

	        if (bl >= 4) {

	            memcpy(ep->vetu,bp,4) ;

	            if (ep->vetu[0] != BABIESFU_VERSION) {
	                rs = SR_PROTONOSUPPORT ;
		    }

	            if ((rs >= 0) && (ep->vetu[1] != ENDIAN)) {
	                rs = SR_PROTOTYPE ;
		    }

	            bp += 4 ;
	            bl -= 4 ;

	        } else {
	            rs = SR_ILSEQ ;
		}

	        if (rs >= 0) {
	            if (bl >= headsize) {
	                header = (uint *) bp ;
	                ep->shmsize = header[babiesfuh_shmsize] ;
	                ep->dbsize = header[babiesfuh_dbsize] ;
	                ep->dbtime = header[babiesfuh_dbtime] ;
	                ep->wtime = header[babiesfuh_wtime] ;
	                ep->atime = header[babiesfuh_atime] ;
	                ep->acount = header[babiesfuh_acount] ;
	                ep->muoff = header[babiesfuh_muoff] ;
	                ep->musize = header[babiesfuh_musize] ;
	                ep->btoff = header[babiesfuh_btoff] ;
	                ep->btlen = header[babiesfuh_btlen] ;
	                bp += headsize ;
	                bl -= headsize ;
	            } else {
	                rs = SR_ILSEQ ;
		    }
	        } /* end if (ok) */

	    } /* end if (isValidMagic) */
	} else { /* write */
	    if (bl >= (magicsize + 4)) {
	        if ((rs = mkmagic(bp,magicsize,magicstr)) >= 0) {
	            bp += magicsize ;
	            bl -= magicsize ;
	    	    memcpy(bp,ep->vetu,4) ;
	    	    bp[0] = BABIESFU_VERSION ;
	    	    bp[1] = ENDIAN ;
	    	    bp += 4 ;
	    	    bl -= 4 ;
	    	    if (bl >= headsize) {
	        	header = (uint *) bp ;
	        	header[babiesfuh_shmsize] = ep->shmsize ;
	        	header[babiesfuh_dbsize] = ep->dbsize ;
	        	header[babiesfuh_dbtime] = ep->dbtime ;
	        	header[babiesfuh_wtime] = ep->wtime ;
	        	header[babiesfuh_atime] = ep->atime ;
	        	header[babiesfuh_acount] = ep->acount ;
	        	header[babiesfuh_muoff] = ep->muoff ;
	        	header[babiesfuh_musize] = ep->musize ;
	        	header[babiesfuh_btoff] = ep->btoff ;
	        	header[babiesfuh_btlen] = ep->btlen ;
	        	bp += headsize ;
	        	bl -= headsize ;
	            } else {
	                rs = SR_OVERFLOW ;
	            }
	        } /* end if (mkmagic) */
	    } else {
	        rs = SR_OVERFLOW ;
	    }
	} /* end if (read-write) */

	return (rs >= 0) ? (bp - hbuf) : rs ;
}
/* end subroutine (babiesfu) */


