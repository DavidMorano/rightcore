/* bwriteblock */

/* copy a file to another file */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/************************************************************************

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


*************************************************************************/


#define	BFILE_MASTER	0


#undef	bcopy

#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */

#undef	bcopy

#define	BFILE_NPAGES	4


/* external subroutines */


/* exported subroutines */


int bwriteblock(ofp,ifp,ulen)
bfile		*ifp, *ofp ;
int		ulen ;
{
	const int	pagesize = getpagesize() ;

	int	rs = SR_OK ;
	int	i, bl, len ;
	int	wlen ;
	int	mlen, blen ;
	int	tlen = 0 ;
	int	f_alloc = FALSE ;

	char	*buf = NULL ;


#if	CF_DEBUGS
	debugprintf("bwriteblock: entered blen=%d\n",ulen) ;
#endif

	if (ifp == NULL)
	    return SR_FAULT ;

	if (ifp->magic != BFILE_MAGIC)
	    return SR_NOTOPEN ;

	if (ifp->f.nullfile) goto ret0 ;

	if (ofp == NULL)
	    return SR_FAULT ;

	if (ofp->magic != BFILE_MAGIC)
	    return SR_NOTOPEN ;

	if (ofp->f.nullfile) goto ret0 ;

/* start in */

	if (ulen < 0)
	    ulen = INT_MAX ;

	blen = pagesize * BFILE_NPAGES ;

	rs = uc_valloc(blen,&buf) ;
	if (rs < 0)
	    goto ret0 ;

	f_alloc = TRUE ;

/* do it! */

#if	CF_DEBUGS
	debugprintf("bwriteblock: before loop\n") ;
#endif

	tlen = 0 ;
	while (tlen < ulen) {

	    mlen = MIN((ulen - tlen),blen) ;
	    rs = bread(ifp,buf,mlen) ;
	    len = rs ;
	    if (rs <= 0)
	        break ;

	    i = 0 ;
	    bl = len ;
	    while ((bl > 0) && ((rs = bwrite(ofp,(buf + i),bl)) < bl)) {
		wlen = rs ;

	        i += wlen ;
	        bl -= wlen ;

	        if (rs < 0) break ;
	    } /* end while */

	    tlen += len ;
	    if (rs < 0) break ;
	} /* end while */

#if	CF_DEBUGS
	debugprintf("bwriteblock: ret rs=%d tlen=%d\n",
	    rs,tlen) ;
#endif

done:
	if (f_alloc && (buf != NULL))
	    uc_free(buf) ;

ret0:
	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (bwriteblock) */



