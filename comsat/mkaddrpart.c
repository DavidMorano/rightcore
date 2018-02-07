/* mkdisphdr */

/* create (in a buffer) a sort of nice mail address for display purposes */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_NONSTANDARD	1		/* assume non-standard addressing */


/* revision history:

	= 2005-06-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2005 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine creates (in a buffer) a sort of nice mail address for
	display purposes.

	Synopsis:

	int mkdisphdr(mbuf,mlen,ap,al)
	char		mbuf[] ;
	int		mlen ;
	const char	*ap ;
	int		al ;

	Arguments:

	mbuf		buffer to hold result
	mlen		length of given buffer
	ap		argument
	al		argument-length

	Returns:

	>=0		length of created result
	<0		error (generally only memory allocation failure)


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<field.h>
#include	<sbuf.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

#if	CF_DEBUGS
static int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* exported subroutines */


int mkdisphdr(char *abuf,int alen,cchar *sp,int sl)
{
	SBUF		b ;
	int		rs ;
	int		len = 0 ;

	if (abuf == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (sl < 0) sl = strlen(sp) ;

#if	CF_DEBUGS
	debugprintf("mkdisphdr: src=>%t<\n",
	    sp,strlinelen(sp,sl,60)) ;
#endif

	if ((rs = sbuf_start(&b,abuf,alen)) >= 0) {
	    FIELD	fsb ;
	    const int	flen = sl ;
	    char	*fbuf ;
	    if ((rs = uc_malloc((flen+1),&fbuf)) >= 0) {
	        if ((rs = field_start(&fsb,sp,sl)) >= 0) {
	            cchar	*fp = fbuf ;
	            int		fl ;
	            int		c = 0 ;
	            while ((fl = field_sharg(&fsb,NULL,fbuf,flen)) >= 0) {
	                if (fl > 0) {
#if	CF_NONSTANDARD
	                    if (c++ > 0) {
	                        rs = sbuf_char(&b,' ') ;
			    }
#endif
	                    if (rs >= 0) {
	                        rs = sbuf_strw(&b,fp,fl) ;
			    }
			}
	                if (rs < 0) break ;
	            } /* end while */
	            field_finish(&fsb) ;
	        } /* end if (field) */
	        uc_free(fbuf) ;
	    } /* end if (memory allocation) */
	    len = sbuf_finish(&b) ;
	    if (rs >= 0) rs = len ;
	} /* end if (sbuf) */

#if	CF_DEBUGS
	debugprintf("mkdisphdr: ret rs=%d len=%u\n",rs,len) ;
	debugprintf("mkdisphdr: abuf=>%t<\n",abuf,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mkdisphdr) */


int mkaddrpart(char *abuf,int alen,cchar *sp,int sl)
{
	return mkdisphdr(abuf,alen,sp,sl) ;
}
/* end subroutine (mkaddrpart) */


int mkdispaddr(char *abuf,int alen,cchar *sp,int sl)
{
	return mkdisphdr(abuf,alen,sp,sl) ;
}
/* end subroutine (mkdispaddr) */


