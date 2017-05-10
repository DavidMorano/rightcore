/* bvihdr */

/* index for bible-verse file */


#define	CF_DEBUG 	0		/* run-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine reads and write a bible-verse-index file.

	Synopsis:

	int bvihdr(ep,f,hbuf,hlen)
	BVIHDR		*ep ;
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
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<endianstr.h>
#include	<localmisc.h>

#include	"bvihdr.h"


/* local defines */


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkmagic(char *,const char *,int) ;
extern int	cfhexi(const char *,int,uint *) ;
extern int	cfdecui(const char *,int,uint *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */

enum his {
	hi_fsize,			/* file size */
	hi_wtime,			/* creation time */
	hi_vioff,			/* key-string table */
	hi_vilen,
	hi_vloff,			/* key-string table */
	hi_vllen,
	hi_nverses,
	hi_nzverses,
	hi_maxbook,
	hi_maxchapter,
	hi_overlast
} ;


/* forward references */


/* local variables */


/* exported subroutines */


int bvihdr(BVIHDR *ep,int f,char *hbuf,int hlen)
{
	uint		*header ;
	const int	magicsize = BVIHDR_MAGICSIZE ;
	int		rs = SR_OK ;
	int		headsize ;
	int		bl, cl ;
	const char	*magicstr = BVIHDR_MAGICSTR ;
	const char	*tp, *cp ;
	char		*bp ;

	if (ep == NULL) return SR_FAULT ;
	if (hbuf == NULL) return SR_FAULT ;

	bp = hbuf ;
	bl = hlen ;
	headsize = hi_overlast * sizeof(uint) ;
	if (f) { /* read */

/* the magic string is within the first 15 bytes */

	    if ((rs >= 0) && (bl > 0)) {

	        if (bl >= magicsize) {

	            cp = bp ;
	            cl = magicsize ;
	            if ((tp = strnchr(cp,cl,'\n')) != NULL)
	                cl = (tp - cp) ;

	            bp += magicsize ;
	            bl -= magicsize ;

/* verify the magic string */

	            if (strncmp(cp,magicstr,cl) != 0)
	                rs = SR_NXIO ;

	        } else
	            rs = SR_ILSEQ ;

	    } /* end if (item) */

/* read out the VETU information */

	    if ((rs >= 0) && (bl > 0)) {

	        if (bl >= 4) {

	            memcpy(ep->vetu,bp,4) ;

	            if (ep->vetu[0] != BVIHDR_VERSION)
	                rs = SR_PROTONOSUPPORT ;

	            if ((rs >= 0) && (ep->vetu[1] != ENDIAN))
	                rs = SR_PROTOTYPE ;

	            bp += 4 ;
	            bl -= 4 ;

	        } else
	            rs = SR_ILSEQ ;

	    } /* end if (item) */

	    if ((rs >= 0) && (bl > 0)) {

	        if (bl >= headsize) {

	            header = (uint *) bp ;

	            ep->fsize = header[hi_fsize] ;
	            ep->wtime = header[hi_wtime] ;
	            ep->vioff = header[hi_vioff] ;
	            ep->vilen = header[hi_vilen] ;
	            ep->vloff = header[hi_vloff] ;
	            ep->vllen = header[hi_vllen] ;
	            ep->nverses = header[hi_nverses] ;
	            ep->nzverses = header[hi_nzverses] ;
	            ep->maxbook = header[hi_maxbook] ;
	            ep->maxchapter = header[hi_maxchapter] ;

	            bp += headsize ;
	            bl -= headsize ;

	        } else
	            rs = SR_ILSEQ ;

	    } /* end if (item) */

	} else { /* write */

	    if ((rs >= 0) && (bl >= (magicsize + 4))) {

	    mkmagic(bp,magicstr,magicsize) ;
	    bp += magicsize ;
	    bl -= magicsize ;

	    memcpy(bp,ep->vetu,4) ;
	    bp[0] = BVIHDR_VERSION ;
	    bp[1] = ENDIAN ;
	    bp += 4 ;
	    bl -= 4 ;

	    } else
		rs = SR_OVERFLOW ;

	    if ((rs >= 0) && (bl >= headsize)) {

	        header = (uint *) bp ;

	        header[hi_fsize] = ep->fsize ;
	        header[hi_wtime] = ep->wtime ;
	        header[hi_vioff] = ep->vioff ;
	        header[hi_vilen] = ep->vilen ;
	        header[hi_vloff] = ep->vloff ;
	        header[hi_vllen] = ep->vllen ;
	        header[hi_nverses] = ep->nverses ;
	        header[hi_nzverses] = ep->nzverses ;
	        header[hi_maxbook] = ep->maxbook ;
	        header[hi_maxchapter] = ep->maxchapter ;

	        bp += headsize ;
	        bl -= headsize ;

	    } else
		rs = SR_OVERFLOW ;

	} /* end if */

#if	CF_DEBUGS
	debugprintf("bvidu: f=%d rs=%d\n",f,rs) ;
#endif

	return (rs >= 0) ? (bp - hbuf) : rs ;
}
/* end subroutine (bvihdr) */


