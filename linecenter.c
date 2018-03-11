/* linecenter */

/* text fill for line-centering */


#define	CF_DEBUGS	0		/* used for little object below */
#define	CF_SAFE		1		/* (some) safety */


/* revision history:

	= 2008-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object mediates filling out text to an output file using the BFILE
	package.


*******************************************************************************/


#define	LINECENTER_MASTER	0


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
#include	<ascii.h>
#include	<localmisc.h>

#include	"linecenter.h"


/* local defines */

#ifndef	WORDBUFLEN
#define	WORDBUFLEN	100
#endif


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,char **) ;


/* external variables */


/* local structures */


/* forward references */

int		linecenter_mklines(LINECENTER *,int,int) ;

static int	linecenter_mkline(LINECENTER *,int,char *,int) ;
static int	linecenter_storeline(LINECENTER *,const char *,int) ;
static int	linecenter_hasbrk(LINECENTER *,int,int) ;

static int	lenpercent(int,double) ;


/* local variables */


/* exported subroutines */


int linecenter_start(op,fn)
LINECENTER	*op ;
const char	fn[] ;
{
	const int	ne = LINECENTER_DEFLINES ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(LINECENTER)) ;

	if ((rs = sncpy1(op->fn,LINECENTER_FNLEN,fn)) >= 0) {
	    if ((rs = fifostr_start(&op->sq)) >= 0) {
	        int	size = (ne + 1) * sizeof(const char **) ;
	        if ((rs = uc_malloc(size,&op->lines)) >= 0) {
	            op->le = ne ;
	            op->magic = LINECENTER_MAGIC ;
	        }
	        if (rs < 0)
	            fifostr_finish(&op->sq) ;
	    } /* end if (fifostr_start) */
	} /* end if (sncpy) */

	return rs ;
}
/* end subroutine (linecenter_start) */


int linecenter_finish(op)
LINECENTER	*op ;
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (op == NULL)
	    return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != LINECENTER_MAGIC)
	    return SR_NOTOPEN ;
#endif

	if (op->lines != NULL) {
	    for (i = 0 ; i < op->li ; i += 1) {
	        if (op->lines[i] != NULL) {
	            rs1 = uc_free(op->lines[i]) ;
		    if (rs >= 0) rs = rs1 ;
		}
	    }
	    rs1 = uc_free(op->lines) ;
	    if (rs >= 0) rs = rs1 ;
	    op->lines = NULL ;
	} /* end if */

	rs1 = fifostr_finish(&op->sq) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (linecenter_finish) */


