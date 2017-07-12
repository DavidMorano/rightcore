/* cyifu */

/* text-index for VAR-INDEX file */


#define	CF_DEBUGS 	0		/* run-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine writes out the hash file.

	Synopsis:

	int cyifu(ep,f,hbuf,hlen)
	CYIFU		*ep ;
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

#include	"cyifu.h"


/* local defines */


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkmagic(char *,int,cchar *) ;
extern int	cfhexi(const char *,int,uint *) ;
extern int	cfdecui(const char *,int,uint *) ;

extern char	*strwcpy(char *,const char *,int) ;
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


int cyifu(CYIFU *ep,int f,char *hbuf,int hlen)
{
	uint		*header ;
	const int	magicsize = CYIFU_MAGICSIZE ;
	int		rs = SR_OK ;
	int		headsize ;
	int		bl, cl ;
	const char	*magicstr = CYIFU_MAGICSTR ;
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

	        if (ep->vetu[0] != CYIFU_VERSION)
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

		} else
		    rs = SR_ILSEQ ;
	    } /* end if (item) */

	} else { /* write */

	    mkmagic(bp, magicsize, magicstr) ;
	    bp += magicsize ;
	    bl -= magicsize ;

	    memcpy(bp,ep->vetu,4) ;
	    bp[0] = CYIFU_VERSION ;
	    bp[1] = ENDIAN ;
	    bp += 4 ;
	    bl -= 4 ;

	    if ((rs >= 0) && (bl >= headsize)) {

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

	    } /* end if */

	} /* end if */

	return (rs >= 0) ? (bp - hbuf) : rs ;
}
/* end subroutine (cyifu) */


