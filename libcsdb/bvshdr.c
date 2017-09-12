/* bvshdr */

/* index for Bible-Verse-Structure (BVS) file */


#define	CF_DEBUG 	0		/* run-time debugging */


/* revision history:

	= 2008-10-01, David A­D­ Morano
	This module was originally written.

	= 2017-08-17, David A­D­ Morano
	I enhanced to use |isValidMagic()|.

*/

/* Copyright © 2009,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine reads or writes the file header for
        bible-verse-structure (BVS) files.

	Synopsis:

	int bvshdr(ep,f,hbuf,hlen)
	BVSHDR		*ep ;
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
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<endian.h>
#include	<localmisc.h>

#include	"bvshdr.h"


/* local defines */


/* external subroutines */

extern int	mkmagic(char *,int,cchar *) ;
extern int	isValidMagic(cchar *,int,cchar *) ;


/* external variables */


/* local structures */

enum his {
	hi_fsize,			/* file size */
	hi_wtime,			/* creation time */
	hi_nverses,			/* total verses */
	hi_nzverses,			/* non-zero verses */
	hi_nzbooks,			/* number of non-zero books */
	hi_btoff,			/* book-table */
	hi_btlen,
	hi_ctoff,			/* chapter-table */
	hi_ctlen,
	hi_overlast
} ;


/* forward references */


/* local variables */


/* exported subroutines */


int bvshdr(BVSHDR *ep,int f,char *hbuf,int hlen)
{
	uint		*header ;
	const int	headsize = hi_overlast * sizeof(uint) ;
	const int	magicsize = BVSHDR_MAGICSIZE ;
	int		rs = SR_OK ;
	int		bl = hlen ;
	const char	*magicstr = BVSHDR_MAGICSTR ;
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

	            if (ep->vetu[0] != BVSHDR_VERSION) {
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

	            ep->fsize = header[hi_fsize] ;
	            ep->wtime = header[hi_wtime] ;
	            ep->nverses = header[hi_nverses] ;
	            ep->nzverses = header[hi_nzverses] ;
	            ep->nzbooks = header[hi_nzbooks] ;
	            ep->btoff = header[hi_btoff] ;
	            ep->btlen = header[hi_btlen] ;
	            ep->ctoff = header[hi_ctoff] ;
	            ep->ctlen = header[hi_ctlen] ;

	            bp += headsize ;
	            bl -= headsize ;

	            } else {
	                rs = SR_ILSEQ ;
		    }
	        } /* end if (item) */

	    } /* end if (isValidMagic) */
	} else { /* write */

	    if (bl >= (magicsize + 4)) {
	        if ((rs = mkmagic(bp,magicsize,magicstr)) >= 0) {
	            bp += magicsize ;
	            bl -= magicsize ;
	    	    memcpy(bp,ep->vetu,4) ;
	    	    bp[0] = BVSHDR_VERSION ;
	    	    bp[1] = ENDIAN ;
	    	    bp += 4 ;
	    	    bl -= 4 ;
	    	    if (bl >= headsize) {
	        	header = (uint *) bp ;
	        	header[hi_fsize] = ep->fsize ;
	        	header[hi_wtime] = ep->wtime ;
	        	header[hi_nverses] = ep->nverses ;
	        	header[hi_nzverses] = ep->nzverses ;
	        	header[hi_nzbooks] = ep->nzbooks ;
	        	header[hi_btoff] = ep->btoff ;
	        	header[hi_btlen] = ep->btlen ;
	        	header[hi_ctoff] = ep->ctoff ;
	        	header[hi_ctlen] = ep->ctlen ;
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
/* end subroutine (bvshdr) */


