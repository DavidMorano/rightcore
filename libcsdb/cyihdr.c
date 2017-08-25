/* cyihdr */

/* text-index for VAR-INDEX file */


#define	CF_DEBUGS 	0		/* run-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

	= 2017-08-22, David A­D­ Morano
	I enhanced to use |isValidMagic()|.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine writes out the hash file.

	Synopsis:

	int cyihdr(ep,f,hbuf,hlen)
	CYIHDR		*ep ;
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

#include	"cyihdr.h"


/* local defines */


/* external subroutines */

extern int	mkmagic(char *,int,cchar *) ;
extern int	isValidMagic(cchar *,int,cchar *) ;

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */

enum his {
	hi_fsize,			/* file size */
	hi_wtime,			/* creation time */
	hi_diroff,			/* directory-name */
	hi_caloff,			/* calendar-name */
	hi_vioff,			/* key-string table */
	hi_vilen,
	hi_vloff,			/* key-string table */
	hi_vllen,
	hi_nentries,
	hi_nskip,			/* used in hash-collision algorithm */
	hi_year,			/* the year index was created */
	hi_overlast
} ;


/* forward references */


/* local variables */


/* exported subroutines */


int cyihdr(CYIHDR *ep,int f,char *hbuf,int hlen)
{
	uint		*header ;
	const int	headsize = hi_overlast * sizeof(uint) ;
	const int	magicsize = CYIHDR_MAGICSIZE ;
	int		rs = SR_OK ;
	int		bl = hlen ;
	const char	*magicstr = CYIHDR_MAGICSTR ;
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

	        if (ep->vetu[0] != CYIHDR_VERSION)
	            rs = SR_PROTONOSUPPORT ;

	        if ((rs >= 0) && (ep->vetu[1] != ENDIAN))
	            rs = SR_PROTOTYPE ;

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
	                ep->diroff = header[hi_diroff] ;
	                ep->caloff = header[hi_caloff] ;
	                ep->vioff = header[hi_vioff] ;
	                ep->vilen = header[hi_vilen] ;
	                ep->vloff = header[hi_vloff] ;
	                ep->vllen = header[hi_vllen] ;
	                ep->nentries = header[hi_nentries] ;
	                ep->nskip = header[hi_nskip] ;
	                ep->year = header[hi_year] ;
	                bp += headsize ;
	        	bl -= headsize ;
	            } else {
	                rs = SR_OVERFLOW ;
	            }
	        } /* end if (mkmagic) */

	    } else {
	        rs = SR_OVERFLOW ;
	    }
	} else { /* write */

	    if (bl >= (magicsize + 4)) {
	        if ((rs = mkmagic(bp,magicsize,magicstr)) >= 0) {
	            bp += magicsize ;
	            bl -= magicsize ;
	    	    memcpy(bp,ep->vetu,4) ;
	    	    bp[0] = CYIHDR_VERSION ;
	    	    bp[1] = ENDIAN ;
	    	    bp += 4 ;
	    	    bl -= 4 ;
	    	    if (bl >= headsize) {
	        	header = (uint *) bp ;
	        	header[hi_fsize] = ep->fsize ;
	        	header[hi_wtime] = ep->wtime ;
	        	header[hi_diroff] = ep->diroff ;
	        	header[hi_caloff] = ep->caloff ;
	        	header[hi_vioff] = ep->vioff ;
	        	header[hi_vilen] = ep->vilen ;
	        	header[hi_vloff] = ep->vloff ;
	        	header[hi_vllen] = ep->vllen ;
	        	header[hi_nentries] = ep->nentries ;
	        	header[hi_nskip] = ep->nskip ;
	        	header[hi_year] = ep->year ;
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
/* end subroutine (cyihdr) */


