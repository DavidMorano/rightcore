/* bprintline */

/* print out a single line */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_RESERVE	0		/* use |breserve()| */


/* revision history:

	= 2004-02-09, David A­D­ Morano

	This subroutine was originally written (due to laziness?).


*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is very simple.  It just avoids calling
	'bprintf(3b)' when that would have been completely fine! :-)

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

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;


/* exported subroutines */


#if	CF_RESERVE
int bprintline(fp,lbuf,llen)
bfile		*fp ;
const char	lbuf[] ;
int		llen ;
{
	int	rs = SR_OK ;
	int	wlen = 0 ;
	int	f_needeol ;


	if (fp == NULL) return SR_FAULT ;
	if (lbuf == NULL) return SR_FAULT ;
	if (fp->magic != BFILE_MAGIC) return SR_NOTOPEN ;

	if (! fp->f.nullfile) {

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
/* end subroutine (bprintline) */
#else /* CF_RESERVE */
int bprintline(fp,lbuf,llen)
bfile		*fp ;
const char	lbuf[] ;
int		llen ;
{
	int	rs = SR_OK ;
	int	wlen = 0 ;


	if (fp == NULL) return SR_FAULT ;
	if (lbuf == NULL) return SR_FAULT ;
	if (fp->magic != BFILE_MAGIC) return SR_NOTOPEN ;

	if (! fp->f.nullfile) {
	    int	f_needeol ;

	    if (llen < 0) llen = strlen(lbuf) ;

	    f_needeol = ((llen == 0) || (lbuf[llen-1] != CH_NL)) ;
	    if (f_needeol) {
	        const int	jlen = LINEBUFLEN ;
	        char		jbuf[LINEBUFLEN+1] ;
	        char		*jp ;
	        if ((llen+1) > jlen) {
	            const int	jsize = (llen+2) ;
	            char		*bp ;
	            if ((rs = uc_malloc(jsize,&bp)) >= 0) {
	                jp = bp ;
	                if (llen > 0) jp = strwcpy(jp,lbuf,llen) ;
	                *jp++ = CH_NL ;
	                rs = bwrite(fp,bp,(jp-bp)) ;
	                wlen += rs ;
	                uc_free(bp) ;
	            } /* end if (memory-allocation) */
	        } else {
		    jp = jbuf ;
	            if (llen > 0) jp = strwcpy(jp,lbuf,llen) ;
	            *jp++ = CH_NL ;
	            rs = bwrite(fp,jbuf,(jp-jbuf)) ;
	            wlen += rs ;
	        }
	    } else {
	        rs = bwrite(fp,lbuf,llen) ;
	        wlen += rs ;
	    }

	} /* end if (not nullfile) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (bprintline) */
#endif /* CF_RESERVE */


