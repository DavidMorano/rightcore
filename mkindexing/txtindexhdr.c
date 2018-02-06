/* txtindexhdr */

/* text-index hash file */


#define	CF_DEBUGS 	0		/* compile-time debugging */


/* revision history:

	= 2008-03-01, David A­D­ Morano
	This subroutine was originally written.

	= 2017-08-17, David A­D­ Morano
	I enhanced to use |isValidMagic()|.

*/

/* Copyright © 2008,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine writes out the hash file.

	Synopsis:

	int txtindexhdr(ep,f,hbuf,hlen)
	TXTINDEXFU	*ep ;
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

#include	"txtindexhdr.h"


/* local defines */


/* external subroutines */

extern int	mkmagic(char *,int,cchar *) ;
extern int	isValidMagic(cchar *,int,cchar *) ;

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */

enum his {
	hi_hfsize,
	hi_tfsize,
	hi_ersize,
	hi_eisize,
	hi_wtime,
	hi_sdnoff,
	hi_sfnoff,
	hi_listoff,
	hi_esoff,
	hi_essize,
	hi_eroff,
	hi_erlen,
	hi_eioff,
	hi_eilen,
	hi_eiskip,
	hi_taboff,
	hi_tablen,			/* length (in entries) of hash table */
	hi_taglen,			/* number of tags */
	hi_maxtags,			/* maximum tags in any list */
	hi_minwlen,
	hi_maxwlen,
	hi_overlast
} ;


/* forward references */


/* local variables */


/* exported subroutines */


int txtindexhdr(TXTINDEXFU *ep,int f,char *hbuf,int hlen)
{
	uint		*header ;
	const int	headsize = hi_overlast * sizeof(uint) ;
	const int	magicsize = TXTINDEXFU_MAGICSIZE ;
	int		rs = SR_OK ;
	int		bl = hlen ;
	const char	*magicstr = TXTINDEXFU_MAGICSTR ;
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

	        if (ep->vetu[0] != TXTINDEXFU_VERSION) {
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
	        	ep->hfsize = header[hi_hfsize] ;
	        	ep->tfsize = header[hi_tfsize] ;
	        	ep->ersize = header[hi_ersize] ;
	        	ep->eisize = header[hi_eisize] ;
	        	ep->wtime = header[hi_wtime] ;
	        	ep->sdnoff = header[hi_sdnoff] ;
	        	ep->sfnoff = header[hi_sfnoff] ;
	        	ep->listoff = header[hi_listoff] ;
	        	ep->esoff = header[hi_esoff] ;
	        	ep->essize = header[hi_essize] ;
	        	ep->eroff = header[hi_eroff] ;
	        	ep->erlen = header[hi_erlen] ;
	        	ep->eioff = header[hi_eioff] ;
	        	ep->eilen = header[hi_eilen] ;
	        	ep->eiskip = header[hi_eiskip] ;
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

	    } /* end if (mkmagic) */
	} else { /* write */

	    if (bl >= (magicsize + 4)) {
	        if ((rs = mkmagic(bp,magicsize,magicstr)) >= 0) {
	            bp += magicsize ;
	            bl -= magicsize ;
	    	    memcpy(bp,ep->vetu,4) ;
	    	    bp[0] = TXTINDEXFU_VERSION ;
	    	    bp[1] = ENDIAN ;
	    	    bp += 4 ;
	    	    bl -= 4 ;
	    	    if (bl >= headsize) {
	        	header = (uint *) bp ;
	        	header[hi_hfsize] = ep->hfsize ;
	        	header[hi_tfsize] = ep->tfsize ;
	        	header[hi_ersize] = ep->ersize ;
	        	header[hi_eisize] = ep->eisize ;
	        	header[hi_wtime] = ep->wtime ;
	        	header[hi_sdnoff] = ep->sdnoff ;
	        	header[hi_sfnoff] = ep->sfnoff ;
	        	header[hi_listoff] = ep->listoff ;
	        	header[hi_esoff] = ep->esoff ;
	        	header[hi_essize] = ep->essize ;
	        	header[hi_eroff] = ep->eroff ;
	        	header[hi_erlen] = ep->erlen ;
	        	header[hi_eioff] = ep->eioff ;
	        	header[hi_eilen] = ep->eilen ;
	        	header[hi_eiskip] = ep->eiskip ;
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

	return (rs >= 0) ? (bp - hbuf) : rs ;
}
/* end subroutine (txtindexhdr) */


