/* bfliner */

/* BFILE-liner */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_BFLINERADV	0		/* 'bfliner_adv()' ? */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object provider a way to read lines w/ a push-back feature. This is
        used for reading raw mail-message files for analysis.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<localmisc.h>

#include	"bfliner.h"


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	hasuc(const char *,int) ;
extern int	isprintlatin(int) ;
extern int	iceil(int,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* exported subroutines */


int bfliner_start(lsp,ifp,foff,to)
BFLINER		*lsp ;
bfile		*ifp ;
offset_t	foff ;
int		to ;
{
	const int	lsize = (LINEBUFLEN+1) ;
	int		rs = SR_OK ;
	char		*p ;

	if (lsp == NULL) return SR_FAULT ;
	if (ifp == NULL) return SR_FAULT ;

	lsp->ifp = ifp ;
	lsp->poff = 0 ;
	lsp->foff = foff ;
	lsp->to = to ;
	lsp->ll = -1 ;
	if ((rs = uc_malloc(lsize,&p)) >= 0)
	   lsp->lbuf = (char *) p ;

	return rs ;
}
/* end subroutine (bfliner_start) */


int bfliner_finish(lsp)
BFLINER		*lsp ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	lsp->ll = 0 ;
	if (lsp->lbuf != NULL) {
	    rs1 = uc_free(lsp->lbuf) ;
	    if (rs >= 0) rs = rs1 ;
	    lsp->lbuf = NULL ;
	}

	return rs ;
}
/* end subroutine (bfliner_finish) */


int bfliner_readpending(lsp)
BFLINER		*lsp ;
{
	return (lsp->ll >= 0) ;
}
/* end subroutine (bfliner_readpending) */


int bfliner_readline(lsp,llen,lpp)
BFLINER		*lsp ;
int		llen ;
const char	**lpp ;
{
	int		rs = SR_OK ;
	int		len = 0 ;

#if	CF_DEBUGS
	debugprintf("bfliner_readline: ent llen=%d\n",llen) ;
	debugprintf("bfliner_readline: ll=%d to=%d\n", lsp->ll, lsp->to) ;
#endif

	if (lsp->ll < 0) {
	    bfile	*ifp = lsp->ifp ;

	    lsp->poff = lsp->foff ;
	    rs = breadlinetimed(ifp,lsp->lbuf,llen,lsp->to) ;
	    len = rs ;

#if	CF_DEBUGS
	    debugprintf("bfliner_readline: breadlinetimed() rs=%d\n",rs) ;
#endif

	    lsp->ll = len ;
	    if (rs > 0)
	        lsp->foff += len ;

	} else {
	    len = lsp->ll ;
	} /* end if (needed a new line) */

	if (lpp != NULL) {
	    *lpp = (rs >= 0) ? lsp->lbuf : NULL ;
	}

#if	CF_DEBUGS
	debugprintf("bfliner_readline: ret rs=%d llen=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (bfliner_readline) */


int bfliner_readover(lsp)
BFLINER		*lsp ;
{

	lsp->ll = -1 ;
	lsp->lbuf[0] = '\0' ;
	return SR_OK ;
}
/* end subroutine (bfliner_readover) */


int bfliner_getpoff(lsp,rp)
BFLINER		*lsp ;
offset_t	*rp ;
{
	int		rs = SR_OK ;

	if (rp == NULL) return SR_FAULT ;

	*rp = lsp->poff ;
	return rs ;
}
/* end subroutine (bfliner_getpoff) */


#if	CF_BFLINERADV
static int bfliner_adv(lsp,inc)
BFLINER		*lsp ;
int		inc ;
{
	int		rs = SR_OK ;

	lsp->poff = lsp->foff ;
	if (inc > 0) {

	    if (! lsp->f_reg) {
	        int	mlen ;
	        int	rlen = inc ;
	        while ((rs >= 0) && (rlen > 0)) {
	            mlen = MIN(rlen,LINEBUFLEN) ;
	            rs = filebuf_read(&lsp->mfb,lsp->lbuf,mlen,lsp->to) ;
	            rlen -= rs ;
	        } /* end while */
	    } else
	        rs = filebuf_adv(&lsp->mfb,inc) ;

	    lsp->foff += inc ;

	} /* end if */

	lsp->ll = -1 ;
	lsp->lbuf[0] = '\0' ;
	return rs ;
}
/* end subroutine (bfliner_adv) */
#endif /* CF_BFLINERADV */


