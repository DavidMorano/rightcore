/* mkquoted */

/* subroutine to quote arguments for safe passage through a SHELL */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine examines a string supplied by the caller and produces
	an output string with any necessary quotes applied such that
	interpretation by a POSIX-conforming SHELL will result in the original
	string.

	Synopsis:

	int mkquoted(qbuf,qlen,abuf,alen)
	const char	abuf[] ;
	char		qbuf[] ;
	int		alen, qlen ;

	Arguments:

	abuf		shell argument to be quoted
	alen		length of shell argument to be quoted
	qbuf		buffer to place result in
	qlen		length of user supplied buffer for result

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
#include	<sbuf.h>
#include	<baops.h>
#include	<ascii.h>
#include	<localmisc.h>


/* external subroutines */

extern int	snwcpy(char *,int,cchar *,int) ;
extern int	fieldterms(uchar *,int,cchar *) ;
extern int	haswhite(cchar *,int) ;

extern char	*strnpbrk(const char *,int,const char *) ;


/* forward references */

static int	mkquoted_quote(cuchar *,int,char *,int,cchar *,int) ;
static int	siterms(const uchar *,const char *,int) ;


/* local variables */


/* exported subroutines */


int mkquoted(char *qbuf,int qlen,cchar *abuf,int alen)
{
	int		rs = SR_OK ;
	int		f_white ;
	const char	*qchars = "\"\\$`" ;
	uchar		qterms[32] ;

	if (abuf == NULL) return SR_FAULT ;
	if (qbuf == NULL) return SR_FAULT ;

	if (alen < 0)
	    alen = strlen(abuf) ;

#if	CF_DEBUGS
	debugprintf("mkquoted: ent a=>%t<\n",abuf,alen) ;
#endif

/* what are the SHELL characters that we should look out for? */

	fieldterms(qterms,0,qchars) ;

	f_white = haswhite(abuf,alen) ;

	if (f_white || (siterms(qterms,abuf,alen) >= 0)) {
	    rs = mkquoted_quote(qterms,f_white,qbuf,qlen,abuf,alen) ;
	} else {
	    rs = snwcpy(qbuf,qlen,abuf,alen) ;
	}

#if	CF_DEBUGS
	debugprintf("mkquoted: ret rs=%d q=>%t<\n",rs,
	    qbuf,MAX(0,rs)) ;
#endif

	return rs ;
}
/* end subroutine (mkquoted) */


/* local subroutines */


static int mkquoted_quote(qterms,f_allquote,qbuf,qlen,abuf,alen)
const uchar	qterms[] ;
int		f_allquote ;
char		qbuf[] ;
const char	abuf[] ;
int		qlen, alen ;
{
	SBUF		b ;
	int		rs ;
	int		al ;
	int		len = 0 ;
	const char	*ap ;

	ap = abuf ;
	al = alen ;
	if ((rs = sbuf_start(&b,qbuf,qlen)) >= 0) {
	    int		i ;

	    if (f_allquote)
	        sbuf_char(&b,CH_DQUOTE) ;

	    while ((i = siterms(qterms,ap,al)) >= 0) {

#if	CF_DEBUGS
	        debugprintf("mkquoted: got one at %u >%c<\n",i,MKCHAR(ap[i])) ;
#endif

	        sbuf_strw(&b,ap,i) ;
	        sbuf_char(&b,CH_BSLASH) ;
	        rs = sbuf_char(&b,ap[i]) ;

	        ap += (i+1) ;
	        al -= (i+1) ;

	        if (rs < 0) break ;
	    } /* end while */

	    if ((rs >= 0) && (al > 0)) {
	        rs = sbuf_strw(&b,ap,al) ;
	    }

	    if ((rs >= 0) && f_allquote) {
	        rs = sbuf_char(&b,CH_DQUOTE) ;
	    }

	    len = sbuf_finish(&b) ;
	    if (rs >= 0) rs = len ;
	} /* end if (sbuf) */

#if	CF_DEBUGS
	debugprintf("mkquoted_quote: ret rs=%d len=%d\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mkquoted_quote) */


/* find index into buffer starting at a terminating character */
static int siterms(const uchar *qterms,cchar *sbuf,int slen)
{
	int		i ;
	int		ch ;
	int		f = FALSE ;

	if (slen < 0)
	    slen = strlen(sbuf) ;

	for (i = 0 ; (i < slen) && sbuf[i] ; i += 1) {
	    ch = MKCHAR(sbuf[i]) ;
	    f = BATST(qterms,ch)  ;
	    if (f) break ;
	} /* end for */

	return (f) ? i : -1 ;
}
/* end subroutine (siterms) */


