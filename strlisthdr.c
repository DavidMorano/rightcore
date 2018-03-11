/* strlisthdr */

/* spellcheck file header */


#define	CF_DEBUGS 	0		/* compile-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine writes out a STRLIST file.

	Synopsis:

	int strlisthdr(ep,f,hbuf,hlen)
	STRLISTHDR	*ep ;
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

#include	"strlisthdr.h"


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

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */

enum his {
	hi_fsize,			/* file size */
	hi_wtime,			/* write (creation) time */
	hi_stoff,			/* string table offset */
	hi_stlen,			/* string table length (entries) */
	hi_rtoff,			/* record table offset */
	hi_rtlen,			/* record table length */
	hi_itoff,			/* index (hash) table offset */
	hi_itlen,			/* index (hash) table length */
	hi_nstrs,			/* total number of strings */
	hi_nskip,			/* hash lookup paramter */
	hi_overlast
} ;


/* forward references */


/* local variables */


/* exported subroutines */


int strlisthdr(ep,f,hbuf,hlen)
STRLISTHDR	*ep ;
int		f ;
char		hbuf[] ;
int		hlen ;
{
	uint		*header ;
	const int	magicsize = STRLISTHDR_MAGICSIZE ;
	int		rs = SR_OK ;
	int		headsize ;
	int		bl, cl ;
	const char	*magicstr = STRLISTHDR_MAGICSTR ;
	const char	*tp, *cp ;
	char		*bp ;

	if (ep == NULL) return SR_FAULT ;
	if (hbuf == NULL) return SR_FAULT ;

	bp = hbuf ;
	bl = hlen ;
	headsize = hi_overlast * sizeof(uint) ;
	if (f) { /* read */

/* the magic string is with the first 16 bytes */

	    if ((rs >= 0) && (bl > 0)) {
	        if (bl >= magicsize) {

	            cp = bp ;
	            cl = magicsize ;
	            if ((tp = strnchr(cp,cl,'\n')) != NULL)
	                cl = (tp - cp) ;

	            bp += magicsize ;
	            bl -= magicsize ;

/* verify the magic string */

	            if (strncmp(magicstr,cp,cl) != 0)
	                rs = SR_NXIO ;

	        } else
	            rs = SR_ILSEQ ;

	    } /* end if (item) */

/* read out the VETU information */

	    if ((rs >= 0) && (bl > 0)) {
	        if (bl >= 4) {

	            memcpy(ep->vetu,bp,4) ;

	            if (ep->vetu[0] != STRLISTHDR_VERSION)
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
	            ep->stoff = header[hi_stoff] ;
	            ep->stlen = header[hi_stlen] ;
	            ep->rtoff = header[hi_rtoff] ;
	            ep->rtlen = header[hi_rtlen] ;
	            ep->itoff = header[hi_itoff] ;
	            ep->itlen = header[hi_itlen] ;
	            ep->nstrs = header[hi_nstrs] ;
	            ep->nskip = header[hi_nskip] ;

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
	    *bp = STRLISTHDR_VERSION ;
	    bp += 4 ;
	    bl -= 4 ;

	    if ((rs >= 0) && (bl >= headsize)) {

	        header = (uint *) bp ;

	        header[hi_fsize] = ep->fsize ;
	        header[hi_wtime] = ep->wtime ;
	        header[hi_stoff] = ep->stoff ;
	        header[hi_stlen] = ep->stlen ;
	        header[hi_rtoff] = ep->rtoff ;
	        header[hi_rtlen] = ep->rtlen ;
	        header[hi_itoff] = ep->itoff ;
	        header[hi_itlen] = ep->itlen ;
	        header[hi_nstrs] = ep->nstrs ;
	        header[hi_nskip] = ep->nskip ;

	        bp += headsize ;
	        bl -= headsize ;

	    } /* end if */

	} /* end if */

	return (rs >= 0) ? (bp - hbuf) : rs ;
}
/* end subroutine (strlisthdr) */


