/* bprintline */

/* print out a single line */


/* revision history:

	= 2004-02-09, David A­D­ Morano
	This subroutine was originally written (due to laziness?).

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is very simple.  It just avoids calling 'bprintf(3b)'
	when that would have been completely fine! :-)

	Synopsis:

	int bprintline(fp,lbuf,llen)
	bfile		*fp ;
	const char	lbuf[] ;
	int		llen ;

	Arguments:

	fp		file-pointer
	lbuf		buffer of characters to print out
	llen		length of characters to print

	Returns:

	<0		error
	>=0		number of characters printed


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<ascii.h>
#include	<localmisc.h>


/* local defines */


/* exported subroutines */


int bprintln(bfile *fp,cchar *lbuf,int llen)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (fp == NULL) return SR_FAULT ;
	if (lbuf == NULL) return SR_FAULT ;

	if (fp->magic != BFILE_MAGIC) return SR_NOTOPEN ;

	if (! fp->f.nullfile) {
	    int		f_needeol ;

	    if (llen < 0) llen = strlen(lbuf) ;

	    f_needeol = ((llen == 0) || (lbuf[llen-1] != CH_NL)) ;
	    if (f_needeol) rs = breserve(fp,(llen+1)) ;

	    if ((rs >= 0) && (llen > 0)) {
	        rs = bwrite(fp,lbuf,llen) ;
	        wlen += rs ;
	    }

	    if ((rs >= 0) && f_needeol) {
	        rs = bputc(fp,CH_NL) ;
	        wlen += rs ;
	    }

	} /* end if (not nullfile) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (bprintln) */


int bprint(bfile *fp,cchar *lbuf,int llen)
{
	return bprintln(fp,lbuf,llen) ;
}
/* end subroutine (bprint) */


int bprintline(bfile *fp,cchar *lbuf,int llen)
{
	return bprintln(fp,lbuf,llen) ;
}
/* end subroutine (bprintline) */


