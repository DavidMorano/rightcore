/* pwihdr */

/* PassWord Index file */


#define	CF_DEBUGS 	0		/* compile-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

	= 2017-08-17, David A­D­ Morano
	I enhanced to use |isValidMagic()|.

*/

/* Copyright © 1998,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine reads from and write to a buffer which represents a PWI
        file header when written out to a file.

	Synopsis:

	int pwihdr(ep,f,hbuf,hlen)
	PWIHDR		*ep ;
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
#include	<endian.h>
#include	<localmisc.h>

#include	"pwihdr.h"


/* local defines */


/* external subroutines */

extern int	mkmagic(char *,int,cchar *) ;
extern int	isValidMagic(cchar *,int,cchar *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int pwihdr(PWIHDR *ep,int f,char *hbuf,int hlen)
{
	uint		*header ;
	const int	headsize = pwihdr_overlast * sizeof(uint) ;
	const int	magicsize = PWIHDR_MAGICSIZE ;
	int		rs = SR_OK ;
	int		bl = hlen ;
	const char	*magicstr = PWIHDR_MAGICSTR ;
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

	            if (ep->vetu[0] != PWIHDR_VERSION) {
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

	    if ((rs >= 0) && (bl > 0)) {

	        if (bl >= headsize) {

	            header = (uint *) bp ;

	            ep->fsize = header[pwihdr_fsize] ;
	            ep->wrtime = header[pwihdr_wrtime] ;
	            ep->wrcount = header[pwihdr_wrcount] ;
	            ep->rectab = header[pwihdr_rectab] ;
	            ep->reclen = header[pwihdr_reclen] ;
	            ep->recsize = header[pwihdr_recsize] ;
	            ep->strtab = header[pwihdr_strtab] ;
	            ep->strlen = header[pwihdr_strlen] ;
	            ep->strsize = header[pwihdr_strsize] ;
	            ep->idxlen = header[pwihdr_idxlen] ;
	            ep->idxsize = header[pwihdr_idxsize] ;
	            ep->idxl1 = header[pwihdr_idxl1] ;
	            ep->idxl3 = header[pwihdr_idxl3] ;
	            ep->idxf = header[pwihdr_idxf] ;
	            ep->idxfl3 = header[pwihdr_idxfl3] ;
	            ep->idxun = header[pwihdr_idxun] ;

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
	    	    bp[0] = PWIHDR_VERSION ;
	    	    bp[1] = ENDIAN ;
	    	    bp += 4 ;
	    	    bl -= 4 ;
	    	    if (bl >= headsize) {
	        	header = (uint *) bp ;
			header[pwihdr_fsize] = ep->fsize ;
			header[pwihdr_wrtime] = ep->wrtime ;
			header[pwihdr_wrcount] = ep->wrcount ;
			header[pwihdr_rectab] = ep->rectab ;
			header[pwihdr_reclen] = ep->reclen ;
			header[pwihdr_recsize] = ep->recsize ;
			header[pwihdr_strtab] = ep->strtab ;
			header[pwihdr_strlen] = ep->strlen ;
			header[pwihdr_strsize] = ep->strsize ;
			header[pwihdr_idxlen] = ep->idxlen ;
			header[pwihdr_idxsize] = ep->idxsize ;
			header[pwihdr_idxl1] = ep->idxl1 ;
			header[pwihdr_idxl3] = ep->idxl3 ;
			header[pwihdr_idxf] = ep->idxf ;
			header[pwihdr_idxfl3] = ep->idxfl3 ;
			header[pwihdr_idxun] = ep->idxun ;
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

#if	CF_DEBUGS
	debugprintf("bvidu: ret f=%d rs=%d\n",f,rs) ;
#endif

	return (rs >= 0) ? (bp - hbuf) : rs ;
}
/* end subroutine (pwihdr) */


