/* mailaddrquote */

/* subroutine to quote mail-addresses */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine examines a string supplied by the caller and produces an
        output string with any necessary quotes applied appropriate for Internet
        mail addresses.

	Synopsis:

	int mailaddrquote_start(op,ap,al,rpp)
	MAILDDRQUOTE	*op ;
	const char	ap[] ;
	int		al ;
	const char	**rpp ;

	Arguments:

	op		object pointer
	ap		raw mail-address to quote
	al		length of specified raw mail-address
	rpp		pointer to returned result

	Returns:

	>=0		length of used destination buffer from conversion
	<0		destination buffer was not big enough or other problem


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<ascii.h>
#include	<bufstr.h>
#include	<localmisc.h>

#include	"mailaddrquote.h"


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	fieldterms(uchar *,int,const char *) ;
extern int	haswhite(const char *,int) ;

extern char	*strnpbrk(const char *,int,const char *) ;


/* forward references */

static int	mailaddrquote_quote(MAILADDRQUOTE *,const uchar *,
			const char *,int) ;
static int	siterms(const uchar *,const char *,int) ;


/* local variables */


/* exported subroutines */


int mailaddrquote_start(op,abuf,alen,rpp)
MAILADDRQUOTE	*op ;
const char	abuf[] ;
int		alen ;
const char	**rpp ;
{
	int		rs = SR_OK ;
	int		len = 0 ;
	int		f_white ;
	uchar		qterms[32] ;
	const char	*qchars = "\"\\<>()" ;
	const char	*rp ;

	if (op == NULL) return SR_FAULT ;
	if (abuf == NULL) return SR_FAULT ;

	if (alen < 0)
	    alen = strlen(abuf) ;

#if	CF_DEBUGS
	debugprintf("mkquoted: ent a=>%t<\n",abuf,alen) ;
#endif

/* what are the SHELL characters that we should look out for? */

	fieldterms(qterms,0,qchars) ;

	f_white = haswhite(abuf,alen) ;

	rp = abuf ;
	len = alen ;
	if (f_white || (siterms(qterms,abuf,alen) >= 0)) {
	    if ((rs = mailaddrquote_quote(op,qterms,abuf,alen)) >= 0) {
	        rs = bufstr_get(&op->qaddr,&rp) ;
	        len = rs ;
	    }
	}

	if (rpp != NULL)
	    *rpp = (rs >= 0) ? rp : NULL ;

#if	CF_DEBUGS
	debugprintf("mkquoted: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mailaddrquote_start) */


int mailaddrquote_finish(op)
MAILADDRQUOTE	*op ;
{
	int		rs = SR_OK ;
	int		len = 0 ;

	if (op == NULL)
	    return SR_FAULT ;

	if (op->f.qaddr) {
	    op->f.qaddr = FALSE ;
	    len = bufstr_finish(&op->qaddr) ;
	    if (rs >= 0) rs = len ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mailaddrquote_finish) */


/* local subroutines */


static int mailaddrquote_quote(op,qterms,abuf,alen)
MAILADDRQUOTE	*op ;
const uchar	qterms[] ;
const char	abuf[] ;
int		alen  ;
{
	BUFSTR		*bsp = &op->qaddr ;
	int		rs ;
	int		i ;
	int		al ;
	const char	*ap ;

	if ((rs = bufstr_start(bsp)) >= 0) {
	    op->f.qaddr = TRUE ;
	    bufstr_char(bsp,CH_DQUOTE) ;
	    ap = abuf ;
	    al = alen ;
	    while ((rs >= 0) && ((i = siterms(qterms,ap,al)) >= 0)) {
	        bufstr_strw(bsp,ap,i) ;
	        bufstr_char(bsp,CH_BSLASH) ;
	        rs = bufstr_char(bsp,ap[i]) ;
	        ap += (i+1) ;
	        al -= (i+1) ;
	    } /* end while */
	    if ((rs >= 0) && (al > 0)) {
	        rs = bufstr_strw(bsp,ap,al) ;
	    }
	    if (rs >= 0) {
	        rs = bufstr_char(bsp,CH_DQUOTE) ;
	    }
	} /* end if (buffer_start) */

#if	CF_DEBUGS
	debugprintf("mkquoted_quote: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mkquoted_quote) */


/* find index into buffer starting at a terminating character */
static int siterms(qterms,sbuf,slen)
const uchar	qterms[] ;
const char	sbuf[] ;
int		slen ;
{
	int		i ;
	int		ch ;
	int		f = FALSE ;

	if (slen < 0)
	    slen = strlen(sbuf) ;

	for (i = 0 ; (i < slen) && sbuf[i] ; i += 1) {
	    ch = (sbuf[i] & 0xff) ;
	    f = BATST(qterms,ch)  ;
	    if (f) break ;
	} /* end for */

	return (f) ? i : -1 ;
}
/* end subroutine (siterms) */


