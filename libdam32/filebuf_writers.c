/* filebuf_writers */

/* extra write methods for the FILEBUF object */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Name:

	filebuf_writeblanks

	Description:

	Write a specified number of blanks to a FILEBUF object.

	Synopsis:

	int filebuf_writeblanks(FILEBUF *fbp,int n)

	Arguments:

	fbp		pointer to object
	n		number of bytes to write

	Returns:

	<0		error
	>=0		number of bytes written


	Name:

	filebuf_writefill

	Description:

	Write enough data (bytes) to fill something.

	Synopsis:

	int filebuf_writefill(bp,sp,sl)
	FILEBUF		*bp ;
	const char	sp[] ;
	int		sl ;

	Arguments:

	bp		FILEBUF object pointer
	sp		source buffer
	sl		source buffer length

	Returns:

	<0		error
	>=0		number of bytes written


	Name:

	filebuf_writealign

	Description:

        Align the file-pointer to the specified alignment (zero-filling as
        needed).

	Synopsis:

	int filebuf_writealign(bp,align)
	FILEBUF		*bp ;
	int		align ;

	Arguments:

	bp		FILEBUF object pointer
	align		source buffer length

	Returns:

	<0		error
	>=0		number of bytes written


	Name:

	filebuf_writezero

	Description:

	We provide some extra small function for special circumstances.

	Synopsis:

	int filebuf_writezero(bp,size)
	FILEBUF		*bp ;
	int		size ;

	Arguments:

	bp		FILEBUF object pointer
	size		amount of zeros to write

	Returns:

	<0		error
	>=0		number of bytes written


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<filebuf.h>
#include	<localmisc.h>


/* local defines */

#undef	NBLANKS
#define	NBLANKS		8


/* external subroutines */

int		filebuf_writealign(FILEBUF *,int) ;
int		filebuf_writezero(FILEBUF *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* local structures */


/* forward references */

int		filebuf_writealign(FILEBUF *,int) ;
int		filebuf_writezero(FILEBUF *,int) ;


/* local variables */

static const char	blanks[NBLANKS] = "        " ;

static cchar	zerobuf[4] = {
	0, 0, 0, 0 
} ;


/* exported subroutines */


int filebuf_writeblanks(FILEBUF *fbp,int n)
{
	int		rs = SR_OK ;
	int		ml ;
	int		wlen = 0 ;

	while ((rs >= 0) && (wlen < n)) {
	    ml = MIN((n-wlen),NBLANKS) ;
	    rs = filebuf_write(fbp,blanks,ml) ;
	    wlen += rs ;
	} /* end while */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (filebuf_writeblanks) */


int filebuf_writefill(FILEBUF *bp,cchar *sp,int sl)
{
	const int	asize = sizeof(int) ;
	int		rs ;
	int		wlen = 0 ;

	if (sl < 0) sl = (strlen(sp) + 1) ;

	if ((rs = filebuf_write(bp,sp,sl)) >= 0) {
	    wlen = rs ;
	    rs = filebuf_writealign(bp,asize) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (filebuf_writefill) */


int filebuf_writealign(FILEBUF *bp,int asize)
{
	offset_t	foff ;
	int		rs ;
	int		wlen = 0 ;

	if ((rs = filebuf_tell(bp,&foff)) >= 0) {
	    int	r = (int) (foff & (asize - 1)) ;
	    if (r > 0) {
	        const int	nzero = (asize - r) ;
	        if (nzero > 0) {
	            rs = filebuf_writezero(bp,nzero) ;
	            wlen += rs ;
	        }
	    }
	} /* end if (filebuf_tell) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (filebuf_writeallign) */


int filebuf_writezero(FILEBUF *fp,int size)
{
	int		rs = SR_OK ;
	int		ml ;
	int		rlen = size ;
	int		wlen = 0 ;

	while ((rs >= 0) && (rlen > 0)) {
	    ml = MIN(rlen,4) ;
	    rs = filebuf_write(fp,zerobuf,ml) ;
	    rlen -= rs ;
	    wlen += rs ;
	} /* end while */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (filebuf_writezero) */


