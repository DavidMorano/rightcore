/* pcsunsfh */

/* management for PCSUNS shared-memory segment */


#define	CF_DEBUG 	0		/* run-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine reads and writes the BABIES shared-memory segment.

	Synopsis:

	int pcsunsfh(ep,f,hbuf,hlen)
	PCSUNSFH	*ep ;
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

#include	"pcsunsfh.h"


/* local defines */


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	mkmagic(char *,int,cchar *) ;
extern int	cfhexi(const char *,int,uint *) ;
extern int	cfdecui(const char *,int,uint *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int pcsunsfh(PCSUNSFH *ep,int f,char *hbuf,int hlen)
{
	uint		*header ;
	const int	magicsize = PCSUNSFH_MAGICSIZE ;
	int		rs = SR_OK ;
	int		headsize ;
	int		bl, cl ;
	const char	*magicstr = PCSUNSFH_MAGICSTR ;
	const char	*tp, *cp ;
	char		*bp ;

	if (ep == NULL) return SR_FAULT ;
	if (hbuf == NULL) return SR_FAULT ;

	bp = hbuf ;
	bl = hlen ;
	headsize = pcsunsfhh_overlast * sizeof(uint) ;
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

	            if (ep->vetu[0] != PCSUNSFH_VERSION)
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

	            ep->shmsize = header[pcsunsfhh_shmsize] ;
	            ep->dbsize = header[pcsunsfhh_dbsize] ;
	            ep->dbtime = header[pcsunsfhh_dbtime] ;
	            ep->wtime = header[pcsunsfhh_wtime] ;
	            ep->atime = header[pcsunsfhh_atime] ;
	            ep->acount = header[pcsunsfhh_acount] ;
	            ep->muoff = header[pcsunsfhh_muoff] ;
	            ep->musize = header[pcsunsfhh_musize] ;
	            ep->btoff = header[pcsunsfhh_btoff] ;
	            ep->btlen = header[pcsunsfhh_btlen] ;

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
	    bp[0] = PCSUNSFH_VERSION ;
	    bp[1] = ENDIAN ;
	    bp += 4 ;
	    bl -= 4 ;

	    if ((rs >= 0) && (bl >= headsize)) {

	        header = (uint *) bp ;

	        header[pcsunsfhh_shmsize] = ep->shmsize ;
	        header[pcsunsfhh_dbsize] = ep->dbsize ;
	        header[pcsunsfhh_dbtime] = ep->dbtime ;
	        header[pcsunsfhh_wtime] = ep->wtime ;
	        header[pcsunsfhh_atime] = ep->atime ;
	        header[pcsunsfhh_acount] = ep->acount ;
	        header[pcsunsfhh_muoff] = ep->muoff ;
	        header[pcsunsfhh_musize] = ep->musize ;
	        header[pcsunsfhh_btoff] = ep->btoff ;
	        header[pcsunsfhh_btlen] = ep->btlen ;

	        bp += headsize ;
	        bl -= headsize ;

	    } /* end if */

	} /* end if */

	return (rs >= 0) ? (bp - hbuf) : rs ;
}
/* end subroutine (pcsunsfh) */


