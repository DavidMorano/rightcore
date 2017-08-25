/* uunamefu */

/* UUCP name data-base file header */


#define	CF_DEBUG 	0		/* run-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine writes out the hash file.

	Synopsis:

	int uunamefu(ep,f,hbuf,hlen)
	UUNAMEFU	*ep ;
	int		f ;
	char		hbuf[] ;
	int		hlen ;

	Arguments:

	- ep		object pointer
	- f		read=1, write=0
	- hbuf		buffer containing object
	- hlen	length of buffer

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

#include	"uunamefu.h"


/* local defines */

#define	UUNAMEFU_MAGICSTR	"MKINVHASH"
#define	UUNAMEFU_MAGICSIZE	16

#ifndef	ITEMLEN
#define	ITEMLEN		100
#endif


/* external subroutines */

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */

enum his {
	hi_hfsize,
	hi_tfsize,
	hi_wtime,
	hi_sdnoff,
	hi_sfnoff,
	hi_listoff,
	hi_taboff,
	hi_tablen,			/* length (in entries) of hash table */
	hi_taglen,			/* number of tags */
	hi_maxtags,			/* maximum tags in any list */
	hi_minwlen,
	hi_maxwlen,
	hi_overlast
} ;


/* forward references */

extern int	mkmagic(char *,int,cchar *) ;
extern int	isValidMagic(cchar *,int,cchar *) ;


/* local variables */


/* exported subroutines */


int uunamefu(ep,f,hbuf,hlen)
UUNAMEFU	*ep ;
int		f ;
char		hbuf[] ;
int		hlen ;
{
	uint		*header ;
	const int	headsize = hi_overlast * sizeof(uint) ;
	const int	magicsize = UUNAMEFU_MAGICSIZE ;
	int		rs = SR_OK ;
	int		bl = hlen ;
	int		cl ;
	const char	*magicstr = UUNAMEFU_MAGICSTR ;
	char		*bp = hbuf ;
	char		*tp, *cp ;

	if (ep == NULL) return SR_FAULT ;
	if (buf == NULL) return SR_FAULT ;

	if (buflen < 1) return SR_OVERFLOW ;

	if (f) { /* read */
	    if ((bl > magicsize) && isValidMagic(bp,magicsize,magicstr)) {
	        bp += magicsize ;
	        bl -= magicsize ;

/* read out the VETU information */

	        if (bl >= 4) {

	            memcpy(ep->vetu,bp,4) ;

	            if (ep->vetu[0] != UUNAMEFU_VERSION)
	                rs = SR_PROTONOSUPPORT ;

	            if ((rs >= 0) && (ep->vetu[1] != ENDIAN))
	                rs = SR_INVALID ;

	            bp += 4 ;
	            bl -= 4 ;

	        } else {
	            rs = SR_ILSEQ ;
	        } /* end if (item) */

	        if (rs >= 0) {
		    if (bl >= headsize) {
	        	header = (uint *) bp ;
	        	ep->hfsize = header[hi_hfsize] ;
	        	ep->tfsize = header[hi_tfsize] ;
	        	ep->wtime = header[hi_wtime] ;
	        	ep->sdnoff = header[hi_sdnoff] ;
	        	ep->sfnoff = header[hi_sfnoff] ;
	        	ep->listoff = header[hi_listoff] ;
	        	ep->taboff = header[hi_taboff] ;
	        	ep->tablen = header[hi_tablen] ;
	        	ep->taglen = header[hi_taglen] ;
	        	ep->maxtags = header[hi_maxtags] ;
	        	ep->minwlen = header[hi_minwlen] ;
	        	ep->maxwlen = header[hi_maxwlen] ;
	        	bp += headsize ;
	        	bl -= headsize ;
	            } else {
	                rs = SR_ILSEQ ;
	            }
	        } /* end if (item) */

	    } else {
	        rs = SR_ILSEQ ;
	    } /* end if (isValidMagic) */
	} else { /* write */
	    if (bl >= (magicsize + 4)) {
	        if ((rs = mkmagic(bp,magicsize,magicstr)) >= 0) {
	            bp += magicsize ;
	            bl -= magicsize ;
	    	    memcpy(bp,ep->vetu,4) ;
	    	    *bp = UUNAMEFU_VERSION ;
	    	    bp += 4 ;
	    	    bl -= 4 ;
	    	    if (bl >= headsize) {
	        	header = (uint *) bp ;
	        	header[hi_hfsize] = ep->hfsize ;
	        	header[hi_tfsize] = ep->tfsize ;
	        	header[hi_wtime] = ep->wtime ;
	        	header[hi_sdnoff] = ep->sdnoff ;
	        	header[hi_sfnoff] = ep->sfnoff ;
	        	header[hi_listoff] = ep->listoff ;
	        	header[hi_taboff] = ep->taboff ;
	        	header[hi_tablen] = ep->tablen ;
	        	header[hi_taglen] = ep->taglen ;
	        	header[hi_maxtags] = ep->maxtags ;
	        	header[hi_minwlen] = ep->minwlen ;
	        	header[hi_maxwlen] = ep->maxwlen ;
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

	return (rs >= 0) ? (bp - buf) : rs ;
}
/* end subroutine (uunamefu) */


