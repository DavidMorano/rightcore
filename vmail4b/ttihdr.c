/* ttihdr */

/* Termianl-Translate-Index file management (file header) */


#define	CF_DEBUGS 	0		/* compile-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine manages the header for TERMTRANS lookup-table
        index-file.

	Synopsis:

	int ttihdr(ep,f,hbuf,hlen)
	TTIHDR		*ep ;
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
#include	<endianstr.h>
#include	<localmisc.h>

#include	"ttihdr.h"


/* local defines */


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkmagic(char *,int,cchar *) ;
extern int	cfhexi(const char *,int,uint *) ;
extern int	cfdecui(const char *,int,uint *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */

#ifdef	COMMENT
	uint		fsize ;		/* file-size */
	uint		ctime ;		/* create-time */
	uint		rectab ;	/* record-table */
	uint		reclen ;	/* recotd-table-length */
	uint		ostrtab ;	/* overflow-string-table */
	uint		ostrlen ;	/* overflow-string-table length */
	uchar		vetu[4] ;	/* VETU */
#endif /* COMMENT */

enum his {
	hi_fsize,			/* file-size */
	hi_ctime,			/* creation-time */
	hi_rectab,			/* record-table */
	hi_reclen,			/* record-legnth */
	hi_ostrtab,			/* overflow-string-table */
	hi_ostrlen,			/* overflow-string-table length */
	hi_overlast
} ;


/* forward references */


/* local variables */


/* exported subroutines */


int ttihdr(ep,f,hbuf,hlen)
TTIHDR		*ep ;
int		f ;
char		hbuf[] ;
int		hlen ;
{
	uint		*header ;
	const int	headsize = hi_overlast * sizeof(uint) ;
	const int	magicsize = TTIHDR_MAGICSIZE ;
	int		rs = SR_OK ;
	int		bl = hlen ;
	int		cl ;
	const char	*magicstr = TTIHDR_MAGICSTR ;
	const char	*tp, *cp ;
	char		*bp = hbuf ;

	if (ep == NULL) return SR_FAULT ;
	if (hbuf == NULL) return SR_FAULT ;

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

	            if (ep->vetu[0] != TTIHDR_VERSION)
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
	            ep->ctime = header[hi_ctime] ;
	            ep->rectab = header[hi_rectab] ;
	            ep->reclen = header[hi_reclen] ;
	            ep->ostrtab = header[hi_ostrtab] ;
	            ep->ostrlen = header[hi_ostrlen] ;

	            bp += headsize ;
	            bl -= headsize ;

	        } else
	            rs = SR_ILSEQ ;

	    } /* end if (item) */

	} else { /* write */

	    if ((rs >= 0) && (bl >= (magicsize + 4))) {

	    mkmagic(bp, magicsize, magicstr) ;
	    bp += magicsize ;
	    bl -= magicsize ;

	    memcpy(bp,ep->vetu,4) ;
	    bp[0] = TTIHDR_VERSION ;
	    bp[1] = ENDIAN ;
	    bp += 4 ;
	    bl -= 4 ;

	    } else
		rs = SR_OVERFLOW ;

	    if ((rs >= 0) && (bl >= headsize)) {

	        header = (uint *) bp ;

	        header[hi_fsize] = ep->fsize ;
	        header[hi_ctime] = ep->ctime ;
	        header[hi_rectab] = ep->rectab ;
	        header[hi_reclen] = ep->reclen ;
	        header[hi_ostrtab] = ep->ostrlen ;
	        header[hi_ostrlen] = ep->ostrlen ;

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
/* end subroutine (ttihdr) */


