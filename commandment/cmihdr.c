/* cmihdr */

/* index for Commandment-entry file */


#define	CF_DEBUG 	0		/* run-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine reads and write a commandment-entry index file.

	Synopsis:

	int cmihdr(ep,f,hbuf,hlen)
	CMIHDR		*ep ;
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

#include	"cmihdr.h"


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
	hi_dbsize,			/* DB file size */
	hi_dbtime,			/* DB modification-time */
	hi_idxsize,			/* IDX file size */
	hi_idxtime,			/* IDX modification-time */
	hi_vioff,			/* key-string table */
	hi_vilen,
	hi_vloff,			/* key-string table */
	hi_vllen,
	hi_nents,			/* number of entries */
	hi_maxent,			/* maximum commandment-number */
	hi_overlast
} ;


/* forward references */


/* local variables */


/* exported subroutines */


int cmihdr(CMIHDR *ep,int f,char *hbuf,int hlen)
{
	uint		*header ;
	const int	magicsize = CMIHDR_MAGICSIZE ;
	int		rs = SR_OK ;
	int		headsize ;
	int		bl, cl ;
	const char	*magicstr = CMIHDR_MAGICSTR ;
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

	            if (ep->vetu[0] != CMIHDR_VERSION) {
	                rs = SR_PROTONOSUPPORT ;
		    }

	            if ((rs >= 0) && (ep->vetu[1] != ENDIAN)) {
	                rs = SR_PROTOTYPE ;
		    }

	            bp += 4 ;
	            bl -= 4 ;

	        } else
	            rs = SR_ILSEQ ;

	    } /* end if (item) */

	    if ((rs >= 0) && (bl > 0)) {

	        if (bl >= headsize) {

	            header = (uint *) bp ;

	            ep->dbsize = header[hi_dbsize] ;
	            ep->dbtime = header[hi_dbtime] ;
	            ep->idxsize = header[hi_idxsize] ;
	            ep->idxtime = header[hi_idxtime] ;
	            ep->vioff = header[hi_vioff] ;
	            ep->vilen = header[hi_vilen] ;
	            ep->vloff = header[hi_vloff] ;
	            ep->vllen = header[hi_vllen] ;
	            ep->nents = header[hi_nents] ;
	            ep->maxent = header[hi_maxent] ;

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
	    bp[0] = CMIHDR_VERSION ;
	    bp[1] = ENDIAN ;
	    bp += 4 ;
	    bl -= 4 ;

	    } else
		rs = SR_OVERFLOW ;

	    if ((rs >= 0) && (bl >= headsize)) {

	        header = (uint *) bp ;

	        header[hi_dbsize] = ep->dbsize ;
	        header[hi_dbtime] = ep->dbtime ;
	        header[hi_idxsize] = ep->idxsize ;
	        header[hi_idxtime] = ep->idxtime ;
	        header[hi_vioff] = ep->vioff ;
	        header[hi_vilen] = ep->vilen ;
	        header[hi_vloff] = ep->vloff ;
	        header[hi_vllen] = ep->vllen ;
	        header[hi_nents] = ep->nents ;
	        header[hi_maxent] = ep->maxent ;

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
/* end subroutine (cmihdr) */


