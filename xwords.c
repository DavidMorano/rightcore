/* xwords */

/* extract extra words from a single given word */


#define	CF_DEBUGS 	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object extracts extra possible words from a single given word.
	The given word is always returned (as one extraction) but additional
	subwords may also be returned.


*******************************************************************************/


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecobj.h>
#include	<localmisc.h>

#include	"xwords.h"


/* local defines */

#define	XWORDS_WI	struct xwords_wi


/* external subroutines */

extern int	sichr(const char *,int,int) ;
extern int	isprintlatin(int) ;
extern int	isalnumlatin(int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */

struct xwords_wi {
	const char	*wp ;
	int		wl ;
} ;


/* forward references */

static int	xwords_more(XWORDS *,const char *,int,int) ;


/* local variables */


/* exported subroutines */


int xwords_start(XWORDS *op,cchar *wbuf,int wlen)
{
	int		rs = SR_OK ;
	int		si ;
	int		el ;
	int		i = 0 ;

	memset(op,0,sizeof(XWORDS)) ;

/* always enter the whole word */

	op->words[i].wp = wbuf ;
	op->words[i].wl = wlen ;
	i += 1 ;

/* try for possessives */

	if (wlen > 2) {
	    int		f = FALSE ;

	    el = wlen - 2 ;
	    if (strncmp((wbuf + el),"'s",2) == 0) {
	        f = TRUE ;
	    } else if (strncmp((wbuf + el),"s'",2) == 0) {
	        f = TRUE ;
	        el += 1 ;
	    }

	    if (f) {
	        op->words[i].wp = wbuf ;
	        op->words[i].wl = el ;
	        i += 1 ;
	    }

	} /* end if (long enough for extra words) */

	op->nwords = i ;
	if ((si = sichr(wbuf,wlen,'-')) >= 0) {
	    rs = xwords_more(op,wbuf,wlen,si) ;
	    i = rs ;
	    op->nwords = rs ;
	}

/* done */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (xwords_start) */


int xwords_get(XWORDS *op,int i,cchar **rpp)
{
	XWORDS_WORD	*w = op->words ;
	int		wl = 0 ;

	if (i < 0) return SR_INVALID ;

	if (op->xa != NULL) w = op->xa ;

#if	CF_DEBUGS && 0
	{
		int	j ;
		debugprintf("xwords_get: i=%d\n",i) ;
		for (j = 0 ; j < op->nwords ; j += 1)
		debugprintf("xwords_get: w=%t\n",w[j].wp,w[j].wl) ;
	}
#endif /* CF_DEBUGS */

	if ((i < op->nwords) && (w[i].wp != NULL))
	    wl = w[i].wl ;

	if (rpp != NULL)
	    *rpp = (wl > 0) ? w[i].wp : NULL ;

	return wl ;
}
/* end subroutine (xwords_get) */


int xwords_finish(XWORDS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->xa != NULL) {
	    rs1 = uc_free(op->xa) ;
	    if (rs >= 0) rs = rs1 ;
	    op->xa = NULL ;
	}

	op->nwords = 0 ;
	return rs ;
}
/* end subroutine (xwords_finish) */


/* private subroutines */


static int xwords_more(XWORDS *op,cchar wbuf[],int wlen,int si)
{
	vecobj		wil ;
	const int	esize = sizeof(XWORDS_WI) ;
	int		rs ;
	int		rs1 ;
	int		n = 0 ;

	if ((rs = vecobj_start(&wil,esize,2,0)) >= 0) {
	    XWORDS_WI	wi ;
	    wi.wp = wbuf ;
	    wi.wl = si ;
	    if ((rs = vecobj_add(&wil,&wi)) >= 0) {
		int	wl = (wlen-(si+1)) ;
		cchar	*wp = (wbuf+(si+1)) ;
		while ((si = sichr(wp,wl,'-')) >= 0) {
		    if (si > 0) {
	    	        wi.wp = wp ;
	    	        wi.wl = si ;
	    	        rs = vecobj_add(&wil,&wi) ;
		    }
		    wl -= (si+1) ;
		    wp += (si+1) ;
		    if (rs < 0) break ;
		} /* end while */
		if ((rs >= 0) && (wl > 0)) {
	    	    wi.wp = wp ;
	    	    wi.wl = wl ;
	    	    rs = vecobj_add(&wil,&wi) ;
		}
		if (rs >= 0) {
		    XWORDS_WI	*ep ;
		    int		i, j ;
		    n = op->nwords + vecobj_count(&wil) ;
		    if (n > XWORDS_MAX) {
		        const int	size = (n * esize) ;
		        void		*p ;
		        if ((rs = uc_malloc(size,&p)) >= 0) {
			    op->xa = p ;
			    for (j = 0 ; j < op->nwords ; j += 1) {
			        op->xa[j].wp = op->words[j].wp ;
			        op->xa[j].wl = op->words[j].wl ;
			    }
			    for (i = 0 ; vecobj_get(&wil,i,&ep) >= 0 ; i += 1) {
			        op->xa[j].wp = ep->wp ;
			        op->xa[j].wl = ep->wl ;
			        j += 1 ;
			    } /* end for */
		        } /* end if (memory-allocation) */
		    } else {
			j = op->nwords ;
			for (i = 0 ; vecobj_get(&wil,i,&ep) >= 0 ; i += 1) {
			    op->words[j].wp = ep->wp ;
			    op->words[j].wl = ep->wl ;
			    j += 1 ;
			} /* end for */
		    }
		} /* end if (ok) */
	    } /* end if (add first word) */
	    rs1 = vecobj_finish(&wil) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecobj) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (xwords_more) */


