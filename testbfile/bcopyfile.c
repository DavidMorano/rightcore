/* bcopyfile */

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

	int bcopyfile(ifp,ofp,ubuf,ulen)
	bfile		*ifp, *ofp ;
	char		ubuf[] ;
	int		ulen ;

	Arguments:

	+ ifp		input file pointer to copy from
	+ ofp		output file pointer to copy to
	+ buf		buffer to use for the operation
	+ len		length of supplied buffer

	Returns:

	>=0		length of data copied or error return
	<0		error


*******************************************************************************/


#define	BFILE_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */


/* exported subroutines */


int bcopyfile(ifp,ofp,ubuf,ulen)
bfile		*ifp, *ofp ;
char		ubuf[] ;
int		ulen ;
{
	int	rs = SR_OK ;
	int	i, bl ;
	int	len ;
	int	blen ;
	int	tlen = 0 ;
	int	f_alloc = FALSE ;

	char	*buf = NULL ;


#if	CF_DEBUGS
	rs = 0 ;
	debugprintf("bcopyfile: entered blen=%d\n",ulen) ;
#endif

	if ((ifp == NULL) || (ofp == NULL))
	    return SR_FAULT ;

	blen = ulen ;
	if ((ubuf == NULL) || (ulen < 0)) {

#if	CF_DEBUGS
	    debugprintf("bcopyfile: alternate buffer\n") ;
#endif

	    blen = getpagesize() ;

	    rs = uc_valloc(blen,&buf) ;

	    if (rs < 0)
	        goto bad0 ;

	    f_alloc = TRUE ;

	} else
	    buf = ubuf ;

/* do it ! */

#if	CF_DEBUGS
	debugprintf("bcopyfile: before loop\n") ;
#endif

	while ((rs = bread(ifp,buf,blen)) > 0) {
	    len = rs ;

	    i = 0 ;
	    bl = len ;
	    while ((bl > 0) && 
	        ((rs = bwrite(ofp,(buf + i),bl)) < bl)) {

	        if (rs < 0)
	            goto done ;

	        i += rs ;
	        bl -= rs ;

	    } /* end while */

	    tlen += len ;

	    if (rs < 0) break ;
	} /* end while */

done:
	if (f_alloc && (buf != NULL)) {
	    uc_free(buf) ;
	    buf = NULL ;
	}

ret0:

#if	CF_DEBUGS
	debugprintf("bcopyfile: ret rs=%d tlen=%d\n",
	    rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;

/* bad stuff */
bad0:
	goto ret0 ;
}
/* end subroutine (bcopyfile) */



