/* babiesfu */

/* header management for BABIES shared-memory segment */


#define	CF_DEBUGS 	0		/* run-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine reads and writes the BABIES shared-memory segment
	header.

	Synopsis:

	int babiesfu(ep,f,hbuf,hlen)
	BABIESFU	*ep ;
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

#include	"babiesfu.h"


/* local defines */


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkmagic(char *,const char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	cfhexi(const char *,int,uint *) ;
extern int	cfdecui(const char *,int,uint *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int babiesfu(BABIESFU *ep,int f,char *hbuf,int hlen)
{
	uint		*header ;
	const int	magicsize = BABIESFU_MAGICSIZE ;
	int		rs = SR_OK ;
	int		headsize ;
	int		bl, cl ;
	const char	*magicstr = BABIESFU_MAGICSTR ;
	const char	*tp, *cp ;
	char		*bp ;

	if (ep == NULL)
	    return SR_FAULT ;

	if (hbuf == NULL)
	    return SR_FAULT ;

	bp = hbuf ;
	bl = hlen ;
	headsize = babiesfuh_overlast * sizeof(uint) ;
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

	            if (ep->vetu[0] != BABIESFU_VERSION)
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

	            ep->shmsize = header[babiesfuh_shmsize] ;
	            ep->dbsize = header[babiesfuh_dbsize] ;
	            ep->dbtime = header[babiesfuh_dbtime] ;
	            ep->wtime = header[babiesfuh_wtime] ;
	            ep->atime = header[babiesfuh_atime] ;
	            ep->acount = header[babiesfuh_acount] ;
	            ep->muoff = header[babiesfuh_muoff] ;
	            ep->musize = header[babiesfuh_musize] ;
	            ep->btoff = header[babiesfuh_btoff] ;
	            ep->btlen = header[babiesfuh_btlen] ;

	            bp += headsize ;
	            bl -= headsize ;

	        } else
	            rs = SR_ILSEQ ;

	    } /* end if (item) */

	} else { /* write */

	    mkmagic(bp,magicstr,magicsize) ;
	    bp += magicsize ;
	    bl -= magicsize ;

	    memcpy(bp,ep->vetu,4) ;
	    bp[0] = BABIESFU_VERSION ;
	    bp[1] = ENDIAN ;
	    bp += 4 ;
	    bl -= 4 ;

	    if ((rs >= 0) && (bl >= headsize)) {

	        header = (uint *) bp ;

	        header[babiesfuh_shmsize] = ep->shmsize ;
	        header[babiesfuh_dbsize] = ep->dbsize ;
	        header[babiesfuh_dbtime] = ep->dbtime ;
	        header[babiesfuh_wtime] = ep->wtime ;
	        header[babiesfuh_atime] = ep->atime ;
	        header[babiesfuh_acount] = ep->acount ;
	        header[babiesfuh_muoff] = ep->muoff ;
	        header[babiesfuh_musize] = ep->musize ;
	        header[babiesfuh_btoff] = ep->btoff ;
	        header[babiesfuh_btlen] = ep->btlen ;

	        bp += headsize ;
	        bl -= headsize ;

	    } /* end if */

	} /* end if */

	return (rs >= 0) ? (bp - hbuf) : rs ;
}
/* end subroutine (babiesfu) */



