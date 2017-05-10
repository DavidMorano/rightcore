/* bwriteblock */

/* copy a file to another file */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine copies the remainder of the input file
	to the output file.

	Synospsis:

	int bwriteblock(ofp,ifp,ulen)
	bfile		*ifp, *ofp ;
	int		ulen ;

	Arguments:

	ofp		output file pointer to copy to
	ifp		input file pointer to copy from
	ulen		length of supplied buffer

	Returns:

	>=0		length of data copied or error return
	<0		error


*******************************************************************************/

#define	BFILE_MASTER	0

#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */

#define	BFILE_NPAGES	4


/* external subroutines */


/* exported subroutines */


int bwriteblock(ofp,ifp,ulen)
bfile		*ofp ;
bfile		*ifp ;
int		ulen ;
{
	int	rs = SR_OK ;
	int	wlen = 0 ;

#if	CF_DEBUGS
	debugprintf("bwriteblock: ent ulen=%d\n",ulen) ;
#endif

	if (ofp == NULL) return SR_FAULT ;
	if (ifp == NULL) return SR_FAULT ;
	if (ofp->magic != BFILE_MAGIC) return SR_NOTOPEN ;
	if (ifp->magic != BFILE_MAGIC) return SR_NOTOPEN ;

	if ((! ofp->f.nullfile) && (ulen != 0)) {
	    int		bsize = (ofp->pagesize * BFILE_NPAGES) ;
	    char	*buf ;
	    if (ulen >= 0) {
		if (ulen < bsize) bsize = ulen ;
	    } else
	        ulen = INT_MAX ;
	    if ((rs = uc_valloc(bsize,&buf)) >= 0) {
	        int	bl, len ;
	        int	mlen ;
		int	i ;

	        while (wlen < ulen) {

	            mlen = MIN((ulen - wlen),bsize) ;
	            rs = bread(ifp,buf,mlen) ;
	            len = rs ;
	            if (rs <= 0)
	                break ;

	            i = 0 ;
	            bl = len ;
	            while ((bl > 0) && ((rs = bwrite(ofp,(buf+i),bl)) < bl)) {
	                i += rs ;
	                bl -= rs ;
	                if (rs < 0) break ;
	            } /* end while */

	            wlen += len ;
	            if (rs < 0) break ;
	        } /* end while */

	        uc_free(buf) ;
	    } /* end if (memory-allocation) */
	} /* end if (not nullfile) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (bwriteblock) */