int linecenter_addword(op,lbuf,llen)
LINECENTER	*op ;
const char	lbuf[] ;
int		llen ;
{
	int		rs = SR_OK ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (lbuf == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != LINECENTER_MAGIC)
	    return SR_NOTOPEN ;
#endif

	if (llen < 0)
	    llen = strlen(lbuf) ;

	if (llen > 0) {
	    c += 1 ;
	    rs = fifostr_add(&op->sq,lbuf,llen) ;
	}

	if (rs >= 0) {
	    op->wc += 1 ;
	    op->cc += llen ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (linecenter_addword) */


int linecenter_addline(op,lbuf,llen)
LINECENTER	*op ;
const char	lbuf[] ;
int		llen ;
{
	int		rs = SR_OK ;
	int		sl, cl ;
	int		c = 0 ;
	const char	*sp, *cp ;

	if (op == NULL) return SR_FAULT ;
	if (lbuf == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != LINECENTER_MAGIC)
	    return SR_NOTOPEN ;
#endif

	if (llen < 0)
	    llen = strlen(lbuf) ;

	sp = lbuf ;
	sl = llen ;
	while ((cl = nextfield(sp,sl,&cp)) > 0) {

	    c += 1 ;
	    rs = fifostr_add(&op->sq,cp,cl) ;
	    if (rs < 0)
	        break ;

	    if (rs >= 0) {
	        op->wc += 1 ;
	        op->cc += cl ;
	    }

	    sl -= ((cp + cl) - sp) ;
	    sp = (cp + cl) ;

	} /* end while */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (linecenter_addline) */


int linecenter_addlines(op,lbuf,llen)
LINECENTER	*op ;
const char	lbuf[] ;
int		llen ;
{
	int		rs = SR_OK ;
	int		sl, cl ;
	int		c = 0 ;
	const char	*tp, *sp, *cp ;

	if (op == NULL) return SR_FAULT ;
	if (lbuf == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != LINECENTER_MAGIC)
	    return SR_NOTOPEN ;
#endif

	if (llen < 0)
	    llen = strlen(lbuf) ;

	sp = lbuf ;
	sl = llen ;
	while (sl > 0) {

	    while ((cl = nextfield(sp,sl,&cp)) > 0) {

	        c += 1 ;
	        rs = fifostr_add(&op->sq,cp,cl) ;
	        if (rs < 0)
	            break ;

	        if (rs >= 0) {
	            op->wc += 1 ;
	            op->cc += cl ;
	        }

	        sl -= ((cp + cl) - sp) ;
	        sp = (cp + cl) ;

	    } /* end while */

	    if (rs < 0)
	        break ;

	    if ((tp = strnchr(sp,sl,CH_NL)) == NULL)
	        break ;

	    sl -= ((tp + 1) - sp) ;
	    sp = (tp + 1) ;

	} /* end while (reading lines) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (linecenter_addlines) */


int linecenter_mklinefull(op,lbuf,llen)
LINECENTER	*op ;
char		lbuf[] ;
int		llen ;
{

#if	CF_DEBUGS
	debugprintf("linecenter_mklinefull: ent llen=%d\n",llen) ;
#endif
	return linecenter_mkline(op,FALSE,lbuf,llen) ;
}
/* end subroutine (linecenter_mklinefull) */


int linecenter_mklinepart(op,lbuf,llen)
LINECENTER	*op ;
char		lbuf[] ;
int		llen ;
{

#if	CF_DEBUGS
	debugprintf("linecenter_mklinepart: ent llen=%d\n",llen) ;
#endif
	return linecenter_mkline(op,TRUE,lbuf,llen) ;
}
/* end subroutine (linecenter_mklinepart) */


int linecenter_getline(op,i,lpp)
LINECENTER	*op ;
int		i ;
const char	**lpp ;
{
	int		rs = SR_OK ;
	int		len = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (lpp == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != LINECENTER_MAGIC)
	    return SR_NOTOPEN ;
#endif

	if (i < 0)
	    return SR_INVALID ;

	*lpp = NULL ;

	if (i < op->li) {
	    *lpp = op->lines[i] ;
	    len = strlen(*lpp) ;
	} else
	    rs = SR_NOTFOUND ;

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (linecenter_getline) */


int linecenter_mklines(op,llen,linebrklen)
LINECENTER	*op ;
int		llen ;
int		linebrklen ;
{
	int		rs = SR_OK ;
	int		size ;
	int		len ;
	int		linetmplen ;
	int		c = 0 ;
	char		*lbuf = NULL ;

#if	CF_DEBUGS
	debugprintf("linecenter_mklines: ent llen=%d lbrklen=%d\n",
	    llen,linebrklen) ;
#endif

	if (op == NULL)
	    return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != LINECENTER_MAGIC)
	    return SR_NOTOPEN ;
#endif

	if (llen < 1)
	    return SR_INVALID ;

	size = (llen + 2) ;
	if ((rs = uc_malloc(size,&lbuf)) >= 0) {

	    if (linebrklen <= 0)
	        linebrklen = lenpercent(llen,LINECENTER_DEFPERCENT) ;

	    while (rs >= 0) {
	        linetmplen = linecenter_hasbrk(op,llen,linebrklen) ;
	        if (linetmplen <= 0) linetmplen = llen ;
	        rs = linecenter_mklinefull(op,lbuf,linetmplen) ;
	        if (rs == 0) break ;
	        if (rs >= 0) {
	            len = rs ;
	            rs = linecenter_storeline(op,lbuf,len) ;
	            c += 1 ;
	        }
	    } /* end while */

	    while (rs >= 0) {
	        linetmplen = linecenter_hasbrk(op,llen,linebrklen) ;
	        if (linetmplen <= 0) linetmplen = llen ;
	        rs = linecenter_mklinepart(op,lbuf,linetmplen) ;
	        if (rs == 0) break ;
	        if (rs >= 0) {
	            len = rs ;
	            rs = linecenter_storeline(op,lbuf,len) ;
	            c += 1 ;
	        }
	    } /* end while */

	    uc_free(lbuf) ;
	} /* end if (m-a) */

#if	CF_DEBUGS
	debugprintf("linecenter_mklines: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (linecenter_mklines) */


/* private subroutines */


static int linecenter_storeline(op,lbuf,llen)
LINECENTER	*op ;
const char	lbuf[] ;
int		llen ;
{
	int		rs = SR_OK ;

	if (llen < 0)
	    return SR_INVALID ;

	if ((op->li + 1) >= op->le) {
	    int		size ;
	    int		ne = (op->le + 5) ;
	    void	**nlines = NULL ;
	    size = (ne + 1) * sizeof(char **) ;
	    if ((rs = uc_realloc(op->lines,size,&nlines)) >= 0) {
	        op->le = ne ;
	        op->lines = (const char **) nlines ;
	    }
	} /* end if */

	if (rs >= 0) {
	    const char	*cp ;
	    if ((rs = uc_mallocstrw(lbuf,llen,&cp)) >= 0) {
	        op->lines[op->li] = cp ;
	        op->li += 1 ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (linecenter_storeline) */


static int linecenter_mkline(op,f_part,lbuf,llen)
LINECENTER	*op ;
int		f_part ;
char		lbuf[] ;
int		llen ;
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		ll ;
	int		wl, nl ;
	int		c ;
	int		ql = 0 ;
	int		tlen = 0 ;
	int		f_give = f_part ;
	char		*lp ;

	if (op == NULL) return SR_FAULT ;
	if (lbuf == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != LINECENTER_MAGIC)
	    return SR_NOTOPEN ;
#endif

	if (llen < 1)
	    return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("linecenter_mkline: f_p=%u ll=%u\n",f_part,llen) ;
#endif

	if (! f_part) {

#ifdef	COMMENT
	    int	i ;


	    ql = 0 ;
	    c = 0 ;
	    for (i = 0 ; (wl = fifostr_entlen(&op->sq,i)) >= 0 ; i += 1) {
	        if (wl == 0) continue ;

	        if (c++ > 0)
	            ql += 1 ;

	        ql += wl ;
	        if (ql >= (llen - 1)) {
	            f_give = TRUE ;
	            break ;
	        }

	    } /* end for */
#else /* COMMENT */
	    ql = op->cc ;
	    if (op->wc > 0)
	        ql += (op->wc - 1) ;

	    if (ql >= (llen - 1))
	        f_give = TRUE ;
#endif /* COMMENT */

	} /* end if (not requested partial) */

	if (f_give) {

#if	CF_DEBUGS
	    debugprintf("linecenter_mkline: f_g=%u\n",f_give) ;
#endif

	    c = 0 ;
	    lp = lbuf ;
	    ll = llen ;
	    while (ll > 0) {

	        rs1 = fifostr_headlen(&op->sq) ;
	        wl = rs1 ;
	        if (rs1 == SR_NOTFOUND) break ;

/* ignore zero-length words */

	        if (wl == 0) continue ;

/* calculate needed-length ('nl') for this word */

	        nl = (c > 0) ? (wl + 1) : wl ;

/* can this word fit in the current line? */

	        if (nl > ll)
	            break ;

/* yes: so remove the word from the FIFO to the line */

	        if (c++ > 0) {
	            *lp++ = ' ' ;
	            ll -= 1 ;
	        }

	        rs = fifostr_remove(&op->sq,lp,ll) ;
	        if (rs < 0)
	            break ;

	        if (rs >= 0) {
	            op->wc -= 1 ;
	            op->cc -= wl ;
	        }

	        lp += wl ;
	        ll -= wl ;

	    } /* end while (getting words) */

	    tlen = (llen - ll) ;

	} /* end if (giving) */

#if	CF_DEBUGS
	debugprintf("linecenter_mkline: ret rs=%d tlen=%u\n",rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (linecenter_mkline) */


static int linecenter_hasbrk(op,llen,brklen)
LINECENTER	*op ;
int		llen ;
int		brklen ;
{
	int		rs = SR_OK ;
	int		i ;
	int		len ;
	int		wl ;
	int		curlen ;
	int		tmplen = 0 ;

	curlen = op->cc ;
	if (op->wc > 0)
	    curlen += (op->wc - 1) ;

	if (curlen > brklen) {
	    const int	wlen = WORDBUFLEN ;
	    const char	*wp ;
	    char	wbuf[WORDBUFLEN + 1] ;
	    wp = wbuf ;
	    len = 0 ;
	    for (i = 0 ; (wl = fifostr_entread(&op->sq,wbuf,wlen,i)) >= 0 ; 
	        i += 1) {

	    if (wl == 0) continue ;
	    len = wl ;

	    if (i > 0)
	        len += 1 ;

	    if (len >= llen)
	        break ;

	    if (len >= brklen) {

	        if ((wp[wl-1] == ',') || (wp[wl-1] == '.')) {
	            tmplen = len ;
	            break ;
	        }

	    } /* end if (break-length test) */

	} /* end for */
	} /* end if (greater) */

	return (rs >= 0) ? tmplen : rs ;
}
/* end subroutine (linecenter_hasbrk) */


static int lenpercent(len,percent)
int	len ;
double	percent ;
{
	double		flen ;
	int		rlen ;

	flen = len ;
	flen = flen * percent ;
	rlen = flen ;
	return rlen ;
}
/* end subroutine (lenpercent) */


