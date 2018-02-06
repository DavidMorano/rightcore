/* wordfill */

/* text (word) fill */


#define	CF_DEBUGS	0		/* used for little object below */
#define	CF_SAFE		1		/* (some) safety */


/* revision history:

	= 1999-03-04, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object mediates filling out text lines for a specified line length.


*******************************************************************************/


#define	WORDFILL_MASTER	0


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<fifostr.h>
#include	<localmisc.h>

#include	"wordfill.h"


/* local defines */

#ifndef	WORDBUFLEN
#define	WORDBUFLEN	100
#endif


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,char **) ;


/* external variables */


/* local structures */


/* forward references */

static int	wordfill_mkline(WORDFILL *,int,char *,int) ;

int		wordfill_addlines(WORDFILL *,const char *,int) ;


/* local variables */


/* exported subroutines */


int wordfill_start(WORDFILL *op,cchar *lp,int ll)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(WORDFILL)) ;

	if ((rs = fifostr_start(&op->sq)) >= 0) {
	    op->magic = WORDFILL_MAGIC ;
	    if (lp != NULL) {
	        rs = wordfill_addlines(op,lp,ll) ;
	    }
	    if (rs < 0) {
		fifostr_finish(&op->sq) ;
	        op->magic = 0 ;
	    }
	} /* end if (fifostr-start) */

	return rs ;
}
/* end subroutine (wordfill_start) */


int wordfill_finish(WORDFILL *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != WORDFILL_MAGIC) return SR_NOTOPEN ;
#endif

	rs1 = fifostr_finish(&op->sq) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (wordfill_finish) */


