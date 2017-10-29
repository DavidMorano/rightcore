/* varhdr */

/* text-index header for VAR-INDEX file */


#define	CF_DEBUGS 	0		/* compile-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine writes out the hash file.

	Synopsis:

	int varhdr(ep,f,hbuf,hlen)
	VARHDR		*ep ;
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

#include	"varhdr.h"


/* local defines */

#ifndef	ITEMLEN
#define	ITEMLEN		100
#endif


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkmagic(char *,int,cchar *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	cfhexi(const char *,int,uint *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	isValidMagic(cchar *,int,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */

enum his {
	hi_fsize,			/* file size */
	hi_wtime,			/* creation time */
	hi_ksoff,			/* key-string table */
	hi_kslen,
	hi_vsoff,			/* value-string table */
	hi_vslen,
	hi_rtoff,			/* record table */
	hi_rtlen,			
	hi_itoff,			/* index (hash) table */
	hi_itlen,			
	hi_nvars,			/* number of variables */
	hi_nskip,
	hi_overlast
} ;


/* forward references */


/* local variables */


/* exported subroutines */


int varhdr(VARHDR *ep,int f,char hbuf[],int hlen)
{
	uint		*header ;
	const int	headsize = hi_overlast * sizeof(uint) ;
	const int	magicsize = VARHDR_MAGICSIZE ;
	int		rs = SR_OK ;
	int		bl = hlen ;
	int		cl ;
	const char	*magicstr = VARHDR_MAGICSTR ;
	const char	*tp, *cp ;
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

	            if (ep->vetu[0] != VARHDR_VERSION)
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
	            ep->ksoff = header[hi_ksoff] ;
	            ep->kslen = header[hi_kslen] ;
	            ep->vsoff = header[hi_vsoff] ;
	            ep->vslen = header[hi_vslen] ;
	            ep->rtoff = header[hi_rtoff] ;
	            ep->rtlen = header[hi_rtlen] ;
	            ep->itoff = header[hi_itoff] ;
	            ep->itlen = header[hi_itlen] ;
	            ep->nvars = header[hi_nvars] ;
	            ep->nskip = header[hi_nskip] ;

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
	    	    *bp = VARHDR_VERSION ;
	    	    bp += 4 ;
	    	    bl -= 4 ;
	            if (bl >= headsize) {
	        	header = (uint *) bp ;
	        	header[hi_fsize] = ep->fsize ;
	        	header[hi_wtime] = ep->wtime ;
	        	header[hi_ksoff] = ep->ksoff ;
	        	header[hi_kslen] = ep->kslen ;
	        	header[hi_vsoff] = ep->vsoff ;
	        	header[hi_vslen] = ep->vslen ;
	        	header[hi_rtoff] = ep->rtoff ;
	        	header[hi_rtlen] = ep->rtlen ;
	        	header[hi_itoff] = ep->itoff ;
	        	header[hi_itlen] = ep->itlen ;
	        	header[hi_nvars] = ep->nvars ;
	        	header[hi_nskip] = ep->nskip ;
	        	bp += headsize ;
	        	bl -= headsize ;
		    } /* end if */
		} /* end if (mkmagic) */
	    } else {
	        rs = SR_OVERFLOW ;
	    } /* end if */

	} /* end if (read-write) */

	return (rs >= 0) ? (bp - hbuf) : rs ;
}
/* end subroutine (varhdr) */