int wordfill_addword(WORDFILL *op,cchar *lbuf,int llen)
{
	int		rs = SR_OK ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (lbuf == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != WORDFILL_MAGIC) return SR_NOTOPEN ;
#endif

	if (llen < 0)
	    llen = strlen(lbuf) ;

	if (llen > 0) {
	    c += 1 ;
	    if ((rs = fifostr_add(&op->sq,lbuf,llen)) >= 0) {
	        op->wc += 1 ;
	        op->cc += llen ;
	    }
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (wordfill_addword) */


int wordfill_addline(WORDFILL *op,cchar *lbuf,int llen)
{
	int		rs = SR_OK ;
	int		sl, cl ;
	int		c = 0 ;
	const char	*sp, *cp ;

	if (op == NULL) return SR_FAULT ;
	if (lbuf == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != WORDFILL_MAGIC) return SR_NOTOPEN ;
#endif

	if (llen < 0)
	    llen = strlen(lbuf) ;

	sp = lbuf ;
	sl = llen ;
	while ((cl = nextfield(sp,sl,&cp)) > 0) {

	    c += 1 ;
	    rs = fifostr_add(&op->sq,cp,cl) ;

	    op->wc += 1 ;
	    op->cc += cl ;

	    sl -= ((cp + cl) - sp) ;
	    sp = (cp + cl) ;

	    if (rs < 0) break ;
	} /* end while */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (wordfill_addline) */


int wordfill_addlines(WORDFILL *op,cchar *lbuf,int llen)
{
	int		rs = SR_OK ;
	int		sl, cl ;
	int		c = 0 ;
	const char	*tp, *sp, *cp ;

	if (op == NULL) return SR_FAULT ;
	if (lbuf == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != WORDFILL_MAGIC) return SR_NOTOPEN ;
#endif

	if (llen < 0)
	    llen = strlen(lbuf) ;

	sp = lbuf ;
	sl = llen ;
	while (sl > 0) {

	    while ((cl = nextfield(sp,sl,&cp)) > 0) {

	        c += 1 ;
	        rs = fifostr_add(&op->sq,cp,cl) ;

		op->wc += 1 ;
		op->cc += cl ;

	        sl -= ((cp + cl) - sp) ;
	        sp = (cp + cl) ;

	        if (rs < 0) break ;
	    } /* end while */

	    if ((tp = strnchr(sp,sl,'\n')) == NULL)
	        break ;

	    sl -= ((tp + 1) - sp) ;
	    sp = (tp + 1) ;

	    if (rs < 0) break ;
	} /* end while */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (wordfill_addlines) */


int wordfill_mklinefull(WORDFILL *op,char *lbuf,int llen)
{

	return wordfill_mkline(op,FALSE,lbuf,llen) ;
}
/* end subroutine (wordfill_mklinefull) */


int wordfill_mklinepart(WORDFILL *op,char *lbuf,int llen)
{

	return wordfill_mkline(op,TRUE,lbuf,llen) ;
}
/* end subroutine (wordfill_mklinepart) */


/* private subroutines */


static int wordfill_mkline(WORDFILL *op,int f_part,char *lbuf,int llen)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		ll ;
	int		wl, nl ;
	int		ql ;
	int		c ;
	int		f_give = f_part ;
	int		tlen = 0 ;
	char		*lp ;			/* pointer to write to */

	if (op == NULL) return SR_FAULT ;
	if (lbuf == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != WORDFILL_MAGIC) return SR_NOTOPEN ;
#endif

	if (llen < 1) return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("wordfill_mkline: f_p=%u ll=%u\n",f_part,llen) ;
#endif

	if (! f_part) {

#ifdef	COMMENT
	    {
	        int	i = 0 ;
	        ql = 0 ;
	        c = 0 ;
	        while ((wl = fifostr_entlen(&op->sq,i)) >= 0) {
	            if (wl >  0) {
	                if (c++ > 0) ql += 1 ;
	                ql += wl ; 
		        if (ql >= (llen - 1)) {
	                    f_give = TRUE ; break ;
	                }
		    } /* end if (greater-than-zero) */
		    i += 1 ;
	        } /* end while */
	    } /* end block */
#else /* COMMENT */
	    {
	        ql = op->cc ;
	        if (op->wc > 0)
		    ql += (op->wc - 1) ;

	        if (ql >= (llen - 1))
	            f_give = TRUE ;
	    } /* end block */
#endif /* COMMENT */

	} /* end if (not requested partial) */

#if	CF_DEBUGS
	debugprintf("wordfill_mkline: f_g=%u\n",f_give) ;
#endif

	if (f_give) {

	    c = 0 ;
	    lp = lbuf ;
	    ll = llen ;
	    while (ll > 0) {

	        rs1 = fifostr_headlen(&op->sq) ;
	        wl = rs1 ;

#if	CF_DEBUGS
	debugprintf("wordfill_mkline: fifostr_headlen() rs=%d\n",rs1) ;
#endif

	        if (rs1 == SR_NOTFOUND)
	            break ;

/* ignore zero-length words */

	        if (wl == 0) continue ;

/* calculate needed-length ('nl') for this word */

	        nl = (c > 0) ? (wl + 1) : wl ;

#if	CF_DEBUGS
	debugprintf("wordfill_mkline: nl=%u ll=%u\n",nl,ll) ;
#endif

/* can this word fit in the current line? */

	        if (nl > ll)
	            break ;

/* yes: so remove the word from the FIFO to the line */

	        if (c++ > 0) {
	            *lp++ = ' ' ;
	            ll -= 1 ;
	        }

	        rs = fifostr_remove(&op->sq,lp,ll) ;

#if	CF_DEBUGS
	debugprintf("wordfill_mkline: fifostr_remove() rs=%d\n",rs) ;
	if (rs >= 0)
	debugprintf("wordfill_mkline: w=>%t<\n",lp,rs) ;
#endif

		if (rs >= 0) {
		    op->wc -= 1 ;
		    op->cc -= wl ;
		}

	        lp += wl ;
	        ll -= wl ;

	        if (rs < 0) break ;
	    } /* end while (getting words) */

	    tlen = (llen - ll) ;

	} /* end if (giving) */

#if	CF_DEBUGS
	debugprintf("wordfill_mkline: ret rs=%d tlen=%u\n",rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (wordfill_mkline) */


